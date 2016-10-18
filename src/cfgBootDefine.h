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

#ifndef __CFG_BOOT_DEFINE_H__
#define __CFG_BOOT_DEFINE_H__

//------------------------------------------------------------------------------
//  Load from NSIH On/Off.
//------------------------------------------------------------------------------
#define CFG_NSIH_EN (1)

//------------------------------------------------------------------------------
//  Chip product
//------------------------------------------------------------------------------
//#define ARCH_S5P4418
//#define ARCH_NXP4330
#define ARCH_NXP5430

//------------------------------------------------------------------------------
//  Memory Type
//------------------------------------------------------------------------------
#ifdef MEMTYPE_DDR3
#define MEM_TYPE_DDR3
#endif
#ifdef MEMTYPE_LPDDR3
#define MEM_TYPE_LPDDR23
#endif

//------------------------------------------------------------------------------
//  System optional.
//------------------------------------------------------------------------------
#define MULTICORE_SLEEP_CONTROL (1)
#define MULTICORE_BRING_UP (0)
#define CCI400_COHERENCY_ENABLE (1)
#define CONFIG_RESET_AFFINITY_ID (1) // Dependent kernel 3.18.x

#endif //	__CFG_BOOT_DEFINE_H__
