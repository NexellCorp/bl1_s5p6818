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
#ifndef __NX_HPM_H__
#define __NX_HPM_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup	High-Performance Matrix Base
//------------------------------------------------------------------------------
//@{
#define NUMBER_OF_HPM_MODULE	6

	struct NX_HPM_MI		// Master Interface
	{
		volatile U32 QoSTidemark;			// 0x000 QoS Tidemark for MIx
		volatile U32 QoSAC;					// 0x004 QoS Access Control for MIx
		volatile U32 ARChAV;				// 0x008 AR channel arbitration value for MIx
		volatile U32 AWChAV;				// 0x00C AW channel arbitration value for MIx
	};

	struct	NX_HPM_RegisterSet
	{
		volatile U32 _Reserved0[0x100];		// 0x000~0x3FC
		struct NX_HPM_MI MI[64];			// 0x400~0x7FC Master Interface
		volatile U32 _Reserved1[0x1F0];		// 0x800~0xFBC
		volatile U32 PCConfigR[4];			// 0xFC0~0xFCC PrimeCell Configuration
		volatile U32 _Reserved2[4];			// 0xFD0~0xFDC
		volatile U32 PCPeriR[4];			// 0xFE0~0xFEC
		volatile U32 PCIDR[4];				// 0xFF0~0xFFC
	};

typedef enum
{
	HPM_BUS_IOPERI		= 0,
	HPM_BUS_BOT		= 1,
	HPM_BUS_TOP		= 2,
	HPM_BUS_DISP		= 3,
	HPM_BUS_SFR		= 4,
	HPM_BUS_STATIC		= 5
}HPM_BUS;

enum
{
	HPM_IOPERI_AHBDMA	= 0,
	HPM_IOPERI_AHBUSB	= 1,
	HPM_IOPERI_AHBSDMMC	= 2,
	HPM_IOPERI_GMAC		= 3
};

enum
{
	HPM_STATIC_CCI		= 0,
	HPM_STATIC_IOPERIM0	= 1,
	HPM_STATIC_IOPERIM2	= 2
};

enum
{
	HPM_TOP_VIP0		= 0,
	HPM_TOP_VIP1		= 1,
	HPM_TOP_VIP2		= 2,
	HPM_TOP_IOPERI		= 3
};

enum
{
	HPM_SFR_CCI			= 0,
	HPM_SFR_AHBDMA		= 1
};

enum
{
	HPM_DISP_MLC0		= 0,
	HPM_DISP_MLC1		= 1
};

enum
{
	HPM_BOT_MALI		= 0,
	HPM_BOT_CODA		= 1,
	HPM_BOT_SCALER		= 2,
	HPM_BOT_DEINTERLACE	= 3
};
//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_HPM_Initialize( void );
U32		NX_HPM_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_HPM_GetPhysicalAddress( void );
U32		NX_HPM_GetSizeOfRegisterSet( void );

void	NX_HPM_SetBaseAddress( void* BaseAddress );
void*	NX_HPM_GetBaseAddress( void );

CBOOL	NX_HPM_OpenModule( void );
CBOOL	NX_HPM_CloseModule( void );
CBOOL	NX_HPM_CheckBusy( void );
CBOOL	NX_HPM_CanPowerDown( void );
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
U32	NX_HPM_GetMINumber(U32 ModuleIndex);
U32	NX_HPM_GetSINumber(U32 ModuleIndex);

//@}

#ifdef	__cplusplus
}
#endif


#endif // __NX_HPM_H__

