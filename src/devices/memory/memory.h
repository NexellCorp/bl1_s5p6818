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
#ifndef __MEMORY_H__
#define __MEMORY_H__

void enter_self_refresh(void);
void exit_self_refresh(void);

/* (ddr3/lpdde3) sdram memory function define */
#ifdef MEM_TYPE_DDR3
 int ddr3_initialize(unsigned int);
#endif
#ifdef MEM_TYPE_LPDDR23
 int lpddr3_initialize(unsigned int);
#endif

 int memory_initialize(int is_resume);

#endif
