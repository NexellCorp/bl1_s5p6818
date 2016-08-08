/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Sangjong, Han <hans@nexell.co.kr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __NX_SDHCBOOT_H__
#define __NX_SDHCBOOT_H__

#include <nx_peridot.h>
#include <nx_type.h>

#include <nx_sdmmc.h>

//#define VERBOSE
//#define NX_DEBUG

#if 1
#define SDXC_CLKGENSRC (0) // PLL0 = 800 MHz (0, 1, 2)
#define SDXC_CLKGENDIV (4) // PLL0 / 2 / 2 = 200 MHz
#if 0
#define SDXC_CLKGENDIV_400KHZ (250) // PLL0 / 250 / 2 = 1600KHz
#define SDXC_CLKDIV (4)		    // 50MHz, 400KHz
#else
#define SDXC_CLKGENDIV_400KHZ (125) // PLL0 / 125 / 2 = 3200KHz
#define SDXC_CLKDIV (8)		    // 25MHz, 400KHz
#endif
#else
#define SDXC_CLKGENSRC (2)	  // PLL2 = 96 MHz (0, 1, 2)
#define SDXC_CLKGENDIV (2)	  // PLL2 / 2 / 2 = 24 MHz
#define SDXC_CLKGENDIV_400KHZ (120) // PLL2 / 120 / 2 = 400KHz
#define SDXC_CLKDIV (0)
#endif

#define BLOCK_LENGTH (512)

#define NX_SDMMC_TIMEOUT (0x100000)
#define NX_SDMMC_TIMEOUT_IDENTIFY (0x100000)

#if (0)
#define INFINTE_LOOP()                                                         \
	{                                                                      \
		while (1)                                                      \
			;                                                      \
	}
#else
#define INFINTE_LOOP()
#endif

//------------------------------------------------------------------------------
#define NX_SDMMC_STATUS_NOERROR 0
#define NX_SDMMC_STATUS_ERROR (1U << 31)
#define NX_SDMMC_STATUS_CMDBUSY (NX_SDMMC_STATUS_ERROR | (1U << 0))
#define NX_SDMMC_STATUS_CMDTOUT (NX_SDMMC_STATUS_ERROR | (1U << 1))
#define NX_SDMMC_STATUS_RESCRCFAIL (NX_SDMMC_STATUS_ERROR | (1U << 2))
#define NX_SDMMC_STATUS_RESERROR (NX_SDMMC_STATUS_ERROR | (1U << 3))
#define NX_SDMMC_STATUS_RESTOUT (NX_SDMMC_STATUS_ERROR | (1U << 4))
#define NX_SDMMC_STATUS_UNKNOWNCMD (NX_SDMMC_STATUS_ERROR | (1U << 5))
#define NX_SDMMC_STATUS_DATABUSY (NX_SDMMC_STATUS_ERROR | (1U << 6))

//------------------------------------------------------------------------------
#define NX_SDMMC_RSPIDX_NONE (0)
#define NX_SDMMC_RSPIDX_R1 (1)
#define NX_SDMMC_RSPIDX_R1B (1 | 0x80)
#define NX_SDMMC_RSPIDX_R2 (2)
#define NX_SDMMC_RSPIDX_R3 (3)
#define NX_SDMMC_RSPIDX_R4 (4)
#define NX_SDMMC_RSPIDX_R5 (5)
#define NX_SDMMC_RSPIDX_R6 (6)
#define NX_SDMMC_RSPIDX_R7 (7)

//------------------------------------------------------------------------------
// Command
#define GO_IDLE_STATE ((0) | ((NX_SDMMC_RSPIDX_NONE) << 8))
#define SEND_OP_COND ((1) | ((NX_SDMMC_RSPIDX_R3) << 8)) // for MMC
#define ALL_SEND_CID ((2) | ((NX_SDMMC_RSPIDX_R2) << 8))
#define SET_RELATIVE_ADDR ((3) | ((NX_SDMMC_RSPIDX_R1) << 8))  // for MMC
#define SEND_RELATIVE_ADDR ((3) | ((NX_SDMMC_RSPIDX_R6) << 8)) // for SD
#define SWITCH_FUNC ((6) | ((NX_SDMMC_RSPIDX_R1B) << 8))       // for MMC
#define SELECT_CARD ((7) | ((NX_SDMMC_RSPIDX_R1B) << 8))
#define SEND_IF_COND ((8) | ((NX_SDMMC_RSPIDX_R7) << 8)) // for SD
#define SEND_EXT_CSD ((8) | ((NX_SDMMC_RSPIDX_R1) << 8)) // for MMC
#define SEND_CSD ((9) | ((NX_SDMMC_RSPIDX_R2) << 8))
#define SEND_CID ((10) | ((NX_SDMMC_RSPIDX_R2) << 8))
#define STOP_TRANSMISSION ((12) | ((NX_SDMMC_RSPIDX_R1B) << 8))
#define SEND_STATUS ((13) | ((NX_SDMMC_RSPIDX_R1) << 8))
#define SET_BLOCKLEN ((16) | ((NX_SDMMC_RSPIDX_R1) << 8))
#define READ_SINGLE_BLOCK ((17) | ((NX_SDMMC_RSPIDX_R1) << 8))
#define READ_MULTIPLE_BLOCK ((18) | ((NX_SDMMC_RSPIDX_R1) << 8))
#define WRITE_SINGLE_BLOCK ((24) | ((NX_SDMMC_RSPIDX_R1) << 8))
#define WRITE_MULTIPLE_BLOCK ((25) | ((NX_SDMMC_RSPIDX_R1) << 8))
#define SELECT_PARTITION ((43) | ((NX_SDMMC_RSPIDX_R1B) << 8)) // for eSD
#define APP_CMD ((55) | ((NX_SDMMC_RSPIDX_R1) << 8))

// Application Command
#define SET_BUS_WIDTH ((6) | ((NX_SDMMC_RSPIDX_R1) << 8) | (APP_CMD << 16))
#define SD_STATUS ((13) | ((NX_SDMMC_RSPIDX_R1) << 8) | (APP_CMD << 16))
#define SD_SEND_OP_COND ((41) | ((NX_SDMMC_RSPIDX_R3) << 8) | (APP_CMD << 16))
#define SET_CLR_CARD_DETECT                                                    \
	((42) | ((NX_SDMMC_RSPIDX_R1) << 8) | (APP_CMD << 16))
#define SEND_SCR ((51) | ((NX_SDMMC_RSPIDX_R1) << 8) | (APP_CMD << 16))

// EXT_CSD fields

#define EXT_CSD_PARTITIONING_SUPPORT 160 /* RO */
#define EXT_CSD_ERASE_GROUP_DEF 175      /* R/W */
#define EXT_CSD_PART_CONF 179		 /* R/W */
#define EXT_CSD_BUS_WIDTH 183		 /* R/W */
#define EXT_CSD_HS_TIMING 185		 /* R/W */
#define EXT_CSD_REV 192			 /* RO */
#define EXT_CSD_CARD_TYPE 196		 /* RO */
#define EXT_CSD_SEC_CNT 212		 /* RO, 4 bytes */
#define EXT_CSD_HC_ERASE_GRP_SIZE 224    /* RO */
#define EXT_CSD_BOOT_MULT 226		 /* RO */

#define EXT_CSD_BOOT_CONFIG 179		  /* R/W */
#define EXT_CSD_BOOT_BUS_WIDTH 177	/* R/W */
#define EXT_CSD_BOOT_WP_STATUS 174	/* RO */
#define EXT_CSD_BOOT_WRITE_PROTECTION 173 /* R/W */

// EXT_CSD field definitions
#define EXT_CSD_CMD_SET_NORMAL (1 << 0)
#define EXT_CSD_CMD_SET_SECURE (1 << 1)
#define EXT_CSD_CMD_SET_CPSECURE (1 << 2)

#define EXT_CSD_CARD_TYPE_26 (1 << 0) /* Card can run at 26MHz */
#define EXT_CSD_CARD_TYPE_52 (1 << 1) /* Card can run at 52MHz */

#define EXT_CSD_BUS_WIDTH_1 0 /* Card is in 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4 1 /* Card is in 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8 2 /* Card is in 8 bit mode */

#define EXT_CSD_NO_BOOT (0 << 6)
#define EXT_CSD_BOOT_ACK (1 << 6)

#define EXT_CSD_BOOT_PARTITION_ACCESS_ENABLE_NUM(n) ((n << 3) | n)

#define EXT_CSD_BOOT_PARTITION_NOT_ENABLE (0x0 << 3)
#define EXT_CSD_BOOT_PARTITION_1_ENABLE (0x1 << 3)
#define EXT_CSD_BOOT_PARTITION_2_ENABLE (0x2 << 3)
#define EXT_CSD_BOOT_PARTITION_3_ENABLE (0x3 << 3)
#define EXT_CSD_BOOT_PARTITION_4_ENABLE (0x4 << 3)
#define EXT_CSD_BOOT_PARTITION_ENABLE_NUM(n) (n << 3)

#define EXT_CSD_BOOT_PARTITION_NO_ACCESS (0x0)
#define EXT_CSD_BOOT_PARTITION_1_ACCESS (0x1)
#define EXT_CSD_BOOT_PARTITION_2_ACCESS (0x2)
#define EXT_CSD_BOOT_PARTITION_3_ACCESS (0x3)
#define EXT_CSD_BOOT_PARTITION_4_ACCESS (0x4)
#define EXT_CSD_BOOT_PARTITION_ACCESS_NUM(n) (n)

#define EXT_CSD_BOOT_BUS_WIDTH_1 (0x0)
#define EXT_CSD_BOOT_BUS_WIDTH_4 (0x1)
#define EXT_CSD_BOOT_BUS_WIDTH_8 (0x2)

#define MMC_SWITCH_MODE_CMD_SET 0x00 /* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS                                               \
	0x01 /* Set bits in EXT_CSD byte                                       \
		 addressed by index which are                                  \
		 1 in value field */
#define MMC_SWITCH_MODE_CLEAR_BITS                                             \
	0x02				/* Clear bits in EXT_CSD byte          \
					    addressed by index, which are      \
					    1 in value field */
#define MMC_SWITCH_MODE_WRITE_BYTE 0x03 /* Set target byte to value */

typedef struct tag_NX_SDMMC_COMMAND {
	U32 cmdidx;
	U32 arg;
	U32 flag;
	U32 status;
	U32 response[4];
} NX_SDMMC_COMMAND;

//------------------------------------------------------------------------------
typedef enum {
	NX_SDMMC_CARDTYPE_MMC,
	NX_SDMMC_CARDTYPE_SDMEM,
	NX_SDMMC_CARDTYPE_SDIO,
	NX_SDMMC_CARDTYPE_UNKNOWN
} NX_SDMMC_CARDTYPE;

typedef struct tag_SDXCBOOTSTATUS {
	NX_SDMMC_CARDTYPE CardType;

	U32 rca; // relative card address of device
	CBOOL bHighCapacity;
	U32 SDPort;
} SDXCBOOTSTATUS;

//------------------------------------------------------------------------------
// SDXC Module's Register Set
//------------------------------------------------------------------------------

#define NX_SDXC_CTRL_USEINDMAC (1U << 25)
#define NX_SDXC_CTRL_READWAIT (1U << 6)
#define NX_SDXC_CTRL_DMAMODE_EN (1U << 5)
#define NX_SDXC_CTRL_DMARST (1U << 2)
#define NX_SDXC_CTRL_FIFORST (1U << 1)
#define NX_SDXC_CTRL_CTRLRST (1U << 0)

//--------------------------------------------------------------------------
#define NX_SDXC_CLKENA_LOWPWR (1U << 16)
#define NX_SDXC_CLKENA_CLKENB (1U << 0)

//--------------------------------------------------------------------------
#define NX_SDXC_STATUS_FIFOCOUNT (0x1FFFU << 17)
#define NX_SDXC_STATUS_FSMBUSY (1U << 10)
#define NX_SDXC_STATUS_DATABUSY (1U << 9)
#define NX_SDXC_STATUS_FIFOFULL (1U << 3)
#define NX_SDXC_STATUS_FIFOEMPTY (1U << 2)

//--------------------------------------------------------------------------
#define NX_SDXC_CMDFLAG_STARTCMD (1U << 31)
#define NX_SDXC_CMDFLAG_USE_HOLD_REG (1U << 29)
#define NX_SDXC_CMDFLAG_VOLT_SWITCH (1U << 28)
#define NX_SDXC_CMDFLAG_BOOT_MODE (1U << 27)
#define NX_SDXC_CMDFLAG_DISABLE_BOOT (1U << 26)
#define NX_SDXC_CMDFLAG_EXPECTBOOTACK (1U << 25)
#define NX_SDXC_CMDFLAG_ENABLE_BOOT (1U << 24)
#define NX_SDXC_CMDFLAG_CCS_EXPECTED (1U << 23)
#define NX_SDXC_CMDFLAG_READCEATADEVICE (1U << 22)
#define NX_SDXC_CMDFLAG_UPDATECLKONLY (1U << 21)
#define NX_SDXC_CMDFLAG_SENDINIT (1U << 15)
#define NX_SDXC_CMDFLAG_STOPABORT (1U << 14)
#define NX_SDXC_CMDFLAG_WAITPRVDAT (1U << 13)
#define NX_SDXC_CMDFLAG_SENDAUTOSTOP (1U << 12)
#define NX_SDXC_CMDFLAG_BLOCK (0U << 11)
#define NX_SDXC_CMDFLAG_STREAM (1U << 11)
#define NX_SDXC_CMDFLAG_TXDATA (3U << 9)
#define NX_SDXC_CMDFLAG_RXDATA (1U << 9)
#define NX_SDXC_CMDFLAG_CHKRSPCRC (1U << 8)
#define NX_SDXC_CMDFLAG_SHORTRSP (1U << 6)
#define NX_SDXC_CMDFLAG_LONGRSP (3U << 6)

//--------------------------------------------------------------------------
#define NX_SDXC_RINTSTS_SDIO (1U << 16)
#define NX_SDXC_RINTSTS_EBE (1U << 15)
#define NX_SDXC_RINTSTS_ACD (1U << 14)
#define NX_SDXC_RINTSTS_SBE (1U << 13)
#define NX_SDXC_RINTSTS_HLE (1U << 12)
#define NX_SDXC_RINTSTS_FRUN (1U << 11)
#define NX_SDXC_RINTSTS_HTO (1U << 10)
#define NX_SDXC_RINTSTS_DRTO (1U << 9)
#define NX_SDXC_RINTSTS_RTO (1U << 8)
#define NX_SDXC_RINTSTS_DCRC (1U << 7)
#define NX_SDXC_RINTSTS_RCRC (1U << 6)
#define NX_SDXC_RINTSTS_RXDR (1U << 5)
#define NX_SDXC_RINTSTS_TXDR (1U << 4)
#define NX_SDXC_RINTSTS_DTO (1U << 3)
#define NX_SDXC_RINTSTS_CD (1U << 2)
#define NX_SDXC_RINTSTS_RE (1U << 1)

CBOOL NX_SDMMC_Init(SDXCBOOTSTATUS *pSDXCBootStatus);

//------------------------------------------------------------------------------
CBOOL NX_SDMMC_Terminate(SDXCBOOTSTATUS *pSDXCBootStatus);

//------------------------------------------------------------------------------
CBOOL NX_SDMMC_Open(SDXCBOOTSTATUS *pSDXCBootStatus); // U32 option );

//------------------------------------------------------------------------------
CBOOL NX_SDMMC_Close(SDXCBOOTSTATUS *pSDXCBootStatus);

//------------------------------------------------------------------------------
CBOOL NX_SDMMC_ReadSectors(SDXCBOOTSTATUS *pSDXCBootStatus, U32 SectorNum,
			   U32 numberOfSector, U32 *pdwBuffer);

void NX_SDPADSetALT(U32 PortNum);

void NX_SDPADSetGPIO(U32 PortNum);

#endif // __NX_SDHCBOOT_H__
