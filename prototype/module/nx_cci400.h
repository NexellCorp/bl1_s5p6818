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
#ifndef __NX_CCI400_H__
#define __NX_CCI400_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	CCI-400 Base Bus Controller
//------------------------------------------------------------------------------
//@{

struct NX_CCISlaveIF_RegisterSet
{
	volatile U32 SCR;			// 0x0000 Snoop Control
	volatile U32 SOR;			// 0x0004 Shareable Override for slave interface
	volatile U32 _Reserved0[0x3E];		// 0x0008~0x00FC
	volatile U32 RCQoSVOR;			// 0x0100 Read Channel QoS Value Override
	volatile U32 WCQoSVOR;			// 0x0104 Write Channel QoS Value Override
	volatile U32 _Reserved1;		// 0x0108
	volatile U32 QoSCR;			// 0x010C QoS Control
	volatile U32 MOTR;			// 0x0110 Max OT
	volatile U32 _Reserved2[0x7];		// 0x0114~0x012C
	volatile U32 RTR;			// 0x0130 Regulator Target
	volatile U32 QoSRSFR;			// 0x0134 QoS Regulator Scale Factor
	volatile U32 QoSRR;			// 0x0138 QoS Range
	volatile U32 _Reserved3[0x3B1];		// 0x013C~0x0FFC
};

struct NX_CCIPerCnt_RegisterSet
{
	volatile U32 ESR;			// 0x0000 Event Select
	volatile U32 ECR;			// 0x0004 Event Count
	volatile U32 CCTRLR;			// 0x0008 Counter Control
	volatile U32 OFFSR;			// 0x000C Overflow Flag Status
	volatile U32 _Reserved[0x3FC];		// 0x0010~0x0FFC
};

struct	NX_CCI400_RegisterSet
{
	volatile U32 COR;			// 0x0000 Control Override
	volatile U32 SCR;			// 0x0004 Speculation Control
	volatile U32 SAR;			// 0x0008 Secure Access
	volatile U32 STSR;			// 0x000C Status
	volatile U32 IER;			// 0x0010 Imprecise Error
	volatile U32 _Reserved0[0x3B];		// 0x0014~0x00FC
	volatile U32 PMCR;			// 0x0100 Performance Monitor Control
	volatile U32 _Reserved1[0x3B3];		// 0x0104~0x0FCC
	volatile U32 CPIDR[0xC];		// 0x0FD0~0x0FFC
	struct NX_CCISlaveIF_RegisterSet CSI[5];// 0x1000~0x5FFC
	volatile U32 _Reserved2[0xC01];		// 0x6000~0x9000
	volatile U32 CCR;			// 0x9004 Cycle counter
	volatile U32 CCTRLR;			// 0x9008 Count Control
	volatile U32 OFFSR;			// 0x900C Overflow Flag Status
	volatile U32 _Reserved3[0x3FC];		// 0x9010~0x9FFC
	struct NX_CCIPerCnt_RegisterSet CPC[4];	// 0xA000~DFFC
	volatile U32 _Reserved4[0x800];		// 0xE000~0xFFFC
};

typedef enum
{
	BUSID_CS	= 0,	// CoreSight
	BUSID_CODA	= 1,
	BUSID_TOP	= 2,
	BUSID_CPUG1	= 3,
	BUSID_CPUG0	= 4
}BUSID;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_CCI400_Initialize( void );
U32	NX_CCI400_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32	NX_CCI400_GetPhysicalAddress( void );
U32	NX_CCI400_GetSizeOfRegisterSet( void );

void	NX_CCI400_SetBaseAddress( void* BaseAddress );
void*	NX_CCI400_GetBaseAddress( void );
CBOOL	NX_CCI400_OpenModule( void );
CBOOL	NX_CCI400_CloseModule( void );
CBOOL	NX_CCI400_CheckBusy( void );
CBOOL	NX_CCI400_CanPowerDown( void );
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


#endif // __NX_CCI400_H__
