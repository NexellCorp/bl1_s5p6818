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

#include "sysheader.h"

#define SOURCE_DIVID    (4UL)
#define BAUD_RATE       (115200)

extern U32  getquotient(int dividend, int divisor);
extern U32  getremainder(int dividend, int divisor);
extern void NX_CLKPWR_SetOSCFreq( U32 FreqKHz );

void    ResetCon(U32 devicenum, CBOOL en);
void    GPIOSetAltFunction(U32 AltFunc);
U32     NX_CLKPWR_GetPLLFrequency(U32 PllNumber);

static struct NX_UART_RegisterSet * pReg_Uart;

const U32 UARTBASEADDR[] =
{
	PHY_BASEADDR_UART0_MODULE,
	PHY_BASEADDR_pl01115_Uart_modem_MODULE,
	PHY_BASEADDR_UART1_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma0_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma1_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma2_MODULE
};
const U32 UARTCLKGENADDR[] =
{
	PHY_BASEADDR_CLKGEN22_MODULE,
	PHY_BASEADDR_CLKGEN24_MODULE,
	PHY_BASEADDR_CLKGEN23_MODULE,
	PHY_BASEADDR_CLKGEN25_MODULE,
	PHY_BASEADDR_CLKGEN26_MODULE
};
const U8 RESETNUM[] =
{
	RESETINDEX_OF_UART0_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_modem_MODULE_nUARTRST,
	RESETINDEX_OF_UART1_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_nodma0_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_nodma1_MODULE_nUARTRST
};
const U32 GPIOALTNUM[] =
{
	PADINDEX_OF_UART0_UARTRXD,
	PADINDEX_OF_UART0_UARTTXD,
	PADINDEX_OF_pl01115_Uart_modem_UARTRXD,
	PADINDEX_OF_pl01115_Uart_modem_UARTTXD,
	PADINDEX_OF_UART1_UARTRXD,
	PADINDEX_OF_UART1_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma0_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma0_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma1_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma1_UARTTXD,
};
const U32 UARTSMC[] =
{
	TIEOFFINDEX_OF_UART0_USESMC,
	TIEOFFINDEX_OF_UART0_SMCTXENB,
	TIEOFFINDEX_OF_UART0_SMCRXENB,
	TIEOFFINDEX_OF_UART2_USESMC,
	TIEOFFINDEX_OF_UART2_SMCTXENB,
	TIEOFFINDEX_OF_UART2_SMCRXENB,
	TIEOFFINDEX_OF_UART1_USESMC,
	TIEOFFINDEX_OF_UART1_SMCTXENB,
	TIEOFFINDEX_OF_UART1_SMCRXENB,
	TIEOFFINDEX_OF_UART3_USESMC,
	TIEOFFINDEX_OF_UART3_SMCTXENB,
	TIEOFFINDEX_OF_UART3_SMCRXENB,
	TIEOFFINDEX_OF_UART4_USESMC,
	TIEOFFINDEX_OF_UART4_SMCTXENB,
	TIEOFFINDEX_OF_UART4_SMCRXENB,
	TIEOFFINDEX_OF_UART5_USESMC,
	TIEOFFINDEX_OF_UART5_SMCTXENB,
	TIEOFFINDEX_OF_UART5_SMCRXENB
};

//------------------------------------------------------------------------------

CBOOL DebugInit( U32 port )
{
	U32 SOURCE_CLOCK;
	pReg_Uart = (struct NX_UART_RegisterSet *)((MPTRS)UARTBASEADDR[port]);
	struct NX_CLKGEN_RegisterSet * const pReg_UartClkGen = (struct NX_CLKGEN_RegisterSet * const)((MPTRS)UARTCLKGENADDR[port]);
	U32 reset_number = RESETNUM[port];
	U32 GPIO_TXD = GPIOALTNUM[port * 2 + 1];
	U32 GPIO_RXD = GPIOALTNUM[port * 2 + 0];

	NX_CLKPWR_SetOSCFreq( OSC_KHZ );

	SOURCE_CLOCK = NX_CLKPWR_GetPLLFreq(NX_CLKSRC_UART);

	GPIOSetAltFunction(GPIO_RXD);
	GPIOSetAltFunction(GPIO_TXD);

	pReg_Tieoff->TIEOFFREG[((UARTSMC[port*3+0])&0xFFFF)>>5] &= (~(1<<((UARTSMC[port*3+0])&0x1F)));
	pReg_Tieoff->TIEOFFREG[((UARTSMC[port*3+1])&0xFFFF)>>5] &= (~(1<<((UARTSMC[port*3+1])&0x1F)));
	pReg_Tieoff->TIEOFFREG[((UARTSMC[port*3+2])&0xFFFF)>>5] &= (~(1<<((UARTSMC[port*3+2])&0x1F)));

	ResetCon(reset_number, CTRUE);	// reset on
	ResetCon(reset_number, CFALSE); // reset negate

	pReg_UartClkGen->CLKENB = (1<<3);								// PCLKMODE : always, Clock Gen Disable
	pReg_UartClkGen->CLKGEN[0]	= ((SOURCE_DIVID-1)<<5) | (NX_CLKSRC_UART<<2);

	//--------------------------------------------------------------------------
	pReg_Uart->LCON 	= 0x3;

	pReg_Uart->UCON 	= 0x113340;

	pReg_Uart->FCON 	= 0x441;

	pReg_Uart->MCON 	= 0x00;

	pReg_Uart->BRDR 	= getquotient(getquotient(SOURCE_CLOCK, SOURCE_DIVID), ((BAUD_RATE/1)*16))-1;
	pReg_Uart->FRACVAL	= getquotient(((getremainder(getquotient(SOURCE_CLOCK, SOURCE_DIVID), ((BAUD_RATE/1)*16))+32)*16), ((BAUD_RATE/1)*16));

	pReg_UartClkGen->CLKENB = (1<<3) | (1<<2);		// PCLKMODE : always, Clock Gen Enable

	pReg_Uart->UCON 	= 0x113345;
	//--------------------------------------------------------------------------

	return CTRUE;
}

void	DebugPutch( S8 ch )
{
	const U32 TX_FIFO_FULL	= 1<<24;
	while( pReg_Uart->FSTATUS & TX_FIFO_FULL )	{ ; }
	pReg_Uart->THR = (U32)ch;
}

CBOOL	DebugIsUartTxDone(void)
{
	const U32 TX_TRANS_EMPTY	= 1<<2;
	if ( (pReg_Uart->FSTATUS >> 16) & 0xFF )
		return (CBOOL)CFALSE;
	else
		return (CBOOL)(pReg_Uart->USTATUS & TX_TRANS_EMPTY) ? CTRUE : CFALSE;
}

CBOOL	DebugIsTXEmpty(void)
{
	return (CBOOL)( (pReg_Uart->FSTATUS >> 16) & 0xFF) ? CFALSE : CTRUE;
}

CBOOL	DebugIsBusy(void)
{
	const U32 TX_TRANS_EMPTY	= 1<<2;
	return (CBOOL)(pReg_Uart->USTATUS & TX_TRANS_EMPTY) ? CTRUE : CFALSE;
}

S8	DebugGetch( void )
{
	const U32 RX_FIFO_EMPTY	= 1<<4;
	while( pReg_Uart->FSTATUS & RX_FIFO_EMPTY )	{ ; }
	return (S8)pReg_Uart->RBR;
}
