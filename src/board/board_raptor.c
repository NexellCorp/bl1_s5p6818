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
#include <raptor.h>
#include <i2c_gpio.h>
#include <nxe2000.h>

/*
 * Raptor board - revision check. (H/W: GPIOE 4,5,6)
 * GPIOE4 (Least Bit), GPIOE6(Most Bit)
 */
unsigned int raptor_check_hw_revision(void)
{
	volatile unsigned int *reg = 0;
	unsigned int val = 0;

	/*
	 * Check to GPIOE PORT
	 * Read to GPIOPAD Status Register
	 */
	reg = (volatile unsigned int*)(0xC001E000 + 0x18);
	val = (*reg & (0x7 << 4)) >> 4;

        return val;
}

#if defined(RAPTOR_PMIC)
/************************************************
  * Raptor Board (PMIC: NXE2000)  - Reference 2016.04.05
  * ARM		: 1.25V
  * CORE	: 1.2V
  * DDR		: 1.5V
  * DDR_IO	: 1.5V
  ************************************************/
void pmic_raptor(void)
{
	int  board_rev = 0;
	char data[4];

	/* Raptor Board Revision Check */
	board_rev = raptor_check_hw_revision();
	if (board_rev >= 0x2) {
		/* I2C init for CORE & NXE2000 power. */
		/* GPIOC, SCL:15(ALT1), SDA:16(ALT1) */
		I2C_INIT(2, 15, 16, 1, 1);
	} else {
		/* I2C init for CORE & NXE2000 power. */
		/* GPIOD, SCL:6, SDA:7 */
		I2C_INIT(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA,
				NXE2000_I2C_SCL_ALT, NXE2000_I2C_SDA_ALT);
	}

	/* ARM voltage change */	// 1.25V
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC1_VOL);
	nxe2000_write(NXE2000_REG_DC1VOL, data, 1);
	/* Core voltage change */	// 1.2V
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC2_VOL);
	nxe2000_write(NXE2000_REG_DC2VOL, data, 1);
	/* DDR3 voltage change */	// 1.5V
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC4_VOL);
	nxe2000_write(NXE2000_REG_DC4VOL, data, 1);
	/* DDR3 IO voltage change */	// 1.5V
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC5_VOL);
	nxe2000_write(NXE2000_REG_DC5VOL, data, 1);

	/*
	 * Raptor Board Rev0X
	 * VDD33_USB0 : VCC3P3_SYS --> VCC2P8_LDO7 (Sleep Mode Power OFF)
	 * AVDD18_PLL : VCC1P8_SYS --> VCC1P8_LDO4 (Sleep Mode Power OFF)
	 */
	/* LDO7 IO voltage change */ // 3.3V
	data[0] = nxe2000_get_ldo7_step(3300000);
	nxe2000_write(NXE2000_REG_LDO7VOL, data, 1);
}
#endif
