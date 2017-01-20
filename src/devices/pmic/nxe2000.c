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

#if defined(NXE2000_PMIC_ENABLE)
#include <i2c_gpio.h>
#include <nxe2000.h>

unsigned char nxe2000_get_ldo7_step(int want_vol)
{
    unsigned int vol_step = 0;
    unsigned int temp = 0;

    if (want_vol < NXE2000_DEF_LDOx_VOL_MIN)
        want_vol = NXE2000_DEF_LDOx_VOL_MIN;
    else if (want_vol > NXE2000_DEF_LDOx_VOL_MAX)
        want_vol = NXE2000_DEF_LDOx_VOL_MAX;
    temp = (want_vol - NXE2000_DEF_LDOx_VOL_MIN);

    vol_step    = (temp / NXE2000_DEF_LDOx_VOL_STEP);

    return    (unsigned char)(vol_step & 0x7F);
}

unsigned char nxe2000_get_dcdc_step(int want_vol)
{
	unsigned int vol_step = 0;

	if (want_vol < NXE2000_DEF_DDCx_VOL_MIN) {
		want_vol = NXE2000_DEF_DDCx_VOL_MIN;
	} else if (want_vol > NXE2000_DEF_DDCx_VOL_MAX) {
		want_vol = NXE2000_DEF_DDCx_VOL_MAX;
	}

	vol_step = (want_vol - NXE2000_DEF_DDCx_VOL_MIN +
		    NXE2000_DEF_DDCx_VOL_STEP - 1) /
		   NXE2000_DEF_DDCx_VOL_STEP;

	return (unsigned char)(vol_step & 0xFF);
}

int nxe2000_write(char addr, char *pdata, int size)
{
	return i2c_gpio_write(I2C_ADDR_NXE2000, addr, pdata, size);
}

int nxe2000_read(char addr, char *pdata, int size)
{
	return i2c_gpio_read(I2C_ADDR_NXE2000, addr, pdata, size);
}

/* Do not use it yet. The reason is that code is not generally available. */
#if 0

#if 0
inline void nxe2000(void)
{
	unsigned char data[4];

	/* nxe200 i2c initialize for core & arm & power. */
	i2c_gpio_init(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA,
			NXE2000_I2C_SCL_ALT, NXE2000_I2C_SDA_ALT);

	/* arm voltage change */// 1.25V
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC1_VOL);
	nxe2000_write(NXE2000_REG_DC1VOL, data, 1);

	/* core voltage change */ // 1.2V
	data[1] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC2_VOL);
	nxe2000_write(NXE2000_REG_DC2VOL, data, 1);

	/* ddr3 voltage change */ // 1.5V
	data[2] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC4_VOL);
	nxe2000_write(NXE2000_REG_DC4VOL, data, 1);
	/* ddr3 IO voltage change */ // 1.5V
	data[3] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC5_VOL);
	nxe2000_write(NXE2000_REG_DC5VOL, data, 1);

	return;
}
#else
inline void nxe2000(void)
{
	unsigned char data[4];

	/* nxe200 i2c initialize for core & arm & power. */
	i2c_gpio_init(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA,
			NXE2000_I2C_SCL_ALT, NXE2000_I2C_SDA_ALT);

#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
	/* arm voltage change */// 1.25V
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC1_VOL);
	nxe2000_write(NXE2000_REG_DC1VOL, data, 1);
#endif

#if defined(SVT_PMIC_INIT) && (MP8845_SYSCCNTL2_CORE == 0)
	/* core voltage change */ // 1.2V
	data[1] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC2_VOL);
	nxe2000_write(NXE2000_REG_DC2VOL, data, 1);
#endif

	/* ddr3 voltage change */ // 1.5V
	data[2] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC4_VOL);
	nxe2000_write(NXE2000_REG_DC4VOL, data, 1);
	/* ddr3 IO voltage change */ // 1.5V
	data[3] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC5_VOL);
	nxe2000_write(NXE2000_REG_DC5VOL, data, 1);

	return;
}
#endif
#endif

#endif // if defined(NXE2000_PMIC_ENABLE)