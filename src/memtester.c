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
#include "sysheader.h"
#include "memtester.h"

#if (0)
#define pr_dbg(msg...) printf(msg)
#else
#define pr_dbg(msg...)							       \
	do {								       \
	} while (0)
#endif

#define MEMTEST_SINGLE_DBG 0
#define MEMTEST_CORE_NUMBER 8

/* TEST Main  */
char progress[] = "-\\|/";
#define PROGRESSLEN 4
#define PROGRESSOFTEN 2500
#define ONE 0x00000001L

struct test tests[] = {{"Random Value", test_random_value},
		       {"Compare XOR", test_xor_comparison},
		       {"Compare SUB", test_sub_comparison},
		       {"Compare MUL", test_mul_comparison},
		       {"Compare DIV", test_div_comparison},
		       {"Compare OR", test_or_comparison},
		       {"Compare AND", test_and_comparison},
		       {"Sequential Increment", test_seqinc_comparison},
		       {"Solid Bits", test_solidbits_comparison},
		       {"Block Sequential", test_blockseq_comparison},
		       {"Checkerboard", test_checkerboard_comparison},
		       {"Bit Spread", test_bitspread_comparison},
		       {"Bit Flip", test_bitflip_comparison},
		       {"Walking Ones", test_walkbits1_comparison},
		       {"Walking Zeroes", test_walkbits0_comparison},
#ifdef TEST_NARROW_WRITES
		       {"8-bit Writes", test_8bit_wide_random},
		       {"16-bit Writes", test_16bit_wide_random},
#endif
		       {0, 0}};

int use_phys = 0;
long physaddrbase = 0;

/* Function definitions. */
int compare_regions(ulv *bufa, ulv *bufb, int count)
{
	int r = 0;
	int i;
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int physaddr = 0;
	// warning prvent.
	physaddr = physaddr;

	for (i = 0; i < count; i++, p1++, p2++) {
		if (*p1 != *p2) {
			if (use_phys) {
				physaddr = physaddrbase + (i * sizeof(ul));
				pr_dbg("FAILURE: 0x%08lx != 0x%08lx at "
				       "physical address 0x%08lx.\r\n",
				       (ul)*p1, (ul)*p2, (ul)physaddr);
			} else {
				printf("FAILURE: address : 0x%p  0x%p\n", p1,
				       p2);

				printf("FAILURE: 0x%08lx != 0x%08lx at offset "
				       "0x%08lx.\r\n",
				       (ul)*p1, (ul)*p2, (ul)(i * sizeof(ul)));
			}
			r = -1;
		}
	}

	return r;
}

#if MEMTEST_UNSUPPORT_FEATUE
int test_stuck_address(ulv *bufa, int count)
{
	ulv *p1 = bufa;
	unsigned int j;
	int i;
	int physaddr;

	for (j = 0; j < 16; j++) {
		p1 = (ulv *)bufa;
		for (i = 0; i < count; i++) {
			*p1 = ((j + i) % 2) == 0 ? (ul)p1 : ~((ul)p1);
			*p1++;
		}

		p1 = (ulv *)bufa;
		for (i = 0; i < count; i++, p1++) {
			if (*p1 != (((j + i) % 2) == 0 ? (ul)p1 : ~((ul)p1))) {
				if (use_phys) {
					physaddr =
					    physaddrbase + (i * sizeof(ul));
					printf("FAILURE: possible bad address "
					       "line at physical "
					       "address 0x%08lx.\r\n",
					       physaddr);
				} else {
					printf("FAILURE: possible bad address "
					       "line at offset "
					       "0x%08lx.\r\n",
					       (ul)(i * sizeof(ul)));
				}
				printf("Skipping to next test...\r\n");
				return -1;
			}
		}
	}
;
	return 0;
}
#endif // MEMTEST_UNSUPPORT_FEATUE

int test_random_value(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i;

	for (i = 0; i < count; i++) {
		*p1++ = *p2++ = (ulv)rand_ul();

		if (!(i % PROGRESSOFTEN)) {
			printf("\b");
			printf("progress[++j  PROGRESSLEN]\r\n");
		}
	}

	return compare_regions(bufa, bufb, count);
}

int test_xor_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i;
	ul q = rand_ul();

	for (i = 0; i < count; i++) {
		*p1++ ^= q;
		*p2++ ^= q;
	}
	return compare_regions(bufa, bufb, count);
}

int test_sub_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i;
	ul q = rand_ul();

	for (i = 0; i < count; i++) {
		*p1++ -= q;
		*p2++ -= q;
	}
	return compare_regions(bufa, bufb, count);
}

int test_mul_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i;
	ul q = rand_ul();

	for (i = 0; i < count; i++) {
		*p1++ *= q;
		*p2++ *= q;
	}
	return compare_regions(bufa, bufb, count);
}

int test_div_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i;
	ul q = rand_ul();

	for (i = 0; i < count; i++) {
		if (!q) {
			q++;
		}
		*p1++ /= q;
		*p2++ /= q;
	}
	return compare_regions(bufa, bufb, count);
}

int test_or_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i;
	ul q = rand_ul();

	for (i = 0; i < count; i++) {
		*p1++ |= q;
		*p2++ |= q;
	}
	return compare_regions(bufa, bufb, count);
}

int test_and_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i;
	ul q = rand_ul();

	for (i = 0; i < count; i++) {
		*p1++ &= q;
		*p2++ &= q;
	}
	return compare_regions(bufa, bufb, count);
}

int test_seqinc_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i;
	ul q = rand_ul();

	for (i = 0; i < count; i++) {
		*p1++ = *p2++ = (i + q);
	}
	return compare_regions(bufa, bufb, count);
}

int test_solidbits_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	ul q;
	int i;

	for (j = 0; j < 64; j++) {
		q = (j % 2) == 0 ? UL_ONEBITS : 0;

		p1 = (ulv *)bufa;
		p2 = (ulv *)bufb;
		for (i = 0; i < count; i++) {
			*p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
		}
		
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}

	return 0;
}

int test_checkerboard_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	ul q;
	int i;

	for (j = 0; j < 64; j++) {
		q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;

		p1 = (ulv *)bufa;
		p2 = (ulv *)bufb;
		for (i = 0; i < count; i++) {
			*p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
		}
		
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}

	return 0;
}

int test_blockseq_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	int i;

	for (j = 0; j < 256; j++) {
		p1 = (ulv *)bufa;
		p2 = (ulv *)bufb;
		
		for (i = 0; i < count; i++) {
			*p1++ = *p2++ = (ul)UL_BYTE(j);
		}
		
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}

	return 0;
}

int test_walkbits0_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	int i;

	for (j = 0; j < UL_LEN * 2; j++) {
		p1 = (ulv *)bufa;
		p2 = (ulv *)bufb;

		for (i = 0; i < count; i++) {
			if (j < UL_LEN) { 			/* Walk it up. */
				*p1++ = *p2++ = ONE << j;
			} else { 				/* Walk it back down. */
				*p1++ = *p2++ = ONE << (UL_LEN * 2 - j - 1);
			}
		}


		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}

	return 0;
}

int test_walkbits1_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	int i;

	for (j = 0; j < UL_LEN * 2; j++) {
		p1 = (ulv *)bufa;
		p2 = (ulv *)bufb;

		for (i = 0; i < count; i++) {
			if (j < UL_LEN) { /* Walk it up. */
				*p1++ = *p2++ = UL_ONEBITS ^ (ONE << j);
			} else { /* Walk it back down. */
				*p1++ = *p2++ =
				    UL_ONEBITS ^ (ONE << (UL_LEN * 2 - j - 1));
			}
		}

		//	printf("testing %3u", j);
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}

	return 0;
}

int test_bitspread_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	int i;

	for (j = 0; j < UL_LEN * 2; j++) {
		p1 = (ulv *)bufa;
		p2 = (ulv *)bufb;

		for (i = 0; i < count; i++) {
			if (j < UL_LEN) { 					/* Walk it up. */
				*p1++ = *p2++ =
				    (i % 2 == 0)
					? (ONE << j) | (ONE << (j + 2))
					: (int)UL_ONEBITS ^			/* UL_ONEBITS (Type Define) */
					      ((ONE << j) | (ONE << (j + 2)));
			} else { 						/* Walk it back down. */
				*p1++ = *p2++ =
				    (i % 2 == 0)
					? (ONE << (UL_LEN * 2 - 1 - j)) |
					      (ONE << (UL_LEN * 2 + 1 - j))
					: (int)UL_ONEBITS ^
					      (ONE << (UL_LEN * 2 - 1 - j) |
					       (ONE << (UL_LEN * 2 + 1 - j)));
			}
		}


		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}

	return 0;
}

int test_bitflip_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j, k;
	ul q;
	int i;

	for (k = 0; k < UL_LEN; k++) {
		q = ONE << k;
		for (j = 0; j < 8; j++) {
			q = ~q;

			p1 = (ulv *)bufa;
			p2 = (ulv *)bufb;
			for (i = 0; i < count; i++) {
				*p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
			}

			if (compare_regions(bufa, bufb, count)) {
				return -1;
			}
		}
	}

	return 0;
}

#ifdef TEST_NARROW_WRITES
int test_8bit_wide_random(ulv *bufa, ulv *bufb, int count)
{
	u8v *p1, *t;
	ulv *p2;
	int attempt;
	unsigned int b; //, j = 0;
	int i;

	for (attempt = 0; attempt < 2; attempt++) {
		if (attempt & 1) {
			p1 = (u8v *)bufa;
			p2 = bufb;
		} else {
			p1 = (u8v *)bufb;
			p2 = bufa;
		}
		for (i = 0; i < count; i++) {
			t = mword8.bytes;
			*p2++ = mword8.val = rand_ul();
			for (b = 0; b < UL_LEN / 8; b++) {
				*p1++ = *t++;
			}
			if (!(i % PROGRESSOFTEN)) {
				printf("\b");
				printf("progress[++j//%// PROGRESSLEN] \r\n");
			}
		}
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}

	return 0;
}

int test_16bit_wide_random(ulv *bufa, ulv *bufb, int count)
{
	u16v *p1, *t;
	ulv *p2;
	int attempt;
	unsigned int b; //, j = 0;
	int i;

	for (attempt = 0; attempt < 2; attempt++) {
		if (attempt & 1) {
			p1 = (u16v *)bufa;
			p2 = bufb;
		} else {
			p1 = (u16v *)bufb;
			p2 = bufa;
		}
		for (i = 0; i < count; i++) {
			t = mword16.u16s;
			*p2++ = mword16.val = rand_ul();
			for (b = 0; b < UL_LEN / 16; b++) {
				*p1++ = *t++;
			}
			if (!(i % PROGRESSOFTEN)) {
				printf("\b");
				printf("progress[++j//%// PROGRESSLEN] \r\n");

			}
		}
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}

	return 0;
}
#endif

/* --------------- MEMORY TEST MIAN CODE --------------  */
struct memtest_result_info {
	unsigned int memtest_done[17];
	unsigned int memtest_flag;
	unsigned int start_addr;
	unsigned int end_addr;
};

static struct memtest_result_info g_mem_info;

ulv g_bufa[8];
ulv g_bufb[8];
/*
 * each of the memory test function. (single)
 */
int memtester_main(unsigned int start, unsigned int end)
{
	ulv bufa = (ulv )start;
	ulv bufb = (ulv )end;
	
	int count = sizeof(tests) / sizeof(struct test) - 1; // Sub - NULL(1)
	int pagesize = 1024;
	int size = (end - start)/2 & pagesize;
	int i, ret = 0;

	srand(1024);
	g_mem_info.start_addr = start;
	g_mem_info.end_addr = end;

	for (i = 0; i < count; i++) {
		if (!tests[i].name)
			break;

		if (!tests[i].fp(((ulv *)bufa), ((ulv *)bufb), size))
			g_mem_info.memtest_done[i] = 0;
		else {
			g_mem_info.memtest_done[i] = -1;
			ret = -1;
		}
	}
	if (ret < 0)
		printf("memtest failed! \r\n");
	else
		printf("memtest success!! \r\n");

	return ret;
}

void simple_memtest(U32 *pStart, U32 *pEnd)
{
	volatile U32 *ptr = pStart;

	printf("memory test start!\r\n");

	printf("\r\nmemory write data to own address\r\n");
	while (ptr < pEnd) {
		*ptr = (U32)((MPTRS)ptr);
		#if 0
		if (((U32)((MPTRS)ptr) & 0x3FFFFFL) == 0)
			printf("0x%16X:\r\n", ptr);
		#endif
		ptr++;
	}

	printf("\r\nmemory compare with address and own data\r\n");
	ptr = pStart;
	while (ptr < pEnd) {
		if (*ptr != (U32)((MPTRS)ptr))
			printf("0x%08X: %16x\r\n", (U32)((MPTRS)ptr), *ptr);
		ptr++;
		#if 0
		if ((((MPTRS)ptr) & 0xFFFFFL) == 0)
			printf("0x%16X:\r\n", ptr);
		#endif
	}

	printf("bit shift test....\r\n");
	printf("write data....\r\n");
	ptr = pStart;
	while (ptr < pEnd) {
		*ptr = (1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2));
		ptr++;
	}
	printf("compare data....\r\n");
	ptr = pStart;
	while (ptr < pEnd) {
		if (*ptr != (1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2)))
			printf("0x%16x\r\n", *ptr);
		ptr++;
	}
	printf("reverse bit test\r\n");
	printf("write data....\r\n");
	ptr = pStart;
	while (ptr < pEnd) {
		*ptr = ~(1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2));
		ptr++;
	}
	printf("compare data....\r\n");
	ptr = pStart;
	while (ptr < pEnd) {
		if (*ptr != ~(1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2)))
			printf("0x%16x\r\n", *ptr);
		ptr++;
	}

	printf("\r\nmemory test done\r\n");
}
