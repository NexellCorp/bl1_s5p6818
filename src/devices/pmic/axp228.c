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

#if defined (AXP228_PMIC_ENABLE)
#include <i2c_gpio.h>
#include <axp228.h>

unsigned char axp228_get_dcdc_step(int want_vol, int step, int min, int max)
{
	U32 vol_step = 0;

	if (want_vol < min) {
		want_vol = min;
	} else if (want_vol > max) {
		want_vol = max;
	}

	vol_step = (want_vol - min + step - 1) / step;

	return (unsigned char)(vol_step & 0xFF);
}

/* Do not use it yet. The reason is that code is not generally available. */
#if 1

int axp228_write(char addr, char *pdata, int size)
{
	return i2c_gpio_write(I2C_ADDR_AXP228, addr, pdata, size);
}

int axp228_read(char addr, char *pdata, int size)
{
	return i2c_gpio_read(I2C_ADDR_AXP228, addr, pdata, size);
}


/* Do not use it yet. The reason is that code is not generally available. */
#if 0
inline void axp228(void)
{
	char pdata[4];

	i2c_gpio_init(AXP_I2C_GPIO_GRP, AXP_I2C_SCL, AXP_I2C_SDA,
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
#endif

#endif

#endif // #if defined (AXP228_PMIC_ENABLE)