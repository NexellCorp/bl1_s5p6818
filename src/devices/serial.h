#ifndef __SERIAL_H__
#define __SERIAL_H__

struct s5p6818_uart_reg
{
	volatile unsigned int	lcon	;	// 0x00	// Line Control
	volatile unsigned int	ucon	;	// 0x04	// Control
	volatile unsigned int	fcon	;	// 0x08	// FIFO Control
	volatile unsigned int	mcon	;	// 0x0C	// Modem Control
	volatile unsigned int	ustatus	;	// 0x10	// Tx/Rx Status
	volatile unsigned int	estatus	;	// 0x14	// Rx Error Status
	volatile unsigned int	fstatus	;	// 0x18	// FIFO Status
	volatile unsigned int	mstatus	;	// 0x1C	// Modem Status
	volatile unsigned int	thr	;	// 0x20	// Transmit Buffer
	volatile unsigned int	rbr	;	// 0x24	// Recieve Buffer
	volatile unsigned int	brdr	;	// 0x28	// Baud Rate Driver
	volatile unsigned int	fracval	;	// 0x2C	// Driver Fractional Value
	volatile unsigned int	intp	;	// 0x30	// Instrrupt Pending
	volatile unsigned int	intsp	;	// 0x34	// Instrrupt Source
	volatile unsigned int	intm	;	// 0x38	// Instrrupt Mask
};

/* Function Define */
 int serial_init(unsigned int channel);

 int serial_getch(void);
void serial_putch(int ch);

 int serial_is_uart_tx_done(void);
 int serial_is_tx_empty(void);
 int serial_is_busy(void);

#endif // __SERIAL_H__
