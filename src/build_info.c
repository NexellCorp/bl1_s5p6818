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
	unsigned int *pNsih_BuildInfo = (unsigned int *)(0xFFFF0000 + 0x1F8);
	unsigned int *p2nd_BuildInfo = (unsigned int *)(0xFFFF0200 + 0x024);
	unsigned int temp;
	int ret = 0;

	// Read Build Infomation.
	temp = (mmio_read_32(p2nd_BuildInfo) & 0xFFFF);

	SYSMSG("\r\n");
	SYSMSG("---------------------------------------------------------------"
	       "-----------------\r\n");
	SYSMSG(" Second Boot by Nexell Co. : Ver%d.%d.%d - Built on %s %s\r\n",
	       ((temp >> 12) & 0xF), ((temp >> 8) & 0xF), (temp & 0xFF), __DATE__,
	       __TIME__);
	SYSMSG("---------------------------------------------------------------"
	       "-----------------\r\n");

	temp = mmio_read_32(pNsih_BuildInfo) & 0xFFFFFF00;
	if ((mmio_read_32(p2nd_BuildInfo) & 0xFFFFFF00) != temp) {
		SYSMSG(" NSIH : Ver%d.%d.xx\r\n", ((temp >> 12) & 0xF),
		       ((temp >> 8) & 0xF));
		ret = -1;
	}

	return ret;
}
