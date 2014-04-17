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

#include <hal/vga.h>
#include <hal/tmu.h>
#include <hal/brd.h>
#include <hal/usb.h>
#include <hal/ukb.h>

#include <net/microudp.h>

#include <checker.h>

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

static void help(void)
{
	puts("Milkymist(tm) BIOS (bootloader)");
	puts("Don't know what to do? Try 'flashboot'.\n");
	puts("Available commands:");
	puts("cons       - switch console mode");
	puts("help       - help");
	puts("reboot     - system reset");
}

void bios_netboot(void);

static void do_command(char *c)
{
	char *token;
	token = get_token(&c);

	if(strcmp(token, "netboot") == 0) {
    printf("Adresse de bios_netboot 0x%08x\n", &bios_netboot);
    bios_netboot();
  } else if(strcmp(token, "help") == 0) help();
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

static unsigned char mac[] = {0x00, 0x0a, 0x35, 0x01, 0x8e, 0xb4};
static unsigned char lip[] = {192, 168, 0, 42};

int main(int i, char **c)
{
	CSR_GPIO_OUT = GPIO_LED1;
	rescue = !((unsigned int)main > FLASH_OFFSET_REGULAR_BIOS);

	irq_setmask(0);
	irq_enable(1);
	uart_init();
  checker_init();

	console_set_write_hook(dummy_write_hook);
	console_set_read_hook(NULL, dummy_read_nonblock_hook);

	putsnonl(banner);
  print_pc();
	crcbios();
	brd_init();

  microudp_start(mac, IPTOINT(lip[0], lip[1], lip[2], lip[3]));

	while(1) {
    prompt();
    printf("Checker dummy start\n");
    checker_dummy_start(0x20000000, 0x0);
	}

	return 0;
}
