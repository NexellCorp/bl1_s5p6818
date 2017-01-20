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
#ifndef __DRONE_H__
#define __DRONE_H__

#define AUTO_VOLTAGE_CONTROL			1
#define ARM_VOLTAGE_CONTROL_SKIP		0

#define AXP_I2C_GPIO_GRP 			3 				// GPIOD
#define AXP_I2C_SCL 				20				// SCL : GPIOD 20
#define AXP_I2C_SDA 				16				// SDA : GPIOD 16
#define AXP_I2C_SCL_ALT				0				// SCL : ALT0
#define AXP_I2C_SDA_ALT				0				// SDA: ALT0

#endif // __DRONE_H__
