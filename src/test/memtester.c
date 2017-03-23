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
#include <memtester.h>
#include <ddr3_sdram.h>

/* External Variable */
extern struct dram_device_info g_ddr3_info;

#if defined(STANDARD_MEMTEST)

#if (0)
#define pr_dbg(msg...) printf(msg)
#else
#define pr_dbg(msg...)							       \
	do {								       \
	} while (0)
#endif

#define MEMTEST_UNSUPPORT_FEATUE	1
#define MEMTEST_SINGLE_DBG		0
#define MEMTEST_CORE_NUMBER		8

/* TEST Main  */
char progress[] = "-\\|/";
#define PROGRESSLEN 			4
#define PROGRESSOFTEN			2500
#define ONE				0x00000001L

struct test tests[] = {
			{"Random Value", test_random_value},
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

int use_phys		= 0;
long physaddrbase 	= 0;

int __aeabi_idivmod(void)
{
	return 0;
}

static void fflush(void)
{
	while(!serial_done());
}

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
				printf("FAILURE: 0x%08X(0x%08X) != 0x%08X(0x%08X) at "
						"physical address 0x%08X.\r\n",
						(ul)*p1, p1,  (ul)*p2, p2, (ul)physaddr);
			} else {
				printf("FAILURE: 0x%08X != 0x%08X at offset "
						"0x%08X.\r\n",
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
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		p1 = (ulv *)bufa;
		printf("setting %3u", j);
		for (i = 0; i < count; i++) {
			*p1 = ((j + i) % 2) == 0 ? (ul)p1 : ~((ul)p1);
			*p1++;
		}
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		printf("testing %3u", j);
		fflush();
		p1 = (ulv *)bufa;
		for (i = 0; i < count; i++, p1++) {
			if (*p1 != (((j + i) % 2) == 0 ? (ul)p1 : ~((ul)p1))) {
				if (use_phys) {
					physaddr =
						physaddrbase + (i * sizeof(ul));
					printf("FAILURE: possible bad address "
							"line at physical "
							"address 0x%08X.\r\n",
							physaddr);
				} else {
					printf("FAILURE: possible bad address "
							"line at offset "
							"0x%08X.\r\n",
							(ul)(i * sizeof(ul)));
				}
				printf("Skipping to next test...\r\n");
				return -1;
			}
		}
	}
	printf("\b\b\b\b\b\b\b\b\b\b\b		 \b\b\b\b\b\b\b\b\b\b\b");

	return 0;
}
#endif // MEMTEST_UNSUPPORT_FEATUE

int test_random_value(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	int i, j = 0;

	putchar(' ');
	fflush();
	for (i = 0; i < count; i++) {
		*p1++ = *p2++ = (ulv)rand_ul();
		if (!(i % PROGRESSOFTEN)) {
			printf("\b");
			printf("%c", progress[++j % PROGRESSLEN]);
			fflush();
		}
	}
	printf("\b \b");

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
	ul q;
	int i;
	unsigned int j = 0;

	printf("	   ");
	fflush();

	for (j = 0; j < 64; j++) {
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		q = (j % 2) == 0 ? UL_ONEBITS : 0;
		printf("setting %3u", j);
		fflush();
		p1 = (ulv *)bufa;
		p2 = (ulv *)bufb;
		for (i = 0; i < count; i++) {
			*p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
		}
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		printf("testing %3u", j);
		fflush();
		if (compare_regions(bufa, bufb, count))
			return -1;

	}
	printf("\b\b\b\b\b\b\b\b\b\b\b		 \b\b\b\b\b\b\b\b\b\b\b");
	printf("\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
	fflush();

	return 0;
}

int test_checkerboard_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	ul q;
	int i;

	printf("	   ");
	fflush();

	for (j = 0; j < 64; j++) {
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;
		printf("setting %3u", j);
		fflush();
		p1 = (ulv *) bufa;
		p2 = (ulv *) bufb;
		for (i = 0; i < count; i++) {
			*p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
		}
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		printf("testing %3u", j);
		fflush();
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}
	printf("\b\b\b\b\b\b\b\b\b\b\b		 \b\b\b\b\b\b\b\b\b\b\b");
	printf("\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
	fflush();

	return 0;
}

int test_blockseq_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	int i;

	printf("	   ");
	fflush();

	for (j = 0; j < 256; j++) {
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		p1 = (ulv *) bufa;
		p2 = (ulv *) bufb;
		printf("setting %3u", j);
		fflush();
		for (i = 0; i < count; i++) {
			*p1++ = *p2++ = (ul) UL_BYTE(j);
		}
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		printf("testing %3u", j);
		fflush();
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}
	printf("\b\b\b\b\b\b\b\b\b\b\b		 \b\b\b\b\b\b\b\b\b\b\b");
	printf("\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
	fflush();

	return 0;
}

int test_walkbits0_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	int i;

	printf("	   ");
	fflush();

	for (j = 0; j < UL_LEN * 2; j++) {
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		p1 = (ulv *) bufa;
		p2 = (ulv *) bufb;
		printf("setting %3u", j);
		fflush();
		for (i = 0; i < count; i++) {
			if (j < UL_LEN) { /* Walk it up. */
				*p1++ = *p2++ = ONE << j;
			} else { /* Walk it back down. */
				*p1++ = *p2++ = ONE << (UL_LEN * 2 - j - 1);
			}
		}
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		printf("testing %3u", j);
		fflush();
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}
	printf("\b\b\b\b\b\b\b\b\b\b\b		 \b\b\b\b\b\b\b\b\b\b\b");
	printf("\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
	fflush();

	return 0;
}

int test_walkbits1_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	int i;

	printf("	   ");
	fflush();

	for (j = 0; j < UL_LEN * 2; j++) {
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		p1 = (ulv *) bufa;
		p2 = (ulv *) bufb;
		printf("setting %3u", j);
		fflush();
		for (i = 0; i < count; i++) {
			if (j < UL_LEN) { /* Walk it up. */
				*p1++ = *p2++ = UL_ONEBITS ^ (ONE << j);
			} else { /* Walk it back down. */
				*p1++ = *p2++ = UL_ONEBITS ^ (ONE << (UL_LEN * 2 - j - 1));
			}
		}
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		printf("testing %3u", j);
		fflush();
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}
	printf("\b\b\b\b\b\b\b\b\b\b\b		 \b\b\b\b\b\b\b\b\b\b\b");
	printf("\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
	fflush();

	return 0;
}

int test_bitspread_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j;
	int i;

	printf("	   ");
	fflush();

	for (j = 0; j < UL_LEN * 2; j++) {
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		p1 = (ulv *) bufa;
		p2 = (ulv *) bufb;
		printf("setting %3u", j);
		fflush();
		for (i = 0; i < count; i++) {
			if (j < UL_LEN) { /* Walk it up. */
				(unsigned int)(*p1++ = *p2++ = (i % 2 == 0))
					? (unsigned int)((ONE << j) | (ONE << (j + 2)))
					: (unsigned int)(UL_ONEBITS ^ ((ONE << j)
								| (ONE << (j + 2))));
			} else { /* Walk it back down. */
				(unsigned int)(*p1++ = *p2++ = (i % 2 == 0))
					? (unsigned int)((ONE << (UL_LEN * 2 - 1 - j)) | (ONE << (UL_LEN * 2 + 1 - j)))
					: (unsigned int)(UL_ONEBITS ^ (ONE << (UL_LEN * 2 - 1 - j)
								| (ONE << (UL_LEN * 2 + 1 - j))));
			}
		}
		printf("\b\b\b\b\b\b\b\b\b\b\b");
		printf("testing %3u", j);
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}
	printf("\b\b\b\b\b\b\b\b\b\b\b		 \b\b\b\b\b\b\b\b\b\b\b");
	printf("\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
	fflush();

	return 0;
}

int test_bitflip_comparison(ulv *bufa, ulv *bufb, int count)
{
	ulv *p1 = bufa;
	ulv *p2 = bufb;
	unsigned int j, k;
	ul q;
	int i;

	printf("	   ");
	fflush();

	for (k = 0; k < UL_LEN; k++) {
		q = ONE << k;
		for (j = 0; j < 8; j++) {
			printf("\b\b\b\b\b\b\b\b\b\b\b");
			q = ~q;
			printf("setting %3u", k * 8 + j);
			fflush();
			p1 = (ulv *) bufa;
			p2 = (ulv *) bufb;
			for (i = 0; i < count; i++) {
				*p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
			}
			printf("\b\b\b\b\b\b\b\b\b\b\b");
			printf("testing %3u", k * 8 + j);
			fflush();
			if (compare_regions(bufa, bufb, count)) {
				return -1;
			}
		}
	}
	printf("\b\b\b\b\b\b\b\b\b\b\b		 \b\b\b\b\b\b\b\b\b\b\b");
	printf("\b\b\b\b\b\b           \b\b\b\b\b\b\b\b\b\b\b");
	fflush();

	return 0;
}

#ifdef TEST_NARROW_WRITES
int test_8bit_wide_random(ulv *bufa, ulv *bufb, int count)
{
	u8v *p1, *t;
	ulv *p2;
	int attempt;
	unsigned int b, j = 0;
	int i;

	putchar(' ');
	fflush();
	for (attempt = 0; attempt < 2;	attempt++) {
		if (attempt & 1) {
			p1 = (u8v *) bufa;
			p2 = bufb;
		} else {
			p1 = (u8v *) bufb;
			p2 = bufa;
		}
		for (i = 0; i < count; i++) {
			t = mword8.bytes;
			*p2++ = mword8.val = rand_ul();
			for (b=0; b < UL_LEN/8; b++) {
				*p1++ = *t++;
			}
			if (!(i % PROGRESSOFTEN)) {
				putchar('\b');
				printf("%c", progress[++j % PROGRESSLEN]);
				fflush();
			}
		}
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}
	printf("\b \b");
	fflush();

	return 0;
}

int test_16bit_wide_random(ulv *bufa, ulv *bufb, int count)
{
	u16v *p1, *t;
	ulv *p2;
	int attempt;
	unsigned int b, j = 0;
	int i;

	putchar( ' ' );
	fflush();
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
				putchar('\b');
				printf("%c", progress[++j % PROGRESSLEN]);
				fflush();
			}
		}
		if (compare_regions(bufa, bufb, count)) {
			return -1;
		}
	}
	printf("\b \b");
	fflush();

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
int memtester_main(ulv bufa, ulv bufb, int repeat)
{
	int count = sizeof(tests) / sizeof(struct test) - 1;			// Sub - NULL(1)
	int pagesize = 1024;							// 1024, 2048
	int size = (bufb - bufa)/1000;///2 & pagesize;
	int i, k, ret = 0;

	srand(1024);
	g_mem_info.start_addr = bufa;
	g_mem_info.end_addr = bufb;

	for(k = 0; k < repeat; k++) {
		printf("Loop [%d]St. \r\n", k);
		printf("physaddrbase : 0x%08X, use_phys: 0x%08X \r\n",
				physaddrbase, use_phys);
		printf("bufa: 0x%08X, bufb: 0x%08X, size: 0x%04X \r\n",
			bufa, bufb, size);
		for (i = 0; i < count; i++) {
			if (!tests[i].name)
				break;
	#if 0 //MEMTEST_UNSUPPORT_FEATUE
			use_phys = 1;
			physaddrbase = bufa;
			printf("  %-20s: ", "Stuck_Address: ");
			test_stuck_address(bufa, size*2);
			printf("\r\n");
	#endif
			printf("  %-20s: ", tests[i].name);
			if (!tests[i].fp(((ulv *)bufa), ((ulv *)bufb), size)) {
				g_mem_info.memtest_done[i] = 0;
				printf("OK!!");
			}
			else {
				g_mem_info.memtest_done[i] = -1;
				ret = -1;
				printf("Failed!!");
			}
			printf("\r\n");
		}
		if (ret < 0)
			break;
	}

	if (ret < 0)
		printf("Memtest Failed!! \r\n");
	else
		printf("Memtest Success!! \r\n");

	return ret;
}

int standard_memtester(void)
{
	int bufa = 0x40000000;
	int bufb = (bufa + (g_ddr3_info.sdram_size * 1024 * 1024 / 2));	//MB -> Byte
	int repeat = 0x1;

	use_phys = 1;
	physaddrbase = bufa;

	return memtester_main(bufa, bufb, repeat);
}

#elif defined(SIMPLE_MEMTEST)

#define MPTRS unsigned int

void simple_memtest(void)
{
	unsigned int* start, *end, *ptr;
	unsigned int  size;
	int repeat = 0x1;

	start = ((unsigned int *)0x40000000);
	end   = ((unsigned int *)(0x40000000
			+ (g_ddr3_info.sdram_size * 1024 * 1024 - 1)));	//MB -> Byte
	ptr = start;
	size = (unsigned int)(end - start) * sizeof(unsigned int);

	printf("############## Simple Memory Test Start!! ###############\r\n");
	printf("Start: 0x%08X, End: 0x%08X, Size: 0x%08X \r\n",
		start, end, size);

	/* step xx. data write */
	printf("Read/Write : ");
	printf("Write  ");
	while (ptr < end) {
		*ptr = (unsigned int)((MPTRS)ptr);
#if 0
		if (((unsigned int)((MPTRS)ptr) & 0x3FFFFFL) == 0)
			printf("0x%16X:\r\n", ptr);
#endif
#if 0
		if (((unsigned int)((MPTRS)ptr) % PROGRESSOFTEN) == 0) {
			printf("\b");
			printf("%c", progress[++j%  PROGRESSLEN]);
		}
#endif
		ptr++;
	}
	printf("\b\b\b\b\b\b\b");

	printf("Compare  ");
	ptr = start;
	while (ptr < end) {
#if 0
		if (*ptr != (unsigned int)((MPTRS)ptr))
			printf("0x%08X: %16X\r\n", (unsigned int)((MPTRS)ptr), *ptr);
#else
		unsigned int data0 = *ptr, data1 = (unsigned int)((MPTRS)ptr);
		unsigned int i = 0;

		for (i = 0; i < 32; i++) {
			data0 &= 1UL << i;
			data1 &= 1UL << i;

			if (data0 != data1) {
//				printf("[%dbit] 0x%08X: %08X\r\n", i,
//				(unsigned int)((MPTRS)ptr), *ptr);
				printf("--------------------------------------"
						"\r\n");
				printf("[%dbit] 0x%08X: %08X(0x%08X: %08X)\r\n",
						i, data1, data0, (unsigned int)((MPTRS)ptr),
						*ptr);
				printf("--------------------------------------"
						"\r\n");
//				mask_bit |= 1UL << i;
			}
#if 0
			if ( (mask_bit != 0) && (i == 31) ) {
				printf("[%Xbit] 0x%08X: %08X(0x%08X: %08X)\r\n",
						mask_bit, data1, data0, (unsigned int)((MPTRS)ptr), *ptr);
			}
#endif
		}

		ptr++;
#endif

#if 0
		if ((((MPTRS)ptr) & 0xFFFFFL) == 0)
			printf("0x%16X:\r\n", ptr);
#endif
	}
	printf("\b\b\b\b\b\b\b\b\b");
	printf("Done!   \r\n");

	/* step xx. bit shift test */
	printf("Bit Shift  : ");
	printf("Write  ");
	ptr = start;
	while (ptr < end) {
		*ptr = (1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2));
		ptr++;
	}
	printf("\b\b\b\b\b\b\b");

	printf("Compare  ");
	ptr = start;
	while (ptr < end) {
		if (*ptr != (1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2)))
			printf("0x%16x : 0x%16x\r\n", ptr, *ptr);
		ptr++;
	}
	printf("\b\b\b\b\b\b\b\b\b");
	printf("Done!   \r\n");

	/* step xx. reserve bit test */
	printf("Reverse Bit: ");
	printf("Write  ");
	ptr = start;
	while (ptr < end) {
		*ptr = ~(1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2));
		ptr++;
	}
	printf("\b\b\b\b\b\b\b");

	printf("Compare  ");
	ptr = start;
	while (ptr < end) {
		if (*ptr != ~(1UL << ((((MPTRS)ptr) & 0x1F << 2) >> 2)))
			printf("0x%16x : 0x%16x\r\n", ptr, *ptr);
		ptr++;
	}
	printf("\b\b\b\b\b\b\b\b\b");
	printf("Done!   \r\n");

	printf("############## Simple Memory Test Done!!! ###############\r\n");
}
#endif
