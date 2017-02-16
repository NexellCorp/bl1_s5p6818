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
#include <memory.h>

int memory_initialize(int is_resume)
{
	int ret = 0;

	SYSMSG("\r\nMemory Intialize. (%d) \r\n", is_resume);

	/* step xx. memory initialize  */
#if defined(MEM_TYPE_DDR3)
	/*
	 * DDR initialization fails, a temporary code
	 * code for the defense.
	 */
	int mem_retry = 3;
	while (mem_retry--) {
		ret = ddr3_initialize(is_resume);
		if (ret >= 0)
			break;
	}
#endif

#if  defined(MEM_TYPE_LPDDR23)
	ret = lpddr3_initialize(0);
	if (ret == CFALSE)
		lpddr3_initialize(0);
#endif

	/* step xx. exit the (sdram) self-refresh  */
	if (is_resume)
		exit_self_refresh();

	SYSMSG("Memory Initialize Done!\r\n");

	return ret;
}
