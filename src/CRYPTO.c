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

#include <nx_peridot.h>
#include <nx_type.h>
#include <nx_debug2.h>

#include <nx_chip.h>

#include <nx_crypto.h>
#include <nx_clkgen.h>
#include <nx_ecid.h>

#ifdef SW_CRYPTO_EMUL
U32 erk[64];
U32 drk[64]; /* decryption round keys */
int nr;      /* number of rounds */

/* uncomment the following line to use pre-computed tables */
/* otherwise the tables will be generated at the first run */

/* forward S-box & tables */

U32 FSb[256];
U32 FT0[256];
U32 FT1[256];
U32 FT2[256];
U32 FT3[256];

/* reverse S-box & tables */

U32 RSb[256];
U32 RT0[256];
U32 RT1[256];
U32 RT2[256];
U32 RT3[256];

/* round constants */

U32 RCON[10];

/* tables generation flag */

int do_init = 1;

/* tables generation routine */

#define ROTR8(x) (((x << 24) & 0xFFFFFFFF) | ((x & 0xFFFFFFFF) >> 8))

#define XTIME(x) ((x << 1) ^ ((x & 0x80) ? 0x1B : 0x00))
#define MUL(x, y) ((x && y) ? pow[(log[x] + log[y]) % 255] : 0)

void aes_gen_tables(void)
{
	int i;
	U8 x, y;
	U8 pow[256];
	U8 log[256];

	/* compute pow and log tables over GF(2^8) */

	for (i = 0, x = 1; i < 256; i++, x ^= XTIME(x)) {
		pow[i] = x;
		log[x] = i;
	}

	/* calculate the round constants */

	for (i = 0, x = 1; i < 10; i++, x = XTIME(x)) {
		RCON[i] = (U32)x << 24;
	}

	/* generate the forward and reverse S-boxes */

	FSb[0x00] = 0x63;
	RSb[0x63] = 0x00;

	for (i = 1; i < 256; i++) {
		x = pow[255 - log[i]];

		y = x;
		y = (y << 1) | (y >> 7);
		x ^= y;
		y = (y << 1) | (y >> 7);
		x ^= y;
		y = (y << 1) | (y >> 7);
		x ^= y;
		y = (y << 1) | (y >> 7);
		x ^= y ^ 0x63;

		FSb[i] = x;
		RSb[x] = i;
	}

	/* generate the forward and reverse tables */

	for (i = 0; i < 256; i++) {
		x = (U8)FSb[i];
		y = XTIME(x);

		FT0[i] = (U32)(x ^ y) ^ ((U32)x << 8) ^ ((U32)x << 16) ^
			 ((U32)y << 24);

		FT0[i] &= 0xFFFFFFFF;

		FT1[i] = ROTR8(FT0[i]);
		FT2[i] = ROTR8(FT1[i]);
		FT3[i] = ROTR8(FT2[i]);

		y = (unsigned char)RSb[i];

		RT0[i] = ((U32)MUL(0x0B, y)) ^ ((U32)MUL(0x0D, y) << 8) ^
			 ((U32)MUL(0x09, y) << 16) ^ ((U32)MUL(0x0E, y) << 24);

		RT0[i] &= 0xFFFFFFFF;

		RT1[i] = ROTR8(RT0[i]);
		RT2[i] = ROTR8(RT1[i]);
		RT3[i] = ROTR8(RT2[i]);
	}
}

/* platform-independant 32-bit integer manipulation macros */

#define GET_UINT32(n, b, i)                                                    \
	{                                                                      \
		(n) = ((U32)(b)[(i)] << 24) | ((U32)(b)[(i) + 1] << 16) |      \
		      ((U32)(b)[(i) + 2] << 8) | ((U32)(b)[(i) + 3]);          \
	}

#define PUT_UINT32(n, b, i)                                                    \
	{                                                                      \
		(b)[(i)] = (U8)((n) >> 24);                                    \
		(b)[(i) + 1] = (U8)((n) >> 16);                                \
		(b)[(i) + 2] = (U8)((n) >> 8);                                 \
		(b)[(i) + 3] = (U8)((n));                                      \
	}

/* decryption key schedule tables */

int KT_init = 1;

U32 KT0[256];
U32 KT1[256];
U32 KT2[256];
U32 KT3[256];

/* AES key scheduling routine */

int aes_set_key(U8 *key, int nbits)
{
	int i;
	U32 *RK, *SK;

	if (do_init) {
		aes_gen_tables();

		do_init = 0;
	}

	nr = 10;
	RK = erk;

	for (i = 0; i < (nbits >> 5); i++) {
		GET_UINT32(RK[i], key, i * 4);
	}

	/* setup encryption round keys */

	for (i = 0; i < 10; i++, RK += 4) {
		RK[4] = RK[0] ^ RCON[i] ^ (FSb[(U8)(RK[3] >> 16)] << 24) ^
			(FSb[(U8)(RK[3] >> 8)] << 16) ^
			(FSb[(U8)(RK[3])] << 8) ^ (FSb[(U8)(RK[3] >> 24)]);

		RK[5] = RK[1] ^ RK[4];
		RK[6] = RK[2] ^ RK[5];
		RK[7] = RK[3] ^ RK[6];
	}

	/* setup decryption round keys */

	if (KT_init) {
		for (i = 0; i < 256; i++) {
			KT0[i] = RT0[FSb[i]];
			KT1[i] = RT1[FSb[i]];
			KT2[i] = RT2[FSb[i]];
			KT3[i] = RT3[FSb[i]];
		}

		KT_init = 0;
	}

	SK = drk;

	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;

	for (i = 1; i < nr; i++) {
		RK -= 8;

		*SK++ = KT0[(U8)(*RK >> 24)] ^ KT1[(U8)(*RK >> 16)] ^
			KT2[(U8)(*RK >> 8)] ^ KT3[(U8)(*RK)];
		RK++;

		*SK++ = KT0[(U8)(*RK >> 24)] ^ KT1[(U8)(*RK >> 16)] ^
			KT2[(U8)(*RK >> 8)] ^ KT3[(U8)(*RK)];
		RK++;

		*SK++ = KT0[(U8)(*RK >> 24)] ^ KT1[(U8)(*RK >> 16)] ^
			KT2[(U8)(*RK >> 8)] ^ KT3[(U8)(*RK)];
		RK++;

		*SK++ = KT0[(U8)(*RK >> 24)] ^ KT1[(U8)(*RK >> 16)] ^
			KT2[(U8)(*RK >> 8)] ^ KT3[(U8)(*RK)];
		RK++;
	}

	RK -= 8;

	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;
	*SK++ = *RK++;

	return (0);
}

/* AES 128-bit block decryption routine */

void aes_decrypt(U8 input[16], U8 output[16])
{
	U32 *RK, X0, X1, X2, X3, Y0, Y1, Y2, Y3;

	RK = drk;

	GET_UINT32(X0, input, 0);
	X0 ^= RK[0];
	GET_UINT32(X1, input, 4);
	X1 ^= RK[1];
	GET_UINT32(X2, input, 8);
	X2 ^= RK[2];
	GET_UINT32(X3, input, 12);
	X3 ^= RK[3];

#define AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3)                             \
	{                                                                      \
		RK += 4;                                                       \
                                                                               \
		X0 = RK[0] ^ RT0[(U8)(Y0 >> 24)] ^ RT1[(U8)(Y3 >> 16)] ^       \
		     RT2[(U8)(Y2 >> 8)] ^ RT3[(U8)(Y1)];                       \
                                                                               \
		X1 = RK[1] ^ RT0[(U8)(Y1 >> 24)] ^ RT1[(U8)(Y0 >> 16)] ^       \
		     RT2[(U8)(Y3 >> 8)] ^ RT3[(U8)(Y2)];                       \
                                                                               \
		X2 = RK[2] ^ RT0[(U8)(Y2 >> 24)] ^ RT1[(U8)(Y1 >> 16)] ^       \
		     RT2[(U8)(Y0 >> 8)] ^ RT3[(U8)(Y3)];                       \
                                                                               \
		X3 = RK[3] ^ RT0[(U8)(Y3 >> 24)] ^ RT1[(U8)(Y2 >> 16)] ^       \
		     RT2[(U8)(Y1 >> 8)] ^ RT3[(U8)(Y0)];                       \
	}

	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 1 */
	AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 2 */
	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 3 */
	AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 4 */
	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 5 */
	AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 6 */
	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 7 */
	AES_RROUND(X0, X1, X2, X3, Y0, Y1, Y2, Y3); /* round 8 */
	AES_RROUND(Y0, Y1, Y2, Y3, X0, X1, X2, X3); /* round 9 */

	/* last round */

	RK += 4;

	X0 = RK[0] ^ (RSb[(U8)(Y0 >> 24)] << 24) ^ (RSb[(U8)(Y3 >> 16)] << 16) ^
	     (RSb[(U8)(Y2 >> 8)] << 8) ^ (RSb[(U8)(Y1)]);

	X1 = RK[1] ^ (RSb[(U8)(Y1 >> 24)] << 24) ^ (RSb[(U8)(Y0 >> 16)] << 16) ^
	     (RSb[(U8)(Y3 >> 8)] << 8) ^ (RSb[(U8)(Y2)]);

	X2 = RK[2] ^ (RSb[(U8)(Y2 >> 24)] << 24) ^ (RSb[(U8)(Y1 >> 16)] << 16) ^
	     (RSb[(U8)(Y0 >> 8)] << 8) ^ (RSb[(U8)(Y3)]);

	X3 = RK[3] ^ (RSb[(U8)(Y3 >> 24)] << 24) ^ (RSb[(U8)(Y2 >> 16)] << 16) ^
	     (RSb[(U8)(Y1 >> 8)] << 8) ^ (RSb[(U8)(Y0)]);

	PUT_UINT32(X0, output, 0);
	PUT_UINT32(X1, output, 4);
	PUT_UINT32(X2, output, 8);
	PUT_UINT32(X3, output, 12);
}

U8 key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

void Decrypt(U32 *SrcAddr, U32 *DestAddr, U32 Size)
{
	register U32 i = 0;

	aes_set_key(key, 128);

	while (i < (Size >> 2)) {
		aes_decrypt((U8 *)&DestAddr[i], (U8 *)&SrcAddr[i]);
		i += 4;
	}
}
#else
void ResetCon(U32 devicenum, CBOOL en);

static struct NX_ECID_RegisterSet *const pECIDReg =
    (struct NX_ECID_RegisterSet *)PHY_BASEADDR_ECID_MODULE;
static struct NX_CLKGEN_RegisterSet *const pCryptoClkGenReg =
    (struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN34_MODULE;
static NX_CRYPTO_RegisterSet *const pCrypto =
    (NX_CRYPTO_RegisterSet *)PHY_BASEADDR_CRYPTO_MODULE;

void Decrypt(U32 *SrcAddr, U32 *DestAddr, U32 Size)
{
	register U32 i=0, DataSize = ((Size+15) & 0xFFFFFFF0);
	ResetCon(RESETINDEX_OF_ECID_MODULE_i_nRST, CTRUE);	// reset on
	ResetCon(RESETINDEX_OF_ECID_MODULE_i_nRST, CFALSE);	// reset negate

	while(!(pECIDReg->EC[2] & 0x1<<15));    // wait for ecid ready

	ResetCon(RESETINDEX_OF_CRYPTO_MODULE_i_nRST, CTRUE);	// reset on
	ResetCon(RESETINDEX_OF_CRYPTO_MODULE_i_nRST, CFALSE);	// reset negate

	pCryptoClkGenReg->CLKENB = 1<<3;    // pclk always mode.

	while(i < (DataSize>>2))            // 128bits == 4bytes x 4
	{
		U32 j, temp[4];
		U8 *LittleEndian, *BigEndian;
		pCrypto->CRYPTO_AES_CTRL0 =
			0x1<<15 |                    // 0: Enable, 1: Disable  Use Fuse Key
			0x1<< 9 |                    // 0: Big Endian, 1: Little Endian output swap
			0x1<< 8 |                    // 0: Big Endian, 1: Little Endian input swap
			0x0<< 6 |                    // 0: ECB, 1: CBC, 2: CTR mode
			0x1<< 3 |                    // 64bit counter
			0x0<< 2 |                    // 0: FIFO Mode, 1: DMA Mode
			0x0<< 1 |                    // 0: Decoder, 1: Encoder
			0x0<< 0;                     // 0: Disable, 1: Enable AES Enable?

		LittleEndian = (U8*)&SrcAddr[i];
		BigEndian = (U8*)temp;
		for(j=0; j<16; j++)
			BigEndian[j] = LittleEndian[15-j];

		pCrypto->CRYPTO_AES_TIN0 = temp[3];
		pCrypto->CRYPTO_AES_TIN1 = temp[2];
		pCrypto->CRYPTO_AES_TIN2 = temp[1];
		pCrypto->CRYPTO_AES_TIN3 = temp[0];

		pCrypto->CRYPTO_AES_CTRL0 =
			0x1<<15 |                    // 0: Enable, 1: Disable  Use Fuse Key
			0x1<< 9 |                    // 0: Big Endian, 1: Little Endian output swap
			0x1<< 8 |                    // 0: Big Endian, 1: Little Endian input swap
			0x0<< 6 |                    // 0: ECB, 1: CBC, 2: CTR mode
			0x1<< 3 |                    // 64bit counter
			0x0<< 2 |                    // 0: FIFO Mode, 1: DMA Mode
			0x0<< 1 |                    // 0: Decoder, 1: Encoder
			0x1<< 0;                     // 0: Disable, 1: Enable AES Enable?

		pCrypto->CRYPTO_CRT_CTRL0 |= 0x1<<0;    // Decryption run

		while( !(pCrypto->CRYPTO_CRT_CTRL0 & 0x1<<0) );

		temp[3] = pCrypto->CRYPTO_AES_TOUT0;
		temp[2] = pCrypto->CRYPTO_AES_TOUT1;
		temp[1] = pCrypto->CRYPTO_AES_TOUT2;
		temp[0] = pCrypto->CRYPTO_AES_TOUT3;

		LittleEndian = (U8*)&DestAddr[i];
		BigEndian = (U8*)temp;
		for(j=0; j<16; j++)
			LittleEndian[j] = BigEndian[15-j];

		i += 4;
	}
}
#endif
