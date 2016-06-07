/*
 * Copyright (C) 2012 Nexell Co., All Rights Reserved
 * Nexell Co. Proprietary & Confidential
 *
 * NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 * AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * Module	: boot
 * File		: bootheader.h
 * Description	: This must be synchronized with NSIH.txt
 * Author	: hans
 * History	: 2016.05.12 create
 */

#ifndef __NX_BOOTHEADER_H__
#define __NX_BOOTHEADER_H__

#if 0
#define HEADER_ID			\
	((((uint32_t)'N')<< 0) |	\
	 (((uint32_t)'S')<< 8) |	\
	 (((uint32_t)'I')<<16) |	\
	 (((uint32_t)'H')<<24))


enum	BOOT_FROM {
	BOOT_FROM_USB   = 0UL,
	BOOT_FROM_SPI   = 1UL,
	BOOT_FROM_NAND  = 2UL,
	BOOT_FROM_SDMMC = 3UL,
	BOOT_FROM_SDFS  = 4UL,
	BOOT_FROM_UART  = 5UL
};
#endif

struct nx_nandbootinfo {
	uint64_t deviceaddr;

	uint8_t addrstep;
	uint8_t tCOS;
	uint8_t tACC;
	uint8_t tOCH;

	uint8_t pagesize;	/* 512bytes unit */
	uint8_t tioffset;	/* 3rd boot image copy offset. 1MB unit. */
	uint8_t copycount;	/* 3rd boot image copy count */
	uint8_t loaddevicenum;	/* device chip select number */

	uint8_t cryptokey[16];
};

struct nx_spibootinfo {
	uint64_t deviceaddr;

	uint8_t addrstep;
	uint8_t _reserved0[2];
	uint8_t portnumber;

	uint8_t _reserved1[3];
	uint8_t loaddevicenum;

	uint8_t cryptokey[16];
};

struct nx_uartbootinfo {
	uint64_t __reserved;

	uint32_t _reserved0;

	uint8_t _reserved1[3];
	uint8_t loaddevicenum;

	uint8_t cryptokey[16];
};

struct nx_sdmmcbootinfo {
	uint64_t deviceaddr;

	uint8_t _reserved0[3];
	uint8_t portnumber;

	uint8_t _reserved1[3];
	uint8_t loaddevicenum;

	uint8_t cryptokey[16];
};

struct nx_sdfsbootinfo {
	char bootfileheadername[8];		/* 8.3 format ex)"nxdata.bh" */

	uint8_t _reserved0[3];
	uint8_t portnumber;

	uint8_t _reserved1[3];
	uint8_t loaddevicenum;

	uint8_t cryptokey[16];
};

struct nx_gmacbootinfo {
	uint8_t macaddr[8];
	uint8_t serverip[8];
	uint8_t clientip[8];
	uint8_t gateway[8];
};

union nx_devicebootinfo {
	struct nx_nandbootinfo	nandbi;
	struct nx_spibootinfo	spibi;
	struct nx_sdmmcbootinfo sdmmcbi;
	struct nx_sdfsbootinfo	sdfsbi;
	struct nx_uartbootinfo	uartbi;
	struct nx_gmacbootinfo	gmaci;
};

struct nx_ddrinitinfo {
	uint8_t chipnum;	/* 0x00 */
	uint8_t chiprow;	/* 0x01 */
	uint8_t buswidth;	/* 0x02 */
	uint8_t chipcol;	/* 0x03 */

	uint16_t chipmask;	/* 0x04 */
	uint16_t chipsize;	/* 0x06 */

	uint8_t cwl;		/* 0x08 */
	uint8_t cl;		/* 0x09 */
	uint8_t mr1_al;		/* 0x0a, mr2_rlwl (lpddr3) */
	uint8_t mr0_wr;		/* 0x0b, mr1_nwr (lpddr3) */

	uint32_t  _reserved0;	/* 0x0c */

	uint32_t  readdelay;	/* 0x10 */
	uint32_t  writedelay;	/* 0x14 */

	uint32_t  timingpzq;	/* 0x18 */
	uint32_t  timingaref;	/* 0x1c */
	uint32_t  timingrow;	/* 0x20 */
	uint32_t  timingdata;	/* 0x24 */
	uint32_t  timingpower;	/* 0x28 */
	uint32_t  _reserved1;	/* 0x2c */
};

struct nx_ddr3dev_drvdsinfo {
	uint8_t mr2_rtt_wr;
	uint8_t mr1_ods;
	uint8_t mr1_rtt_nom;
	uint8_t _reserved0;

	uint32_t  _reserved1;
};

struct nx_lpddr3dev_drvdsinfo {
	uint8_t mr3_ds      : 4;
	uint8_t mr11_dq_odt : 2;
	uint8_t mr11_pd_con : 1;
	uint8_t _reserved0  : 1;

	uint8_t _reserved1[3];
	uint32_t  _reserved2;
};

#define LVLTR_WR_LVL    (1 << 0)
#define LVLTR_CA_CAL    (1 << 1)
#define LVLTR_GT_LVL    (1 << 2)
#define LVLTR_RD_CAL    (1 << 3)
#define LVLTR_WR_CAL    (1 << 4)

union nx_ddrdrvrsinfo {
	struct nx_ddr3dev_drvdsinfo ddr3drvr;
	struct nx_lpddr3dev_drvdsinfo lpddr3drvr;
};

struct nx_ddrphy_drvdsinfo {
	uint8_t drvds_byte0;	/* data slice 0 */
	uint8_t drvds_byte1;	/* data slice 1 */
	uint8_t drvds_byte2;	/* data slice 2 */
	uint8_t drvds_byte3;	/* data slice 3 */

	uint8_t drvds_ck;	/* ck */
	uint8_t drvds_cke;	/* cke */
	uint8_t drvds_cs;	/* cs */
	uint8_t drvds_ca;	/* ca[9:0], ras, cas, wen, odt[1:0], reset, bank[2:0] */

	uint8_t zq_dds;		/* zq mode driver strength selection. */
	uint8_t zq_odt;
	uint8_t _reserved0[2];

	uint32_t  _reserved1;
};

struct nx_tbbinfo {
	uint32_t vector[8];			/* 0x000 ~ 0x01c */
	uint32_t vector_rel[8];			/* 0x020 ~ 0x03c */

	uint32_t _reserved0[4];			/* 0x040 ~ 0x04C */

	uint32_t loadsize;			/* 0x050 */
	uint32_t crc32;				/* 0x054 */
	uint64_t loadaddr;			/* 0x058 ~ 0x05c */
	uint64_t startaddr;			/* 0x060 ~ 0x064 */

	uint32_t _reserved1[2];			/* 0x068 ~ 0x06c */

	uint8_t validslot[4];			/* 0x070 ~ 0x073 */
	uint8_t loadorder[4];			/* 0x074 ~ 0x077 */

	uint32_t _reserved2[2];			/* 0x078 ~ 0x07c */

	union nx_devicebootinfo dbi[4];		/* 0x080 ~ 0x0fc */

	uint32_t pll[8];			/* 0x100 ~ 0x11c */
	uint32_t pllspread[8];			/* 0x120 ~ 0x13c */

	uint32_t dvo[12];			/* 0x140 ~ 0x16c */

	struct nx_ddrinitinfo dii;		/* 0x170 ~ 0x19c */

	union nx_ddrdrvrsinfo sdramdrvr;	/* 0x1a0 ~ 0x1a4 */

	struct nx_ddrphy_drvdsinfo phy_dsinfo;	/* 0x1a8 ~ 0x1b8 */

	uint16_t lvltr_mode;			/* 0x1bc ~ 0x1bd */
	uint16_t flyby_mode;			/* 0x1be ~ 0x1bf */

	uint8_t _reserved3[14*4];		/* 0x1c0 ~ 0x1f7 */

	uint32_t buildinfo;			/* 0x1f8 */

	/* "NSIH": nexell system infomation header */
	uint32_t signature;			/* 0x1fc */
} __attribute__ ((packed, aligned(16)));

struct asymmetrickey {
	uint8_t rsapublickey[2048/8];		/* 0x200 ~ 0x2ff */
	uint8_t rsaencryptedsha256hash[2048/8];	/* 0x300 ~ 0x3ff */
};

struct nx_bootheader {
	struct nx_tbbinfo tbbi;
	struct asymmetrickey rsa_public;
};
#endif
