/*
 * Milkymist SoC (Software)
 * Copyright (C) 2013, 2008, 2009, 2010 Sebastien Bourdeauducq
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
 *
 * This file is mostly based on the Linux code
 */

#ifndef _MMC_H_
#define _MMC_H_

#define MMC_CMD_GO_IDLE_STATE			0
#define MMC_CMD_SEND_OP_COND			1
#define MMC_CMD_ALL_SEND_CID			2
#define MMC_CMD_SET_RELATIVE_ADDR		3
#define MMC_CMD_SET_DSR					4
#define MMC_CMD_SWITCH					6
#define MMC_CMD_SELECT_CARD				7
#define MMC_CMD_SEND_EXT_CSD			8
#define MMC_CMD_SEND_CSD				9
#define MMC_CMD_SEND_CID				10
#define MMC_CMD_STOP_TRANSMISSION		12
#define MMC_CMD_SEND_STATUS				13
#define MMC_CMD_SET_BLOCKLEN			16
#define MMC_CMD_READ_SINGLE_BLOCK		17
#define MMC_CMD_READ_MULTIPLE_BLOCK		18
#define MMC_CMD_WRITE_SINGLE_BLOCK		24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_ERASE_GROUP_START		35
#define MMC_CMD_ERASE_GROUP_END			36
#define MMC_CMD_ERASE					38
#define MMC_CMD_APP_CMD					55
#define MMC_CMD_SPI_READ_OCR			58
#define MMC_CMD_SPI_CRC_ON_OFF			59

#define SD_CMD_SEND_RELATIVE_ADDR		3
#define SD_CMD_SWITCH_FUNC				6
#define SD_CMD_SEND_IF_COND				8

#define SD_CMD_APP_SET_BUS_WIDTH		6
#define SD_CMD_ERASE_WR_BLK_START		32
#define SD_CMD_ERASE_WR_BLK_END			33
#define SD_CMD_APP_SEND_OP_COND			41
#define SD_CMD_APP_SEND_SCR				51

/* SCR definitions in different words */
#define SD_HIGHSPEED_BUSY				0x00020000
#define SD_HIGHSPEED_SUPPORTED			0x00020000

#define MMC_HS_TIMING					0x00000100
#define MMC_HS_52MHZ					0x2

#define OCR_BUSY						0x80000000
#define OCR_HCS							0x40000000
#define OCR_VOLTAGE_MASK				0x007FFF80
#define OCR_ACCESS_MODE					0x60000000

#define SECURE_ERASE					0x80000000

#define MMC_STATUS_MASK					(~0x0206BF7F)
#define MMC_STATUS_RDY_FOR_DATA 		(1 << 8)
#define MMC_STATUS_CURR_STATE			(0xf << 9)
#define MMC_STATUS_ERROR				(1 << 19)

#define MMC_STATE_PRG					(7 << 9)

#define MMC_VDD_165_195					0x00000080	/* VDD voltage 1.65 - 1.95 */
#define MMC_VDD_20_21					0x00000100	/* VDD voltage 2.0 ~ 2.1 */
#define MMC_VDD_21_22					0x00000200	/* VDD voltage 2.1 ~ 2.2 */
#define MMC_VDD_22_23					0x00000400	/* VDD voltage 2.2 ~ 2.3 */
#define MMC_VDD_23_24					0x00000800	/* VDD voltage 2.3 ~ 2.4 */
#define MMC_VDD_24_25					0x00001000	/* VDD voltage 2.4 ~ 2.5 */
#define MMC_VDD_25_26					0x00002000	/* VDD voltage 2.5 ~ 2.6 */
#define MMC_VDD_26_27					0x00004000	/* VDD voltage 2.6 ~ 2.7 */
#define MMC_VDD_27_28					0x00008000	/* VDD voltage 2.7 ~ 2.8 */
#define MMC_VDD_28_29					0x00010000	/* VDD voltage 2.8 ~ 2.9 */
#define MMC_VDD_29_30					0x00020000	/* VDD voltage 2.9 ~ 3.0 */
#define MMC_VDD_30_31					0x00040000	/* VDD voltage 3.0 ~ 3.1 */
#define MMC_VDD_31_32					0x00080000	/* VDD voltage 3.1 ~ 3.2 */
#define MMC_VDD_32_33					0x00100000	/* VDD voltage 3.2 ~ 3.3 */
#define MMC_VDD_33_34					0x00200000	/* VDD voltage 3.3 ~ 3.4 */
#define MMC_VDD_34_35					0x00400000	/* VDD voltage 3.4 ~ 3.5 */
#define MMC_VDD_35_36					0x00800000	/* VDD voltage 3.5 ~ 3.6 */

#define MMC_SWITCH_MODE_CMD_SET			0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS		0x01 /* Set bits in EXT_CSD byte
						addressed by index which are 1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02 /* Clear bits in EXT_CSD byte
						addressed by index, which are 1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03 /* Set target byte to value */

#define SD_SWITCH_CHECK		0
#define SD_SWITCH_SWITCH	1

/*
 * EXT_CSD fields
 */
#define EXT_CSD_PARTITIONING_SUPPORT	160	/* RO */
#define EXT_CSD_ERASE_GROUP_DEF		175	/* R/W */
#define EXT_CSD_PART_CONF		179	/* R/W */
#define EXT_CSD_BUS_WIDTH		183	/* R/W */
#define EXT_CSD_HS_TIMING		185	/* R/W */
#define EXT_CSD_REV			192	/* RO */
#define EXT_CSD_CARD_TYPE		196	/* RO */
#define EXT_CSD_SEC_CNT			212	/* RO, 4 bytes */
#define EXT_CSD_HC_ERASE_GRP_SIZE	224	/* RO */
#define EXT_CSD_BOOT_MULT		226	/* RO */

/*
 * EXT_CSD field definitions
 */

#define EXT_CSD_CMD_SET_NORMAL		(1 << 0)
#define EXT_CSD_CMD_SET_SECURE		(1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1 << 2)

#define EXT_CSD_CARD_TYPE_26	(1 << 0)	/* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52	(1 << 1)	/* Card can run at 52MHz */

#define EXT_CSD_BUS_WIDTH_1	0	/* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4	1	/* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8	2	/* Card is in 8 bit mode */

#define MMCPART_NOAVAILABLE	(0xff)
#define PART_ACCESS_MASK	(0x7)
#define PART_SUPPORT		(0x1)

#endif /* _MMC_H_ */
