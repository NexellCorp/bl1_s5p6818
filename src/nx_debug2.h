/*
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved
 *      Nexell Co. Proprietary & Confidential
 *
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 *FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      Module          : Base
 *      File            : nx_debug.h
 *      Description     : support assert, trace routine
 *      Author          : Goofy
 *      History         : 2017.04.04 Goofy First draft
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
