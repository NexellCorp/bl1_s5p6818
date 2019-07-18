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

#include <sysheader.h>

#include <gpio.h>

static struct s5p6818_gpio_reg (*const g_gpio_reg)[1] =
    (struct s5p6818_gpio_reg(*)[])(PHY_BASEADDR_GPIOA_MODULE);

void gpio_set_alt_function(unsigned int alt_num)
{
	register struct s5p6818_gpio_reg *gpio_reg =
		(struct s5p6818_gpio_reg *)(&g_gpio_reg[(alt_num >> 8) & 0x7]);

	int reg_num = (alt_num >> 7) & 0x1;
	int alt_clear = (((alt_num >> 3) & 0xF) * 2);
	int reg_value = 0;

	reg_value = mmio_read_32(&gpio_reg->altfn[reg_num]);
	reg_value &= ~(0x3UL << alt_clear);
	reg_value |= ((alt_num & 0x3) << alt_clear);
	mmio_write_32(&gpio_reg->altfn[reg_num], reg_value);
}

void gpio_set_pad_function(u32 grp, u32 io, u32 padfunc)
{
	register struct s5p6818_gpio_reg *gpio_reg =
		(struct s5p6818_gpio_reg *)(&g_gpio_reg[grp]);

	mmio_clear_32(&gpio_reg->altfn[io >> 4], (3 << ((io & 0xF) << 1)));
	mmio_set_32  (&gpio_reg->altfn[io >> 4], (padfunc << ((io & 0xF) << 1)));
}

void gpio_set_output_value(u32 grp, u32 io, int value)
{
	register struct s5p6818_gpio_reg *gpio_reg =
		(struct s5p6818_gpio_reg *)(&g_gpio_reg[grp]);

	if (value)
		mmio_set_32(&gpio_reg->out,	(1 << io));
	else
		mmio_clear_32(&gpio_reg->out, (1 << io));
}

void gpio_set_output_enable(u32 grp, u32 io, int en)
{
	register struct s5p6818_gpio_reg *gpio_reg =
		(struct s5p6818_gpio_reg *)(&g_gpio_reg[grp]);

	if (en)
		mmio_set_32(&gpio_reg->outenb,	(1 << io));
	else
		mmio_clear_32(&gpio_reg->outenb,	(1 << io));
}
