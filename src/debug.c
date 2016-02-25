/*                                                                              
 *      Copyright (C) 2012 Nexell Co., All Rights Reserved                      
 *      Nexell Co. Proprietary & Confidential                                   
 *                                                                              
 *      NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE  
 *      AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 *      BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 *      FOR A PARTICULAR PURPOSE.                                               
 *                                                                              
 *      Module          : 2ndboot debug initialize
 *      File            : degug.c
 *      Description     : initialize uart port
 *      Author          : Hans
 *      History         : 2014.08.21 Hans Create
 */

#include "sysHeader.h"

#define UARTSRC         1
#define SOURCE_DIVID    (16UL)
#define BAUD_RATE       (115200)

extern U32  getquotient(int dividend, int divisor);
extern U32  getremainder(int dividend, int divisor);
extern void NX_CLKPWR_SetOSCFreq( U32 FreqKHz );

void    ResetCon(U32 devicenum, CBOOL en);
void    GPIOSetAltFunction(U32 AltFunc);
U32     NX_CLKPWR_GetPLLFrequency(U32 PllNumber);

#ifdef AVN      // avn debug port is UART3
static struct NX_UART_RegisterSet   * const pReg_Uart       = (struct NX_UART_RegisterSet       * const)PHY_BASEADDR_pl01115_Uart_nodma0_MODULE;
static struct NX_CLKGEN_RegisterSet * const pReg_UartClkGen = (struct NX_CLKGEN_RegisterSet     * const)PHY_BASEADDR_CLKGEN25_MODULE;
#else
static struct NX_UART_RegisterSet   * const pReg_Uart       = (struct NX_UART_RegisterSet       * const)PHY_BASEADDR_UART0_MODULE;
static struct NX_CLKGEN_RegisterSet * const pReg_UartClkGen = (struct NX_CLKGEN_RegisterSet     * const)PHY_BASEADDR_CLKGEN22_MODULE;
#endif

#if 0
const U32 UARTBASEADDR[6] =
{
	PHY_BASEADDR_UART0_MODULE,
	PHY_BASEADDR_UART1_MODULE,
	PHY_BASEADDR_pl01115_Uart_modem_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma0_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma1_MODULE,
	PHY_BASEADDR_pl01115_Uart_nodma2_MODULE
};
const U32 UARTCLKGENADDR[6] =
{
	PHY_BASEADDR_CLKGEN22_MODULE,
	PHY_BASEADDR_CLKGEN23_MODULE,
	PHY_BASEADDR_CLKGEN24_MODULE,
	PHY_BASEADDR_CLKGEN25_MODULE,
	PHY_BASEADDR_CLKGEN26_MODULE,
	PHY_BASEADDR_CLKGEN27_MODULE
};
const U8 RESETNUM[6] =
{
	RESETINDEX_OF_UART0_MODULE_nUARTRST,
	RESETINDEX_OF_UART1_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_modem_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_nodma0_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_nodma1_MODULE_nUARTRST,
	RESETINDEX_OF_pl01115_Uart_nodma2_MODULE_nUARTRST,
};
const U32 GPIOALTNUM[12] =
{
	PADINDEX_OF_UART0_UARTRXD,
	PADINDEX_OF_UART0_UARTTXD,
	PADINDEX_OF_UART1_UARTRXD,
	PADINDEX_OF_UART1_UARTTXD,
	PADINDEX_OF_pl01115_Uart_modem_UARTRXD,
	PADINDEX_OF_pl01115_Uart_modem_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma0_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma0_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma1_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma1_UARTTXD,
	PADINDEX_OF_pl01115_Uart_nodma2_UARTRXD,
	PADINDEX_OF_pl01115_Uart_nodma2_UARTTXD,
};
#endif

//------------------------------------------------------------------------------

CBOOL DebugInit( void )
{
	U32 SOURCE_CLOCK;

	NX_CLKPWR_SetOSCFreq( OSC_KHZ );

	SOURCE_CLOCK = NX_CLKPWR_GetPLLFreq(NX_CLKSRC_UART);

#ifdef AVN
#if DIRECT_IO
	register U32 *pGPIOxReg = (U32 *)&pReg_GPIO[(PADINDEX_OF_pl01115_Uart_nodma0_UARTRXD>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_pl01115_Uart_nodma0_UARTRXD>>7)&0x1];
	*pGPIOxReg = (*pGPIOxReg & ~0x0000000C) | 0x00000004;
	pGPIOxReg = (U32 *)&pReg_GPIO[(PADINDEX_OF_pl01115_Uart_nodma0_UARTTXD>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_pl01115_Uart_nodma0_UARTTXD>>7)&0x1];
	*pGPIOxReg = (*pGPIOxReg & ~0x00000C00) | 0x00000400;
#else
	GPIOSetAltFunction(PADINDEX_OF_pl01115_Uart_nodma0_UARTRXD);
	GPIOSetAltFunction(PADINDEX_OF_pl01115_Uart_nodma0_UARTTXD);
#endif
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART3_USESMC)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART3_USESMC)&0x1F)));
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART3_SMCTXENB)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART3_SMCTXENB)&0x1F)));
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART3_SMCRXENB)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART3_SMCRXENB)&0x1F)));

	ResetCon(RESETINDEX_OF_pl01115_Uart_nodma0_MODULE_nUARTRST, CTRUE);	// reset on
	ResetCon(RESETINDEX_OF_pl01115_Uart_nodma0_MODULE_nUARTRST, CFALSE);	// reset negate
#else
#if DIRECT_IO
	register U32 *pGPIOxReg = (U32 *)&pReg_GPIO[(PADINDEX_OF_UART0_UARTRXD>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_UART0_UARTRXD>>7)&0x1];
	*pGPIOxReg = (*pGPIOxReg & ~0x30000000) | 0x10000000;
	pGPIOxReg = (U32 *)&pReg_GPIO[(PADINDEX_OF_UART0_UARTTXD>>8)&0x7]->GPIOxALTFN[(PADINDEX_OF_UART0_UARTTXD>>7)&0x1];
	*pGPIOxReg = (*pGPIOxReg & ~0x00000030) | 0x00000010;
#else
	GPIOSetAltFunction(PADINDEX_OF_UART0_UARTRXD);
	GPIOSetAltFunction(PADINDEX_OF_UART0_UARTTXD);
#endif
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART0_USESMC)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART0_USESMC)&0x1F)));
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART0_SMCTXENB)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART0_SMCTXENB)&0x1F)));
	pReg_Tieoff->TIEOFFREG[((TIEOFFINDEX_OF_UART0_SMCRXENB)&0xFFFF)>>5] &= (~(1<<((TIEOFFINDEX_OF_UART0_SMCRXENB)&0x1F)));

	ResetCon(RESETINDEX_OF_UART0_MODULE_nUARTRST, CTRUE);	// reset on
	ResetCon(RESETINDEX_OF_UART0_MODULE_nUARTRST, CFALSE);	// reset negate
#endif

	pReg_UartClkGen->CLKENB	= (1<<3);								// PCLKMODE : always, Clock Gen Disable
	pReg_UartClkGen->CLKGEN[0]	= ((SOURCE_DIVID-1)<<5) | (NX_CLKSRC_UART<<2);

	//--------------------------------------------------------------------------
	pReg_Uart->LCON		= 0x3;

	pReg_Uart->UCON		= 0x113340;

	pReg_Uart->FCON		= 0x441;

	pReg_Uart->MCON		= 0x00;

	pReg_Uart->BRDR		= getquotient(getquotient(SOURCE_CLOCK, SOURCE_DIVID), ((BAUD_RATE/1)*16))-1;
	pReg_Uart->FRACVAL	= getquotient(((getremainder(getquotient(SOURCE_CLOCK, SOURCE_DIVID), ((BAUD_RATE/1)*16))+32)*16), ((BAUD_RATE/1)*16));

	pReg_UartClkGen->CLKENB	= (1<<3) | (1<<2);		// PCLKMODE : always, Clock Gen Enable

	pReg_Uart->UCON		= 0x113345;
	//--------------------------------------------------------------------------

	return CTRUE;
}

#if 0
void TestUartInit(U32 port)
{

	if(port == 0)
	{
	}else
		if(port == 1)
		{
		}else
			if(port == 2)
			{
			}else
				if(port == 3)
				{
				}

}
#endif

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

#if 0
//------------------------------------------------------------------------------

void	DebugPutString( const S8* const String )
{
	const S8 *pString;

	pString = (const S8 *)String;
	while( CNULL != *pString )
		DebugPutch(*pString++);
}

S32		DebugGetString( S8* const pStringBuffer )
{
	S8	*pString = pStringBuffer;
	S8	buf;
	S32		iSize	= 0;

	while(1)
	{
		/* get character */
		buf = DebugGetch();

		/* backspace */
		if( buf == 0x08 )
		{
			if( iSize > 0 )
			{
				DebugPutch(buf);
				DebugPutch(' ');
				DebugPutch(buf);

				pString--;
				iSize--;
			}

			continue;
		}

		/* print character */
		DebugPutch(buf);

		if(buf == '\r')	break;

		/* increase string index */
		*pString++ = buf;
		iSize++;
	}

	*pString++ = '\0';

	return iSize;
}

void	DebugPrint( const S8* const FormatString, ... )
{
	static S8 String[256];

	va_list argptr;
	va_start(argptr, FormatString);
	vsprintf((S8*)String, FormatString, argptr);
	va_end(argptr);

	DebugPutString(String);
}

//------------------------------------------------------------------------------
void	DebugPutDec( S32 value )
{
	S8 ch[16];
	U32 data;
	S32	i, chsize;

	data = (value < 0) ? (U32)(-value) : (U32)value;

	chsize = 0;
	do
	{
		ch[chsize++] = getremainder(data , 10) + '0';
		data = getquotient(data,10);
	} while( data != 0 );

	if( value < 0 )		DebugPutch( '-' );

	for( i=0 ; i<chsize ; i++ )
	{
		DebugPutch( ch[chsize - i - 1] );
	}
}

//------------------------------------------------------------------------------
void	DebugPutHex( S32 value )
{
	S8 ch;
	U32 data;
	S32	i;

	data = (U32)value;

	DebugPutch( '0' );
	DebugPutch( 'x' );

	for( i=0 ; i<8 ; i++ )
	{
		ch = (S8)(( data >> (28 - i*4) ) & 0xF);
		ch = (ch > 9 ) ? (ch - 10 + 'A') : (ch + '0');
		DebugPutch( ch );
	}
}

//------------------------------------------------------------------------------
void	DebugPutByte( S8 value )
{
	S8 ch;
	U32 data;
	S32	i;

	data = (U32)value;

	for( i=0 ; i<2 ; i++ )
	{
		ch = (S8)(( data >> (4 - i*4) ) & 0xF);
		ch = (ch > 9 ) ? (ch - 10 + 'A') : (ch + '0');
		DebugPutch( ch );
	}
}

//------------------------------------------------------------------------------
void	DebugPutWord( S16 value )
{
	S8 ch;
	U32 data;
	S32	i;

	data = (U32)value;

	for( i=0 ; i<4 ; i++ )
	{
		ch = (S8)(( data >> (12 - i*4) ) & 0xF);
		ch = (ch > 9 ) ? (ch - 10 + 'A') : (ch + '0');
		DebugPutch( ch );
	}
}

//------------------------------------------------------------------------------
void	DebugPutDWord( S32 value )
{
	S8 ch;
	U32 data;
	S32	i;

	data = (U32)value;

	for( i=0 ; i<8 ; i++ )
	{
		ch = (S8)(( data >> (28 - i*4) ) & 0xF);
		ch = (ch > 9 ) ? (ch - 10 + 'A') : (ch + '0');
		DebugPutch( ch );
	}
}
#endif
