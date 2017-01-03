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
#include <clock.h>

/* Global Varaiables */
static unsigned int g_osc_khz;

/* External Varaiables Define */
extern struct NX_SecondBootInfo * const pSBI;

/* External Function */
extern unsigned int getquotient(int dividend, int divisor);
extern unsigned int getremainder(int dividend, int divisor);

void __pllchange(volatile unsigned int data,
	volatile unsigned int* addr,unsigned int delaycount);

void clkpwr_set_oscfreq(unsigned int freq_khz)
{
	g_osc_khz = freq_khz;
}

unsigned int clkpwr_get_pllfreq(unsigned int pll_num)
{
	unsigned int reg_value, reg_value1, nP, nM, nS, nK;
	unsigned int temp = 0;

	reg_value  = mmio_read_32(&pReg_ClkPwr->PLLSETREG[pll_num]);
	reg_value1 = mmio_read_32(&pReg_ClkPwr->PLLSETREG_SSCG[pll_num]);
	nP = (reg_value  >> PLL_P) & 0x3F;
	nM = (reg_value  >> PLL_M) & 0x3FF;
	nS = (reg_value  >> PLL_S) & 0xFF;
	nK = (reg_value1 >> PLL_K) & 0xFFFF;

	if ((pll_num > 1) && nK)
		temp = (getquotient((getquotient((nK * 1000), 65536) * g_osc_khz),nP)>>nS);

	temp = ((getquotient((nM * g_osc_khz),nP)>>nS)*1000) + temp;

	return temp;
}

unsigned int clkpwr_get_srcpll(unsigned int divider)
{
	return (mmio_read_32(&pReg_ClkPwr->DVOREG[divider]) & 0x7);
}

unsigned int clkpwr_get_divide_value(unsigned int divider)
{
	unsigned int reg_value
		= mmio_read_32(&pReg_ClkPwr->DVOREG[divider]);

	return ((((reg_value >> DVO3) & 0x3F) + 1) << 24) |
	       ((((reg_value >> DVO2) & 0x3F) + 1) << 16) |
	       ((((reg_value >> DVO1) & 0x3F) + 1) <<  8) |
	       ((((reg_value >> DVO0) & 0x3F) + 1) <<  0);
}

#if defined(MEM_TYPE_LPDDR23)
void clock_set_mem_pll(int ca_after)
{
#if 1
	unsigned int PLL_PMS, PLL23_K;
	volatile unsigned int delay = 0x100000;

	if (ca_after) {
#if (CFG_NSIH_EN == 0)
		GET_PLL23(800, PLL_PMS);
		GET_PLL23K(800, PLL23_K);
#else
		PLL_PMS = pSBI->PLL[3];
		PLL23_K = pSBI->PLLSPREAD[1];
#endif
	} else {
		GET_PLL23(50, PLL_PMS);
		GET_PLL23K(50, PLL23_K);
	}

	mmio_write_32(&pReg_ClkPwr->PLLSETREG[3], ((1UL << 28) | PLL_PMS));
	mmio_write_32(&pReg_ClkPwr->PLLSETREG_SSCG[3], PLL23_K);
#endif

	__pllchange(mmio_read_32(&pReg_ClkPwr->PWRMODE) | (0x1 << 15),
			&pReg_ClkPwr->PWRMODE, 0x20000);			// 533 ==> 800MHz:#0xED00, 1.2G:#0x17000, 1.6G:#0x1E000

	while (((mmio_read_32(&pReg_ClkPwr->PWRMODE) & (0x1 << 15))) && delay--); // it's never checked here, just for insure

	if (mmio_read_32(&pReg_ClkPwr->PWRMODE) & (0x1 << 15)) {
//		printf("pll does not locked\r\nsystem halt!\r\r\n");			// in this point, it's not initialized uart debug port yet
		while (1);							// system reset code need.
	}
}
#endif // #if defined(MEM_TYPE_LPDDR23)

void clock_initialize(void)
{
#if (CFG_NSIH_EN == 0)
	unsigned int PLL_PMS;
#endif
	volatile unsigned int delay;

	clkpwr_set_oscfreq(OSC_KHZ);

	/* PLL Change */
#if (CFG_NSIH_EN == 0)
	/* PLL0 for Memory */
	GET_PLL01(500, PLL_PMS);
	pSBI->PLL[0] = unsigned int)PLL_PMS;

	/* PLL1 for CPU */
	GET_PLL01(800, PLL_PMS);
	pSBI->PLL[1] = unsigned int)PLL_PMS;

	/* PLL2 for BCLK, 3DCLK */
	GET_PLL23(614, PLL_PMS);
	pSBI->PLL[2] = (unsigned int)PLL_PMS;
	pSBI->PLLSPREAD[2] = (PLL23_PMS_614MHZ_K << 16 | (2 << 0));

	/* PLL3 for others (Audio) */
	GET_PLL23(800, PLL_PMS);
	pSBI->PLL[3] =  (unsigned int)PLL_PMS;
//	pSBI->PLLSPREAD[3] =  (PLL23_PMS_614MHZ_K<<16 | (2 << 0));
	pSBI->PLLSPREAD[3] =  (2 << 0));

	/* CPUDVOREG */
	pSBI->DVO[0] =  ((NX_CLKSRC_PLL_1 << CLKSRC) |				// PLL Select
			((1 - 1) << DVO0) |					// FCLK ==> CPU Group 0
	                ((4 - 1) << DVO1)));					// HCLK ==> CPU bus (max 250MHz)

	/* CPUDVOREG */
	pSBI->DVO[7] =  ((NX_CLKSRC_PLL_1 << CLKSRC) |				// PLL Select
	                ((1 - 1) << DVO0)|					// FCLK ==> CPU Group1
	                ((4 - 1) << DVO1));					// HCLK ==> CPU bus (max 250MHz)

	/* BUSDVOREG */
	pSBI->DVO[1] = ((NX_CLKSRC_PLL_3 << CLKSRC) |				// PLL Select
	                ((2 - 1) << DVO0)|					// BCLK ==> System bus (max 333MHz)
	                ((2 - 1) << DVO1)));                 			// PCLK ==> Peripheral bus (max 166MHz)

	/* MEMDVOREG */
	pSBI->DVO[2] = 	((NX_CLKSRC_PLL_3 << CLKSRC) |				// PLL Select
	                ((1 - 1) << DVO0) |					// MDCLK ==> Memory DLL (max 800MHz)
	                ((1 - 1) << DVO1) |					// MCLK  ==> Memory DDR (max 800MHz)
	                ((2 - 1) << DVO2) |					// MBCLK ==> MCU bus (max 400MHz)
	                ((2 - 1) << DVO3)));					// MPCLK ==> MCU Peripheral bus (max 200MHz)

	pSBI->DVO[3] =  ((NX_CLKSRC_PLL_3 << CLKSRC) |				// GRP3DVOREG
	                ((2 - 1) << DVO0)|                  			// GR3DBCLK ==> GPU bus & core (max 333MHz)
	                ((2 - 1) << DVO1)));                 			// GR3DPCLK ==> not used

	pSBI->DVO[4] = 	((NX_CLKSRC_PLL_3 << CLKSRC) |				// MPEGDVOREG
	                ((2 - 1) << DVO0) |					// MPEGBCLK ==> MPEG bus & core (max 300MHz)
	                ((2 - 1) << DVO1)));					// MPEGPCLK ==> MPEG control if (max 150MHz)

	pSBI->DVO[5] = 	((NX_CLKSRC_PLL_3 << CLKSRC) |				// DISPLAY BUS
	                ((2 - 1) << DVO0) |					// DISPLAY BCLK ==> DISPLAY bus (max 300MHz)
	                ((2 - 1) << DVO1)));					// DISPLAY PCLK ==> DISPLAY control if (max 150MHz)

	pSBI->DVO[6] = 	((NX_CLKSRC_PLL_3 << CLKSRC) |				// HDMI (max 100MHz)
	                ((8 - 1) << DVO0)));					// HDMI PCLK ==> HDMI Control if

	pSBI->DVO[8] = 	((NX_CLKSRC_PLL_3 << CLKSRC) |				// FAST BUS (max 400MHz)
	                ((2 - 1) << DVO0) |					// FAST BUS BCLK ==> FAST bus (max 400MHz)
	                ((2 - 1) << DVO1)));					// FAST BUS PCLK ==> FAST BUS control if (max 200MHz)
#endif

	mmio_write_32(&pReg_ClkPwr->PLLSETREG[0], (pSBI->PLL[0] | (1UL << 28)));
	mmio_write_32(&pReg_ClkPwr->PLLSETREG[1], (pSBI->PLL[1] | (1UL << 28)));
	mmio_write_32(&pReg_ClkPwr->PLLSETREG[3], (pSBI->PLL[3] | (1UL << 28)));

	mmio_write_32(&pReg_ClkPwr->PLLSETREG_SSCG[2], pSBI->PLLSPREAD[0]);
	mmio_write_32(&pReg_ClkPwr->PLLSETREG_SSCG[3], pSBI->PLLSPREAD[1]);

	mmio_write_32(&pReg_ClkPwr->DVOREG[0], pSBI->DVO[0]);
	mmio_write_32(&pReg_ClkPwr->DVOREG[1], pSBI->DVO[1]);
	mmio_write_32(&pReg_ClkPwr->DVOREG[2], pSBI->DVO[2]);
	mmio_write_32(&pReg_ClkPwr->DVOREG[3], pSBI->DVO[3]);
	mmio_write_32(&pReg_ClkPwr->DVOREG[4], pSBI->DVO[4]);
	mmio_write_32(&pReg_ClkPwr->DVOREG[5], pSBI->DVO[5]);
	mmio_write_32(&pReg_ClkPwr->DVOREG[6], pSBI->DVO[6]);
	mmio_write_32(&pReg_ClkPwr->DVOREG[7], pSBI->DVO[7]);
	mmio_write_32(&pReg_ClkPwr->DVOREG[8], pSBI->DVO[8]);

#if 1
	delay = 0x100000;

//	pSBI->PLL[2] &= ~(1UL<<28);

	__pllchange(mmio_read_32(&pReg_ClkPwr->PWRMODE) | (0x1 << 15),
			&pReg_ClkPwr->PWRMODE, 0x20000);			// 533 ==> 800MHz:#0xED00, 1.2G:#0x17000, 1.6G:#0x1E000

	while (((mmio_read_32(&pReg_ClkPwr->PWRMODE) & (0x1 << 15))) && delay--); // it's never checked here, just for insure

	if (mmio_read_32(&pReg_ClkPwr->PWRMODE) & (0x1 << 15)) {
//		printf("pll does not locked\r\nsystem halt!\r\r\n");			// in this point, it's not initialized uart debug port yet
		while (1);							// system reset code need.
	}
#endif

	delay = 0x100000;

	mmio_write_32(&pReg_ClkPwr->PLLSETREG[2], (pSBI->PLL[2] | (1UL << 28)));

	__pllchange(mmio_read_32(&pReg_ClkPwr->PWRMODE) | (0x1 << 15),
			&pReg_ClkPwr->PWRMODE, 0x20000);			// 533 ==> 800MHz:#0xED00, 1.2G:#0x17000, 1.6G:#0x1E000

	while (((mmio_read_32(&pReg_ClkPwr->PWRMODE) & (0x1 << 15))) && delay--); // it's never checked here, just for insure

	if (mmio_read_32(&pReg_ClkPwr->PWRMODE) & (0x1 << 15)) {
//		printf("pll does not locked\r\nsystem halt!\r\r\n");			// in this point, it's not initialized uart debug port yet
		while (1);							// system reset code need.
	}
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

