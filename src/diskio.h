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

/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file  R0.07   (C)ChaN, 2009
/-----------------------------------------------------------------------*/

#ifndef _DISKIO

#define _READONLY 0 /* 1: Read-only mode */
#define _USE_IOCTL 1

/* Status of Disk Functions */
typedef U8 DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0, /* 0: Successful */
	RES_ERROR,  /* 1: R/W Error */
	RES_WRPRT,  /* 2: Write Protected */
	RES_NOTRDY, /* 3: Not Ready */
	RES_PARERR  /* 4: Invalid Parameter */
} DRESULT;

/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS disk_initialize(U8);
DSTATUS disk_status(U8);
DRESULT disk_read(U8, U8 *, U32, U8, U32 *);

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT 0x01  /* Drive not initialized */
#define STA_NODISK 0x02  /* No medium in the drive */
#define STA_PROTECT 0x04 /* Write protected */

/* Command code for disk_ioctrl() */

/* Generic command */
#define CTRL_SYNC 0	/* Mandatory for write functions */
#define GET_SECTOR_COUNT 1 /* Mandatory for only f_mkfs() */
#define GET_SECTOR_SIZE 2
#define GET_BLOCK_SIZE 3 /* Mandatory for only f_mkfs() */
#define CTRL_POWER 4
#define CTRL_LOCK 5
#define CTRL_EJECT 6
/* MMC/SDC command */
#define MMC_GET_TYPE 10
#define MMC_GET_CSD 11
#define MMC_GET_CID 12
#define MMC_GET_OCR 13
#define MMC_GET_SDSTAT 14
/* ATA/CF command */
#define ATA_GET_REV 20
#define ATA_GET_MODEL 21
#define ATA_GET_SN 22

#define _DISKIO
#endif
