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

#ifndef __PMIC_MP8845_H__
#define __PMIC_MP8845_H__

#define MP8845C_REG_VSEL 0x00
#define MP8845C_REG_SYSCNTL1 0x01
#define MP8845C_REG_SYSCNTL2 0x02
#define MP8845C_REG_ID1 0x03
#define MP8845C_REG_ID2 0x04
#define MP8845C_REG_STATUS 0x05

#define I2C_ADDR_MP8845 (0x38 >> 1) // SVT & ASB

#endif // ifdef __PMIC_MP8845_H__
