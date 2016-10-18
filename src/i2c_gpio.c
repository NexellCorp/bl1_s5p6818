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
#include <nx_gpio.h>
#include "sysheader.h"

#define I2C_GPIO_DEBUG 0

#define dprintf printf
//#define dprintf(x)

#define STRETCHING_TIMEOUT 100
#define I2C_DELAY_TIME 2

#define I2CREAD 1
#define I2CWRITE 0

static CBOOL started;
static U8 g_I2C_GPIO_GRP;
static U8 g_I2C_GPIO_SCL;
static U8 g_I2C_GPIO_SDA;

static inline void I2CDELAY(U32 us)
{
	volatile U32 i = 0, j = 0;
	for (i = 0; i < us * 2; i++)
		for (j = 0; j < 5; j++)
			;
}

static void SDA_LOW(void)
{
	SetIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUTENB, (1 << g_I2C_GPIO_SDA));
}

static void SCL_LOW(void)
{
	SetIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUTENB, (1 << g_I2C_GPIO_SCL));
}

static CBOOL SDA_READ(void)
{
	//	NX_GPIO_SetOutputEnable(g_I2C_GPIO_GRP, g_I2C_GPIO_SDA, CFALSE);
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUTENB,
		  (1 << g_I2C_GPIO_SDA));

	//	return NX_GPIO_GetInputValue(g_I2C_GPIO_GRP, g_I2C_GPIO_SDA);
	return (CBOOL)(
	    (ReadIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxPAD) >> g_I2C_GPIO_SDA) &
	    1);
}

static CBOOL SCL_READ(void)
{
	//	NX_GPIO_SetOutputEnable(g_I2C_GPIO_GRP, g_I2C_GPIO_SCL, CFALSE);
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUTENB,
		  (1 << g_I2C_GPIO_SCL));

	//	return NX_GPIO_GetInputValue(g_I2C_GPIO_GRP, g_I2C_GPIO_SCL);
	return (CBOOL)(
	    (ReadIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxPAD) >> g_I2C_GPIO_SCL) &
	    1);
}

void I2C_Init(U8 gpioGRP, U8 gpioSCL, U8 gpioSDA, U32 gpioSCLAlt, U32 gpioSDAAlt)
{
	started = CFALSE;

	g_I2C_GPIO_GRP = gpioGRP;
	g_I2C_GPIO_SCL = gpioSCL;
	g_I2C_GPIO_SDA = gpioSDA;

	//	printf("I2C_Init\r\n");
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUT,
		  (1 << g_I2C_GPIO_SCL)); // low
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUT,
		  (1 << g_I2C_GPIO_SDA)); // low
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUTENB,
		  (1 << g_I2C_GPIO_SCL)); // input
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUTENB,
		  (1 << g_I2C_GPIO_SDA)); // input
#if 0
	ChangeIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SCL>>4], 3<<((g_I2C_GPIO_SCL&0xF)<<1), NX_GPIO_PADFUNC_0<<((g_I2C_GPIO_SCL&0xF)<<1));	// to gpio
	ChangeIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SDA>>4], 3<<((g_I2C_GPIO_SDA&0xF)<<1), NX_GPIO_PADFUNC_0<<((g_I2C_GPIO_SDA&0xF)<<1));	// to gpio
#else
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SCL >> 4],
		  NX_GPIO_PADFUNC_3 << ((g_I2C_GPIO_SCL & 0xF) << 1));
	SetIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SCL >> 4],
		gpioSCLAlt << ((g_I2C_GPIO_SCL & 0xF) << 1));

	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SDA >> 4],
		  NX_GPIO_PADFUNC_3 << ((g_I2C_GPIO_SDA & 0xF) << 1));
	SetIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SDA >> 4],
		gpioSDAAlt << ((g_I2C_GPIO_SDA & 0xF) << 1));
#endif
	SetIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOx_PULLSEL,
		(1 << g_I2C_GPIO_SDA)); // pullup
	SetIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOx_PULLENB,
		(1 << g_I2C_GPIO_SDA)); // pull enable
	SetIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOx_PULLSEL,
		(1 << g_I2C_GPIO_SCL)); // pullup
	SetIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOx_PULLENB,
		(1 << g_I2C_GPIO_SCL)); // pull enable
}

#if 0
void I2C_Deinit( void )
{
	//	printf("I2C_Deinit\r\n");
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUT,         (1<<g_I2C_GPIO_SCL) );  // low
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUT,         (1<<g_I2C_GPIO_SDA) );  // low
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUTENB,      (1<<g_I2C_GPIO_SCL) );  // input
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxOUTENB,      (1<<g_I2C_GPIO_SDA) );  // input
#if 0
	ChangeIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SCL>>4], 3<<((g_I2C_GPIO_SCL&0xF)<<1), NX_GPIO_PADFUNC_0<<((g_I2C_GPIO_SCL&0xF)<<1));	// to gpio
	ChangeIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SDA>>4], 3<<((g_I2C_GPIO_SDA&0xF)<<1), NX_GPIO_PADFUNC_0<<((g_I2C_GPIO_SDA&0xF)<<1));	// to gpio
#else
	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SCL>>4], NX_GPIO_PADFUNC_3<<((g_I2C_GPIO_SCL&0xF)<<1));
	SetIO32  (&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SCL>>4], NX_GPIO_PADFUNC_0<<((g_I2C_GPIO_SCL&0xF)<<1));

	ClearIO32(&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SDA>>4], NX_GPIO_PADFUNC_3<<((g_I2C_GPIO_SDA&0xF)<<1));
	SetIO32  (&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOxALTFN[g_I2C_GPIO_SDA>>4], NX_GPIO_PADFUNC_0<<((g_I2C_GPIO_SDA&0xF)<<1));
#endif
	SetIO32  (&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOx_PULLSEL,    (1<<g_I2C_GPIO_SDA) );  // pullup
	SetIO32  (&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOx_PULLENB,    (1<<g_I2C_GPIO_SDA) );  // pull enable
	SetIO32  (&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOx_PULLSEL,    (1<<g_I2C_GPIO_SCL) );  // pullup
	SetIO32  (&pReg_GPIO[g_I2C_GPIO_GRP]->GPIOx_PULLENB,    (1<<g_I2C_GPIO_SCL) );  // pull enable
}
#endif

static CBOOL I2C_Start(void)
{
	U32 timeout = STRETCHING_TIMEOUT;
	if (started) {
		SDA_READ();

		I2CDELAY(I2C_DELAY_TIME);

		while (SCL_READ() ==
		       CFALSE) // clock stretching.... need timeout code
		{
			if (timeout-- == 0) {
				dprintf("I2C_Start CLK Timeout Arbitration "
					"Fail\r\n");
				return CFALSE;
			}
			I2CDELAY(I2C_DELAY_TIME);
		}

		I2CDELAY(I2C_DELAY_TIME);
	}

	if (SDA_READ() == CFALSE) {
		dprintf("I2C_Start SDA Arbitration Fail\r\n");
		return CFALSE; // arbitration_lost
	}

	SDA_LOW();

	I2CDELAY(I2C_DELAY_TIME);

	SCL_LOW();

	started = CTRUE;

	return CTRUE;
}

static CBOOL I2C_Stop(void)
{
	U32 timeout = STRETCHING_TIMEOUT;

	SDA_LOW();

	I2CDELAY(I2C_DELAY_TIME);

	while (SCL_READ() == CFALSE) // need timeout code
	{
		if (timeout-- == 0) {
			dprintf("I2C_Stop CLK Timeout Arbitration Fail\r\n");
			return CFALSE;
		}
		I2CDELAY(I2C_DELAY_TIME);
	}

	I2CDELAY(I2C_DELAY_TIME);

	SDA_READ();

	started = CFALSE;
	timeout = STRETCHING_TIMEOUT;
	while (SDA_READ() == CFALSE) {
		if (timeout-- == 0) {
			dprintf("I2C_Stop SDA Arbitration Fail\r\n");
			return CFALSE; // arbitration_lost
		}
		I2CDELAY(I2C_DELAY_TIME);
	}

	I2CDELAY(I2C_DELAY_TIME);

	return CTRUE;
}

static CBOOL I2C_WriteBit(CBOOL Bit)
{
	U32 timeout = STRETCHING_TIMEOUT;

	I2CDELAY(I2C_DELAY_TIME);

	if (Bit)
		SDA_READ();
	else
		SDA_LOW();

	I2CDELAY(I2C_DELAY_TIME);

	while (SCL_READ() == CFALSE) // clock stretching.... need timeout code
	{
		if (timeout-- == 0) {
			dprintf(
			    "I2C_WriteBit CLK Timeout Arbitration Fail\r\n");
			return CFALSE;
		}
		I2CDELAY(I2C_DELAY_TIME);
	}

	if (Bit && (SDA_READ() == CFALSE)) {
		dprintf("I2C_WriteBit SDA Arbitration Fail\r\n");
		return CFALSE; // arbitration_lost
	}

	I2CDELAY(I2C_DELAY_TIME);

	SCL_LOW();

	return CTRUE;
}

static CBOOL I2C_ReadBit(CBOOL *Bit)
{
	U32 timeout = STRETCHING_TIMEOUT;

	I2CDELAY(I2C_DELAY_TIME);

	SDA_READ();

	I2CDELAY(I2C_DELAY_TIME);

	while (SCL_READ() == CFALSE)
		; // clock stretching.... need timeout code
	{
		if (timeout-- == 0) {
			dprintf("I2C_ReadBit CLK Timeout Arbitration Fail\r\n");
			return CFALSE;
		}
		I2CDELAY(I2C_DELAY_TIME);
	}

	*Bit = SDA_READ(); // data or nack read

	I2CDELAY(I2C_DELAY_TIME);

	SCL_LOW();

	return CTRUE;
}

static CBOOL I2C_WriteByte(CBOOL SendStart, CBOOL SendStop, U8 Data,
			   CBOOL *nAck)
{
	U32 bit;
	CBOOL Arbitration;

#if (I2C_GPIO_DEBUG == 1)
	dprintf("I2C_WriteByte %X\r\n", Data);
#endif

	if (SendStart) {
		Arbitration = I2C_Start();
		if (Arbitration == CFALSE)
			return CFALSE;
	}

	for (bit = 0; bit < 8; bit++) {
		Arbitration = I2C_WriteBit((CBOOL)((Data & 0x80) != 0));
		if (Arbitration == CFALSE)
			return CFALSE;
		Data <<= 1;
	}

	Arbitration = I2C_ReadBit(nAck);
	if (Arbitration == CFALSE)
		return CFALSE;
	if (*nAck == CTRUE) {
		dprintf("I2C_WriteByte nack returned\r\n");
		return CFALSE;
	}

	if (SendStop) {
		Arbitration = I2C_Stop();
		if (Arbitration == CFALSE)
			return CFALSE;
	}

	return CTRUE;
}

static CBOOL I2C_ReadByte(CBOOL nAck, CBOOL SendStop, U8 *pData)
{
	U8 byte = 0;
	U32 bit;
	CBOOL result, rBit;

#if (I2C_GPIO_DEBUG == 1)
	dprintf("I2C_ReadByte %s\r\n", SendStop ? "Stop" : "Nstop");
#endif
	for (bit = 0; bit < 8; bit++) {
		result = I2C_ReadBit(&rBit);
		if (result == CFALSE)
			return CFALSE;
		byte = (byte << 1) | (rBit ? 1 : 0);
	}
	*pData = byte;

	result = I2C_WriteBit(nAck);
	if (result == CFALSE)
		return CFALSE;

	if (SendStop) {
		result = I2C_Stop();
		if (result == CFALSE)
			return CFALSE;
	}

	return CTRUE;
}

CBOOL I2C_Read(U8 DeviceAddress, U8 RegisterAddress, U8 *pData, U32 Length)
{
	CBOOL nAck, result;
	U32 byte;

#if (I2C_GPIO_DEBUG == 1)
	dprintf("I2C_Read %d\r\n", Length);
#endif
	result =
	    I2C_WriteByte(CTRUE, CFALSE, DeviceAddress << 1 | I2CWRITE, &nAck);
	if (result == CFALSE) {
		dprintf("I2C Device Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if (nAck == CTRUE) {
		dprintf("I2C Device Address Write Acknowledge Error\r\n");
		return CFALSE;
	}

	result = I2C_WriteByte(CFALSE, CFALSE, RegisterAddress, &nAck);
	if (result == CFALSE) {
		dprintf("I2C Register Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if (nAck == CTRUE) {
		dprintf("I2C Register Address Write Acknowledge Error\r\n");
		return CFALSE;
	}
	result =
	    I2C_WriteByte(CTRUE, CFALSE, DeviceAddress << 1 | I2CREAD, &nAck);
	if (result == CFALSE) {
		dprintf("I2C Device Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if (nAck == CTRUE) {
		dprintf("I2C Device Address Write Acknowledge Error\r\n");
		return CFALSE;
	}

	for (byte = 0; byte < Length;) {
		byte++;
		result =
		    I2C_ReadByte((byte == Length) ? CTRUE : CFALSE,
				 (byte == Length) ? CTRUE : CFALSE, pData++);
		if (result == CFALSE) {
			dprintf("I2C Data Read Abitration Error\r\n");
			return CFALSE;
		}
	}

	return CTRUE;
}

CBOOL I2C_Write(U8 DeviceAddress, U8 RegisterAddress, U8 *pData, U32 Length)
{
	CBOOL nAck, result;
	U32 byte;

#if (I2C_GPIO_DEBUG == 1)
	dprintf("I2C_Write %d\r\n", Length);
#endif
	result =
	    I2C_WriteByte(CTRUE, CFALSE, DeviceAddress << 1 | I2CWRITE, &nAck);
	if (result == CFALSE) {
		dprintf("I2C Device Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if (nAck == CTRUE) {
		dprintf("I2C Device Address Write Acknowledge Error\r\n");
		return CFALSE;
	}

	result = I2C_WriteByte(CFALSE, CFALSE, RegisterAddress, &nAck);
	if (result == CFALSE) {
		dprintf("I2C Register Address Write Abitration Error\r\n");
		return CFALSE;
	}
	if (nAck == CTRUE) {
		dprintf("I2C Register Address Write Acknowledge Error\r\n");
		return CFALSE;
	}

	for (byte = 0; byte < Length;) {
		byte++;
		result = I2C_WriteByte(
		    CFALSE, (byte == Length) ? CTRUE : CFALSE, *pData++, &nAck);
		if (result == CFALSE) {
			dprintf("I2C Data Write Abitration Error\r\n");
			return CFALSE;
		}
		if (nAck == CTRUE) {
			dprintf("I2C Data Write Acknowledge Error\r\n");
			return CFALSE;
		}
	}

	return CTRUE;
}
