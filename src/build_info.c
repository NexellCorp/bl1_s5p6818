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

#include <sysheader.h>

int build_information(void)
{
#if (SUPPORT_KERNEL_3_4 == 1)
	unsigned int offset = 0x224;
#else
	unsigned int offset = 0x424;
#endif
	unsigned int *info = (unsigned int *)(0xFFFF0000 + offset);
	unsigned int value = 0;

	value = (mmio_read_32(info) & 0xFFFF);

	SYSMSG("\r\n");
	SYSMSG("---------------------------------------------------------------"
	       "-----------------\r\n");
	SYSMSG(" Second Boot by Nexell Co. : Ver%d.%d.%d - Built on %s %s\r\n",
	       ((value >> 12) & 0xF), ((value >> 8) & 0xF), (value & 0xFF),
	       __DATE__, __TIME__);
	SYSMSG("---------------------------------------------------------------"
	       "-----------------\r\n");

	return 0;
}
