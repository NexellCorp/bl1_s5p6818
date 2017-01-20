/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: DeokJin, Lee <truevirtue@nexell.co.kr>
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
#ifndef __TYPE_H__
#define __TYPE_H__

typedef char				int8_t;					///< 8bit signed integer(s.7) value
typedef short				int16_t;				///< 16bit signed integer(s.15) value
typedef int				int32_t;				///< 32bit signed integer(s.31) value
typedef long long			int64_t;				///< 64bit signed integer(s.63) value
typedef unsigned char			uint8_t;				///< 8bit unsigned integer value
typedef unsigned short			uint16_t;				///< 16bit unsigned integer value
typedef unsigned int			uint32_t;				///< 32bit unsigned integer value
typedef unsigned long long		uint64_t;				///< 64bit unsigned integer vlaue

#define u8				unsigned char
#define u16				unsigned short
#define u32				unsigned int
#define u64				unsigned long long

#define s8				char
#define s16				short
#define s32				int
#define s64				long long

#define true				0
#define false				-1

#define TRUE				1
#define FALSE				0

#define IO_ADDRESS(x)			(x)

#define mmio_read_32(addr)		(*(volatile unsigned int  *)(addr))
#define mmio_read_16(addr)		(*(volatile unsigned short*)(addr))
#define mmio_read_8(addr)		(*(volatile unsigned char *)(addr))

#define mmio_write_32(addr,data)	(*(volatile unsigned int  *)(addr))  =  ((unsigned int  )(data))
#define mmio_write_16(addr,data)	(*(volatile unsigned short*)(addr))  =  ((unsigned short)(data))
#define mmio_write_8(addr,data)		(*(volatile unsigned char *)(addr))  =  ((unsigned char )(data))

#define mmio_set_32(addr,data)		(*(volatile unsigned int  *)(addr)) |=  ((unsigned int  )(data))
#define mmio_set_16(addr,data)		(*(volatile unsigned short*)(addr)) |=  ((unsigned short)(data))
#define mmio_set_8(addr,data)		(*(volatile unsigned char *)(addr)) |=  ((unsigned char )(data))

#define mmio_clear_32(addr,data)	(*(volatile unsigned int  *)(addr)) &= ~((unsigned int  )(data))
#define mmio_clear_16(addr,data)	(*(volatile unsigned short*)(addr)) &= ~((unsigned short)(data))
#define mmio_clear_8(addr,data)		(*(volatile unsigned char *)(addr)) &= ~((unsigned char )(data))

#define readb(addr)			mmio_read_8(addr)
#define readw(addr)			mmio_read_16(addr)
#define readl(addr)			mmio_read_32(addr)

#define writeb(data, addr)		({U8  *_v = (U8 *)addr;  mmio_write_8(_v, data);})
#define writew(data, addr)		({unsigned short *_v = (unsigned short *)addr; mmio_write_16(_v, data);})
#define writel(data, addr)		({unsigned int *_v = (unsigned int *)addr; mmio_write_32(_v, data);})

#endif	// __TYPE_H__
