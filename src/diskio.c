/*
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved
 *      Nexell Co. Proprietary & Confidential
 *
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 *FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      Module          : SDMMC File System
 *      File            : diskio.c
 *      Description     :
 *      Author          : Hans
 *      History         :
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
