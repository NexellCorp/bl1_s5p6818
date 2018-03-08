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

#include <nx_peridot.h>
#include <nx_type.h>
#include <nx_debug2.h>

#include <nx_chip.h>

#include <nx_gpio.h>

struct NXPYROPE_GPIO_RegSet {
	struct NX_GPIO_RegisterSet NXGPIO;
	U8 Reserved[0x1000 - sizeof(struct NX_GPIO_RegisterSet)];
};

#if !DIRECT_IO
static struct NXPYROPE_GPIO_RegSet (*const pBaseGPIOReg)[1] =
    (struct NXPYROPE_GPIO_RegSet(*)[])(PHY_BASEADDR_GPIOA_MODULE);
void GPIOSetAltFunction(U32 AltFunc)
{
	register struct NX_GPIO_RegisterSet *pGPIOReg =
	    &pBaseGPIOReg[(AltFunc >> 8) & 0x7]->NXGPIO;
	pGPIOReg->GPIOxALTFN[(AltFunc >> 7) & 0x1] =
	    (pGPIOReg->GPIOxALTFN[(AltFunc >> 7) & 0x1] &
	     ~(0x3UL << (((AltFunc >> 3) & 0xF) * 2))) |
	    ((AltFunc & 0x3) << (((AltFunc >> 3) & 0xF) * 2));
}
#endif
