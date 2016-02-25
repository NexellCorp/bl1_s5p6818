/*                                                                             
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                             
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                             
 *      Module          : UART
 *      File            : iUARTBOOT.c
 *      Description     : 
 *      Author          : Hans
 *      History         : 2013.01.10 First Implementation
 *			  2013.08.24 Uart Port Selectable Feature Add
 *			  2014.02.23 change to new uart device.
 */

#include "sysHeader.h"

//------------------------------------------------------------------------------
#define SOURCE_CLOCK	NX_CLKSRCPLL1_FREQ
#define SOURCE_DIVID	(10UL)				// 147456000/10 = 14745600
#define BAUD_RATE		(115200)

void ResetCon(U32 devicenum, CBOOL en);
void Decrypt(U32 *SrcAddr, U32 *DestAddr, U32 Size);
void GPIOSetAltFunction(U32 AltFunc);
U32 get_fcs(U32 fcs, U8 data);
/*
   uart 0
   rxd d 14 1

   uart 1
   rxd d 15 1
 */
extern struct NX_GPIO_RegisterSet (* const pGPIOReg)[1];

static U32 const UARTResetNum[2] =
{
	RESETINDEX_OF_UART0_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_modem_MODULE_nUARTRST
};
static struct NX_CLKGEN_RegisterSet * const pgUartClkGenReg[2] =
{
	(struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN22_MODULE,
	(struct NX_CLKGEN_RegisterSet *)PHY_BASEADDR_CLKGEN24_MODULE
};
static struct NX_UART_RegisterSet * const pgUARTReg[2] =
{
	(struct NX_UART_RegisterSet *)PHY_BASEADDR_UART0_MODULE,
	(struct NX_UART_RegisterSet *)PHY_BASEADDR_pl01115_Uart_modem_MODULE
};

CBOOL	iUARTBOOT( U32 option )
{
	register struct NX_SecondBootInfo *pSBI;
	register U8 *pdwBuffer =(U8*)BASEADDR_SRAM;
	register S32 iRxSize = 0;
	register U32 *pGPIOxReg = (U32 *)&pGPIOReg[GPIO_GROUP_D]->GPIOxALTFN[0];	// uart 0 gpio d 14 alt 1, uart 1 gpio d 15 alt 1
	register U32 UartPort = 0;
	register U32 regvalue;
	CBOOL	ret = CFALSE;

	register struct NX_UART_RegisterSet * const pUARTReg = pgUARTReg[UartPort];

	struct NX_CLKGEN_RegisterSet * const pUartClkGenReg = pgUartClkGenReg[UartPort];

	pUartClkGenReg->CLKENB	= (1<<3);								// PCLKMODE : always, Clock Gen Disable
	pUartClkGenReg->CLKGEN[0]	= ((SOURCE_DIVID-1)<<5) | (1<<2);	// UARTCLK = PLL1 / 10 = 147,456,000 / 10 = 14,745,600 Hz

	ResetCon(UARTResetNum[UartPort], CTRUE);	// reset on
	ResetCon(UARTResetNum[UartPort], CFALSE);	// reset negate

	if(UartPort == 0)
		*pGPIOxReg = (*pGPIOxReg & ~0x30000000) | 0x10000000;	// uart 0 gpio d 14 alt 1
	else
		*pGPIOxReg = (*pGPIOxReg & ~0xC0000000) | 0x40000000;	// uart 1 gpio d 15 alt 1

	//--------------------------------------------------------------------------
	// LCON - [normal mode] / [no parity] / [one stop bit per frame] / [word length 8]
	//regvalue = NX_UART_NORMAL_MODE_OPERATION | NX_UART_NO_PARITY | NX_UART_ONE_STOP_BIT_PER_FRAME | NX_UART_8BIT_DATA;
	regvalue = 0x3;
	pUARTReg->LCON			= regvalue;

	// UCON - [tx dma 4 burst] / [rx dma 4 burst] / [rx timeout interrupt interval : 3] / [disable rx timeout when rx fifo empty]
	//		/ [disable suspending rx dma fsm] / [tx interrupt type - level] / [rx interrupt type - level] / [disable rx timeout]
	//		/ [enable rx error status interrupt] / [disable loop back] / [disable send break signal] / [transmit mode - disable]
	//		/ [receive mode - polling]
	//regvalue = NX_UART_TX_DMA_BURST_SIZE_4 | NX_UART_RX_DMA_BURST_SIZE_4 | (0x3<<12) | NX_UART_DISABLE_RX_TIMEOUT_WITH_EMPTY_RXFIFO
	//			| NX_UART_DISABLE_SUSPENDING_RX_DMA_FSM | NX_UART_TX_INTERRUPT_TYPE_LEVEL | NX_UART_RX_INTERRUPT_TYPE_LEVEL
	//			| NX_UART_RX_DISABLE_TIMEOUT_INTERRUPT | NX_UART_GENERATE_RECIEVE_ERROR_STATUS_INTERRUPT | NX_UART_ONE_WAY_OPERATION
	//			| NX_UART_NORMAL_TRANSMIT | NX_UART_DISABLE_TRANSMIT | NX_UART_DISABLE_RECIEVE;
	regvalue = 0x113340;
	pUARTReg->UCON			= regvalue;

	// FCON - [tx fifo trigger level - 4] | [rx fifo trigger level - 4] | [disable tx fifo reset] | [disable rx fifo reset]
	//		| [enbale fifo]
	//regvalue = NX_UART_TX_FIFO_TRIGGER_LEVEL_4 | NX_UART_RX_FIFO_TRIGGER_LEVEL_4 | NX_UART_FIFO_ENABLE;
	regvalue = 0x441;
	pUARTReg->FCON			= regvalue;

	if(option & 1<<UARTBAUD)
	{
		//	#ifdef ENV_TEST_FAST
		//		pUARTReg->BRDR		= (U16)(0x1);	// IBRD = (2-1), 460800bps
		//	#else
		pUARTReg->BRDR		= (U16)(0x7);	// IBRD = (8-1), 115200bps
		//	#endif
		pUARTReg->FRACVAL	= (U16)(0x0);	// FBRD = 0,
	}
	else
	{
		//pUARTReg->BRDR		= (U16)(SOURCE_CLOCK/SOURCE_DIVID / ((BAUD_RATE/6)*16));	// IBRD = 48, 19200bps
		//pUARTReg->FRACVAL	= (U16)(SOURCE_CLOCK/SOURCE_DIVID % ((BAUD_RATE/6)*16));	// FBRD = 0,
		pUARTReg->BRDR		= (U16)(0x47);	// IBRD = (48-1), 19200bps
		pUARTReg->FRACVAL	= (U16)(0x0);	// FBRD = 0,
	}

	pUartClkGenReg->CLKENB	= (1<<3) | (1<<2);		// PCLKMODE : always, Clock Gen Enable
	// UCON - [tx dma 4 burst] / [rx dma 4 burst] / [rx timeout interrupt interval : 3] / [disable rx timeout when rx fifo empty]
	//		/ [disable suspending rx dma fsm] / [tx interrupt type - level] / [rx interrupt type - level] / [disable rx timeout]
	//		/ [enable rx error status interrupt] / [disable loop back] / [disable send break signal] / [transmit mode - disable]
	//		/ [receive mode - polling]
	//regvalue = NX_UART_TX_DMA_BURST_SIZE_4 | NX_UART_RX_DMA_BURST_SIZE_4 | (0x3<<12) | NX_UART_DISABLE_RX_TIMEOUT_WITH_EMPTY_RXFIFO
	//			| NX_UART_DISABLE_SUSPENDING_RX_DMA_FSM | NX_UART_TX_INTERRUPT_TYPE_LEVEL | NX_UART_RX_INTERRUPT_TYPE_LEVEL
	//			| NX_UART_RX_DISABLE_TIMEOUT_INTERRUPT | NX_UART_GENERATE_RECIEVE_ERROR_STATUS_INTERRUPT | NX_UART_ONE_WAY_OPERATION
	//			| NX_UART_NORMAL_TRANSMIT | NX_UART_DISABLE_TRANSMIT | NX_UART_POLLING_MODE_RECIEVE;
	regvalue = 0x113341;
	pUARTReg->UCON			= regvalue;

	while( 512 > iRxSize )
	{
		if( (pUARTReg->FSTATUS & 0xFF) != 0 )	// check if RX fifo is not empty
		{
			register U32 RXDATA;

			if( pUARTReg->ESTATUS & 0xF )
			{
				NX_DEBUG_MSG("Data Receive Error!\r\n");
				break;
			}
			RXDATA = pUARTReg->RBR;

			pdwBuffer[iRxSize++] = (U8)(RXDATA & 0xFF);
		}
	}

	if(option & 1<< DECRYPT)
		Decrypt((U32*)pdwBuffer, (U32*)pdwBuffer, 512);

	pSBI = (struct NX_SecondBootInfo *)pdwBuffer;

	if(pSBI->SIGNATURE == HEADER_ID )
	{
		S32 i, BootSize = pSBI->LOADSIZE;
		U32 fcs;


		pdwBuffer = (U8*)(BASEADDR_SRAM+sizeof(struct NX_SecondBootInfo));
		iRxSize = 0;

		while( BootSize > iRxSize )
		{
			if( (pUARTReg->FSTATUS & 0xFF) != 0 )	// check if RX fifo is not empty
			{
				if( pUARTReg->ESTATUS & 0xF )
				{
					NX_DEBUG_MSG("Data Receive Error!\r\n");
					break;
				}

				pdwBuffer[iRxSize++] = (U8)(pUARTReg->RBR & 0xFF);
			}
		}
		if(option & 1<< DECRYPT)
			Decrypt((U32*)pdwBuffer, (U32*)pdwBuffer, BootSize);

		fcs = 0;
		for(i=0; i<BootSize; i++)
			fcs = get_fcs(fcs, pdwBuffer[i]);

		if(fcs != 0 && fcs == pSBI->DBI.UARTBI.CRC32)
			ret = CTRUE;
	}

	pUARTReg->UCON			= 0x0;					// all disable

	pUartClkGenReg->CLKENB	= (1<<3);				// PCLKMODE : always, Clock Gen Disable

	if(UartPort == 0)
		*pGPIOxReg &= ~0x30000000;	// uart 0 gpio d 14 alt 0
	else
		*pGPIOxReg &= ~0xC0000000;	// uart 1 gpio d 15 alt 0

	ResetCon(UARTResetNum[UartPort], CTRUE);	// reset on

	//--------------------------------------------------------------------------
	return ret;
}

