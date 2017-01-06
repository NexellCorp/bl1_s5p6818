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
#include <clkpwr.h>

/* Global Varaiables */
struct s5p6818_clkpwr_reg* g_clkpwr_reg;
static unsigned int g_osc_khz;

int clkpwr_get_baseaddr(void)
{
	return PHY_BASEADDR_CLKPWR_MODULE;
}

void clkpwr_set_oscfreq(unsigned int freq_khz)
{
	g_osc_khz = freq_khz;
}

int clkpwr_get_pllfreq(unsigned int pll_num)
{
	unsigned int reg_value, reg_value1, nP, nM, nS, nK;
	unsigned int temp = 0;

	reg_value  = mmio_read_32(&g_clkpwr_reg->pllset[pll_num]);
	reg_value1 = mmio_read_32(&g_clkpwr_reg->pllset_sscg[pll_num]);
	nP = (reg_value  >> PLL_P_BITPOS) & 0x3F;
	nM = (reg_value  >> PLL_M_BITPOS) & 0x3FF;
	nS = (reg_value  >> PLL_S_BITPOS) & 0xFF;
	nK = (reg_value1 >> PLL_K_BITPOS) & 0xFFFF;

	if ((pll_num > 1) && nK)
		temp = (getquotient((getquotient((nK * 1000), 65536) * g_osc_khz),nP) >> nS);

	temp = ((getquotient((nM * g_osc_khz),nP) >> nS) * 1000) + temp;

	return temp;
}

int clkpwr_get_srcpll(unsigned int divider)
{
	return (mmio_read_32(&g_clkpwr_reg->dvo[divider]) & 0x7);
}

int clkpwr_get_divide_value(unsigned int divider)
{
	unsigned int reg_value
		= mmio_read_32(&g_clkpwr_reg->dvo[divider]);

	return ((((reg_value >> DVO3_BITPOS) & 0x3F) + 1) << 24) |
	       ((((reg_value >> DVO2_BITPOS) & 0x3F) + 1) << 16) |
	       ((((reg_value >> DVO1_BITPOS) & 0x3F) + 1) <<  8) |
	       ((((reg_value >> DVO0_BITPOS) & 0x3F) + 1) <<  0);
}

#if defined(MEM_TYPE_LPDDR23)
int clock_set_mem_pll(int ca_after)
{
	unsigned int PLL_PMS, PLL23_K;
	volatile unsigned int delay = 0x100000;

	if (ca_after) {
		GET_PLL23(800, PLL_PMS);
		GET_PLL23K(800, PLL23_K);
	} else {
		GET_PLL23(50, PLL_PMS);
		GET_PLL23K(50, PLL23_K);
	}

	mmio_write_32(&g_clkpwr_reg->pllset[3], ((1UL << 28) | PLL_PMS));
	mmio_write_32(&g_clkpwr_reg->pllset_sscg[3], PLL23_K);

	return clock_is_stable();
}
#endif // #if defined(MEM_TYPE_LPDDR23)
