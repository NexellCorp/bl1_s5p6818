/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: DeokJin, Lee <truevirtue@nexell.co.kr>
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

#include <sysheader.h>

/*
 * Depending on the kernel version,
 * the crc algorithm changes
 */
#if (SUPPORT_KERNEL_3_4 == 1)
#undef SUPPORT_SW_CRC_CHECK
#define SUPPORT_SW_CRC_CHECK	1
#endif

/* Software CRC Check - Function Define */
#if (SUPPORT_SW_CRC_CHECK == 1)
#if (SUPPORT_KERNEL_3_4 == 1)

#define CRC_POLY 	0x04C11DB7L		// Reverse

unsigned int get_fcs(int fcs, unsigned char data)
{
	int i;

	fcs ^= (int)data;
	for (i = 0; i < 8; i++) {
		if (fcs & 0x01)
			fcs ^= CRC_POLY;
		fcs >>= 1;
	}

	return fcs;
}

unsigned int sget_fcs(int fcs, unsigned short data)
{
         int i;

         fcs ^= data;
         for (i = 0; i < 16; i++)
                 if (fcs & 1)
			fcs ^= CRC_POLY;
		fcs >>= 1;

         return fcs;
}

unsigned int iget_fcs(unsigned int fcs, unsigned int data)
{
	int i;

	fcs ^= data;
	for (i = 0; i < 32; i++) {
		if (fcs & 0x01)
			fcs ^= CRC_POLY;
		fcs >>= 1;
	}

	return fcs;
}
#else	// (SUPPORT_KERNEL_3_4 == 1)

#define CRC_POLY	0xEDB88320L		// Original

unsigned int get_fcs(unsigned int fcs, unsigned char data)
{
	int i;

	fcs ^= (unsigned int)data;
	for (i = 0; i < 8; i++) {
		if (fcs & 0x01)
			fcs = (fcs >> 1) ^ CRC_POLY;
		else
			fcs >>= 1;
	}

	return fcs;
}

unsigned int sget_fcs(int fcs, unsigned short data)
{
         int i;

         fcs ^= data;
         for (i = 0; i < 16; i++)
                 if (fcs & 1)
                         fcs = (fcs >> 1) ^ CRC_POLY;
                 else
                         fcs >>= 1;

         return fcs;
}

unsigned int iget_fcs(unsigned int fcs, unsigned int data)
{
	int i;

	fcs ^= data;
	for (i = 0; i < 32; i++) {
		if (fcs & 0x01)
			fcs = (fcs >> 1) ^ CRC_POLY;
		else
			fcs >>= 1;
	}

	return fcs;
}
#endif	// (SUPPORT_KERNEL_3_4 == 1)
#else	//#if (SUPPORT_SW_CRC_CHECK == 1)

/* ARM(Hardware) CRC Check Function */
extern int get_fcs  (unsigned int fcs, unsigned char data);
extern int sget_fcs(unsigned int fcs, unsigned short data);
extern int iget_fcs (unsigned int fcs, unsigned int data);

#endif

/*
  * CRC Calcurate Function
  * CRC_CHKSTRIDE is Data Stride.
  */
#if (SUPPORT_KERNEL_3_4 == 1)
#define CRC_CHKSTRIDE		8
#else
#define CRC_CHKSTRIDE		1
#endif

unsigned int calc_crc(void *addr, int len)
{
	unsigned int *c = (unsigned int *)addr;
	unsigned int crc = 0, chkcnt = ((len + 3) / 4);
	unsigned int i, stride = CRC_CHKSTRIDE;

	for (i = 0; chkcnt > i; i += stride, c += stride)
		crc = iget_fcs(crc, *c);

	return crc;
}

/*
 * Add CRC Check Function.
 * When there is a problem, except for boot device(SD/eMMC/USB/etc),
 * you can check the error.
 */
int crc_check(void* buf, int size, int ref_crc)
{
	int crc, ret = 0;

	crc = calc_crc((void*)buf, (int)size);
	if (ref_crc != crc)
		ret = -1;

	NOTICE("CRC Check %s!! (%08X:%08X) \r\n",
		((ret < 0) ? "Failed":"Success"), ref_crc, crc);
	return ret;
}
