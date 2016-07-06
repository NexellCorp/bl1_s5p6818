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
#define HEADER_ID		\
	((((unsigned int)'N')<< 0) |	\
	 (((unsigned int)'S')<< 8) |	\
	 (((unsigned int)'I')<<16) |	\
	 (((unsigned int)'H')<<24))


enum	BOOT_FROM {
	BOOT_FROM_USB   = 0,
	BOOT_FROM_SPI   = 1,
	BOOT_FROM_NAND  = 2,
	BOOT_FROM_SDMMC = 3,
	BOOT_FROM_SDFS  = 4,
	BOOT_FROM_UART  = 5
};
#endif

struct nx_nandbootinfo {
	unsigned long long deviceaddr;

	unsigned char addrstep;
	unsigned char tCOS;
	unsigned char tACC;
	unsigned char tOCH;

	unsigned char pagesize;	/* 512bytes unit */
	unsigned char tioffset;	/* 3rd boot image copy offset. 1MB unit. */
	unsigned char copycount;	/* 3rd boot image copy count */
	unsigned char loaddevicenum;	/* device chip select number */

	unsigned char cryptokey[16];
};

struct nx_spibootinfo {
	unsigned long deviceaddr;

	unsigned char addrstep;
	unsigned char _reserved0[2];
	unsigned char portnumber;

	unsigned char _reserved1[3];
	unsigned char loaddevicenum;

	unsigned char cryptokey[16];
};

struct nx_uartbootinfo {
	unsigned long long __reserved;

	unsigned int _reserved0;

	unsigned char _reserved1[3];
	unsigned char loaddevicenum;

	unsigned char cryptokey[16];
};

struct nx_sdmmcbootinfo {
	/* Address of the device to find the image you want to load.*/
	unsigned long long deviceaddr;

	unsigned char _reserved0[3];
	/* port number */
	unsigned char portnumber;

	unsigned char _reserved1[3];
	/* Boot device id */
	unsigned char loaddevicenum;

	unsigned char cryptokey[16];
};

struct nx_sdfsbootinfo {
	char bootfileheadername[8];		/* 8.3 format ex)"nxdata.bh" */

	unsigned char _reserved0[3];
	unsigned char portnumber;

	unsigned char _reserved1[3];
	unsigned char loaddevicenum;

	unsigned char cryptokey[16];
};

struct nx_gmacbootinfo {
	unsigned char macaddr[8];
	unsigned char serverip[8];
	unsigned char clientip[8];
	unsigned char gateway[8];
};

union nx_devicebootinfo {			/* size:0x20 */
	struct nx_nandbootinfo	nandbi;
	struct nx_spibootinfo	spibi;
	struct nx_sdmmcbootinfo sdmmcbi;
	struct nx_sdfsbootinfo	sdfsbi;
	struct nx_uartbootinfo	uartbi;
	struct nx_gmacbootinfo	gmaci;
};

struct nx_ddrinitinfo {
	unsigned char chipnum;	/* 0x00 */
	unsigned char chiprow;	/* 0x01 */
	unsigned char buswidth;	/* 0x02 */
	unsigned char chipcol;	/* 0x03 */

	unsigned short chipmask;	/* 0x04 */
	unsigned short chipsize;	/* 0x06 */

	unsigned char cwl;		/* 0x08 */
	unsigned char cl;		/* 0x09 */
	unsigned char mr1_al;		/* 0x0a, mr2_rlwl (lpddr3) */
	unsigned char mr0_wr;		/* 0x0b, mr1_nwr (lpddr3) */

	unsigned int  _reserved0;	/* 0x0c */

	unsigned int  readdelay;	/* 0x10 */
	unsigned int  writedelay;	/* 0x14 */

	unsigned int  timingpzq;	/* 0x18 */
	unsigned int  timingaref;	/* 0x1c */
	unsigned int  timingrow;	/* 0x20 */
	unsigned int  timingdata;	/* 0x24 */
	unsigned int  timingpower;	/* 0x28 */
	unsigned int  _reserved1;	/* 0x2c */
};

struct nx_ddr3dev_drvdsinfo {
	unsigned char mr2_rtt_wr;
	unsigned char mr1_ods;
	unsigned char mr1_rtt_nom;
	unsigned char _reserved0;

	unsigned int  _reserved1;
};

struct nx_lpddr3dev_drvdsinfo {
	unsigned char mr3_ds      : 4;
	unsigned char mr11_dq_odt : 2;
	unsigned char mr11_pd_con : 1;
	unsigned char _reserved0  : 1;

	unsigned char _reserved1[3];
	unsigned int  _reserved2;
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
	unsigned char drvds_byte0;	/* data slice 0 */
	unsigned char drvds_byte1;	/* data slice 1 */
	unsigned char drvds_byte2;	/* data slice 2 */
	unsigned char drvds_byte3;	/* data slice 3 */

	unsigned char drvds_ck;	/* ck */
	unsigned char drvds_cke;	/* cke */
	unsigned char drvds_cs;	/* cs */
	unsigned char drvds_ca;	/* ca[9:0], ras, cas, wen, odt[1:0], reset, bank[2:0] */

	unsigned char zq_dds;		/* zq mode driver strength selection. */
	unsigned char zq_odt;
	unsigned char _reserved0[2];

	unsigned int  _reserved1;
};

struct nx_tbbinfo {
	unsigned int vector[8];				/* 0x000 ~ 0x01f */
	unsigned int vector_rel[8];			/* 0x020 ~ 0x03f */

	unsigned int _reserved0[4];			/* 0x040 ~ 0x04f */

	unsigned int LoadSize;				/* 0x050 */
	unsigned int CRC32;				/* 0x054 */
	unsigned long long LoadAddr;			/* 0x058 ~ 0x05f */
	unsigned long long StartAddr;			/* 0x060 ~ 0x067 */

	unsigned int _reserved1[2];			/* 0x068 ~ 0x06f */

	unsigned char Validslot[4];			/* 0x070 ~ 0x073 */
	unsigned char loadorder[4];			/* 0x074 ~ 0x077 */

	unsigned int _reserved2[2];			/* 0x078 ~ 0x07f */

	union nx_devicebootinfo dbi[4];			/* 0x080 ~ 0x0ff */

	unsigned int pll[8];				/* 0x100 ~ 0x11f */
	unsigned int pllspread[8];				/* 0x120 ~ 0x13f */

	unsigned int dvo[12];				/* 0x140 ~ 0x16f */

	struct nx_ddrinitinfo dii;			/* 0x170 ~ 0x19f */

	union nx_ddrdrvrsinfo sdramdrvr;		/* 0x1a0 ~ 0x1a7 */

	struct nx_ddrphy_drvdsinfo phy_dsinfo;		/* 0x1a8 ~ 0x1b7 */

	unsigned short lvltr_mode;			/* 0x1b8 ~ 0x1b9 */
	unsigned short flyby_mode;			/* 0x1ba ~ 0x1bb */

	unsigned char _reserved3[15*4];			/* 0x1bc ~ 0x1f7 */

	/* version */
	unsigned int buildinfo;				/* 0x1f8 */

	/* "NSIH": nexell system infomation header */
	unsigned int signature;				/* 0x1fc */
} __attribute__ ((packed, aligned(16)));

struct asymmetrickey {
	unsigned char rsapublickey[2048/8];			/* 0x200 ~ 0x2ff */
	unsigned char rsaencryptedsha256hash[2048/8];		/* 0x300 ~ 0x3ff */
};

struct nx_bootheader {
	struct nx_tbbinfo tbbi;
	struct asymmetrickey rsa_public;
};
#endif
