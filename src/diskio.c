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

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include <nx_type.h>
#include "diskio.h"
#include "printf.h"
#include <nx_debug2.h>
#include <iSDHCBOOT.h>

#ifdef DEBUG
#define dprintf(x, ...) printf(x, ...)
#else
#define dprintf(x, ...)
#endif

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */

#define MMC 0
#define USB 1
#define ATA 2

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize(U8 drv /* Physical drive nmuber (0..) */
			)
{
	//	DSTATUS stat = RES_OK;
	//	int result;

	switch (drv) {
	case MMC:
		//		result = MMC_disk_initialize();
		// translate the reslut code here

		return RES_OK; // stat;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status(U8 drv /* Physical drive nmuber (0..) */
		    )
{
	//	DSTATUS stat;
	//	int result;

	switch (drv) {
	case MMC:
		//		result = MMC_disk_status();
		// translate the reslut code here

		return RES_OK; // stat;
		//		return STA_NOINIT;//stat;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read(U8 drv,     /* Physical drive nmuber (0..) */
		  U8 *buff,   /* Data buffer to store read data */
		  U32 sector, /* Sector address (LBA) */
		  U8 count,   /* Number of sectors to read (1..255) */
		  U32 *diskhandle)
{
	SDXCBOOTSTATUS *pSDXCBootStatus = (SDXCBOOTSTATUS *)((MPTRS)diskhandle);
	dprintf("disk_read: sector: %d count: %d\r\n", sector, count);

	switch (drv) {
	case MMC:
		//		result = MMC_disk_read(buff, sector, count);
		// translate the reslut code here
		NX_SDMMC_ReadSectors(pSDXCBootStatus, sector, count,
				     (U32 *)buff);
#if 0
{
	U32 k;
	for(k=0; k<count; k++)
	{
		U32 i;
		for(i = 0; i<512; i++)
		{
			U32 j;
			for(j=0; j<16; j++)
			{
				DebugPutch(buff[i+j]);
				DebugPutch(' ');
			}
			DebugPutch(' ');
			for(j=0; j<16; j++)
			{
				if(buff[i+j]<0x20 || buff[i+j]>0x80)
				{
					DebugPutch('.');
				}
				else
				{
					DebugPutch(buff[i+j]);
				}
			}
			DebugPutch('\r');
			DebugPutch('\n');
			i+= 16;
		}
	}
}
#endif
		return RES_OK;
	}
	return RES_PARERR;
}
