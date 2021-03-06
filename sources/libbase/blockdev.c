/*
 * Milkymist SoC (Software)
 * Copyright (C) 2007, 2008, 2009, 2010 Sebastien Bourdeauducq
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

#include <hw/flash.h>
#include <hw/memcard.h>
#include <string.h>

#include <blockdev.h>
#include <stdio.h>

#include "mmc.h"

#define MEMCARD_DEBUG

static char mmc_high_capacity;

static void memcard_start_cmd_tx(void)
{
	CSR_MEMCARD_ENABLE = MEMCARD_ENABLE_CMD_TX;
}

static void memcard_start_cmd_rx(void)
{
	CSR_MEMCARD_PENDING = MEMCARD_PENDING_CMD_RX;
	CSR_MEMCARD_START = MEMCARD_START_CMD_RX;
	CSR_MEMCARD_ENABLE = MEMCARD_ENABLE_CMD_RX;
}

static void memcard_start_cmd_dat_rx(void)
{
	CSR_MEMCARD_PENDING = MEMCARD_PENDING_CMD_RX|MEMCARD_PENDING_DAT_RX;
	CSR_MEMCARD_START = MEMCARD_START_CMD_RX|MEMCARD_START_DAT_RX;
	CSR_MEMCARD_ENABLE = MEMCARD_ENABLE_CMD_RX|MEMCARD_ENABLE_DAT_RX;
}

static void memcard_send_command(unsigned char cmd, unsigned int arg)
{
	unsigned char packet[6];
	int a;
	int i;
	unsigned char data;
	unsigned char crc;

	packet[0] = cmd | 0x40;
	packet[1] = ((arg >> 24) & 0xff);
	packet[2] = ((arg >> 16) & 0xff);
	packet[3] = ((arg >> 8) & 0xff);
	packet[4] = (arg & 0xff);

	crc = 0;
	for(a=0;a<5;a++) {
		data = packet[a];
		for(i=0;i<8;i++) {
			crc <<= 1;
			if((data & 0x80) ^ (crc & 0x80))
				crc ^= 0x09;
			data <<= 1;
		}
	}
	crc = (crc<<1) | 1;

	packet[5] = crc;

#ifdef MEMCARD_DEBUG
	printf(">> %02x %02x %02x %02x %02x %02x\n", packet[0], packet[1], packet[2], packet[3], packet[4], packet[5]);
#endif

	for(i=0;i<6;i++) {
		CSR_MEMCARD_CMD = packet[i];
		while(CSR_MEMCARD_PENDING & MEMCARD_PENDING_CMD_TX);
	}
}

static void memcard_send_dummy(void)
{
	CSR_MEMCARD_CMD = 0xff;
	while(CSR_MEMCARD_PENDING & MEMCARD_PENDING_CMD_TX);
}

static int memcard_receive_command(unsigned char *buffer, int len)
{
	int i;
	int timeout;

	for(i=0;i<len;i++) {
		timeout = 2000000;
		while(!(CSR_MEMCARD_PENDING & MEMCARD_PENDING_CMD_RX)) {
			timeout--;
			if(timeout == 0) {
				#ifdef MEMCARD_DEBUG
				printf("Command receive timeout\n");
				#endif
				return 0;
			}
		}
		buffer[i] = CSR_MEMCARD_CMD;
		CSR_MEMCARD_PENDING = MEMCARD_PENDING_CMD_RX;
	}

	while(!(CSR_MEMCARD_PENDING & MEMCARD_PENDING_CMD_RX));

	#ifdef MEMCARD_DEBUG
	printf("<< ");
	for(i=0;i<len;i++)
		printf("%02x ", buffer[i]);
	printf("\n");
	#endif

	return 1;
}

static int memcard_receive_command_data(unsigned char *command, unsigned int *data)
{
	int i, j;
	int timeout;

	i = 0;
	j = 0;
	while(j < 128) {
		timeout = 2000000;
		while(!(CSR_MEMCARD_PENDING & (MEMCARD_PENDING_CMD_RX|MEMCARD_PENDING_DAT_RX))) {
			timeout--;
			if(timeout == 0) {
				#ifdef MEMCARD_DEBUG
				printf("Command receive timeout\n");
				#endif
				return 0;
			}
		}
		if(CSR_MEMCARD_PENDING & MEMCARD_PENDING_CMD_RX) {
			command[i++] = CSR_MEMCARD_CMD;
			CSR_MEMCARD_PENDING = MEMCARD_PENDING_CMD_RX;
			if(i == 6)
				CSR_MEMCARD_ENABLE = MEMCARD_ENABLE_DAT_RX;
		}
		if(CSR_MEMCARD_PENDING & MEMCARD_PENDING_DAT_RX) {
			data[j++] = CSR_MEMCARD_DAT;
			CSR_MEMCARD_PENDING = MEMCARD_PENDING_DAT_RX;
		}
	}

	/* Get CRC (ignored) */
	for(i=0;i<2;i++) {
		while(!(CSR_MEMCARD_PENDING & MEMCARD_PENDING_DAT_RX));
		#ifdef MEMCARD_DEBUG
		printf("CRC: %08x\n", CSR_MEMCARD_DAT);
		#endif
		CSR_MEMCARD_PENDING = MEMCARD_PENDING_DAT_RX;
	}

	while(!(CSR_MEMCARD_PENDING & MEMCARD_PENDING_DAT_RX));

	#ifdef MEMCARD_DEBUG
	printf("<< %02x %02x %02x %02x %02x %02x\n", command[0], command[1], command[2], command[3], command[4], command[5]);
	#endif

	//for(i=0;i<128;i++)
	//	printf("%08x ", data[i]);
	//printf("\n");

	return 1;
}

static int memcard_init(void)
{
	unsigned char b[17];
	unsigned int rca;
	unsigned int cmd_arg;
	unsigned int supported_voltage = MMC_VDD_32_33 | MMC_VDD_33_34;
	unsigned int sd_version_2 = 0;

	CSR_MEMCARD_CLK2XDIV = 250;

	/* CMD0 */
	memcard_start_cmd_tx();
	memcard_send_command(MMC_CMD_GO_IDLE_STATE, 0);

	memcard_send_dummy();

	/* CMD8 (Ignore no response) */
	cmd_arg = (((supported_voltage & 0xff8000) != 0) << 8) | 0xaa;
	memcard_send_command(SD_CMD_SEND_IF_COND, cmd_arg);
	memcard_start_cmd_rx();
	if(memcard_receive_command(b, 6)) {
		if (((unsigned int)(b[3] << 8)|b[4]) != cmd_arg)
			return 0;
		else 
			sd_version_2 = 1;
	}

	/* ACMD41 - initialize */
	while(1) {
		memcard_start_cmd_tx();
		memcard_send_command(MMC_CMD_APP_CMD, 0);
		memcard_start_cmd_rx();
		if(!memcard_receive_command(b, 6)) return 0;
		memcard_start_cmd_tx();
		cmd_arg = supported_voltage | ((sd_version_2) ? OCR_HCS : 0);
		memcard_send_command(41, cmd_arg);
		memcard_start_cmd_rx();
		if(!memcard_receive_command(b, 6)) return 0;
		if(((unsigned int)b[1] << 24) & OCR_BUSY) break;
		#ifdef MEMCARD_DEBUG
		printf("Card is busy, retrying\n");
		#endif
	}
	mmc_high_capacity = ((((unsigned int)b[1] << 24) & OCR_HCS) == OCR_HCS);

	/* CMD2 - get CID */
	memcard_start_cmd_tx();
	memcard_send_command(MMC_CMD_ALL_SEND_CID, 0);
	memcard_start_cmd_rx();
	if(!memcard_receive_command(b, 17)) return 0;

	/* CMD3 - get RCA */
	memcard_start_cmd_tx();
	memcard_send_command(MMC_CMD_SET_RELATIVE_ADDR, 0);
	memcard_start_cmd_rx();
	if(!memcard_receive_command(b, 6)) return 0;
	rca = (((unsigned int)b[1]) << 8)|((unsigned int)b[2]);
	#ifdef MEMCARD_DEBUG
	printf("RCA: %04x\n", rca);
	#endif

	/* CMD7 - select card */
	memcard_start_cmd_tx();
	memcard_send_command(MMC_CMD_SELECT_CARD, rca << 16);
	memcard_start_cmd_rx();
	if(!memcard_receive_command(b, 6)) return 0;

	/* ACMD6 - set bus width */
	memcard_start_cmd_tx();
	memcard_send_command(MMC_CMD_APP_CMD, rca << 16);
	memcard_start_cmd_rx();
	if(!memcard_receive_command(b, 6)) return 0;
	memcard_start_cmd_tx();
	memcard_send_command(SD_CMD_APP_SET_BUS_WIDTH, 2);
	memcard_start_cmd_rx();
	if(!memcard_receive_command(b, 6)) return 0;

	CSR_MEMCARD_CLK2XDIV = 3;

	return 1;
}

static int memcard_readblock(unsigned int block, void *buffer)
{
	unsigned char b[6];
	unsigned int address = block; /* SDHC and SDXC cards use block unit address */
 
 	if (!mmc_high_capacity)
		address *= 512; /* SDSC cards use byte unit address */

	/* CMD17 - read block */
	memcard_start_cmd_tx();
	memcard_send_command(MMC_CMD_READ_SINGLE_BLOCK, address);
	memcard_start_cmd_dat_rx();
	if(!memcard_receive_command_data(b, (unsigned int *)buffer)) return 0;
	return 1;
}

int bd_init(int devnr)
{
	return memcard_init();
}

int bd_readblock(unsigned int block, void *buffer)
{
	return memcard_readblock(block, buffer);
}

void bd_done(void)
{
}

int bd_has_part_table(int devnr)
{
	return 1;
}
