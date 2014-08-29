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
#include <hw/csr_ddr3.h>

#include <hal/vga.h>
#include <hal/tmu.h>
#include <hal/brd.h>
#include <hal/usb.h>
#include <hal/ukb.h>

#include <net/microudp.h>
#include <net/tftp.h>

#include <mpu.h>
#include <mpu_int.h>

#include <csr_ddr3.h>

static unsigned char mac[] = {0x00, 0x0a, 0x35, 0x01, 0x8e, 0xb4};
static unsigned char lip[] = {192, 168, 0, 42};
static unsigned char rip[] = {192, 168, 0, 14};

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
  puts("mpu_start  - Start MPU");
	puts("mpu_dl     - tftp download checker mpu binary : mpu.bin");
	puts("mpu_dump   - Dumps the 0x100 first bytes of the mpu program");
	puts("hm_read    - start checker in host memory read mode on specified page");
	puts("hm_stat    - Displays host memory module stats");
	puts("hm_dump    - Dumps the 0x100 first bytes of the page read in hm");
	puts("pci_info   - Prints information about the PCI state of the core");
	puts("fc_info    - Prints information about the core flow control");
	puts("mr         - read address space");
	puts("mw         - write address space");
	puts("mc         - copy address space");
	puts("ddr_stat   - CSR ddr3 stat");
	puts("ddr_read   - CSR ddr3 read");
	puts("ddr_write  - CSR ddr3 write");
}

static void do_command(char *c)
{
	char *token;
	token = get_token(&c);

	if(strcmp(token, "help") == 0) {
    help();
  } else if(strcmp(token, "reboot") == 0) {
    printf("Adresse de reboot 0x%08x\n", &reboot);
    reboot();
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
    mpu_start();
  } else if(strcmp(token, "hm_read") == 0) {
//     unsigned int low, high;
//     low = atoi(get_token(&c));
//     high = atoi(get_token(&c));
//     checker_read_start(low, high);
  } else if(strcmp(token, "hm_stat") == 0) {
//     checker_print_hm_stat();
  } else if(strcmp(token, "hm_dump") == 0) {
//     unsigned int *ptr = (unsigned int *)CHECKER_ADDR_HM;
// 	  dump_bytes(ptr, 0x100, (unsigned)ptr);
  } else if(strcmp(token, "mpu_dump") == 0) {
    unsigned int *ptr = (unsigned int *)&MPU_MEMORY_ADDR;
	  dump_bytes(ptr, 0x100, (unsigned)ptr);
  } else if(strcmp(token, "mr") == 0) { 
    mr(get_token(&c), get_token(&c));
  } else if(strcmp(token, "mw") == 0) {
    mw(get_token(&c), get_token(&c), get_token(&c));
  } else if(strcmp(token, "mc") == 0) {
    mc(get_token(&c), get_token(&c), get_token(&c));
  } else if(strcmp(token, "ddr_stat") == 0) {
    csr_ddr3_print_stat();
  } else if(strcmp(token, "ddr_read") == 0) {
    int i;
    void * addr = (void *)atoi(get_token(&c));
    int j = atoi(get_token(&c));
    for (i = 0; i < j; i++) {
      csr_ddr3_read_256(addr + 4 * i);
      if (!(CSR_DDR3_CSR_STAT & 0x2)) {
        printf("Read command failed !\n");
        break;
      }
    }
  } else if(strcmp(token, "ddr_write") == 0) {
    int i;
    void * addr = (void *)atoi(get_token(&c));
    uint32_t v = atoi(get_token(&c));
    int j = atoi(get_token(&c));
    for (i = 0; i < j; i++) {
      csr_ddr3_write_256(addr + 4 * i, v);
      if (!(CSR_DDR3_CSR_STAT & 0x1)) {
        printf("Write command failed !\n");
        break;
      }
    }
  } else if(strcmp(token, "pci_info") == 0) {
//     checker_pci_dump_command(); printf("\n");
//     checker_pci_dump_address(); printf("\n");
//     checker_pci_dump_dstatus(); printf("\n");
//     checker_pci_dump_dcommand(); printf("\n");
//     checker_pci_dump_lstatus(); printf("\n");
//     checker_pci_dump_lcommand(); printf("\n");
//     checker_pci_dump_dcommand2();
  } else if(strcmp(token, "fc_info") == 0) {
//     checker_fc_dump_all();
  }
}

void prompt(void) {
	char buffer[64];
  int z, k;
  for (k=0; k < 0x100; k++)
    for (z=0; z< 0x100; z++)
      asm("nop;");
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
	irq_enable(IRQ_UART);
	uart_init();

	console_set_write_hook(dummy_write_hook);
	console_set_read_hook(NULL, dummy_read_nonblock_hook);

	putsnonl(banner);
  print_pc();
	crcbios();
	brd_init();
  
  // checker_memory_test();
  // checker_int_test();

	while(1) {
    prompt();
	}

	return 0;
}
