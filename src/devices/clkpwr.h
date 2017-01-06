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
#ifndef __CLKPWR_H__
#define __CLKPWR_H__

struct	s5p6818_clkpwr_reg
{
		volatile unsigned int clk_mode_reg0;				///< 0x000 : Clock Mode Register 0
		volatile unsigned int reserved_0;				///< 0x004
		volatile unsigned int pllset[4];				///< 0x008 ~ 0x014 : PLL Setting Register
		volatile unsigned int reserved_1[2];				///< 0x018 ~ 0x01C
		volatile unsigned int dvo[9];				///< 0x020 ~ 0x040 : Divider Setting Register
		volatile unsigned int reserved_2;				///< 0x044
		volatile unsigned int pllset_sscg[6];			///< 0x048 ~ 0x05C
		volatile unsigned int reserved_3[8];				///< 0x060 ~ 0x07C
		volatile unsigned char reserved_4[0x200-0x80];			// padding (0x80 ~ 0x1FF)
		volatile unsigned int gpio_wakeup_rise_enb;			///< 0x200 : GPIO Rising Edge Detect Enable Register
		volatile unsigned int gpio_wake_up_fall_enb;			///< 0x204 : GPIO Falling Edge Detect Enable Register
		volatile unsigned int gpio_rst_enb;				///< 0x208 : GPIO Reset Enable Register
		volatile unsigned int gpio_wakeup_enb;				///< 0x20C : GPIO Wakeup Source Enable
		volatile unsigned int gpio_int_enb;				///< 0x210 : Interrupt Enable Register
		volatile unsigned int gpio_int_pend;				///< 0x214 : Interrupt Pend Register
		volatile unsigned int reset_status;				///< 0x218 : Reset Status Register
		volatile unsigned int int_enable;				///< 0x21C : Interrupt Enable Register
		volatile unsigned int int_pend;					///< 0x220 : Interrupt Pend Register
		volatile unsigned int pwr_cont;					///< 0x224 : Power Control Register
		volatile unsigned int pwr_mode;					///< 0x228 : Power Mode Register
		volatile unsigned int reserved_5;				///< 0x22C : Reserved Region
		volatile unsigned int scratch[3];				///< 0x230 ~ 0x238	: Scratch Register
		volatile unsigned int sysrst_config;				///< 0x23C : System Reset Configuration Register.
		volatile unsigned char reserved_6[0x2A0-0x240];		// padding (0x240 ~ 0x29F)
		volatile unsigned int cpu_power_down_req;			///< 0x2A0 : CPU Power Down Request Register
		volatile unsigned int cpu_power_on_req;				///< 0x2A4 : CPU Power On Request Register
		volatile unsigned int cpu_reset_mode;				///< 0x2A8 : CPU Reset Mode Register
		volatile unsigned int cpu_warm_reset_req;			///< 0x2AC : CPU Warm Reset Request Register
		volatile unsigned int reserved_7;				///< 0x2B0
		volatile unsigned int cpu_status;				///< 0x2B4 : CPU Status Register
		volatile unsigned char reserved_8[0x400-0x2B8];		// padding (0x2B8 ~ 0x33F)
};

/* Function Define */
 int clock_set_mem_pll(int ca_after);

void clkpwr_set_oscfreq(unsigned int freq_khz);
 int clkpwr_get_pllfreq(unsigned int pll_num);
 int clkpwr_get_srcpll(unsigned int divider);
 int clkpwr_get_divide_value(unsigned int divider);

#endif	// #ifndef __CLKPWR_H__
