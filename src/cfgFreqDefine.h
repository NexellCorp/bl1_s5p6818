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

#ifndef __CFG_FREQ_DEFINE_H__
#define __CFG_FREQ_DEFINE_H__

//------------------------------------------------------------------------------
//	oscilator
//------------------------------------------------------------------------------
#define OSC_HZ (24000000)
#define OSC_KHZ (OSC_HZ / 1000)
#define OSC_MHZ (OSC_KHZ / 1000)

#define NX_CLKSRC_PLL_0 (0)
#define NX_CLKSRC_PLL_1 (1)
#define NX_CLKSRC_PLL_2 (2)
#define NX_CLKSRC_PLL_3 (3)

//------------------------------------------------------------------------------
#define NX_CLKSRC_UART (NX_CLKSRC_PLL_2)
#define NX_CLKSRC_SDMMC (NX_CLKSRC_PLL_2)
#define NX_CLKSRC_SPI (NX_CLKSRC_PLL_0)

#endif /* __CFG_FREQ_DEFINE_H__ */
