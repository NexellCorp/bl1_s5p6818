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
#ifndef _NX_PWM_H
#define _NX_PWM_H

//------------------------------------------------------------------------------
//  includes
//------------------------------------------------------------------------------
#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
//  defines
//------------------------------------------------------------------------------
#define NUMBER_OF_PWM_CHANNEL	5
#define NX_PWM_INT		4

//------------------------------------------------------------------------------
/// @brief  PWM register set structure
//------------------------------------------------------------------------------
struct NX_PWM_RegisterSet
{
	volatile U32 TCFG0		;	// 0x00	// R/W	// PWM Configuration Register 0 that configures the two 8-bit Prescaler and DeadZone Length
	volatile U32 TCFG1		;	// 0x04	// R/W	// PWM Configuration Register 1 that controls 5 MUX and DMA Mode Select Bit
	volatile U32 TCON       ;	// 0x08	// R/W	// PWM Control Register
	volatile U32 TCNTB0     ;	// 0x0C	// R/W	// PWM 0 Count Buffer Register
	volatile U32 TCMPB0     ;	// 0x10	// R/W	// PWM 0 Compare Buffer Register
	volatile U32 TCNTO0     ;	// 0x14	// R	// PWM 0 Count Observation Register
 	volatile U32 TCNTB1     ;	// 0x18	// R/W	// PWM 1 Count Buffer Register
	volatile U32 TCMPB1     ;	// 0x1C	// R/W	// PWM 1 Compare Buffer Register
	volatile U32 TCNTO1     ;	// 0x20	// R	// PWM 1 Count Observation Register
	volatile U32 TCNTB2     ;	// 0x24	// R/W	// PWM 2 Count Buffer Register
	volatile U32 TCMPB2     ;	// 0x28	// R/W	// PWM 2 Compare Buffer Register
	volatile U32 TCNTO2     ;	// 0x2C	// R	// PWM 2 Count Observation Register
	volatile U32 TCNTB3     ;	// 0x30	// R/W	// PWM 3 Count Buffer Register
	volatile U32 TCMPB3     ;	// 0x34	// R/W	// PWM 3 Compare Buffer Register
	volatile U32 TCNTO3     ;	// 0x38	// R	// PWM 3 Count Observation Register
	volatile U32 TCNTB4     ;	// 0x3C	// R/W	// PWM 4 Count Buffer Register
	volatile U32 TCNTO4     ;	// 0x40	// R	// PWM 4 Count Observation Register
	volatile U32 TINT_CSTAT ;	// 0x44	// R/W	// PWM Interrupt Control and Status Register
};

//------------------------------------------------------------------------------
/// enum
//------------------------------------------------------------------------------

typedef enum 
{
	NX_PWM_DIVIDSELECT_1	= 0UL,
	NX_PWM_DIVIDSELECT_2	= 1UL,
	NX_PWM_DIVIDSELECT_4	= 2UL,
	NX_PWM_DIVIDSELECT_8	= 3UL,
	NX_PWM_DIVIDSELECT_16 	= 4UL,
	NX_PWM_DIVIDSELECT_TCLK	= 5UL

} NX_PWM_DIVIDSELECT;

typedef enum 
{
	NX_PWM_LOADMODE_ONESHOT		= 0UL,
	NX_PWM_LOADMODE_AUTORELOAD	= 1UL
} NX_PWM_LOADMODE;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_PWM_Initialize( void );
U32		NX_PWM_GetNumberOfModule( void );
U32		NX_PWM_GetNumberOfChannel( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_PWM_GetPhysicalAddress( U32 ModuleIndex );
//U32		NX_PWM_GetResetNumber( U32 ModuleIndex, U32 ChannelIndex );
//U32		NX_PWM_GetNumberOfReset( void );
U32		NX_PWM_GetSizeOfRegisterSet( void );
void	NX_PWM_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_PWM_GetBaseAddress( U32 ModuleIndex );
CBOOL	NX_PWM_OpenModule( U32 ModuleIndex );
CBOOL	NX_PWM_CloseModule( U32 ModuleIndex );
CBOOL	NX_PWM_CheckBusy( U32 ModuleIndex );
CBOOL	NX_PWM_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	clock Interface
//------------------------------------------------------------------------------
//@{
U32 NX_PWM_GetClockNumber ( U32 ModuleIndex, U32 SubModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32 NX_PWM_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//@{
U32		NX_PWM_GetInterruptNumber( U32 ModuleIndex, U32 SubModuleIndex );

void	NX_PWM_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_PWM_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL	NX_PWM_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_PWM_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );

void	NX_PWM_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_PWM_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_PWM_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_PWM_ClearInterruptPendingAll( U32 ModuleIndex );

U32		NX_PWM_GetInterruptPendingNumber( U32 ModuleIndex );	// -1 if None
//@}


//------------------------------------------------------------------------------

//--------------------------------------------------------------------------
/// @name	Configuration operations
//--------------------------------------------------------------------------
//@{
void	NX_PWM_SetPrescaler(U32 Channel, U32 value);
U32		NX_PWM_GetPrescaler(U32 Channel);

CBOOL	NX_PWM_SetDividerPath(U32 Channel, NX_PWM_DIVIDSELECT divider);
NX_PWM_DIVIDSELECT NX_PWM_GetDividerPath(U32 Channel);

void	NX_PWM_SetDeadZoneLength(U32 Channel, U32 Length);
U32		NX_PWM_GetDeadZoneLength(U32 Channel);

CBOOL	NX_PWM_SetDeadZoneEnable(U32 Channel, CBOOL Enable);
CBOOL	NX_PWM_GetDeadZoneEnable(U32 Channel);

CBOOL	NX_PWM_SetOutInvert(U32 Channel, CBOOL Enable);
CBOOL	NX_PWM_GetOutInvert(U32 Channel);


void	NX_PWM_SetShotMode(U32 Channel, NX_PWM_LOADMODE ShotMode);
NX_PWM_LOADMODE	NX_PWM_GetShotMode(U32 Channel);

void	NX_PWM_UpdateCounter( U32 Channel );

void	NX_PWM_Run(U32 Channel);
void	NX_PWM_Stop(U32 Channel);
CBOOL	NX_PWM_IsRun(U32 Channel);

void	NX_PWM_SetPeriod(U32 Channel, U32 Period);
U32		NX_PWM_GetPeriod(U32 Channel);

CBOOL	NX_PWM_SetDuty(U32 Channel, U32 Duty);
U32		NX_PWM_GetDuty(U32 Channel);

U32		NX_PWM_GetCurrentCount(U32 Channel);

//@}

#ifdef	__cplusplus
}
#endif

#endif // _NX_PWM_H

