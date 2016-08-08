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
#ifndef __NX_TZPC_H__
#define __NX_TZPC_H__

#include "../base/nx_prototype.h"

#ifdef	__cplusplus
extern "C"
{
#endif

#define NX_TZPC_MODULE_NUMBER	7
#define NX_TZPC_PORT			4
//------------------------------------------------------------------------------
/// @defgroup	TZPC Base
//------------------------------------------------------------------------------
//@{
	struct NX_TZPC_DETPROT
	{
		volatile U32 STATUS;			// Decode Protection Status
		volatile U32 SET;				// Decode Protect Set
		volatile U32 CLEAR;				// Decode Protect Clear
	};

	struct	NX_TZPC_RegisterSet
	{
		volatile U32 R0SIZE;							// 0x000 RAM Region Size
		volatile U32 _Reserved0[0x1FF];					// 0x004~0x7FC
		struct NX_TZPC_DETPROT DETPROT[NX_TZPC_PORT];	// 0x800~0x82C
		volatile U32 _Reserved1[0x1F8-(sizeof(struct NX_TZPC_DETPROT)/4)*NX_TZPC_PORT];		// 0x830~0xFDC
		volatile U32 PERIPHID[4];						// 0xFE0~0xFEC
		volatile U32 PCELLID[4];						// 0xFF0~0xFFC
	};

enum
{
	NX_TZPC_SMODE_SECURE	= 0,
	NX_TZPC_SMODE_NONSECURE	= 1
}NX_TZPC_SMODE;

typedef enum
{
	NX_TZPC_R0SIZE_NONE	= 0,
	NX_TZPC_R0SIZE_4KB	= 1,
	NX_TZPC_R0SIZE_8KB	= 2,
	NX_TZPC_R0SIZE_12KB	= 3,
	NX_TZPC_R0SIZE_16KB	= 4,
	NX_TZPC_R0SIZE_20KB	= 5,
	NX_TZPC_R0SIZE_24KB	= 6,
	NX_TZPC_R0SIZE_28KB	= 7,
	NX_TZPC_R0SIZE_32KB	= 8,
	NX_TZPC_R0SIZE_36KB = 9,
	NX_TZPC_R0SIZE_40KB = 10,
	NX_TZPC_R0SIZE_44KB = 11,
	NX_TZPC_R0SIZE_48KB = 12,
	NX_TZPC_R0SIZE_52KB = 13,
	NX_TZPC_R0SIZE_56KB = 14,
	NX_TZPC_R0SIZE_60KB = 15,
	NX_TZPC_R0SIZE_64KB = 16,
	NX_TZPC_R0SIZE_2MB	= 0x1FF,
	NX_TZPC_R0SIZE_ALL	= 0x200
}NX_TZPC_R0SIZE;

//------------------------------------------------------------------------------
/// @name	Module Interface
//@{
CBOOL	NX_TZPC_Initialize( void );
U32		NX_TZPC_GetNumberOfModule( void );
//@}

//------------------------------------------------------------------------------
///	@name	Basic Interface
//@{
U32		NX_TZPC_GetPhysicalAddress( void );
U32		NX_TZPC_GetSizeOfRegisterSet( void );

void	NX_TZPC_SetBaseAddress( void* BaseAddress );
void*	NX_TZPC_GetBaseAddress( void );

CBOOL	NX_TZPC_OpenModule( void );
CBOOL	NX_TZPC_CloseModule( void );
CBOOL	NX_TZPC_CheckBusy( void );
CBOOL	NX_TZPC_CanPowerDown( void );
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


#endif // __NX_TZPC_H__

