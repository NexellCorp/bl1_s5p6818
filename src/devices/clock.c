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
#include <clock.h>
#include <clkpwr.h>

/* External Global Variables */
extern struct s5p6818_clkpwr_reg* g_clkpwr_reg;

/* External Function */
extern unsigned int getquotient(int dividend, int divisor);
extern unsigned int getremainder(int dividend, int divisor);

extern void pll_change(volatile unsigned int data,
		volatile unsigned int* addr,unsigned int delay);

static int clock_is_stable(void)
{
	int chg_pll_mask = (0x1 << 15);
	int delay = 0x100000;
	int ret = 0;

	pll_change(mmio_read_32(&g_clkpwr_reg->pwr_mode) | chg_pll_mask,
			&g_clkpwr_reg->pwr_mode, (delay * 2));			// 533 ==> 800MHz:#0xED00, 1.2G:#0x17000, 1.6G:#0x1E000

	while (((mmio_read_32(&g_clkpwr_reg->pwr_mode) & chg_pll_mask)) && delay--); // it's never checked here, just for insure

	if (mmio_read_32(&g_clkpwr_reg->pwr_mode) & (0x1 << 15)) {
		ERROR("PLL does not Locked\r\nSystem halt!\r\r\n");		// in this point, it's not initialized uart debug port yet
		while (1);							// system reset code need.
	}
}

int clock_initialize(void)
{
	unsigned int PLL[4], PLL_SPREAD_K[4];
	unsigned int PLL_PMS, PLL_SPREAD;
	unsigned int delay;

	g_clkpwr_reg = (struct s5p6818_clkpwr_reg*)clkpwr_get_baseaddr();

	clkpwr_set_oscfreq(OSC_KHZ);

	/* step 01. calcurate the phase locked loop (0/1/2/3) frequency */
	GET_PLL01_FREQ(CONFIG_S5P_PLL0_FREQ, PLL_PMS);
	PLL[0] = (PLL_PMS | (1UL << 28));

	GET_PLL01_FREQ(CONFIG_S5P_PLL1_FREQ, PLL_PMS);
	PLL[1] = (PLL_PMS | (1UL << 28));

	GET_PLL23_FREQ(CONFIG_S5P_PLL2_FREQ, PLL_PMS);
	PLL[2] = (PLL_PMS | (1UL << 28));
	GET_PLL23K_FREQ(CONFIG_S5P_PLL2_FREQ, PLL_SPREAD);
	PLL_SPREAD_K[2] = (PLL_SPREAD);

	PLL_PMS = CONFIG_S5P_PLL2_FREQ;
	if (PLL_PMS == 614)
		PLL[2] |= (1 << 30);

	GET_PLL23_FREQ(CONFIG_S5P_PLL3_FREQ, PLL_PMS);
	PLL[3] =  (PLL_PMS | (1UL << 28));
	GET_PLL23K_FREQ(CONFIG_S5P_PLL3_FREQ, PLL_SPREAD);
	PLL_SPREAD_K[3] =  (PLL_SPREAD);

	PLL_PMS = CONFIG_S5P_PLL3_FREQ;
	if (PLL_PMS == 614)
		PLL[3] |= (1 << 30);

	/* step 02. set the phase locked loop (0/1/2/3) frequency */
	mmio_write_32(&g_clkpwr_reg->pllset[0], PLL[0]);
	mmio_write_32(&g_clkpwr_reg->pllset[1], PLL[1]);
	mmio_write_32(&g_clkpwr_reg->pllset[2], PLL[2]);
	mmio_write_32(&g_clkpwr_reg->pllset[3], PLL[3]);

	mmio_write_32(&g_clkpwr_reg->pllset_sscg[2], PLL_SPREAD_K[2]);
	mmio_write_32(&g_clkpwr_reg->pllset_sscg[3], PLL_SPREAD_K[3]);

	/* step 03. set the clock divider register */
	mmio_write_32(&g_clkpwr_reg->dvo[0], CONFIG_S5P_PLLx_DVO0);
	mmio_write_32(&g_clkpwr_reg->dvo[1], CONFIG_S5P_PLLx_DVO1);
	mmio_write_32(&g_clkpwr_reg->dvo[2], CONFIG_S5P_PLLx_DVO2);
	mmio_write_32(&g_clkpwr_reg->dvo[3], CONFIG_S5P_PLLx_DVO3);
	mmio_write_32(&g_clkpwr_reg->dvo[4], CONFIG_S5P_PLLx_DVO4);
	mmio_write_32(&g_clkpwr_reg->dvo[5], CONFIG_S5P_PLLx_DVO5);
	mmio_write_32(&g_clkpwr_reg->dvo[6], CONFIG_S5P_PLLx_DVO6);
#if 0
        /*
         * Cluster 0 Clock
         * Use the same clocks for Cluster 0 and 1.
         * Modify the policy if you change it later.
         */
	mmio_write_32(&g_clkpwr_reg->dvo[7], CONFIG_S5P_PLLx_DVO7);
#else
	mmio_write_32(&g_clkpwr_reg->dvo[7], CONFIG_S5P_PLLx_DVO0);
#endif
	mmio_write_32(&g_clkpwr_reg->dvo[8], CONFIG_S5P_PLLx_DVO8);

	return clock_is_stable();
}

void clock_information(void)
{
#if 0
    SYSMSG(" PLL0: %d   PLL1: %d   PLL2: %d   PLL3: %d\r\n\r\n",
            clkpwr_get_pllfreq(0),
            clkpwr_get_pllfreq(1),
            clkpwr_get_pllfreq(2),
            clkpwr_get_pllfreq(3));

    SYSMSG(" Divider0 PLL: %d CPU Group0:%d   CPU BUS:%d\r\n",
            clkpwr_get_srcpll(0),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(0)), ((clkpwr_get_divide_value(0)>> 0)&0x3F)),
            getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(0))
                    ,((clkpwr_get_divide_value(0)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(0)>> 8)&0x3F)));

    SYSMSG(" Divider7 PLL: %d CPU Group1:%d   CPU BUS:%d\r\n",
            clkpwr_get_srcpll(7),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(7)), ((clkpwr_get_divide_value(7)>> 0)&0x3F)),
            getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(7))
                    ,((clkpwr_get_divide_value(7)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(7)>> 8)&0x3F)));

    SYSMSG(" Divider8 PLL: %d FAST BUS BCLK:%d   PCLK:%d\r\n",
            clkpwr_get_srcpll(8),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(8)),((clkpwr_get_divide_value(8)>> 0)&0x3F)),
            getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(8))
                    ,((clkpwr_get_divide_value(8)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(8)>> 8)&0x3F)));

    SYSMSG(" Divider1 PLL: %d BCLK:%d   PCLK:%d\r\n",
            clkpwr_get_srcpll(1),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(1)),((clkpwr_get_divide_value(1)>> 0)&0x3F)),
            getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(1))
                    ,((clkpwr_get_divide_value(1)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(1)>> 8)&0x3F)));

    SYSMSG(" Divider5 PLL: %d DISPLAY BCLK:%d   PCLK:%d\r\n",
            clkpwr_get_srcpll(5),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(5)),((clkpwr_get_divide_value(5)>> 0)&0x3F)),
            getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(5))
                    ,((clkpwr_get_divide_value(5)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(5)>> 8)&0x3F)));

    SYSMSG(" Divider2 PLL: %d MDCLK:%d   MCLK:%d   \r\n\t\t MBCLK:%d   MPCLK:%d\r\n",
            clkpwr_get_srcpll(2),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(2))
                    ,((clkpwr_get_divide_value(2)>> 0)&0x3F)),
            getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(2))
                    ,((clkpwr_get_divide_value(2)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(2)>> 8)&0x3F)),
            getquotient(getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(2))
                    ,((clkpwr_get_divide_value(2)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(2)>> 8)&0x3F))
                    ,((clkpwr_get_divide_value(2)>>16)&0x3F)),
            getquotient(getquotient(getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(2))
                    ,((clkpwr_get_divide_value(2)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(2)>> 8)&0x3F))
                    ,((clkpwr_get_divide_value(2)>>16)&0x3F))
                    ,((clkpwr_get_divide_value(2)>>24)&0x3F)));

    SYSMSG(" Divider3 PLL: %d G3D BCLK:%d\r\n",
            clkpwr_get_srcpll(3),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(3)),((clkpwr_get_divide_value(3)>> 0)&0x3F)));

    SYSMSG(" Divider4 PLL: %d MPEG BCLK:%d   MPEG PCLK:%d\r\n",
            clkpwr_get_srcpll(4),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(4)),((clkpwr_get_divide_value(4)>> 0)&0x3F)),
            getquotient(getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(4))
                    ,((clkpwr_get_divide_value(4)>> 0)&0x3F))
                    ,((clkpwr_get_divide_value(4)>> 8)&0x3F)));

    SYSMSG(" Divider6 PLL: %d HDMI PCLK:%d\r\n",
            clkpwr_get_srcpll(6),
            getquotient(clkpwr_get_pllfreq(clkpwr_get_srcpll(6)),((clkpwr_get_divide_value(6)>> 0)&0x3F)));
#endif
}
