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
#ifndef __CLOCK_H__
#define __CLOCK_H__

#define PLL_P_BITPOS			18
#define PLL_M_BITPOS			8
#define PLL_S_BITPOS			0
#define PLL_K_BITPOS			16

#define CLKSRC_BITPOS			0
#define DVO0_BITPOS			3
#define DVO1_BITPOS			9
#define DVO2_BITPOS			15
#define DVO3_BITPOS			21

#define GET_PLL01(_MHz, _val)						\
    _val = (unsigned int)((PLL01_PMS_##_MHz##MHZ_P << PLL_P_BITPOS) |	\
    			  (PLL01_PMS_##_MHz##MHZ_M << PLL_M_BITPOS) |	\
    			  (PLL01_PMS_##_MHz##MHZ_S << PLL_S_BITPOS));

#define GET_PLL23(_MHz, _val)						\
    _val = (unsigned int)((PLL23_PMS_##_MHz##MHZ_P << PLL_P_BITPOS) |	\
    			  (PLL23_PMS_##_MHz##MHZ_M << PLL_M_BITPOS) |	\
    			  (PLL23_PMS_##_MHz##MHZ_S << PLL_S_BITPOS));

#define GET_PLL23K(_MHz, _val)							\
	_val = (unsigned int)((PLL23_PMS_##_MHz##MHZ_K << PLL_K_BITPOS) | 0x0104);


#define GET_PLL01_FREQ(STR, PMS)	GET_PLL01 (STR, PMS)
#define GET_PLL23_FREQ(STR, PMS)	GET_PLL23 (STR, PMS)
#define GET_PLL23K_FREQ(STR, PMS)	GET_PLL23K(STR, PMS)

/* PMS for PLL0, 1 */
#define PLL01_PMS_1800MHZ_P		3
#define PLL01_PMS_1800MHZ_M		450
#define PLL01_PMS_1800MHZ_S		1

#define PLL01_PMS_1700MHZ_P		3
#define PLL01_PMS_1700MHZ_M		425
#define PLL01_PMS_1700MHZ_S		1

#define PLL01_PMS_1600MHZ_P		3		// pll0 {cpu clock, memory clock}
#define PLL01_PMS_1600MHZ_M		400
#define PLL01_PMS_1600MHZ_S		1

#define PLL01_PMS_1500MHZ_P		3
#define PLL01_PMS_1500MHZ_M		375
#define PLL01_PMS_1500MHZ_S		1

#define PLL01_PMS_1400MHZ_P		3		// 6 // 3
#define PLL01_PMS_1400MHZ_M		350		// 350 // 350
#define PLL01_PMS_1400MHZ_S		1		// 0 // 1

#define PLL01_PMS_1300MHZ_P		3
#define PLL01_PMS_1300MHZ_M		325
#define PLL01_PMS_1300MHZ_S		1

#define PLL01_PMS_1200MHZ_P		3
#define PLL01_PMS_1200MHZ_M		300
#define PLL01_PMS_1200MHZ_S		1

#define PLL01_PMS_1100MHZ_P		3
#define PLL01_PMS_1100MHZ_M		275
#define PLL01_PMS_1100MHZ_S		1

#define PLL01_PMS_1000MHZ_P3
#define PLL01_PMS_1000MHZ_M250
#define PLL01_PMS_1000MHZ_S1

#define PLL01_PMS_933MHZ_P		4
#define PLL01_PMS_933MHZ_M		311
#define PLL01_PMS_933MHZ_S		1

#if 1
#define PLL01_PMS_800MHZ_P		3
#define PLL01_PMS_800MHZ_M		200
#define PLL01_PMS_800MHZ_S		1
#else
#define PLL01_PMS_800MHZ_P		6
#define PLL01_PMS_800MHZ_M		400
#define PLL01_PMS_800MHZ_S		1
#endif

#define PLL01_PMS_700MHZ_P		3
#define PLL01_PMS_700MHZ_M		175
#define PLL01_PMS_700MHZ_S		1

#define PLL01_PMS_666MHZ_P		2
#define PLL01_PMS_666MHZ_M		111
#define PLL01_PMS_666MHZ_S		1

#if 0
#define PLL01_PMS_600MHZ_P		3
#define PLL01_PMS_600MHZ_M		200
#define PLL01_PMS_600MHZ_S		1
#else
#define PLL01_PMS_600MHZ_P		2
#define PLL01_PMS_600MHZ_M		200
#define PLL01_PMS_600MHZ_S		2
#endif

#define PLL01_PMS_533MHZ_P		6
#define PLL01_PMS_533MHZ_M		533
#define PLL01_PMS_533MHZ_S		2

#define PLL01_PMS_400MHZ_P		3
#define PLL01_PMS_400MHZ_M		200
#define PLL01_PMS_400MHZ_S 		2

/* PMS for PLL 2, 3 */
#define PLL23_PMS_933MHZ_P		4
#define PLL23_PMS_933MHZ_M		311
#define PLL23_PMS_933MHZ_S		1
#define PLL23_PMS_933MHZ_K

#define PLL23_PMS_920MHZ_P		3
#define PLL23_PMS_920MHZ_M		230
#define PLL23_PMS_920MHZ_S		1
#define PLL23_PMS_920MHZ_K		0

#define PLL23_PMS_910MHZ_P		4
#define PLL23_PMS_910MHZ_M		303
#define PLL23_PMS_910MHZ_S		1
#define PLL23_PMS_910MHZ_K		0

#define PLL23_PMS_900MHZ_P		3
#define PLL23_PMS_900MHZ_M		225
#define PLL23_PMS_900MHZ_S		1
#define PLL23_PMS_900MHZ_K		0

#define PLL23_PMS_890MHZ_P		2
#define PLL23_PMS_890MHZ_M		148
#define PLL23_PMS_890MHZ_S		1
#define PLL23_PMS_890MHZ_K		0

#define PLL23_PMS_800MHZ_P		3
#define PLL23_PMS_800MHZ_M		200
#define PLL23_PMS_800MHZ_S 		1
#define PLL23_PMS_800MHZ_K		0

#define PLL23_PMS_790MHZ_P		4
#define PLL23_PMS_790MHZ_M		263
#define PLL23_PMS_790MHZ_S		1
#define PLL23_PMS_790MHZ_K		0

#define PLL23_PMS_760MHZ_P		3
#define PLL23_PMS_760MHZ_M		190
#define PLL23_PMS_760MHZ_S		1
#define PLL23_PMS_760MHZ_K		0

#define PLL23_PMS_750MHZ_P		4
#define PLL23_PMS_750MHZ_M		250
#define PLL23_PMS_750MHZ_S		1
#define PLL23_PMS_750MHZ_K		0

#define PLL23_PMS_740MHZ_P		3
#define PLL23_PMS_740MHZ_M		185
#define PLL23_PMS_740MHZ_S		1
#define PLL23_PMS_740MHZ_K		0

#define PLL23_PMS_730MHZ_P		4
#define PLL23_PMS_730MHZ_M		243
#define PLL23_PMS_730MHZ_S		1
#define PLL23_PMS_730MHZ_K		0

#define PLL23_PMS_720MHZ_P		3
#define PLL23_PMS_720MHZ_M		180
#define PLL23_PMS_720MHZ_S		1
#define PLL23_PMS_720MHZ_K		0

#define PLL23_PMS_710MHZ_P		2
#define PLL23_PMS_710MHZ_M		118
#define PLL23_PMS_710MHZ_S		1
#define PLL23_PMS_710MHZ_K		0

#define PLL23_PMS_700MHZ_P		3
#define PLL23_PMS_700MHZ_M		175
#define PLL23_PMS_700MHZ_S		1
#define PLL23_PMS_700MHZ_K		0

#if 0
#define PLL23_PMS_666MHZ_P		4
#define PLL23_PMS_666MHZ_M		222
#define PLL23_PMS_666MHZ_S		1
#define PLL23_PMS_666MHZ_K		0
#else
#define PLL23_PMS_666MHZ_P		2
#define PLL23_PMS_666MHZ_M		111
#define PLL23_PMS_666MHZ_S		1
#define PLL23_PMS_666MHZ_K		0
#endif

#define PLL23_PMS_614MHZ_P		2
#define PLL23_PMS_614MHZ_M		102
#define PLL23_PMS_614MHZ_S		1
#define PLL23_PMS_614MHZ_K		26214

#define PLL23_PMS_600MHZ_P		2
#define PLL23_PMS_600MHZ_M		100
#define PLL23_PMS_600MHZ_S		1
#define PLL23_PMS_600MHZ_K		0

#define PLL23_PMS_550MHZ_P		3
#define PLL23_PMS_550MHZ_M		275
#define PLL23_PMS_550MHZ_S		2
#define PLL23_PMS_550MHZ_K		0

#define PLL23_PMS_500MHZ_P		3
#define PLL23_PMS_500MHZ_M		250
#define PLL23_PMS_500MHZ_S		2
#define PLL23_PMS_500MHZ_K		0

#define PLL23_PMS_400MHZ_P		3
#define PLL23_PMS_400MHZ_M		200
#define PLL23_PMS_400MHZ_S		2
#define PLL23_PMS_400MHZ_K		0

#define PLL23_PMS_360MHZ_P		3
#define PLL23_PMS_360MHZ_M		360
#define PLL23_PMS_360MHZ_S		3
#define PLL23_PMS_360MHZ_K		0

#define PLL23_PMS_333MHZ_P		4
#define PLL23_PMS_333MHZ_M		222
#define PLL23_PMS_333MHZ_S		2
#define PLL23_PMS_333MHZ_K		0

#define PLL23_PMS_300MHZ_P		3
#define PLL23_PMS_300MHZ_M		300
#define PLL23_PMS_300MHZ_S		3
#define PLL23_PMS_300MHZ_K		0

#define PLL23_PMS_295MHZ_P		6
#define PLL23_PMS_295MHZ_M		295
#define PLL23_PMS_295MHZ_S		2
#define PLL23_PMS_295MHZ_K		0

#define PLL23_PMS_266MHZ_P		3
#define PLL23_PMS_266MHZ_M		266
#define PLL23_PMS_266MHZ_S		3
#define PLL23_PMS_266MHZ_K		0

#define PLL23_PMS_250MHZ_P		3
#define PLL23_PMS_250MHZ_M		250
#define PLL23_PMS_250MHZ_S		3
#define PLL23_PMS_250MHZ_K		0

#define PLL23_PMS_240MHZ_P		3
#define PLL23_PMS_240MHZ_M		240
#define PLL23_PMS_240MHZ_S		3
#define PLL23_PMS_240MHZ_K		0

#define PLL23_PMS_200MHZ_P		3
#define PLL23_PMS_200MHZ_M		200
#define PLL23_PMS_200MHZ_S		3
#define PLL23_PMS_200MHZ_K		0

#define PLL23_PMS_125MHZ_P		3
#define PLL23_PMS_125MHZ_M		250
#define PLL23_PMS_125MHZ_S		4
#define PLL23_PMS_125MHZ_K		0

#define PLL23_PMS_100MHZ_P		3
#define PLL23_PMS_100MHZ_M		200
#define PLL23_PMS_100MHZ_S		4
#define PLL23_PMS_100MHZ_K		0

#define PLL23_PMS_50MHZ_P		3
#define PLL23_PMS_50MHZ_M		200
#define PLL23_PMS_50MHZ_S		5
#define PLL23_PMS_50MHZ_K		0

/* Clock Function Define */
 int clock_initialize(void);
void clock_information(void);
#endif
