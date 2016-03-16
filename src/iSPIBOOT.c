/*
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved
 *      Nexell Co. Proprietary & Confidential
 *
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR
 *FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      Module          : SPI
 *      File            : iSPIBOOT.c
 *      Description     :
 *      Author          : Hans
 *      History         : 2014.01.12 First implementation
 */

#include "sysheader.h"
#include <nx_ssp.h>

#define CFG_CS_CTRL_AUTO (0)

//------------------------------------------------------------------------------

#define SER_WREN 0x06  // Set Write Enable Latch
#define SER_WRDI 0x04  // Reset Write Enable Latch
#define SER_RDSR 0x05  // Read Status Register
#define SER_WRSR 0x01  // Write Status Register
#define SER_READ 0x03  // Read Data from Memory Array
#define SER_WRITE 0x02 // Write Data to Memory Array

#define SER_SR_READY 1 << 0 // Ready bit
#define SER_SR_WEN                                                             \
	1 << 1 // Write Enable indicate 0:not write enabled 1:write enabled
#define SER_SR_BPx 3 << 2  // Block Write Protect bits
#define SER_SR_WPEN 1 << 7 // Write Protect Enable bit

//#define SPI_SOURCE_CLOCK	NX_CLKSRCPLL2_FREQ
#define SPI_SOURCE_DIVIDOVER (8UL)  // 550000000/8/2 = 34.375MHz
#define SPI_SOURCE_DIVIDHIGH (18UL) // 550000000/18/2 = 15.277778MHz
#define SPI_SOURCE_DIVIDLOW (48UL)  // 96000000/48/2 = 1.000000MHz

extern U32 getquotient(U32 dividend, U32 divisor);
extern U32 get_fcs(U32 fcs, U8 data);
extern void ResetCon(U32 devicenum, CBOOL en);
extern void Decrypt(U32 *SrcAddr, U32 *DestAddr, U32 Size);
extern void GPIOSetAltFunction(U32 AltFunc);
/*
   spi 0
   clk c 29 1 gpio:0
   frm c 30 1 gpio:0
   txd c 31 1 gpio:0
   rxd d 0  1 gpio:0

   spi 1
   clk e 14 2 gpio:0
   frm e 15 2 gpio:0
   rxd e 18 2 gpio:0
   txd e 19 2 gpio:0

   spi 2
   clk c 9  2 gpio:1 default 0
   frm c 10 2 gpio:1 default 0
   rxd c 11 2 gpio:1 default 0
   txd c 12 2 gpio:1 default 0
 */
//------------------------------------------------------------------------------

static struct NX_CLKGEN_RegisterSet *const pgSSPClkGenReg[3] = {
    (struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN37_MODULE,
    (struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN38_MODULE,
    (struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN39_MODULE};

static struct NX_SSP_RegisterSet *const pgSSPSPIReg[3] = {
    (struct NX_SSP_RegisterSet *)PHY_BASEADDR_SSP0_MODULE,
    (struct NX_SSP_RegisterSet *)PHY_BASEADDR_SSP1_MODULE,
    (struct NX_SSP_RegisterSet *)PHY_BASEADDR_SSP2_MODULE};
static U32 const SPIResetNum[6] = {
    RESETINDEX_OF_SSP0_MODULE_PRESETn, RESETINDEX_OF_SSP0_MODULE_nSSPRST,
    RESETINDEX_OF_SSP1_MODULE_PRESETn, RESETINDEX_OF_SSP1_MODULE_nSSPRST,
    RESETINDEX_OF_SSP2_MODULE_PRESETn, RESETINDEX_OF_SSP2_MODULE_nSSPRST};

#if 1
typedef struct {
	U32 nPllNum;
	U32 nFreqHz;
	U32 nClkDiv;
	U32 nClkGenDiv;
} NX_CLKINFO_SPI;

CBOOL NX_SPI_GetClkParam(NX_CLKINFO_SPI *pClkInfo)
{
	U32 srcFreq;
	U32 nRetry = 1, nTemp = 0;
	CBOOL fRet = CFALSE;

	srcFreq = NX_CLKPWR_GetPLLFreq(pClkInfo->nPllNum);

retry_getparam:
	for (pClkInfo->nClkDiv = 2;; pClkInfo->nClkDiv++) {
		nTemp = (pClkInfo->nFreqHz * pClkInfo->nClkDiv);
		pClkInfo->nClkGenDiv =
		    getquotient(srcFreq, nTemp); // (srcFreq / nTemp)

		if (srcFreq > (pClkInfo->nFreqHz * pClkInfo->nClkDiv))
			pClkInfo->nClkGenDiv++;

		if (pClkInfo->nClkGenDiv < 255)
			break;
	}

	nTemp =
	    getquotient(srcFreq, (pClkInfo->nClkGenDiv * pClkInfo->nClkDiv));
	if (nTemp <= pClkInfo->nFreqHz) {
		fRet = CTRUE;
		goto exit_getparam;
	}

	if (nRetry) {
		nRetry--;
		goto retry_getparam;
	}

exit_getparam:
#if 0
	if (nRetry)
		printf("getClk = %d\r\n", nTemp);
#endif

	return fRet;
}
#endif

void SPI_Init(U32 SPIPort)
{
	register struct NX_SSP_RegisterSet *const pSSPSPIReg =
	    pgSSPSPIReg[SPIPort];
	struct NX_CLKGEN_RegisterSet *const pSSPClkGenReg =
	    pgSSPClkGenReg[SPIPort];
	CBOOL ret = CTRUE;

#if 1
	NX_CLKINFO_SPI clkInfo;

	clkInfo.nPllNum = NX_CLKSRC_SPI;
	clkInfo.nFreqHz = 20000000;

	ret = NX_SPI_GetClkParam(&clkInfo);
	if (ret == CFALSE)
		printf("get clock param faile.\r\n");

	pSSPClkGenReg->CLKENB =
	    0x1 << 3; // pclk mode on but not supply operation clock
	pSSPClkGenReg->CLKGEN[0] =
	    ((clkInfo.nClkGenDiv - 1) << 5) |
	    (clkInfo.nPllNum << 2); // select clock source is pll0, 800MHz and
				    // supply clock is 800/20 = 40.000MHz
	pSSPClkGenReg->CLKENB = 0x1 << 3 | 0x1 << 2; // supply operation clock
#else

	pSSPClkGenReg->CLKENB =
	    0x1 << 3; // pclk mode on but not supply operation clock
	pSSPClkGenReg->CLKGEN[0] = (SPI_SOURCE_DIVIDOVER - 1) << 5 |
				   0x0 << 2; // select clock source is pll0,
					     // 550MHz and supply clock is
					     // 550/8/2 = 34.375MHz
	pSSPClkGenReg->CLKENB = 0x1 << 3 | 0x1 << 2; // supply operation clock
#endif

	ResetCon(SPIResetNum[SPIPort * 2 + 0], CTRUE); // reset on
	ResetCon(SPIResetNum[SPIPort * 2 + 1], CTRUE); // reset on

	ResetCon(SPIResetNum[SPIPort * 2 + 0], CFALSE); // reset negate
	ResetCon(SPIResetNum[SPIPort * 2 + 1], CFALSE); // reset negate

	if (SPIPort == 0) {
		register struct NX_GPIO_RegisterSet *pGPIOxReg =
		    (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_C];
		pGPIOxReg->GPIOxALTFN[1] =
		    (pGPIOxReg->GPIOxALTFN[1] & ~0xFC000000) |
		    0x54000000; // spi 0 GPIO C[29, 30, 31] ALT 1

		pGPIOxReg->GPIOx_SLEW &= ~(1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_SLEW_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_DRV0 |= (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_DRV0_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_DRV1 |= (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_DRV1_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_PULLSEL |= (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_PULLSEL_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_PULLENB &= ~(1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_PULLENB_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);

		pGPIOxReg =
		    (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_D];
		pGPIOxReg->GPIOxALTFN[0] =
		    (pGPIOxReg->GPIOxALTFN[0] & ~0x00000003) |
		    0x00000001; // spi 0 GPIO D[0] ALT 1

		pGPIOxReg->GPIOx_SLEW &= ~(1 << 0);
		pGPIOxReg->GPIOx_SLEW_DISABLE_DEFAULT |= (1 << 0);
		pGPIOxReg->GPIOx_DRV0 |= (1 << 0);
		pGPIOxReg->GPIOx_DRV0_DISABLE_DEFAULT |= (1 << 0);
		pGPIOxReg->GPIOx_DRV1 |= (1 << 0);
		pGPIOxReg->GPIOx_DRV1_DISABLE_DEFAULT |= (1 << 0);
		pGPIOxReg->GPIOx_PULLSEL |= (1 << 0);
		pGPIOxReg->GPIOx_PULLSEL_DISABLE_DEFAULT |= (1 << 0);
		pGPIOxReg->GPIOx_PULLENB &= ~(1 << 0);
		pGPIOxReg->GPIOx_PULLENB_DISABLE_DEFAULT |= (1 << 0);
	} else if (SPIPort == 1) {
		register struct NX_GPIO_RegisterSet *pGPIOxReg =
		    (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_E];

		pGPIOxReg->GPIOxALTFN[0] =
		    (pGPIOxReg->GPIOxALTFN[0] & ~0xF0000000) |
		    0xA0000000; // spi 1 GPIO E[14, 15] ALT 2
		pGPIOxReg->GPIOxALTFN[1] =
		    (pGPIOxReg->GPIOxALTFN[1] & ~0x000000F0) |
		    0x000000A0; // spi 1 GPIO E[18, 19] ALT 2

		pGPIOxReg->GPIOx_SLEW &=
		    ~(1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_SLEW_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_DRV0 |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_DRV0_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_DRV1 |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_DRV1_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_PULLSEL |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_PULLSEL_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_PULLENB &=
		    ~(1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_PULLENB_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
	} else {
		register struct NX_GPIO_RegisterSet *pGPIOxReg =
		    (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_C];
		pGPIOxReg->GPIOxALTFN[0] =
		    (pGPIOxReg->GPIOxALTFN[0] & ~0x03F30000) |
		    0x02A80000; // spi 2 GPIO C[9, 10, 11, 12] ALT 2

		pGPIOxReg->GPIOx_SLEW &= ~(1 << 12 | 1 << 11 | 1 << 10 |
					   1 << 9); // txd, rxd, clk, frm
		pGPIOxReg->GPIOx_SLEW_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_DRV0 |= (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_DRV0_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_DRV1 |= (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_DRV1_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_PULLSEL |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_PULLSEL_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_PULLENB &=
		    ~(1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_PULLENB_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
	}

	pSSPSPIReg->CH_CFG =
	    1 << 6 | // high speed en (0:low speed, 1:high speed)
	    0 << 4 | // master mode   (0:master mode, 1:slave mode)
	    0 << 3 | // ck pol        (0:active high, 1:active low)
	    0 << 2 | // format        (0:format a, 1:format b)
	    0 << 1 | // rx channel on (0:off, 1:on)
	    0 << 0;  // tx channel on

	pSSPSPIReg->MODE_CFG =
	    0 << 29 | // channel width (0:byte, 1:half word, 2:word)
	    0 << 17 | // bus width
	    0 << 11 | // rx fifo trigger level
	    0 << 5 |  // tx fifo trigger level
	    0 << 2 |  // rx dma mode   (0:disable, 1:enable)
	    0 << 1 |  // tx dma mode
	    0 << 0;   // dma transfer type (0:single, 1:4 burst)

#if (CFG_CS_CTRL_AUTO == 1)
	pSSPSPIReg->CS_REG = 1 << 1 | // chip select toggle(0:manual, 1:auto,)
			     1 << 0; // chip select state (0:active, 1:inactive)
#else
	pSSPSPIReg->CS_REG = 0 << 1 | // chip select toggle(0:manual, 1:auto,)
			     1 << 0;  // chip select state (0:active, 1:inactive)
#endif

	pSSPSPIReg->SPI_INT_EN = 0; // all interrupt disasble

	while (pSSPSPIReg->SPI_STATUS & (0x1FF << 6))
		; // wait while TX fifo counter is not 0
	while ((pSSPSPIReg->SPI_STATUS >> 15) &
	       0x1FF)			 // while RX fifo is not empty
		pSSPSPIReg->SPI_RX_DATA; // RX data read
}

void SPI_Deinit(U32 SPIPort)
{
	struct NX_CLKGEN_RegisterSet *const pSSPClkGenReg =
	    pgSSPClkGenReg[SPIPort];

	if (SPIPort == 0) {
		register struct NX_GPIO_RegisterSet *pGPIOxReg =
		    (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_C];

		pGPIOxReg->GPIOxOUT |= 0x40000000;
		pGPIOxReg->GPIOxOUTENB &= ~0x40000000;
		pGPIOxReg->GPIOxALTFN[1] &=
		    ~0xFC000000; // spi 0 GPIO C[29, 30, 31] ==> ALT 0

		pGPIOxReg->GPIOx_SLEW |= (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_SLEW_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_DRV0 &= ~(1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_DRV0_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_DRV1 &= ~(1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_DRV1_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_PULLSEL &= ~(1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_PULLSEL_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_PULLENB &= ~(1 << 31 | 1 << 30 | 1 << 29);
		pGPIOxReg->GPIOx_PULLENB_DISABLE_DEFAULT |=
		    (1 << 31 | 1 << 30 | 1 << 29);

		pGPIOxReg =
		    (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_D];
		pGPIOxReg->GPIOxALTFN[0] &=
		    ~0x00000003; // spi 0 GPIO D[0] ==> ALT 0

		pGPIOxReg->GPIOx_SLEW |= (1 << 0);
		pGPIOxReg->GPIOx_SLEW_DISABLE_DEFAULT |= (1 << 0);
		pGPIOxReg->GPIOx_DRV0 &= ~(1 << 0);
		pGPIOxReg->GPIOx_DRV0_DISABLE_DEFAULT |= (1 << 0);
		pGPIOxReg->GPIOx_DRV1 &= ~(1 << 0);
		pGPIOxReg->GPIOx_DRV1_DISABLE_DEFAULT |= (1 << 0);
		pGPIOxReg->GPIOx_PULLSEL &= ~(1 << 0);
		pGPIOxReg->GPIOx_PULLSEL_DISABLE_DEFAULT |= (1 << 0);
		pGPIOxReg->GPIOx_PULLENB &= ~(1 << 0);
		pGPIOxReg->GPIOx_PULLENB_DISABLE_DEFAULT |= (1 << 0);
	} else if (SPIPort == 1) {
		register struct NX_GPIO_RegisterSet *pGPIOxReg =
		    (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_E];

		pGPIOxReg->GPIOxOUT |= 0x00008000;
		pGPIOxReg->GPIOxOUTENB &= ~0x00008000;
		pGPIOxReg->GPIOxALTFN[0] &=
		    ~0xF0000000; // spi 1 GPIO E[14, 15] ==> ALT 0
		pGPIOxReg->GPIOxALTFN[1] &=
		    ~0x000000F0; // spi 1 GPIO E[18, 19] ==> ALT 0

		pGPIOxReg->GPIOx_SLEW |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_SLEW_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_DRV0 &=
		    ~(1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_DRV0_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_DRV1 &=
		    ~(1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_DRV1_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_PULLSEL &=
		    ~(1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_PULLSEL_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_PULLENB &=
		    ~(1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
		pGPIOxReg->GPIOx_PULLENB_DISABLE_DEFAULT |=
		    (1 << 19 | 1 << 18 | 1 << 15 | 1 << 14);
	} else {
		register struct NX_GPIO_RegisterSet *pGPIOxReg =
		    (struct NX_GPIO_RegisterSet *)&pReg_GPIO[GPIO_GROUP_C];

		pGPIOxReg->GPIOxOUT |= 0x00000400;
		pGPIOxReg->GPIOxOUTENB &= ~0x00000400;
		pGPIOxReg->GPIOxALTFN[0] &=
		    ~0x03FC0000; // spi 2 GPIO C[9, 10, 11, 12] ==> ALT 0

		pGPIOxReg->GPIOx_SLEW |= (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_SLEW_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_DRV0 &=
		    ~(1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_DRV0_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_DRV1 &=
		    ~(1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_DRV1_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_PULLSEL &=
		    ~(1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_PULLSEL_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_PULLENB &=
		    ~(1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
		pGPIOxReg->GPIOx_PULLENB_DISABLE_DEFAULT |=
		    (1 << 12 | 1 << 11 | 1 << 10 | 1 << 9);
	}

	pSSPClkGenReg->CLKENB = 0; // PCLKMODE : disable, Clock Gen Disable

	//--------------------------------------------------------------------------
	ResetCon(SPIResetNum[SPIPort * 2 + 0], CTRUE); // reset on
	ResetCon(SPIResetNum[SPIPort * 2 + 1], CTRUE); // reset on
}

U32 SPI_EEPROMRead(U32 FlashBase, U32 *DDRBase, U32 Size, U32 FlashAddrCount,
		   U32 fcs, U32 SPIPort)
{
	register U8 *pdwBuffer = (U8 *)DDRBase;
	register U32 iRxSize = 0;
	register struct NX_SSP_RegisterSet *pSSPSPIReg;

	pSSPSPIReg = pgSSPSPIReg[SPIPort];

	pSSPSPIReg->CS_REG &= ~(1 << 0); // chip select state to active
	pSSPSPIReg->CH_CFG |= (3 << 0);  // rx, tx channel on
	pSSPSPIReg->SPI_TX_DATA =
	    SER_READ; // read command, Read Data from Memory Array

	while (FlashAddrCount) {
		U32 tmpAddr = (FlashBase >> (8 * (FlashAddrCount - 1))) &
			      0xFF; // start memory array address
		pSSPSPIReg->SPI_TX_DATA = tmpAddr;
		FlashAddrCount--;
	}

	while (pSSPSPIReg->SPI_STATUS & 0x1FF << 6)
		; // wait while TX fifo counter is not 0
	while (!(pSSPSPIReg->SPI_STATUS & 0x1 << 25))
		; // wait for TX fifo is empty
	while (pSSPSPIReg->SPI_STATUS >> 15 &
	       0x1FF)			 // while RX fifo is not empty
		pSSPSPIReg->SPI_RX_DATA; // discard RX data cmd & address

	pSSPSPIReg->SPI_TX_DATA =
	    0xAA; // send dummy data for receive read data.
	pSSPSPIReg->SPI_TX_DATA =
	    0xAA; // send dummy data for receive read data.
	pSSPSPIReg->SPI_TX_DATA =
	    0xAA; // send dummy data for receive read data.
	pSSPSPIReg->SPI_TX_DATA =
	    0xAA; // send dummy data for receive read data.

	while (!(pSSPSPIReg->SPI_STATUS >> 15 & 0x1FF))
		; // wait RX fifo is not empty

	while (iRxSize < Size) {
		register U8 tmpData;

		if (pSSPSPIReg->SPI_STATUS >> 15 &
		    0x1FF) // wait RX fifo is not empty
		{
			pSSPSPIReg->SPI_TX_DATA =
			    0xAA; // send dummy data for receive read data.
			tmpData = (U8)pSSPSPIReg->SPI_RX_DATA;

			pdwBuffer[iRxSize] = tmpData;
			fcs = get_fcs(fcs, tmpData);
			iRxSize++;
		}
	}

	while (pSSPSPIReg->SPI_STATUS & 0x1FF << 6)
		; // wait while TX fifo counter is not 0
	while (!(pSSPSPIReg->SPI_STATUS & 0x1 << 25))
		; // wait for TX fifo is empty
	while ((pSSPSPIReg->SPI_STATUS >> 15) &
	       0x1FF)			 // while RX fifo is not empty
		pSSPSPIReg->SPI_RX_DATA; // RX data read

	pSSPSPIReg->CH_CFG &= ~(3 << 0); // rx, tx channel off
	pSSPSPIReg->CS_REG |= (1 << 0);  // chip select state to inactive

	printf("SPI Read completed!\r\n");

	return fcs;
}

CBOOL iSPIBOOT(struct NX_SecondBootInfo *pTBI)
{
	U32 FlashAddr, *fcsdata;
	U8 *DataAddr;
	S32 DataSize;
	U32 fcs = 0;

	printf("SPI Device Port Number  : %d\r\n", pSBI->DBI.SPIBI.PortNumber);
	printf("SPI Device Read Address : 0x%08X\r\n", pSBI->DEVICEADDR);
	printf("SPI Address step        : 0x%08X\r\n",
	       pSBI->DBI.SPIBI.AddrStep);

	FlashAddr = pSBI->DEVICEADDR;

	SPI_Init(pSBI->DBI.SPIBI.PortNumber);

	//	fcs = SPI_EEPROMWrite(FlashAddr, (U32*)pSBI->LOADADDR,
	//pSBI->LOADSIZE);

	SPI_EEPROMRead(FlashAddr, (U32 *)pTBI, sizeof(struct NX_SecondBootInfo),
		       pSBI->DBI.SPIBI.AddrStep, 0,
		       pSBI->DBI.SPIBI.PortNumber); // get NSIH

	if (pTBI->SIGNATURE != HEADER_ID) {
		printf(
		    "3rd boot Signature is wrong! SPI boot failure. 0x%08X\r\n",
		    pTBI->SIGNATURE);
		fcs = 1;
		goto spifailure;
	}

	DataAddr = (U8 *)((MPTRS)pTBI->LOADADDR);
	DataSize = (U32)pTBI->LOADSIZE;

	printf("SPI 3rd boot Load Address: 0x%08X\r\n", (MPTRS)DataAddr);
	printf("SPI Load Size   : 0x%08X\r\n", DataSize);
	printf("CRC   : 0x%08X\r\n", pTBI->DBI.SPIBI.CRC32);

	FlashAddr += sizeof(struct NX_SecondBootInfo);

	fcs = SPI_EEPROMRead(FlashAddr, (U32 *)DataAddr, DataSize,
			     pSBI->DBI.SPIBI.AddrStep, 0,
			     pSBI->DBI.SPIBI.PortNumber);

	// if all data are zero, then crc result is zero.
	fcsdata = (U32 *)DataAddr;

	while (DataSize > 0) {
		if (*fcsdata++ != 0)
			break;
		DataSize -= 4;
	}

	if (fcs != pTBI->DBI.SPIBI.CRC32) {
		printf("fcs check failure. generated crc is 0x%08X\r\n", fcs);
		fcs = 1;
	} else if (DataSize) {
		printf("SPI 3rd boot image load success!\r\n");
		fcs = 0;
	}

spifailure:
	SPI_Deinit(pSBI->DBI.SPIBI.PortNumber);

	return !fcs;
}
