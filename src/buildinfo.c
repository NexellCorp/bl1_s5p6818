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
//------------------------------------------------------------------------------
CBOOL buildinfo(void)
{
	CBOOL ret = CTRUE;
	U32 *pNsih_BuildInfo = (U32 *)(0xFFFF0000 + 0x1F8);
	U32 *p2nd_BuildInfo = (U32 *)(0xFFFF0200 + 0x024);
	U32 tmp;

	// Read Build Infomation.
	tmp = ReadIO32(p2nd_BuildInfo) & 0xFFFF;

	SYSMSG("\r\n");
	SYSMSG("---------------------------------------------------------------"
	       "-----------------\r\n");
	SYSMSG(" Second Boot by Nexell Co. : Ver%d.%d.%d - Built on %s %s\r\n",
	       ((tmp >> 12) & 0xF), ((tmp >> 8) & 0xF), (tmp & 0xFF), __DATE__,
	       __TIME__);
	SYSMSG("---------------------------------------------------------------"
	       "-----------------\r\n");

	tmp = ReadIO32(pNsih_BuildInfo) & 0xFFFFFF00;
	if ((ReadIO32(p2nd_BuildInfo) & 0xFFFFFF00) != tmp) {
		SYSMSG(" NSIH : Ver%d.%d.xx\r\n", ((tmp >> 12) & 0xF),
		       ((tmp >> 8) & 0xF));
		ret = CFALSE;
	}

	return ret;
}
