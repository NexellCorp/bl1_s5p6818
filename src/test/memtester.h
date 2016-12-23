/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 * Author: Deokjin, Lee <truevirtue@nexell.co.kr>
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

/* Baic Sizes */
static unsigned int y = 1U;

unsigned int rand_r(unsigned int *seedp)
{
    *seedp ^= (*seedp << 13);
    *seedp ^= (*seedp >> 17);
    *seedp ^= (*seedp << 5);

    return *seedp;
}

unsigned int rand(void)
{
    return rand_r(&y);
}

void srand(unsigned int seed)
{
    y = seed;
}
#if 0
#define rand32() ((unsigned int) rand() | ( (unsigned int) rand() << 16))
#define rand_ul() rand32()
#else
unsigned int rand_ul(void)
{
	//return (rand() | rand() << 16);
	return 1;
}
#endif

#define UL_ONEBITS	0xffffffff
#define UL_LEN		32
#define CHECKERBOARD1	0x55555555
#define CHECKERBOARD2	0xaaaaaaaa
#define UL_BYTE(x) ((x | x << 8 | x << 16 | x << 24))

/* Basic Types  */
typedef unsigned long ul;
typedef unsigned long long ull;
typedef unsigned long volatile ulv;
typedef unsigned char volatile u8v;
typedef unsigned short volatile u16v;

struct test {
    char *name;
    int (*fp)(ulv*, ulv*, int);
};

union {
    unsigned char bytes[UL_LEN/8];
    ul val;
} mword8;

union {
    unsigned short u16s[UL_LEN/16];
    ul val;
} mword16;

#define TEST_NARROW_WRITES

/* Function declaration. */
int test_stuck_address(unsigned long volatile *bufa, int count);
int test_random_value(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_xor_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_sub_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_mul_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_div_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_or_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_and_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_seqinc_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_solidbits_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_checkerboard_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_blockseq_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_walkbits0_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_walkbits1_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_bitspread_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_bitflip_comparison(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
#ifdef TEST_NARROW_WRITES
int test_8bit_wide_random(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
int test_16bit_wide_random(unsigned long volatile *bufa, unsigned long volatile *bufb, int count);
#endif
