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
#ifndef __NX_RSTCON_H__
#define __NX_RSTCON_H__

#include "../base/nx_prototype.h"

#define __def_RSTCON__RSTREGISTERCNT 1

#ifdef	__cplusplus
extern "C"
{
#endif

//------------------------------------------------------------------------------
/// @defgroup
//------------------------------------------------------------------------------
//@{

	struct	NX_RSTCON_RegisterSet
	{
		volatile U32	REGRST[(NUMBER_OF_RESET_MODULE_PIN+31)>>5];			//
	};

	typedef enum
	{
		RSTCON_ASSERT	= 0UL,
		RSTCON_NEGATE	= 1UL
	}RSTCON;


	CBOOL	NX_RSTCON_Initialize( void );
	U32 	NX_RSTCON_GetPhysicalAddress( void );
	U32		NX_RSTCON_GetSizeOfRegisterSet( void );

	void	NX_RSTCON_SetBaseAddress( void* BaseAddress );
	void*	NX_RSTCON_GetBaseAddress( void );

	void	NX_RSTCON_SetRST(U32 RSTIndex, RSTCON STATUS);
	RSTCON	NX_RSTCON_GetRST(U32 RSTIndex);



#ifdef	__cplusplus
}
#endif

#endif // __NX_RSTCON_H__
