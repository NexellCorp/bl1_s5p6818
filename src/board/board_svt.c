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
#include <svt.h>
#include <i2c_gpio.h>
#include <nxe2000.h>
#include <mp8845.h>

#if defined(SVT_PMIC)
/************************************************
  * SVT Board (PMIC: NXE2000, MP8845)  - Reference 2016.04.05
  * ARM		: 1.25V
  * CORE	: 1.2V
  * DDR		: 1.5V
  * DDR_IO	: 1.5V
  ************************************************/
void pmic_svt(void)
{
	char data[4];

	I2C_INIT(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA,
			NXE2000_I2C_SCL_ALT, NXE2000_I2C_SDA_ALT);

	/* PFM -> PWM mode */
	mp8845_read(MP8845C_REG_SYSCNTL1, data, 1);
	data[0] |= 1 << 0;
	mp8845_write(MP8845C_REG_SYSCNTL1, data, 1);

	/* Set the Core Voltage */
#if 1
	mp8845_read(MP8845C_REG_SYSCNTL2, data, 1);
	data[0] |= 1 << 5;
	mp8845_write(MP8845C_REG_SYSCNTL2, data, 1);

//	data[0] = 90 | 1<<7;   // 90: 1.2V
//	data[0] = 80 | 1<<7;   // 80: 1.135V
	data[0] = 75 | 1 << 7; // 75: 1.1V
	mp8845_write(MP8845C_REG_VSEL, data, 1);
#else
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC2_VOL);
	nxe2000_write(NXE2000_REG_DC2VOL, data, 1); // core - second power
#endif

	/* Set the Arm Voltage */
#if (ARM_VOLTAGE_CONTROL_SKIP == 0)
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC1_VOL);
	nxe2000_write(NXE2000_REG_DC1VOL, data, 1);
#endif

	/* Set the DDR Voltage */
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC4_VOL);
	nxe2000_write(NXE2000_REG_DC4VOL, data, 1);

	/* Set the DDR_IO Voltage */
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC5_VOL);
	nxe2000_write(NXE2000_REG_DC5VOL, data, 1);
}
#endif
