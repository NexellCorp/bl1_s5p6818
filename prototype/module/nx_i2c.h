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
#ifndef __NX_I2C_H__
#define __NX_I2C_H__

#include "../base/nx_prototype.h"

#ifdef  __cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup   I2C I2C
//------------------------------------------------------------------------------
//@{

    /// @brief  I2C Module's Register List
    struct  NX_I2C_RegisterSet
    {
        volatile U32 ICCR;                  ///< 0x00 : I2C Control Register
        volatile U32 ICSR;                  ///< 0x04 : I2C Status Register
        volatile U32 IAR;                   ///< 0x08 : I2C Address Register
        volatile U32 IDSR;                  ///< 0x0C : I2C Data Register
        volatile U32 ICLC;   	            ///< 0x10 : I2C Line Control Register
    };

    /// @brief Select I2C Mode
    typedef enum
    {
        NX_I2C_TXRXMODE_SLAVE_RX    = 0,    ///< Slave Receive Mode
        NX_I2C_TXRXMODE_SLAVE_TX    = 1,    ///< Slave Transmit Mode
        NX_I2C_TXRXMODE_MASTER_RX   = 2,    ///< Master Receive Mode
        NX_I2C_TXRXMODE_MASTER_TX   = 3     ///< Master Transmit Mode

    } NX_I2C_TXRXMODE ;

    /// @brief  Start/Stop signal
    typedef enum
    {
        NX_I2C_SIGNAL_STOP  = 0,            ///< Stop signal generation
        NX_I2C_SIGNAL_START = 1             ///< Start signal generation

    } NX_I2C_SIGNAL ;

	typedef enum
	{
		NX_I2C_SDA_DELAY_0CLOCK  = 0,
		NX_I2C_SDA_DELAY_5CLOCK  = 1,
		NX_I2C_SDA_DELAY_10CLOCK = 2,
		NX_I2C_SDA_DELAY_15CLOCK = 3,
	} NX_I2C_SDA_DELAY;
//------------------------------------------------------------------------------
/// @name   Module Interface
//@{
CBOOL   NX_I2C_Initialize( void );
U32     NX_I2C_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
/// @name   Basic Interface
//@{
U32     NX_I2C_GetPhysicalAddress( U32 ModuleIndex );
U32     NX_I2C_GetSizeOfRegisterSet( void );

void    NX_I2C_SetBaseAddress( U32 ModuleIndex, void* BaseAddress );
void*    NX_I2C_GetBaseAddress( U32 ModuleIndex );
CBOOL   NX_I2C_OpenModule( U32 ModuleIndex );
CBOOL   NX_I2C_CloseModule( U32 ModuleIndex );
CBOOL   NX_I2C_CheckBusy( U32 ModuleIndex );
CBOOL   NX_I2C_CanPowerDown( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
/// @name   Interrupt Interface
//@{
S32     NX_I2C_GetInterruptNumber( U32 ModuleIndex );

void    NX_I2C_SetInterruptEnable( U32 ModuleIndex, S32 IntNum, CBOOL Enable );
CBOOL   NX_I2C_GetInterruptEnable( U32 ModuleIndex, S32 IntNum );
CBOOL   NX_I2C_GetInterruptPending( U32 ModuleIndex, S32 IntNum );
void    NX_I2C_ClearInterruptPending( U32 ModuleIndex, S32 IntNum );

void    NX_I2C_SetInterruptEnableAll( U32 ModuleIndex, CBOOL Enable );
CBOOL   NX_I2C_GetInterruptEnableAll( U32 ModuleIndex );
CBOOL   NX_I2C_GetInterruptPendingAll( U32 ModuleIndex );
void    NX_I2C_ClearInterruptPendingAll( U32 ModuleIndex );

void    NX_I2C_SetInterruptEnable32( U32 ModuleIndex, U32 EnableFlag );
U32     NX_I2C_GetInterruptEnable32( U32 ModuleIndex );
U32     NX_I2C_GetInterruptPending32( U32 ModuleIndex );
void    NX_I2C_ClearInterruptPending32( U32 ModuleIndex, U32 PendingFlag );

S32     NX_I2C_GetInterruptPendingNumber( U32 ModuleIndex );    // -1 if None
//@}

//------------------------------------------------------------------------------
/// @name   Clock Control Interface
//@{
U32     NX_I2C_GetClockNumber( U32 ModuleIndex );
U32     NX_I2C_GetResetNumber( U32 ModuleIndex );
//@}

//--------------------------------------------------------------------------
/// @name Configuration Function
//--------------------------------------------------------------------------
//@{
void    NX_I2C_SetClockPrescaler( U32 ModuleIndex, U32 PclkDivider, U32 Prescaler );
void    NX_I2C_GetClockPrescaler( U32 ModuleIndex, U32* pPclkDivider, U32* pPrescaler );
void    NX_I2C_SetSlaveAddress( U32 ModuleIndex, U8 Address);
 U32    NX_I2C_GetSlaveAddress( U32 ModuleIndex );

//@}

//------------------------------------------------------------------------------
/// @name Operation Function
//------------------------------------------------------------------------------
//@{
void    NX_I2C_SetAckGenerationEnable( U32 ModuleIndex, CBOOL bAckGen );
CBOOL   NX_I2C_GetAckGenerationEnable( U32 ModuleIndex );

void    NX_I2C_ControlMode ( U32 ModuleIndex, NX_I2C_TXRXMODE TxRxMode, NX_I2C_SIGNAL Signal );
void    NX_I2C_SetTXRXEnable( U32 ModuleIndex, NX_I2C_TXRXMODE TxRxMode );


void		NX_I2C_SetFilterEnable( U32 ModuleIndex, CBOOL bEnable );
CBOOL       NX_I2C_GetFilterEnable( U32 ModuleIndex );
void				NX_I2C_SetSDAOutputDelay( U32 ModuleIndex, NX_I2C_SDA_DELAY Delay );
NX_I2C_SDA_DELAY	NX_I2C_GetSDAOutputDelay( U32 ModuleIndex );


CBOOL   NX_I2C_IsBusy( U32 ModuleIndex );
void    NX_I2C_WriteByte (U32 ModuleIndex, U8 WriteData);
U8      NX_I2C_ReadByte ( U32 ModuleIndex );
void    NX_I2C_BusDisable( U32 ModuleIndex );
//@}

//------------------------------------------------------------------------------
/// @name Checking Function ( extra Interrupt source )
//------------------------------------------------------------------------------
//@{
CBOOL   NX_I2C_IsSlaveAddressMatch( U32 ModuleIndex );

CBOOL   NX_I2C_IsBusArbitFail( U32 ModuleIndex );
CBOOL   NX_I2C_IsACKReceived( U32 ModuleIndex );
CBOOL   NX_I2C_IsTxMode( U32 ModuleIndex );
//@}

//@}

#ifdef  __cplusplus
}
#endif

#endif // __NX_I2C_H__
