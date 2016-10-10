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

void sleepMain(void);
U32 GetCPUID(void);
U32 GetSMCCode(void *);
U32 GetCurrentSMode(void);

void aarch32_monitor(unsigned int lr)
{
	U32 smccode = GetSMCCode((void *)lr);

	if (smccode == 12) {
		if (GetCPUID() > 0) {
			__asm__ __volatile__("dmb sy");
			__asm__ __volatile__("wfi");
		} else {
			printf("smc with suspend request code %d\r\nenter "
			       "suspend...\r\n",
			       smccode);
			while (!DebugIsTXEmpty())
				;
			while (DebugIsBusy())
				;

			sleepMain();

			printf("machine is resumed at mode 0x%x\r\n",
			       GetCurrentSMode());
			while (!DebugIsTXEmpty())
				;
			while (DebugIsBusy())
				;
		}
	} else {
		printf("unknown excption to Monitor mode\r\n");
	}
}
