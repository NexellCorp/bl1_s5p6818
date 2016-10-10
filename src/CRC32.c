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

#include "sysheader.h"
#include <nx_peridot.h>
#include <nx_type.h>

#if 0 //def aarch32
U32 get_fcs(U32 fcs, U8 data)
{
	int i;

	fcs ^= (U32)data;
	for (i = 0; i < 8; i++) {
		if (fcs & 0x01)
			fcs = (fcs >> 1) ^ POLY;
		else
			fcs >>= 1;
	}

	return fcs;
}

U32 sget_fcs(U32 fcs, U16 data)
{
         int i;

         fcs ^= data;
         for (i = 0; i < 16; i++)
                 if (fcs & 1)
                         fcs = (fcs >> 1) ^ POLY;
                 else
                         fcs >>= 1;

         return fcs;
}

U32 iget_fcs(U32 fcs, U32 data)
{
	int i;

	fcs ^= data;
	for (i = 0; i < 32; i++) {
		if (fcs & 0x01)
			fcs = (fcs >> 1) ^ POLY;
		else
			fcs >>= 1;
	}

	return fcs;
}
#endif

extern U32 get_fcs(U32 fcs, U8 data);
extern U32 sget_fcs(U32 fcs, U16 data);
extern U32 iget_fcs(U32 fcs, U32 data);

/* CRC Calcurate Function
 * CHKSTRIDE is Data Stride.
 */
#define CHKSTRIDE 1
U32 __calc_crc(void *addr, int len)
{
	U32 *c = (U32 *)addr;
	U32 crc = 0, chkcnt = ((len + 3) / 4);
	U32 i;

	for (i = 0; chkcnt > i; i += CHKSTRIDE, c += CHKSTRIDE) {
		crc = iget_fcs(crc, *c);
	}

	return crc;
}

/*
 * Add CRC Check Function.
 * When there is a problem, except for boot device(SD/eMMC/USB/etc), you can check the error.
 */
int CRC_Check(void* buf, unsigned int size, unsigned int ref_crc)
{
	unsigned int crc;
#if 0	/* Debug Message */
	printf("CRC - Addr : %X, Size : %X \r\n", buf, size);
#endif
	crc = __calc_crc((void*)buf, (int)size);
	if (ref_crc != crc) {
		printf("CRC Check failed!! (%08X:%08X) \r\n"
			,ref_crc, crc);
		return 0;
	} else {
		printf("CRC Check Success!! (%08X:%08X) \r\n"
			,ref_crc, crc);
	}

	return 1;
}
