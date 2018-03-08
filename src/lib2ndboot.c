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
#include <nx_type.h>
#include "nx_peridot.h"

#ifdef aarch32
U32 getquotient(U32 dividend, U32 divisor)
{
	U32 quotient, remainder;
	U32 t, num_bits;
	U32 q, bit, d = 0;
	U32 i;

	remainder = 0;
	quotient = 0;

	if (divisor == 0)
		return -1;

	if (divisor > dividend) {
		return 0;
	}

	if (divisor == dividend) {
		return 1;
	}

	num_bits = 32;

	while (remainder < divisor) {
		bit = (dividend & 0x80000000) >> 31;
		remainder = (remainder << 1) | bit;
		d = dividend;
		dividend = dividend << 1;
		num_bits--;
	}

	dividend = d;
	remainder = remainder >> 1;
	num_bits++;

	for (i = 0; i < num_bits; i++) {
		bit = (dividend & 0x80000000) >> 31;
		remainder = (remainder << 1) | bit;
		t = remainder - divisor;
		q = !((t & 0x80000000) >> 31);
		dividend = dividend << 1;
		quotient = (quotient << 1) | q;
		if (q) {
			remainder = t;
		}
	}
	return quotient;
}

U32 getremainder(U32 dividend, U32 divisor)
{
	U32 quotient, remainder;
	U32 t, num_bits;
	U32 q, bit, d = 0;
	U32 i;

	remainder = 0;
	quotient = 0;

	if (divisor == 0)
		return -1;

	if (divisor > dividend) {
		return dividend;
	}

	if (divisor == dividend) {
		return 0;
	}

	num_bits = 32;

	while (remainder < divisor) {
		bit = (dividend & 0x80000000) >> 31;
		remainder = (remainder << 1) | bit;
		d = dividend;
		dividend = dividend << 1;
		num_bits--;
	}

	dividend = d;
	remainder = remainder >> 1;
	num_bits++;

	for (i = 0; i < num_bits; i++) {
		bit = (dividend & 0x80000000) >> 31;
		remainder = (remainder << 1) | bit;
		t = remainder - divisor;
		q = !((t & 0x80000000) >> 31);
		dividend = dividend << 1;
		quotient = (quotient << 1) | q;
		if (q) {
			remainder = t;
		}
	}
	return remainder;
}
#endif

#ifdef aarch64
U32 getquotient(U32 dividend, U32 divisor) { return dividend / divisor; }
U32 getremainder(U32 dividend, U32 divisor) { return dividend % divisor; }
#endif

void DMC_Delay(int milisecond)
{
	register volatile int count, temp;

	for (count = 0; count < milisecond; count++) {
		temp ^= count;
	}
}
