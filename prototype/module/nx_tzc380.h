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
#ifndef __NX_TZC380_H__
#define __NX_TZC380_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	TZC-380 Base
//------------------------------------------------------------------------------
//@{

	struct NX_TZC_RegionSetup
	{
		volatile U32 REGION_SETUP_LOW;		// 0x1x0 Region Setup Low
		volatile U32 REGION_SETUP_HIGH;		// 0x1x4 Region Setup High
		volatile U32 REGION_ATTRIBUTES;		// 0x1x8 Region Attributes
		volatile U32 _Reserved;				// 0x1xC
	};

	struct	NX_TZC380_RegisterSet
	{
		volatile U32 CONFIGURATION;			// 0x000 Configuration
		volatile U32 ACTION;				// 0x004 Action
		volatile U32 LOCKDOWN_RANGE;		// 0x008 Lockdown Range
		volatile U32 LOCKDOWN_SELECT;		// 0x00C Lockdown Select
		volatile U32 INT_STATUS;			// 0x010 Interrupt Status
		volatile U32 INT_CLEAR;				// 0x014 Interrupt Clear
		volatile U32 _Reserved0[2];			// 0x018~0x01C
		volatile U32 FAIL_ADDR_LOW;			// 0x020 Fail Address Low
		volatile U32 FAIL_ADDR_HIGH;		// 0x024 Fail Address High
		volatile U32 FAIL_CONTROL;			// 0x028 Fail Control
		volatile U32 FAIL_ID;				// 0x02C Fail ID
		volatile U32 SPECULATION_CONTROL;	// 0x030 Speculation Control
		volatile U32 SECURITY_INVERSION_EN;	// 0x034 Security Inversion Enable
		volatile U32 _Reserved1[50];		// 0x038~0x0FC
		struct NX_TZC_RegionSetup RS[16];	// 0x100~0x1FC
		volatile U32 _Reserved2[0x300];		// 0x200~0xDFC
		volatile U32 ITCRG;					// 0xE00
		volatile U32 ITIP;					// 0xE04
		volatile U32 ITOP;					// 0xE08
		volatile U32 _Reserved3[0x71];		// 0xE0C~0xFCC
		volatile U32 PERIPH_ID4;			// 0xFD0
		volatile U32 _Reserved4[3];			// 0xFD4~0xFDC
		volatile U32 PERIPH_ID[4];			// 0xFE0~0xFEC
		volatile U32 COMPONENT_ID[4];		// 0xFF0~0xFFC
	};

enum {
	TZPORT_IOPERI_BUS_M0	= 0,
	TZPORT_IOPERI_BUS_M1	= 1,
	TZPORT_IOPERI_BUS_M2	= 2,
	TZPORT_TOP_BUS_M0		= 3,
	TZPORT_STATIC_BUS_M0	= 4,
	TZPORT_STATIC_BUS_M1	= 5,
	TZPORT_DISPLAY_BUS_M0	= 6,
	TZPORT_BOT_BUS_M0		= 7,
	TZPORT_SFR_BUS_M0		= 8,
	TZPORT_SFR_BUS_M1		= 9,
	TZPORT_SFR_BUS_M2		=10,
	TZPORT_DREX_SECURE_BOOT_LOCK	= 14,
	TZPORT_GIC400_ENABLE	=15,
	TZPORT_SFR0_BUS_M0		=16,
	TZPORT_SFR0_BUS_M1		=17,
	TZPORT_SFR0_BUS_M2		=18,
	TZPORT_SFR0_BUS_M3		=19,
	TZPORT_SFR0_BUS_M4		=20,
	TZPORT_SFR0_BUS_M5		=21,
	TZPORT_SFR0_BUS_M6		=22,
	TZPORT_SFR1_BUS_M0		=33,
	TZPORT_SFR1_BUS_M1		=34,
	TZPORT_SFR1_BUS_M2		=35,
	TZPORT_SFR1_BUS_M3		=36,
	TZPORT_SFR1_BUS_M4		=37,
	TZPORT_SFR1_BUS_M5		=38,
	TZPORT_SFR1_BUS_M6		=39,
	TZPORT_GPU_XPROT		=40,
	TZPORT_SFR2_BUS_M0		=48,
	TZPORT_SFR2_BUS_M1		=49,
	TZPORT_SFR2_BUS_M2		=50,
	TZPORT_SFR2_BUS_M3		=51,
	TZPORT_SFR2_BUS_M4		=52,
	TZPORT_SFR2_BUS_M5		=53,
}TZPORT;

enum {
	NX_TZASC_REACTION_INTLOW_OKAY		= 0,
	NX_TZASC_REACTION_INTLOW_DECERR		= 1,
	NX_TZASC_REACTION_INTHIGH_OKAY		= 2,
	NX_TZASC_REACTION_INTHIGH_DECERR	= 3
}NX_TZASC_REACTION;


//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_TZC380_Initialize( void );
U32		NX_TZC380_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_TZC380_GetPhysicalAddress( void );
U32		NX_TZC380_GetSizeOfRegisterSet( void );

void	NX_TZC380_SetBaseAddress( void* BaseAddress );
void*	NX_TZC380_GetBaseAddress( void );

CBOOL	NX_TZC380_OpenModule( void );
CBOOL	NX_TZC380_CloseModule( void );
CBOOL	NX_TZC380_CheckBusy( void );
CBOOL	NX_TZC380_CanPowerDown( void );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
// there is no interrupt interface
//@}

//------------------------------------------------------------------------------
/// @name	Clock Management
//@{
// there is no clock interface
//@}

//------------------------------------------------------------------------------
/// @name	Power Management
//@{
//@}

//------------------------------------------------------------------------------
/// @name	Reset Management
//@{
// will find reset feature
//@}


//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_TZC380_H__

