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
#ifndef __NX_ADC_H__
#define __NX_ADC_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	ADC
//------------------------------------------------------------------------------
//@{

struct NX_ADC_RegisterSet
{
	volatile U32	ADCCON;					///< 0x00 : ADC Control Register
	volatile U32	ADCDAT;					///< 0x04 : ADC Output Data Register
	volatile U32	ADCINTENB;				///< 0x08 : ADC Interrupt Enable Register
	volatile U32	ADCINTCLR;				///< 0x0C : ADC Interrutp Pending and Clear Register
	volatile U32	ADCPRESCALE;			///< 0x0E : ADC Prescale enable & value Register
};

typedef enum
{
	DELAY_5CLK = 0,
	DELAY_4CLK = 1,
	DELAY_3CLK = 2,
	DELAY_2CLK = 3,
	DELAY_1CLK = 4,
	NOT_DELAY  = 5
	
} NX_ADC_DATA_DELAY;

	
//------------------------------------------------------------------------------
///	@name	Basic Interface
//------------------------------------------------------------------------------
//@{
CBOOL	NX_ADC_Initialize( void );
U32	NX_ADC_GetNumberOfModule( void );

U32	NX_ADC_GetSizeOfRegisterSet( void );

void	NX_ADC_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*	NX_ADC_GetBaseAddress( U32 ModuleIndex );

U32  	NX_ADC_GetPhysicalAddress ( U32 ModuleIndex );
CBOOL	NX_ADC_OpenModule( U32 ModuleIndex );
CBOOL	NX_ADC_CloseModule( U32 ModuleIndex );
CBOOL	NX_ADC_CheckBusy( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	reset Interface
//------------------------------------------------------------------------------
//@{
U32		NX_ADC_GetResetNumber ( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	Interrupt Interface
//------------------------------------------------------------------------------
//@{
/// @brief	interrupt index for IntNum
U32	NX_ADC_GetInterruptNumber ( U32 ModuleIndex );
void	NX_ADC_SetInterruptEnable( U32 ModuleIndex, U32 IntNum, CBOOL Enable );
CBOOL	NX_ADC_GetInterruptEnable( U32 ModuleIndex, U32 IntNum );
CBOOL	NX_ADC_GetInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_ADC_ClearInterruptPending( U32 ModuleIndex, U32 IntNum );
void	NX_ADC_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_ADC_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL	NX_ADC_GetInterruptPendingAll( U32 ModuleIndex );
void	NX_ADC_ClearInterruptPendingAll( U32 ModuleIndex );
S32	NX_ADC_GetInterruptPendingNumber( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
///	@name	PAD Interface
//------------------------------------------------------------------------------
//@{
U32	NX_ADC_GetNumberOfPADMode ( U32 ModuleIndex );
U32	NX_ADC_EnablePAD ( U32 ModuleIndex, U32 ModeIndex );
//@}


//------------------------------------------------------------------------------
///	@name	ADC Interface
//------------------------------------------------------------------------------
//@{
void	NX_ADC_SetPrescalerValue( U32 ModuleIndex, U32 Value );
U8	NX_ADC_GetPrescalerValue( U32 ModuleIndex );
void	NX_ADC_SetPrescalerEnable( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_ADC_GetPrescalerEnable( U32 ModuleIndex );

void	NX_ADC_SetADCDataDelay( U32 ModuleIndex, NX_ADC_DATA_DELAY Delay );
NX_ADC_DATA_DELAY	NX_ADC_GetADCDataDelay( U32 ModuleIndex );

void	NX_ADC_SetSOCDelay( U32 ModuleIndex, U32 Value );
U8	NX_ADC_GetSOCDelay( U32 ModuleIndex );

void	NX_ADC_SetInputChannel( U32 ModuleIndex, U32 Channel );
U32	NX_ADC_GetInputChannel( U32 ModuleIndex );
void	NX_ADC_SetStandbyMode( U32 ModuleIndex, CBOOL Enable );
CBOOL	NX_ADC_GetStandbyMode( U32 ModuleIndex );
void	NX_ADC_Start( U32 ModuleIndex );
CBOOL	NX_ADC_IsBusy( U32 ModuleIndex );
U16	NX_ADC_GetConvertedData( U32 ModuleIndex );
//@}


//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_ADC_H__
