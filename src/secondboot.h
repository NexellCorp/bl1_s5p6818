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

#ifndef __NX_SECONDBOOT_H__
#define __NX_SECONDBOOT_H__

#include "cfgBootDefine.h"

#define HEADER_ID                                                              \
	((((U32)'N') << 0) | (((U32)'S') << 8) | (((U32)'I') << 16) |          \
	 (((U32)'H') << 24))

#define LVLTR_WR_LVL (1 << 0)
#define LVLTR_CA_CAL (1 << 1)
#define LVLTR_GT_LVL (1 << 2)
#define LVLTR_RD_CAL (1 << 3)
#define LVLTR_WR_CAL (1 << 4)

enum { BOOT_FROM_USB = 0UL,
       BOOT_FROM_SPI = 1UL,
       BOOT_FROM_NAND = 2UL,
       BOOT_FROM_SDMMC = 3UL,
       BOOT_FROM_SDFS = 4UL,
       BOOT_FROM_UART = 5UL };

struct NX_NANDBootInfo {
	U8 AddrStep;
	U8 tCOS;
	U8 tACC;
	U8 tOCH;
#if 0
	U32 PageSize    :24;    // 1 byte unit
	U32 LoadDevice  :8;
#else
	U8 PageSize;   // 512bytes unit
	U8 TIOffset;   // 3rd boot Image copy Offset. 1MB unit.
	U8 CopyCount;  // 3rd boot image copy count
	U8 LoadDevice; // device chip select number
#endif
	U32 CRC32;
};

struct NX_SPIBootInfo {
	U8 AddrStep;
	U8 _Reserved0[2];
	U8 PortNumber;

	U32 _Reserved1 : 24;
	U32 LoadDevice : 8;

	U32 CRC32;
};

struct NX_UARTBootInfo {
	U32 _Reserved0;

	U32 _Reserved1 : 24;
	U32 LoadDevice : 8;

	U32 CRC32;
};

struct NX_SDMMCBootInfo {
#if 1
	U8 PortNumber;
	U8 _Reserved0[3];
#else
	U8 _Reserved0[3];
	U8 PortNumber;
#endif

	U32 _Reserved1 : 24;
	U32 LoadDevice : 8;

	U32 CRC32;
};

struct NX_DDR3DEV_DRVDSInfo {
	U8 MR2_RTT_WR;
	U8 MR1_ODS;
	U8 MR1_RTT_Nom;
	U8 _Reserved0;
};

struct NX_LPDDR3DEV_DRVDSInfo {
	U8 MR3_DS : 4;
	U8 MR11_DQ_ODT : 2;
	U8 MR11_PD_CON : 1;
	U8 _Reserved0 : 1;

	U8 _Reserved1;
	U8 _Reserved2;
	U8 _Reserved3;
};

struct NX_DDRPHY_DRVDSInfo {
	U8 DRVDS_Byte0; // Data Slice 0
	U8 DRVDS_Byte1; // Data Slice 1
	U8 DRVDS_Byte2; // Data Slice 2
	U8 DRVDS_Byte3; // Data Slice 3

	U8 DRVDS_CK;  // CK
	U8 DRVDS_CKE; // CKE
	U8 DRVDS_CS;  // CS
	U8 DRVDS_CA;  // CA[9:0], RAS, CAS, WEN, ODT[1:0], RESET, BANK[2:0]

	U8 ZQ_DDS; // zq mode driver strength selection.
	U8 ZQ_ODT;
	U8 _Reserved0[2];
};

struct NX_SDFSBootInfo {
	char BootFileName[12]; // 8.3 format ex)"NXDATA.TBL"
};

union NX_DeviceBootInfo {
	struct NX_NANDBootInfo NANDBI;
	struct NX_SPIBootInfo SPIBI;
	struct NX_SDMMCBootInfo SDMMCBI;
	struct NX_SDFSBootInfo SDFSBI;
	struct NX_UARTBootInfo UARTBI;
};

struct NX_DDRInitInfo {
	U8 ChipNum;  // 0x88
	U8 ChipRow;  // 0x89
	U8 BusWidth; // 0x8A
	U8 ChipCol;  // 0x8B

	U16 ChipMask; // 0x8C
	U16 ChipSize; // 0x8E

#if 0
	U8  CWL;            // 0x90
	U8  WL;             // 0x91
	U8  RL;             // 0x92
	U8  DDRRL;          // 0x93
#else
	U8 CWL;    // 0x90
	U8 CL;     // 0x91
	U8 MR1_AL; // 0x92, MR2_RLWL (LPDDR3)
	U8 MR0_WR; // 0x93, MR1_nWR (LPDDR3)
#endif

	U32 READDELAY;  // 0x94
	U32 WRITEDELAY; // 0x98

	U32 TIMINGPZQ;   // 0x9C
	U32 TIMINGAREF;  // 0xA0
	U32 TIMINGROW;   // 0xA4
	U32 TIMINGDATA;  // 0xA8
	U32 TIMINGPOWER; // 0xAC
};

struct NX_SecondBootInfo {
	U32 VECTOR[8];     // 0x000 ~ 0x01C
	U32 VECTOR_Rel[8]; // 0x020 ~ 0x03C

	U32 DEVICEADDR; // 0x040

	U32 LOADSIZE;		     // 0x044
	U32 LOADADDR;		     // 0x048
	U32 LAUNCHADDR;		     // 0x04C
	union NX_DeviceBootInfo DBI; // 0x050~0x058

	U32 PLL[4];       // 0x05C ~ 0x068
	U32 PLLSPREAD[2]; // 0x06C ~ 0x070

#if defined(ARCH_NXP4330) || defined(ARCH_S5P4418)
	U32 DVO[5]; // 0x074 ~ 0x084

	struct NX_DDRInitInfo DII; // 0x088 ~ 0x0AC

#if defined(MEM_TYPE_DDR3)
	struct NX_DDR3DEV_DRVDSInfo DDR3_DSInfo; // 0x0B0
#endif
#if defined(MEM_TYPE_LPDDR23)
	struct NX_LPDDR3DEV_DRVDSInfo LPDDR3_DSInfo; // 0x0B0
#endif
	struct NX_DDRPHY_DRVDSInfo PHY_DSInfo; // 0x0B4 ~ 0x0BC

	U16 LvlTr_Mode; // 0x0C0 ~ 0x0C1
	U16 FlyBy_Mode; // 0x0C2 ~ 0x0C3

	U32 Stub[(0x1EC - 0x0C4) / 4]; // 0x0C4 ~ 0x1EC
#endif
#if defined(ARCH_NXP5430)
	U32 DVO[9]; // 0x074 ~ 0x094

	struct NX_DDRInitInfo DII; // 0x098 ~ 0x0BC

#if defined(MEM_TYPE_DDR3)
	struct NX_DDR3DEV_DRVDSInfo DDR3_DSInfo; // 0x0C0
#endif
#if defined(MEM_TYPE_LPDDR23)
	struct NX_LPDDR3DEV_DRVDSInfo LPDDR3_DSInfo; // 0x0C0
#endif
	struct NX_DDRPHY_DRVDSInfo PHY_DSInfo; // 0x0C4 ~ 0x0CC

	U16 LvlTr_Mode; // 0x0D0 ~ 0x0D1
	U16 FlyBy_Mode; // 0x0D2 ~ 0x0D3

#if (BOOTCOUNT == 1)
	U32 Stub[(0x1E4 - 0x0D4) / 4]; // 0x0D4 ~ 0x1E4
	U32 ResetCount;		       // 0x1E4
	U32 BootCount;		       // 0x1E8
#else
	U32 Stub[(0x1EC - 0x0D4) / 4]; // 0x0D4 ~ 0x1EC
#endif
#endif

	U32 MemTestAddr;     // 0x1EC
	U32 MemTestSize;     // 0x1F0
	U32 MemTestTryCount; // 0x1F4

	U32 BuildInfo; // 0x1F8

	U32 SIGNATURE; // 0x1FC    "NSIH"
} __attribute__((packed, aligned(4)));

// [0] : Use ICache
// [1] : Change PLL
// [2] : Decrypt
// [3] : Suspend Check

#endif //__NX_SECONDBOOT_H__
