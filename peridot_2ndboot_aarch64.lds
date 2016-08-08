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

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

ENTRY(Startup)

SECTIONS
{
	. = 0x00000000FFFF0200;
	. = ALIGN(4);
	.text :
	{
		obj/startup_aarch64.o	(.text)
		*(.text)
	}

	. = ALIGN(4);
	.rodata : { *(.rodata*) }

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);
	.got : { *(.got) }

	_end = .;

	. = ALIGN(8);
	__bss_start__ = .;
	.bss_start (OVERLAY) : {
		KEEP(*(.__bss_start));
		__bss_base = .;
	}
	.bss __bss_base (OVERLAY) : {
		*(.bss*)
		. = ALIGN(4);
		__bss_limit = .;
	}
	.bss_end __bss_limit (OVERLAY) : {
		KEEP(*(.__bss_end));
	}
	__bss_end__ = .;
}

