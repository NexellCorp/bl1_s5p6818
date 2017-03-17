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
#include <nxe2000.h>
#include <mp8845.h>

#define AUTO_VOLTAGE_CONTROL			1
#define ARM_VOLTAGE_CONTROL_SKIP		0

#define NXE2000_I2C_GPIO_GRP 			3				// GPIOD
#define NXE2000_I2C_SCL 			6				// SCL : GPIOD06
#define NXE2000_I2C_SDA 			7				// SDA: GPIOD07
#define NXE2000_I2C_SCL_ALT 			0				// SCL: ALT0
#define NXE2000_I2C_SDA_ALT			0				// SDA: ALT 0

#define MP8845_I2C_ARM_GPIO_GRP 		3				// GPIOD
#define MP8845_I2C_ARM_SCL 			2				// SCL : GPIOD02
#define MP8845_I2C_ARM_SDA 			3				// SDA: GPIOD03
#define MP8845_I2C_ARM_SCL_ALT			0				// SCL: ALT0
#define MP8845_I2C_ARM_SDA_ALT			0				// SDA: ALT 0

#define MP8845_I2C_CORE_GPIO_GRP 		3				// GPIOD
#define MP8845_I2C_CORE_SCL 			6				// SCL : GPIOD06
#define MP8845_I2C_CORE_SDA 			7				// SDA: GPIOD07
#define MP8845_I2C_CORE_SCL_ALT			0				// SCL: ALT0
#define MP8845_I2C_CORE_SDA_ALT			0				// SDA: ALT 0

/************************************************
  * SVT Board (PMIC: NXE2000, MP8845)  - Reference 2016.04.05
  * ARM		: 1.25V
  * CORE	: 1.2V
  * DDR		: 1.5V
  * DDR_IO	: 1.5V
  ************************************************/
void pmic_board_init(void)
{
	char data[4];

	/* Set the Arm Voltage */
	I2C_INIT(MP8845_I2C_ARM_GPIO_GRP, MP8845_I2C_ARM_SCL, MP8845_I2C_ARM_SDA,
			MP8845_I2C_ARM_SCL_ALT, MP8845_I2C_ARM_SDA_ALT);

	/* PFM -> PWM mode */
	mp8845_read(MP8845C_REG_SYSCNTL1, data, 1);
	data[0] |= 1 << 0;
	mp8845_write(MP8845C_REG_SYSCNTL1, data, 1);
	/* Start the Vout */
	mp8845_read(MP8845C_REG_SYSCNTL2, data, 1);
	data[0] |= 1 << 5;
	mp8845_write(MP8845C_REG_SYSCNTL2, data, 1);

//	data[0] = 90 | 1<<7;							// 90: 1.2V
//	data[0] = 80 | 1<<7;							// 80: 1.135V
	data[0] = 75 | 1 << 7;							// 75: 1.1V
	mp8845_write(MP8845C_REG_VSEL, data, 1);

	/* Set the Core Voltage */
	I2C_INIT(MP8845_I2C_CORE_GPIO_GRP, MP8845_I2C_CORE_SCL, MP8845_I2C_CORE_SDA,
			MP8845_I2C_CORE_SCL_ALT, MP8845_I2C_CORE_SDA_ALT);

	/* PFM -> PWM mode */
	mp8845_read(MP8845C_REG_SYSCNTL1, data, 1);
	data[0] |= 1 << 0;
	mp8845_write(MP8845C_REG_SYSCNTL1, data, 1);
	/* Start the Vout */
	mp8845_read(MP8845C_REG_SYSCNTL2, data, 1);
	data[0] |= 1 << 5;
	mp8845_write(MP8845C_REG_SYSCNTL2, data, 1);

//	data[0] = 90 | 1<<7;							// 90: 1.2V
//	data[0] = 80 | 1<<7;							// 80: 1.135V
	data[0] = 75 | 1 << 7;							// 75: 1.1V
	mp8845_write(MP8845C_REG_VSEL, data, 1);

	/* Set the NXE2000 */
	I2C_INIT(NXE2000_I2C_GPIO_GRP, NXE2000_I2C_SCL, NXE2000_I2C_SDA,
			NXE2000_I2C_SCL_ALT, NXE2000_I2C_SDA_ALT);

	/* Set the DDR Voltage */
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC4_VOL);
	nxe2000_write(NXE2000_REG_DC4VOL, data, 1);

	/* Set the DDR_IO Voltage */
	data[0] = nxe2000_get_dcdc_step(NXE2000_DEF_DDC5_VOL);
	nxe2000_write(NXE2000_REG_DC5VOL, data, 1);
}
#endif // #if defined(PMIC_ON)
