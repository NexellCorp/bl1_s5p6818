#ifndef __CCI400_H__
#define __CCI400_H__

typedef enum
{
	BUSID_CS	= 0,				// Core Sight
	BUSID_CODA	= 1,
	BUSID_TOP	= 2,
	BUSID_CPUG1	= 3,
	BUSID_CPUG0	= 4
} BUSID;

struct cci_slave_if_reg
{
	volatile unsigned int scr;			// 0x0000 Snoop Control
	volatile unsigned int sor;			// 0x0004 Shareable Override for slave interface
	volatile unsigned int reserved0[0x3E];		// 0x0008~0x00FC
	volatile unsigned int rc_qos_vor;		// 0x0100 Read Channel QoS Value Override
	volatile unsigned int wc_qos_svor;		// 0x0104 Write Channel QoS Value Override
	volatile unsigned int reserved1;		// 0x0108
	volatile unsigned int qos_cr;			// 0x010C QoS Control
	volatile unsigned int motr;			// 0x0110 Max OT
	volatile unsigned int reserved2[0x7];		// 0x0114~0x012C
	volatile unsigned int rtr;			// 0x0130 Regulator Target
	volatile unsigned int qos_rsfr;			// 0x0134 QoS Regulator Scale Factor
	volatile unsigned int qos_rr;			// 0x0138 QoS Range
	volatile unsigned int reserved3[0x3B1];		// 0x013C~0x0FFC
};

struct cci_percnt_reg
{
	volatile unsigned int esr;			// 0x0000 Event Select
	volatile unsigned int ecr;			// 0x0004 Event Count
	volatile unsigned int cctrlr;			// 0x0008 Counter Control
	volatile unsigned int offsr;			// 0x000C Overflow Flag Status
	volatile unsigned int reserved[0x3FC];		// 0x0010~0x0FFC
};

struct	s5p6818_cci400_reg
{
	volatile unsigned int cor;			// 0x0000 Control Override
	volatile unsigned int scr;			// 0x0004 Speculation Control
	volatile unsigned int sar;			// 0x0008 Secure Access
	volatile unsigned int stsr;			// 0x000C Status
	volatile unsigned int ier;			// 0x0010 Imprecise Error
	volatile unsigned int reserved0[0x3B];		// 0x0014~0x00FC
	volatile unsigned int pmcr;			// 0x0100 Performance Monitor Control
	volatile unsigned int reserved1[0x3B3];		// 0x0104~0x0FCC
	volatile unsigned int cpidr[0xC];		// 0x0FD0~0x0FFC
	struct cci_slave_if_reg csi[5];			// 0x1000~0x5FFC
	volatile unsigned int reserved2[0xC01];		// 0x6000~0x9000
	volatile unsigned int ccr;			// 0x9004 Cycle counter
	volatile unsigned int cctrlr;			// 0x9008 Count Control
	volatile unsigned int offsr;			// 0x900C Overflow Flag Status
	volatile unsigned int reserved3[0x3FC];		// 0x9010~0x9FFC
	struct cci_percnt_reg cpc[4];			// 0xA000~DFFC
	volatile unsigned int reserved4[0x800];		// 0xE000~0xFFFC
};

/* Function Define */
void cci400_initialize(void);

#endif // __CCI400_H__
