/*
 * Milkymist SoC (Software)
 * Copyright (C) 2007, 2008, 2009, 2010, 2011 Sebastien Bourdeauducq
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <console.h>
#include <string.h>
#include <uart.h>
#include <blockdev.h>
#include <fatfs.h>
#include <system.h>
#include <board.h>
#include <irq.h>
#include <version.h>
#include <crc.h>
#include <net/mdio.h>
#include <hw/fmlbrg.h>
#include <hw/sysctl.h>
#include <hw/gpio.h>
#include <hw/flash.h>
#include <hw/minimac.h>
#include <hw/interrupts.h>
#include <hw/mpu.h>
#include <hw/hm.h>

#include <hal/brd.h>
#include <hal/sleep.h>

#include <net/microudp.h>
#include <net/tftp.h>

#include <mpu.h>
#include <mpu_int.h>
#include <hm.h>
#include <trn.h>

#include <debug_server.h>

#include <challenge.h>

extern void boot_helper(unsigned int r1, unsigned int r2, unsigned int r3, unsigned int r4, unsigned int addr);

static void __attribute__((noreturn)) boot(unsigned int r1, unsigned int r2, unsigned int r3, unsigned int r4, unsigned int addr)
{
	uart_force_sync(1);
	irq_setmask(0);
	irq_enable(0);
	boot_helper(r1, r2, r3, r4, addr);
	while(1);
}

static inline void wait(void) {
  int z, k;
  for (k=0; k < 0x100; k++)
    for (z=0; z< 0x100; z++)
      asm("nop;");
}

static unsigned char mac[] = {0x00, 0x0a, 0x35, 0x01, 0x8e, 0xb4};
static unsigned char lip[] = {192, 168, 0, 42};
static unsigned char rip[] = {192, 168, 0, 14};
// static unsigned char mac[] = {0x00, 0x0a, 0x35, 0x02, 0xb7, 0xac};
// static unsigned char lip[] = {140, 93, 69, 211};
// static unsigned char rip[] = {140, 93, 69, 14};

/* General address space functions */
static const char banner[]=
  "\n*****************************************\n"
	"SSTIC 2014 remote integrity checking BIOS\n"
  "*****************************************\n\n";

static void dummy_write_hook(char c)
{
  unsigned int oldmask;
  unsigned int i;

  oldmask = irq_getmask();
  irq_setmask(0);

  for (i=0; i<0x1000; i++) asm("nop;");

  irq_setmask(oldmask);
}

static int dummy_read_nonblock_hook(void)
{
  unsigned int oldmask;
	unsigned int i;

  oldmask = irq_getmask();
  irq_setmask(0);

	for (i=0; i<0x1000; i++) asm("nop;");
  
  irq_setmask(oldmask);

	return 0;
}

int rescue;

static void readstr(char *s, int size)
{
	char c[2];
	int ptr;

	c[1] = 0;
	ptr = 0;
	while(1) {
		c[0] = readchar();
		switch(c[0]) {
			case 0x7f:
			case 0x08:
				if(ptr > 0) {
					ptr--;
					putsnonl("\x08 \x08");
				}
				break;
			case '\e':
				break;
			case 0x07:
				break;
			case '\r':
			case '\n':
				s[ptr] = 0x00;
				putsnonl("\n");
				return;
			default:
				putsnonl(c);
				s[ptr] = c[0];
				ptr++;
				break;
		}
	}
}

static char *get_token(char **str)
{
	char *c, *d;

	c = (char *)strchr(*str, ' ');
	if(c == NULL) {
		d = *str;
		*str = *str+strlen(*str);
		return d;
	}
	*c = 0;
	d = *str;
	*str = c+1;
	return d;
}

/* General address space functions */

#define NUMBER_OF_BYTES_ON_A_LINE 16
static void dump_bytes(unsigned int *ptr, int count, unsigned addr)
{
	char *data = (char *)ptr;
	int line_bytes = 0, i = 0;

	putsnonl("Memory dump:");
	while(count > 0){
		line_bytes =
			(count > NUMBER_OF_BYTES_ON_A_LINE)?
				NUMBER_OF_BYTES_ON_A_LINE : count;

		printf("\n0x%08x  ", addr);
		for(i=0;i<line_bytes;i++)
			printf("%02x ", *(unsigned char *)(data+i));

		for(;i<NUMBER_OF_BYTES_ON_A_LINE;i++)
			printf("   ");

		printf(" ");

		for(i=0;i<line_bytes;i++) {
			if((*(data+i) < 0x20) || (*(data+i) > 0x7e))
				printf(".");
			else
				printf("%c", *(data+i));
		}

		for(;i<NUMBER_OF_BYTES_ON_A_LINE;i++)
			printf(" ");

		data += (char)line_bytes;
		count -= line_bytes;
		addr += line_bytes;
	}
	printf("\n");
}

static void mr(char *startaddr, char *len)
{
	char *c;
	unsigned int *addr;
	unsigned int length;

	if(*startaddr == 0) {
		printf("mr <address> [length]\n");
		return;
	}
	addr = (unsigned *)strtoul(startaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	if(*len == 0) {
		length = 1;
	} else {
		length = strtoul(len, &c, 0);
		if(*c != 0) {
			printf("incorrect length\n");
			return;
		}
	}

	dump_bytes(addr, length, (unsigned)addr);
}

static void mw(char *addr, char *value, char *count)
{
	char *c;
	unsigned int *addr2;
	unsigned int value2;
	unsigned int count2;
	unsigned int i;

	if((*addr == 0) || (*value == 0)) {
		printf("mw <address> <value> [count]\n");
		return;
	}
	addr2 = (unsigned int *)strtoul(addr, &c, 0);
	if(*c != 0) {
		printf("incorrect address\n");
		return;
	}
	value2 = strtoul(value, &c, 0);
	if(*c != 0) {
		printf("incorrect value\n");
		return;
	}
	if(*count == 0) {
		count2 = 1;
	} else {
		count2 = strtoul(count, &c, 0);
		if(*c != 0) {
			printf("incorrect count\n");
			return;
		}
	}
	for (i=0;i<count2;i++) *addr2++ = value2;
}

static void mc(char *dstaddr, char *srcaddr, char *count)
{
	char *c;
	unsigned int *dstaddr2;
	unsigned int *srcaddr2;
	unsigned int count2;
	unsigned int i;

	if((*dstaddr == 0) || (*srcaddr == 0)) {
		printf("mc <dst> <src> [count]\n");
		return;
	}
	dstaddr2 = (unsigned int *)strtoul(dstaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect destination address\n");
		return;
	}
	srcaddr2 = (unsigned int *)strtoul(srcaddr, &c, 0);
	if(*c != 0) {
		printf("incorrect source address\n");
		return;
	}
	if(*count == 0) {
		count2 = 1;
	} else {
		count2 = strtoul(count, &c, 0);
		if(*c != 0) {
			printf("incorrect count\n");
			return;
		}
	}
	for (i=0;i<count2;i++) *dstaddr2++ = *srcaddr2++;
}

static void help(void)
{
	puts("ERIC BIOS (dumb & dumber version)");
	puts("Available commands:");
	puts("help       - help");
	puts("reboot     - hard reboot system");
	puts("freboot    - hard reboot netbooted software");
  puts("mpu_start  - Start MPU");
	puts("exp_dl     - tftp download expansion rom : rom.bin");
	puts("mpu_dl     - tftp download mpu binary : mpu.bin");
	puts("mpu_dump   - Dumps the 0x100 first bytes of the mpu program");
	puts("hm_read    - start checker in host memory read mode on specified page");
	puts("hm_stat    - Displays host memory module stats");
	puts("hm_dump    - Dumps the 0x100 first bytes of the page read in hm");
	puts("pci_info   - Prints information about the PCI state of the core");
	puts("fc_info    - Prints information about the core flow control");
	puts("debug_run  - Run debug server");
	puts("run        - Run challenge");
	puts("usleep     - Sleep for a while");
	puts("mr         - read address space");
	puts("mw         - write address space");
	puts("mc         - copy address space");
}

void challenge_start(void) {
  microudp_start(mac, IPTOINT(lip[0], lip[1], lip[2], lip[3]));
  int ip = IPTOINT(rip[0], rip[1], rip[2], rip[3]);
  if (debug_server_init(ip) == -1) {
    printf("Failed to init debug_server\n");
  }
  challenge_init();
  challenge_run();
}

// XXX This is shit
extern uint32_t _yolo;
static void do_command(char *c) {
	char *token;
	token = get_token(&c);

	if(strcmp(token, "help") == 0) {
    help();
  } else if(strcmp(token, "reboot") == 0) {
    printf("Adresse de reboot 0x%08x\n", &reboot);
    reboot();
  } else if(strcmp(token, "freboot") == 0) {
    printf("Adresse de reboot 0x%08x\n", 0x0);
    printf("I: Rebooting...\n");
    boot(0, 0, 0, rescue, 0);
  } else if(strcmp(token, "exp_dl") == 0) {
    challenge_dl(get_token(&c), mac, lip, rip);
  } else if(strcmp(token, "mpu_dl") == 0) {
    int r;
    microudp_start(mac, IPTOINT(lip[0], lip[1], lip[2], lip[3]));
    int ip = IPTOINT(rip[0], rip[1], rip[2], rip[3]);
    char *ptr = (char *)&MPU_MEMORY_ADDR;
	  r = tftp_get(ip, "mpu.bin", ptr);
    // We clear the cache to be sure that every instruction is written to the
    // MPU shared memory
    flush_cpu_dcache();
    printf("Received %d bytes\n", r);
  } else if(strcmp(token, "mpu_start") == 0) {
    int udl, udh;
    if (!mpu_start()) {
      mpu_int_iterate(1, NULL, NULL);
      while (!mpu_int_iterate(0, &udl, &udh)) {
        printf("Iterated %x %x\n", udl, udh);
      }
    } else {
      printf("Stopped on error or waited too long\n");
    }
  } else if(strcmp(token, "hm_read") == 0) {
    unsigned int low, high, count, i, same, j;
    int r;
    low = atoi(get_token(&c));
    high = atoi(get_token(&c));
    count = atoi(get_token(&c));
    count = (!count) ? 1 : count;
    same = atoi(get_token(&c));
    same = (!same) ? 1 : same;
    for (i = 0; i < count; i++) {
      for (j = 0; j < same; j++) {
        r = hm_start((low & ~0xfff) + (i * 0x1000), high);
        if(r) {
          printf("STAT %08x\n", HM_CSR_STAT);
          break;
        }
      }
      if(r) {
        break;
      }
    }
    printf("hm_read end\n");
  } else if(strcmp(token, "hm_stat") == 0) {
    hm_stat();
    trn_stat_dump();
  } else if(strcmp(token, "hm_dump") == 0) {
    flush_cpu_dcache();
    unsigned int *ptr = (unsigned int *)&HM_MEMORY_ADDR;
	  dump_bytes(ptr, 0x100, (unsigned)ptr);
  } else if(strcmp(token, "mpu_dump") == 0) {
    unsigned int *ptr = (unsigned int *)&MPU_MEMORY_ADDR;
	  dump_bytes(ptr, 0x100, (unsigned)ptr);
  } else if(strcmp(token, "mr") == 0) { 
    mr(get_token(&c), get_token(&c));
  } else if(strcmp(token, "mw") == 0) {
    mw(get_token(&c), get_token(&c), get_token(&c));
  } else if(strcmp(token, "mc") == 0) {
    mc(get_token(&c), get_token(&c), get_token(&c));
  } else if(strcmp(token, "pci_info") == 0) {
    trn_pci_dump_command(); printf("\n");
    trn_pci_dump_address(); printf("\n");
    trn_pci_dump_dstatus(); printf("\n");
    trn_pci_dump_dcommand(); printf("\n");
    trn_pci_dump_lstatus(); printf("\n");
    trn_pci_dump_lcommand(); printf("\n");
    trn_pci_dump_dcommand2();
  } else if(strcmp(token, "fc_info") == 0) {
    trn_fc_dump_all();
  } else if(strcmp(token, "usleep") == 0) {
    usleep(atoi(get_token(&c)));
  } else if(strcmp(token, "yolo") == 0) {
    printf("yolo 0x%08x\n", _yolo);
  } else if(strcmp(token, "run") == 0) {
    challenge_start();
  } else if(strcmp(token, "debug_run") == 0) {
    microudp_start(mac, IPTOINT(lip[0], lip[1], lip[2], lip[3]));
    int ip = IPTOINT(rip[0], rip[1], rip[2], rip[3]);
    if (debug_server_init(ip) == -1) {
      printf("Failed to init debug_server\n");
    } else {
      debug_server_run();
    }
  }
}

void prompt(void) {
	char buffer[64];
  wait();
  putsnonl("\e[1mBIOS>\e[0m ");
  readstr(buffer, 64);
  do_command(buffer);
}

extern unsigned int _edata;
static void crcbios(void)
{
	unsigned int offset_bios;
	unsigned int length;
	unsigned int expected_crc;
	unsigned int actual_crc;

	/*
	 * _edata is located right after the end of the flat
	 * binary image. The CRC tool writes the 32-bit CRC here.
	 * We also use the address of _edata to know the length
	 * of our code.
	 */
  // On est recopié en zero
  offset_bios = 0;
	expected_crc = _edata;
	length = (unsigned int)&_edata - offset_bios;
  printf("CRC addresses 0x%08x 0x%08x 0x%08x 0x%08x\n", offset_bios,
      expected_crc, length, &_edata);
	actual_crc = crc32((unsigned char *)offset_bios, length);
	if(expected_crc == actual_crc)
		printf("I: BIOS CRC passed (%08x)\n", actual_crc);
	else {
		printf("W: BIOS CRC failed (expected %08x, got %08x)\n", expected_crc, actual_crc);
		printf("W: The system will continue, but expect problems.\n");
	}
}

void print_pc(void) {
  unsigned int pc = 0;
  __asm__ __volatile__(
      "mv %0,r29;"
      : "=r"(pc): );
  printf("Last called function had pc at 0x%04x\n", pc);
}

int main(int i, char **c)
{
	CSR_GPIO_OUT = GPIO_LED1;
	rescue = !((unsigned int)main > FLASH_OFFSET_REGULAR_BIOS);

	irq_setmask(0);
	irq_enable(IRQ_UART | IRQ_TIMER0 | IRQ_TIMER1);
	uart_init();

	console_set_write_hook(dummy_write_hook);
	console_set_read_hook(NULL, dummy_read_nonblock_hook);

	putsnonl(banner);
  print_pc();
	crcbios();
	brd_init();
  
  // checker_memory_test();
  // checker_int_test();
  
  // XXX run challenge by default

  // Waiting for eth mac
  printf("Waiting for the ethernet mac\n");
  wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait(),
  wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait(),
  wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait(),
  wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait();

//   printf("Auto-boot...\n");
//   int j = 0, r = -1;
//   while (j < 5 && r == -1) {
//     printf("Downloading challenge, attempt %d...\n", j + 1);
//     r = challenge_dl("cpuid.bin", mac, lip, rip);
//     ++j;
//     wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait(), wait();
//   }
//   challenge_start();

	while(1) {
    prompt();
	}

	return 0;
}
