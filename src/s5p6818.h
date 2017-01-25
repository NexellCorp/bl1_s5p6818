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
#ifndef __S5P6818_H__
#define __S5P6818_H__

/* NXPXX30 - Chip Product */
#define ARCH_NXP5430

/* Support the Kernel Version */
#if defined(KERNEL_VER_3_4)
#define SUPPORT_KERNEL_3_4			1
#else
#define SUPPORT_KERNEL_3_4			0
#endif

/* System Option */
#if (SUPPORT_KERNEL_3_4 == 1)
#define MULTICORE_SLEEP_CONTROL			1
#define MULTICORE_BRING_UP			1
#define CCI400_COHERENCY_ENABLE			1
#define CONFIG_RESET_AFFINITY_ID		0				// Dependent kernel 3.18.x
#else	// #if (SUPPORT_KERNEL_3_4 == 1)
#define MULTICORE_SLEEP_CONTROL			1
#define MULTICORE_BRING_UP			0
#define CCI400_COHERENCY_ENABLE			1
#define CONFIG_RESET_AFFINITY_ID		1				// Dependent kernel 3.18.x
#endif

/* DRAM(DDR3/LPDDR3) Memory Configuration */
#ifdef MEMTYPE_DDR3
#define MEM_TYPE_DDR3
#endif
#ifdef MEMTYPE_LPDDR3
#define MEM_TYPE_LPDDR23
#endif

/* CRC Check Configuration */
#define SUPPORT_SW_CRC_CHECK			1

#endif // __S5P6818_H__
