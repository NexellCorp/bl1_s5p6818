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

#ifndef __NX_DEBUG_H__
#define __NX_DEBUG_H__

//------------------------------------------------------------------------------
//	compile mode check
//------------------------------------------------------------------------------
#ifdef NX_DEBUG
#ifdef _NX_COMPILE_MODE_
#error "NX_DEBUG and NX_RELEASE predefine must use exclusively!!!"
#endif

#define _NX_COMPILE_MODE_
#endif

#ifdef NX_RELEASE
#ifdef _NX_COMPILE_MODE_
#error "NX_DEBUG and NX_RELEASE predefine must use exclusively!!!"
#endif

#define _NX_COMPILE_MODE_
#endif

#ifndef _NX_COMPILE_MODE_
#error "You must have pre-definition ( NX_DEBUG or NX_RELEASE )!!!"
#endif

//------------------------------------------------------------------------------
#ifdef printf
#include <debug.h>

#define NX_DEBUG_MSG(_msg_) DebugPutString(_msg_)
#define NX_DEBUG_DEC(_val_) DebugPutDec(_val_)
#define NX_DEBUG_HEX(_val_) DebugPutHex(_val_)
#define NX_DEBUG_BYTE(_val_) DebugPutByte(_val_)
#define NX_DEBUG_WORD(_val_) DebugPutWord(_val_)
#define NX_DEBUG_DWORD(_val_) DebugPutDWord(_val_)
#define NX_DEBUG_PUTCH(_val_) DebugPutch(_val_)

#define NX_ASSERT(expr)                                                        \
	{                                                                      \
		if (!(expr)) {                                                 \
			NX_DEBUG_MSG(__FILE__);                                \
			NX_DEBUG_MSG("(");                                     \
			NX_DEBUG_DEC(__LINE__);                                \
			NX_DEBUG_MSG(") : ASSERT (");                          \
			NX_DEBUG_MSG((#expr));                                 \
			NX_DEBUG_MSG(")\n");                                   \
			while (1)                                              \
				;                                              \
		}                                                              \
	}
#else
#define NX_ASSERT(expr)                                                        \
	{                                                                      \
		if (!(expr)) {                                                 \
			printf("%s(%s) : ASSERT (%s)\r\n", __FILE__, __LINE__, \
			       (#expr));                                       \
		}                                                              \
	}
#endif

#endif //	__NX_DEBUG_H__
