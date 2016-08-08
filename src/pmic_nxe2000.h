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

#ifndef __PMIC_NXE2000_H__
#define __PMIC_NXE2000_H__

#define NXE2000_DEF_DDCx_VOL_MIN	600000		/* UINT = 1uV, 0.6V */
#define NXE2000_DEF_DDCx_VOL_MAX	3500000		/* UINT = 1uV, 3.5V */
#define NXE2000_DEF_DDCx_VOL_STEP 	12500		/* UINT = 1uV, 12.5mV */

#define NXE2000_DEF_LDOx_VOL_MIN	900000		/* UINT = 1uV, 0.9V */
#define NXE2000_DEF_LDOx_VOL_MAX	3500000		/* UINT = 1uV, 3.5V */
#define NXE2000_DEF_LDOx_VOL_STEP	25000		/* UINT = 1uV, 25mV */

#define NXE2000_DEF_DDC1_VOL                                                   \
	1250000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.3V \
		   */
#define NXE2000_DEF_DDC2_VOL                                                   \
	1100000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.2V \
		   */
#define NXE2000_DEF_DDC3_VOL                                                   \
	3300000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 3.3V \
		   */
#define NXE2000_DEF_DDC4_VOL                                                   \
	1500000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) =      \
		   1.65V */
#define NXE2000_DEF_DDC5_VOL                                                   \
	1500000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) =      \
		   1.65V */

#define NXE2000_REG_DC1VOL	0x36
#define NXE2000_REG_DC2VOL	0x37
#define NXE2000_REG_DC3VOL	0x38
#define NXE2000_REG_DC4VOL	0x39
#define NXE2000_REG_DC5VOL	0x3A

#define NXE2000_REG_LDO7VOL	0x52

#define I2C_ADDR_NXE2000	(0x64 >> 1) // SVT & ASB

#endif // ifdef __PMIC_NXE2000_H__
