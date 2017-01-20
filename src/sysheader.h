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
#ifndef __SYS_HEADER_H__
#define __SYS_HEADER_H__

#include <s5p6818.h>
#include <type.h>

#if defined(ARCH_NXP4330) || defined(ARCH_S5P4418)
#include <nx_pyrope.h>
#endif
#if defined(ARCH_NXP5430)
#include <nx_peridot.h>
#endif
#include <nx_type.h>
#include <nx_chip.h>

#include <nx_uart.h>
#include <nx_cci400.h>
#include <nx_gic400.h>
#include <nx_rstcon.h>
#include <nx_clkpwr.h>
#include <nx_ecid.h>
#include <nx_gpio.h>
#include <nx_alive.h>
#include <nx_tieoff.h>
#include <nx_intc.h>
#include <nx_clkgen.h>
#include <nx_ssp.h>
#include <nx_wdt.h>
#include <nx_rtc.h>
#include <nx_tzpc.h>
#include <nx_tzc380.h>

#include "secondboot.h"
#include <clkgen.h>
#include <gpio.h>

#include <serial.h>
#include <printf.h>

#include "configs/s5p6818_avn_ref.h"

#if defined(SYSLOG_ON)
#define SYSMSG printf
#else
#define SYSMSG(x, ...)
#endif

// Memory debug message
#if defined(SYSLOG_ON)
#define MEMMSG printf
#else
#define MEMMSG(x, ...)
#endif

#if 0
#define MEMINF printf
#else
#define MEMINF(x, ...)
#endif

#define DEVMSG_ON	0

/* Set Log Message(Debug) Macro */
#define LOG_LEVEL			30

#define LOG_LEVEL_NONE			0
#define LOG_LEVEL_ERROR			10
#define LOG_LEVEL_NOTICE		20
#define LOG_LEVEL_WARNING		30
#define LOG_LEVEL_INFO			40
#define LOG_LEVEL_VERBOSE		50


#if (LOG_LEVEL >= LOG_LEVEL_NOTICE) && defined(SYSLOG_ON)
# define NOTICE(...)	printf("NOTICE:  " __VA_ARGS__)
#else
# define NOTICE(...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_ERROR) && defined(SYSLOG_ON)
# define ERROR(...)	printf("ERROR:   " __VA_ARGS__)
#else
# define ERROR(...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_WARNING) && defined(SYSLOG_ON)
# define WARN(...)	printf("WARNING: " __VA_ARGS__)
#else
# define WARN(...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_INFO) && defined(SYSLOG_ON)
# define INFO(...)	printf("INFO:    " __VA_ARGS__)
#else
# define INFO(...)
#endif

#if (LOG_LEVEL >= LOG_LEVEL_VERBOSE) && defined(SYSLOG_ON)
# define VERBOSE(...)	printf("VERBOSE: " __VA_ARGS__)
#else
# define VERBOSE(...)
#endif

//------------------------------------------------------------------------------
//  Set global variables
//------------------------------------------------------------------------------

#if defined(__SET_GLOBAL_VARIABLES)

struct NX_SecondBootInfo *const pSBI =
    (struct NX_SecondBootInfo * const)BASEADDR_SRAM;
struct NX_SecondBootInfo *const pTBI =
    (struct NX_SecondBootInfo * const)BASEADDR_SRAM;
struct NX_GPIO_RegisterSet (*const pReg_GPIO)[1] =
    (struct NX_GPIO_RegisterSet (*const)[])PHY_BASEADDR_GPIOA_MODULE;
struct NX_ALIVE_RegisterSet *const pReg_Alive =
    (struct NX_ALIVE_RegisterSet * const)PHY_BASEADDR_ALIVE_MODULE;
struct NX_TIEOFF_RegisterSet *const pReg_Tieoff =
    (struct NX_TIEOFF_RegisterSet * const)PHY_BASEADDR_TIEOFF_MODULE;
struct NX_ECID_RegisterSet *const pReg_ECID =
    (struct NX_ECID_RegisterSet * const)PHY_BASEADDR_ECID_MODULE;
struct NX_CLKPWR_RegisterSet *const pReg_ClkPwr =
    (struct NX_CLKPWR_RegisterSet * const)PHY_BASEADDR_CLKPWR_MODULE;
struct NX_RSTCON_RegisterSet *const pReg_RstCon =
    (struct NX_RSTCON_RegisterSet * const)PHY_BASEADDR_RSTCON_MODULE;

struct NX_WDT_RegisterSet *const pReg_WDT =
    (struct NX_WDT_RegisterSet * const)PHY_BASEADDR_WDT_MODULE;
struct NX_RTC_RegisterSet *const pReg_RTC =
    (struct NX_RTC_RegisterSet * const)PHY_BASEADDR_RTC_MODULE;

struct NX_CCI400_RegisterSet *const pReg_CCI400 =
    (struct NX_CCI400_RegisterSet * const)PHY_BASEADDR_CCI400_MODULE;
struct NX_GIC400_RegisterSet *const pReg_GIC400 =
    (struct NX_GIC400_RegisterSet * const)PHY_BASEADDR_INTC_MODULE;
struct NX_TZPC_RegisterSet (*const pReg_TZPC)[1] =
    (struct NX_TZPC_RegisterSet (*const)[])PHY_BASEADDR_TZPC0_MODULE;
struct NX_TZC380_RegisterSet *const pReg_TZC380 =
    (struct NX_TZC380_RegisterSet *
     const)PHY_BASEADDR_A3BM_AXI_TOP_MASTER_BUS_MODULE;

#else

extern struct NX_SecondBootInfo *const pSBI; // second boot info
extern struct NX_SecondBootInfo *const pTBI; // third boot info
extern struct NX_GPIO_RegisterSet (*const pReg_GPIO)[1];
extern struct NX_ALIVE_RegisterSet *const pReg_Alive;
extern struct NX_TIEOFF_RegisterSet *const pReg_Tieoff;
extern struct NX_ECID_RegisterSet *const pReg_ECID;
extern struct NX_CLKPWR_RegisterSet *const pReg_ClkPwr;
extern struct NX_RSTCON_RegisterSet *const pReg_RstCon;


extern struct NX_WDT_RegisterSet *const pReg_WDT;
extern struct NX_RTC_RegisterSet *const pReg_RTC;
#if defined(ARCH_NXP5430)

extern struct NX_CCI400_RegisterSet *const pReg_CCI400;
extern struct NX_GIC400_RegisterSet *const pReg_GIC400;
extern struct NX_TZPC_RegisterSet (*const pReg_TZPC)[1];
extern struct NX_TZC380_RegisterSet *const pReg_TZC380;
#endif

#endif

#endif //	__SYS_HEADER_H__
