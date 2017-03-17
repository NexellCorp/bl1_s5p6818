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
#if defined(PMIC_ON)
#include <i2c_gpio.h>
#include <axp228.h>

#define AUTO_VOLTAGE_CONTROL			1
#define ARM_VOLTAGE_CONTROL_SKIP		0

#define AXP_I2C_GPIO_GRP 			3 				// GPIOD
#define AXP_I2C_SCL 				20				// SCL : GPIOD 20
#define AXP_I2C_SDA 				16				// SDA : GPIOD 16
#define AXP_I2C_SCL_ALT				0				// SCL : ALT0
#define AXP_I2C_SDA_ALT				0				// SDA: ALT0

void pmic_board_init(void)
{
	char pdata[4];

	I2C_INIT(AXP_I2C_GPIO_GRP, AXP_I2C_SCL, AXP_I2C_SDA,
			AXP_I2C_SCL_ALT, AXP_I2C_SDA_ALT);

	axp228_read(0x80, pdata, 1);
	pdata[0] = (pdata[0] & 0x1F) | DCDC_SYS | DCDC_DDR;
	axp228_write(0x80, pdata, 1);

	/* set the bridge DCDC2 and DCDC3 */
	axp228_read(0x37, pdata, 1);
	pdata[0] |= 0x10;
	axp228_write(0x37, pdata, 1);

	/* change the arm voltage */
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
	pdata[0] = axp228_get_dcdc_step(
	    AXP228_DEF_DDC2_VOL, AXP228_DEF_DDC234_VOL_STEP,
	    AXP228_DEF_DDC234_VOL_MIN, AXP228_DEF_DDC24_VOL_MAX);
	axp228_write(AXP228_REG_DC2VOL, pdata, 1);
#endif

#if 0
	/* set the voltage of DCDC4. */
	pdata[0] = axp228_get_dcdc_step(AXP228_DEF_DDC4_VOL, AXP228_DEF_DDC234_VOL_STEP,
		AXP228_DEF_DDC234_VOL_MIN, AXP228_DEF_DDC24_VOL_MAX);
	axp228_write(AXP228_REG_DC4VOL, pdata, 1);

	/* set the voltage of DCDC5. */
	pdata[0] = axp228_get_dcdc_step(AXP228_DEF_DDC5_VOL, AXP228_DEF_DDC5_VOL_STEP,
		AXP228_DEF_DDC5_VOL_MIN, AXP228_DEF_DDC5_VOL_MAX);
	axp228_write(AXP228_REG_DC5VOL, pdata, 1);
#endif
	return;
}
#endif // #if defined(PMIC_ON)
