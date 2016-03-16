/*
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved
 *      Nexell Co. Proprietary & Confidential
 *
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 *FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      Module          : PMIC NXE2000
 *      File            : pmic_nxe2000
 *      Description     :
 *      Author          : Hans
 *      History         : 2015.05.21 Hans crate
 */

#ifndef __PMIC_NXE2000_H__
#define __PMIC_NXE2000_H__

#define NXE2000_DEF_DDCx_VOL_MIN 600000  /* UINT = 1uV, 0.6V */
#define NXE2000_DEF_DDCx_VOL_MAX 3500000 /* UINT = 1uV, 3.5V */
#define NXE2000_DEF_DDCx_VOL_STEP 12500  /* UINT = 1uV, 12.5mV */

#define NXE2000_DEF_DDC1_VOL                                                   \
	1100000 /* VAL(uV) = 0: 0.60 ~ 3.5V, Step 12.5 mV, default(OTP) = 1.3V \
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

#define NXE2000_REG_DC1VOL 0x36
#define NXE2000_REG_DC2VOL 0x37
#define NXE2000_REG_DC3VOL 0x38
#define NXE2000_REG_DC4VOL 0x39
#define NXE2000_REG_DC5VOL 0x3A

#define I2C_ADDR_NXE2000 (0x64 >> 1) // SVT & ASB

#endif // ifdef __PMIC_NXE2000_H__
