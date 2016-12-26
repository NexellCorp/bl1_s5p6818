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

#define CPU_ALIVE_FLAG_ADDR		0xC0010230

struct nx_subcpu_bringup_info {
	volatile unsigned int jump_addr;
	volatile unsigned int cpu_id;
	volatile unsigned int wakeup_flag;
};

/* External Function */
extern void ResetCon(unsigned int devicenum, int en);
extern void DMC_Delay(int milisecond);

extern void SwitchToEL2(void);
extern void secure_set_gic_all(void);

//------------------------------------------------------------------------------
#if (MULTICORE_BRING_UP == 1)
#ifdef aarch32
static void bringup_slave_cpu(unsigned int cpu_id)
{
    mmio_write_32( &pReg_ClkPwr->CPURESETMODE,      0x1);
    mmio_write_32( &pReg_ClkPwr->CPUPOWERDOWNREQ,   (1 << cpu_id) );
    mmio_write_32( &pReg_ClkPwr->CPUPOWERONREQ,     (1 << cpu_id) );
}

static void set_vector_location(unsigned int cpu_id, int LowHigh)
{
	unsigned int addr, bits, regvalue;

	if (cpu_id & 0x4) { // cpu 4, 5, 6, 7
		addr = (unsigned int)&pReg_Tieoff->TIEOFFREG[95];
		bits = 1 << (12 + (cpu_id & 0x3));
	} else {
		addr = (unsigned int)&pReg_Tieoff->TIEOFFREG[78];
		bits = 1 << (20 + (cpu_id & 0x3));
	}

	regvalue = mmio_read_32(addr);
	if (LowHigh)
		regvalue |= bits;
	else
		regvalue &= ~bits;
	mmio_write_32(addr, regvalue);
}
#endif

#ifdef aarch64
static void bringup_slave_cpu(unsigned int cpu_id)
{
	mmio_write_32(&pReg_ClkPwr->CPURESETMODE, 0x1);
	mmio_write_32(&pReg_ClkPwr->CPUPOWERONREQ, (1 << cpu_id));
	ClearIO32(&pReg_ClkPwr->CPUPOWERONREQ, (1 << cpu_id));
}

static void set_vector_location(unsigned int cpu_id, int LowHigh)
{
	unsigned int regvalue;
	LowHigh = LowHigh;	// for AArch32 comfortable
	if (cpu_id & 0x4) {	// cpu 4, 5, 6, 7
		cpu_id &= 0x3;
		regvalue = mmio_read_32(&pReg_Tieoff->TIEOFFREG[96]);
		regvalue |= 1 << (4 + cpu_id);
		mmio_write_32(&pReg_Tieoff->TIEOFFREG[96], regvalue);
		mmio_write_32(&pReg_Tieoff->TIEOFFREG[97 + (cpu_id << 1)], 0xFFFF0200 >> 2);
	} else {		// cpu 0, 1, 2, 3 
		regvalue = mmio_read_32(&pReg_Tieoff->TIEOFFREG[79]);
		regvalue |= 1 << (12 + cpu_id); // set cpu mode to AArch64
		mmio_write_32(&pReg_Tieoff->TIEOFFREG[79], regvalue);
		mmio_write_32(&pReg_Tieoff->TIEOFFREG[80 + (cpu_id << 1)], 0xFFFF0200 >> 2);
	}
}
#endif

#endif // #if (MULTICORE_BRING_UP == 1)

void s5p6818_subcpu_on(unsigned int cpu_id)
{
	register struct nx_subcpu_bringup_info *pcpu_start_info =
	    (struct nx_subcpu_bringup_info *)CPU_ALIVE_FLAG_ADDR;

#if (SUPPORT_KERNEL_3_4 == 1)
	mmio_write_32( &pReg_GIC400->GICC.CTLR,         0x07 );			// enable cpu interface
	mmio_write_32( &pReg_GIC400->GICC.PMR,          0xFF );			// all high priority
	mmio_write_32( &pReg_GIC400->GICD.ISENABLER[0], 0xFF );			// enable sgi all
	mmio_write_32( &pReg_ClkPwr->CPUPOWERONREQ,     0x00 );			// clear own wakeup req bit
#else
	secure_set_gic_all();
	mmio_write_32(&pReg_GIC400->GICD.ISENABLER[0], 0xFF);			// enable sgi all
	mmio_write_32(&pReg_ClkPwr->CPUPOWERONREQ, 0x00);			// clear own wakeup req bit
#endif

//	printf("Hello World. I'm CPU %d!\r\n", cpu_id);
	pcpu_start_info->wakeup_flag = 1;
	putchar('0' + cpu_id);

#ifdef aarch64
	SwitchToEL2();
#endif
	do {
		register void (*pLaunch)(void);
		__asm__ __volatile__("wfi");

//		mmio_write_32(&pReg_GIC400->GICD.ICPENDR[0], 1<<cpu_id);
//		mmio_write_32(&pReg_GIC400->GICC.EOIR, mmio_read_32(&pReg_GIC400->GICC.IAR));
//		_asm__ __volatile__ ("wfe");
		pLaunch = (void (*)(void))((MPTRS)pcpu_start_info->jump_addr);
		if ((MPTRS)pLaunch != (MPTRS)0xFFFFFFFF) {
			if (cpu_id == pcpu_start_info->cpu_id)
				pLaunch();
		}
	} while (1);
}

//------------------------------------------------------------------------------
int s5p6818_subcpu_bringup(unsigned int cpu_id)
{
	register struct nx_subcpu_bringup_info *pcpu_start_info =
	    (struct nx_subcpu_bringup_info *)CPU_ALIVE_FLAG_ADDR;
	int cpu_num, retry = 0;
	int result = cpu_id;

	mmio_write_32(&pReg_GIC400->GICC.CTLR, 0x07);				// enable cpu interface
	mmio_write_32(&pReg_GIC400->GICC.PMR,  0xFF);				// all high priority
	mmio_write_32(&pReg_GIC400->GICD.CTLR, 0x03);				// distributor enable

	NOTICE("Wakeup Sub CPU ");

#if (MULTICORE_BRING_UP == 1)
	pcpu_start_info->jump_addr = (unsigned int)0xFFFFFFFF;			// set cpu jump info to invalid

	for (cpu_num = 1; cpu_num < 8; ) {
		register volatile unsigned int delay;

		pcpu_start_info->wakeup_flag = 0;
		delay = 0x10000;
		set_vector_location(cpu_num, CTRUE);				// CTRUE: High Vector(0xFFFF0000), CFALSE: Low Vector (0x0)
		bringup_slave_cpu(cpu_num);
		DMC_Delay(10000);
		while((pcpu_start_info->wakeup_flag == 0) && (--delay));
		if(delay == 0) {
			if(retry > 3) {
				ERROR("maybe cpu %d is dead. -_-;\r\n", cpu_num);
				cpu_num++;    // try next cpu bringup
				retry = 0;
				result = CFALSE;
			} else {
				WARN("core %d is not bringup, retry\r\n", cpu_num);
				retry++;
			}
		} else {
			retry = 0;
			result++;
			cpu_num++;    // try next cpu bringup
		}
		DMC_Delay(10000);
	}
#endif  // #if (MULTICORE_BRING_UP == 1)
	printf("\r\n");
	NOTICE("CPU Wakeup Done! WFI is expected.\r\n");
	NOTICE("CPU%d is Master!\r\n\n", cpu_id);
	return result;
}
