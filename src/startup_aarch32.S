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
#include <s5p6818.h>
#include "nx_peridot.h"

	.align 8
/*
 * start and end of BSS
 */

.globl __bss_start__
.globl __bss_end__

/*
 * entry point of main function
 */
.global main
.global s5p6818_subcpu_on

/* exception vector */
.global vectors
vectors:
        b       reset_handler				// 00 - reset
        b       .					// 04 - undefined instructions
        b       .					// 08 - swi  instructions
        b       .					// 0C - instruction fetch aborts
        b       .					// 10 - data access aborts
        b       .					// 14 - reserved (was address exception)
        b       .					// 18 - irq interrupts
        b       .					// 1C - fiq interrupts

	.skip	0x20, 0x00				//20 : Reserved (0x20 ~ 0x40)

header_info:
	.word	0x00008000				// 40 : Device Address
	.word	0x00008000				// 44 : Next Bootloader Load Size
	.word	0xFFFF0000				// 48 : Load Address
	.word	0xFFFF0000				// 4C : Launch Address

	.skip	0xB0, 0x00				// 50 : Reserved (0x50 ~ 0x100)
	.skip	0x100, 0x00				// 100 : Reserved (0x100 ~ 0x200)

#if (SUPPORT_KERNEL_3_4 == 0)
	.skip	0x200, 0x00				// 200 : secure reserved (0x200 ~ 0x400)
#endif
	.skip	0x20, 0x00				// 200/400: support the kernel 3.4 suspend

.global sleep
sleep:
        b       system_sleep				// 20 : System Sleep

build_info:
        .word   0x68181000				//24 : Chip name - 6818, Build num - v0.3.06

reset_handler:
//	mcr     p15, 0 r0, c8, c9, 0			// set debug break;
	mrc     p15, 0, r12, c0, c0, 5			// Get our cpu id
#if (CONFIG_RESET_AFFINITY_ID == 1)
	and     r11, r12, #(0xFF << 8)			// mask cluster id
	and     r12, r12, #0xFF				// mask cpu id
	orr     r12, r11, lsr #6			// save cpu id
#else
	tst     r12, #0x4400				// check processor affinity
	orrne   r12, r12, #4				// mark cpu0 or cpu1
#endif
	ands    r12, r12, #0xF

	msr     CPSR_c, #(Mode_SVC|I_Bit|F_Bit)

	bne     cpu_bringup

	/* release pad holding */
        ldr     r0, =0xc0010800 			// alive base address
        mov     r1, #0x01
        str     r1, [r0, #0x00]				// open write gate
//        mov     r1, #0x3C0
        mov     r1, #0x3FC
        str     r1, [r0, #0x9c]				// disable pad holding

	/* set smp start address to invalid */
        mov     r0, #0xC0000000				// peri base
        orr     r0, r0, #0x00010000			// clk N pwr offset
        orr     r0, r0, #0x00000230			// scratch register
        mov     r1, #0xFFFFFFFF				// mark to invalid address
        str     r1, [r0]				// set invalid jump address

        /* clear area of global data. */
        mov     r2, #0
        ldr     r1, =g_suspend_ready
        str     r2, [r1]
        ldr     r1, =g_wakeup_ready
        str     r2, [r1]
        ldr     r1, =g_store_stack
        str     r2, [r1]
        ldr     r1, =g_store_vaddr
        str     r2, [r1], #4
        str     r2, [r1], #4
        str     r2, [r1], #4
        str     r2, [r1], #4
        str     r2, [r1], #4
        str     r2, [r1], #4
        str     r2, [r1], #4
        str     r2, [r1], #4

#if 0
        ldr     r1, =__bss_start__			// this is auto-relocated!

        mov     r3, #INTERNAL_SRAM_SIZE
        mov     r4, #0xFF000000
        orr     r4, r4, #0x00FF0000
        sub     r2, r3, #(4096)				// sram_size - stack_size
        orr     r2, r2, r4
#else

        ldr     r1, =__bss_start__			// this is auto-relocated!
        ldr     r2, =__bss_end__			// this is auto-relocated!
#endif
        mov     r3, #0x00000000				// prepare zero to clear BSS

clear_bss:
        cmp     r1, r2					// while not at end of BSS
        strlo   r3, [r1]				// clear 32-bit BSS word
        addlo   r1, r1, #4				// move to next
        blo     clear_bss

cpu_bringup:

#if (MULTICORE_BRING_UP == 1)
#if 0
        .word   0xEC510F1F				// mrrc p15, 1, r0, r1, c15        - smp enable
        .word   0xE3800040				// orr r0, r0, #0x40
        .word   0xEC410F1F				// mcrr p15, 1, r0, r1, c15
#else
        mrrc    p15, 1, r0, r1, c15			// smp enable
        orr     r0, r0, #0x40
        mcrr    p15, 1, r0, r1, c15
#endif
#endif
        mov     r0, #0xFF000000
        orr     r0, r0, #0x00FF0000
        add     r0, r0, #INTERNAL_SRAM_SIZE

        mov     r1, #0x20
        sub     r2, r12, #1
        and     r2, r2, #0x7				// cpu 0: -0xE0, cpu 1: -0, cpu 2: -0x20,  3: -0x40, 4: -0x60, 5: -0x80, 6: -0xA0, 7: -0xC0
        mul     r1, r1, r2
        sub     r0, r0, r1

        mov     sp, r0

        movs    r0, r12

        bleq    main					// save this in register for possible long jump
        blne    s5p6818_subcpu_on
        b       .

	/* clock(pll) change */
        .align 8					// below instruction number is 8, 32bytes

.global pll_change
pll_change:						//  r0:data r1:pll address r2:delay count
        mov     r3, #0x100				// for icache prefetch
pll_change_loop:					// this code will be already within i-cache. no bus transaction will make
        subs    r3, r3, #1				// wait for pll change done
        streq   r0, [r1]				// pll change start
        moveqs  r3, r2					// real delay time set
        cmpne   r3, #0x100
        bne     pll_change_loop
        bx      lr
        nop

/* self-refresh test */
.global enterSelfRefresh				// this code is call linux kernel, so here is virtual memory space.
.global s5p6818_suspend
.global vddPowerOff
//.global subcpu_sleep
//.global jump_to_virtual_area

subcpu_sleep:
        mov     r0, #1
        ldr     r1, =g_suspend_ready
        ldr     r2, [r1]
        orr     r2, r2, r0, lsl r3
        str     r2, [r1]

        ldr     r0, =g_wakeup_ready
recheck_cpu0_ready:
        wfi
        ldr     r1, [r0]
        cmp     r1, #0
        beq     recheck_cpu0_ready

        b       jump_to_virtual_area

        .align 8
system_sleep:						// r0:alive r1:drex
        cpsid   if					// disable irq & fiq

        stmfd   sp!, {r0-r4, lr}			// store reg values

        bl      v7_flush_cache_all			// DCache off
        mrc     p15, 0, r1, c1, c0, 0			// Read control register
        bic     r1, r1, #0x4				// Disable DC.
        mcr     p15, 0, r1, c1, c0, 0

        mov     r4, pc
        bic     r4, r4, #0xFF
        bic     r4, r4, #0xFF00

	/* Disable the MMU(Memory Managemnet Unit) */
jump_to_physical_area:
disable_mmu:
//        mrc     p15, 0, r1, c1, c0, 0			// Read control register
        bic     r1, r1, #0x1				// Disable MMU.
//        bic     r1, r1, #0x1000				// Disable IC.
//        bic     r1, r1, #0x4				// Dsiable DC.

        ldr     r0, =physical_start
        cmp     r0, #0					// make sure no stall on "mov pc,r0" below

        mcr     p15, 0, r1, c1, c0, 0			// Disable the MMU

        // Jump to the physical address of the 'physical_start' label.
        mov     pc, r0					//  jump to new physical address
        nop
        nop
        nop

        // MMU & caches now disabled.
physical_start:

        // ALive interrupt foward to only cpu0.
        ldr     r0, =0xC0009824
        ldr     r1, [r0]
        bic     r2, r1, #0xFF
        orr     r1, r2, #0x01
        str     r1, [r0]

        mrc     p15, 0, r3, c0, c0, 5			// Get our cpu id
#if (CONFIG_RESET_AFFINITY_ID == 1)
        and     r2, r3, #(0xFF << 8)			// mask cluster id
        and     r3, r3, #0xFF				// mask cpu id
        orr     r3, r2, lsr #6				// save cpu id
#else
        tst     r3, #0x4400				// check processor affinity
        orrne   r3, r3, #4				// mark cpu0 or cpu1
#endif
        ands    r3, r3, #0xF

        ldr     r0, =g_store_vaddr
        lsl     r2, r3, #2
        str     r4, [r0, r2]
        bne     subcpu_sleep				// if cpu0 then bypass.

#if 0
#if (MULTICORE_SLEEP_CONTROL == 1)
#if 0
        //; wait suspend status
        ldr     r0, =g_suspend_ready
        ldr     r1, =0xFE
recheck_suspend_ready:
        ldr     r2, [r0]
        cmp     r1, r2
        bne     recheck_suspend_ready
#else

        //; wait suspend status
        ldr     r1, =0xFE
recheck_suspend_ready:
        ldr     r0, =0xC0011168         //; TIEOFF90
        ldr     r2, [r0]
        and     r2, r2, #0xF

        ldr     r0, =0xC00111AC         //; TIEOFF107
        ldr     r3, [r0]
        and     r3, r3, #0xF

        orr     r2, r2, r3, lsl #4

        cmp     r1, r2
        bne     recheck_suspend_ready
#endif
#endif  //; #if (MULTICORE_SLEEP_CONTROL == 1)
#endif

        /* store the stack pointer */
        ldr     r0, =g_store_stack
        str     sp, [r0]

        /* set the stack pointer */
        mov     sp, #0xFF000000
        orr     sp, sp, #0x00FF0000
        add     sp, sp, #INTERNAL_SRAM_SIZE

        /* store link address */
        push    {lr}

        /* goto s5p6818_suspend function. */
        bl      s5p6818_suspend

        // Awake other cpus.
        ldr     r0, =0xC0009F00
        ldr     r1, =(0xFE << 16)
        str     r1, [r0]

        // Set cpu0 status already.
        ldr     r0, =g_wakeup_ready
        mov     r1, #1
        str     r1, [r0]

        // Restore link address
        pop     {lr}

        // Restore stack pointer
        ldr     r0, =g_store_stack
        ldr     sp, [r0]

	/* enable the mmu */
        .align 8
jump_to_virtual_area:
enable_mmu:
        ldr     r0, =g_store_vaddr

        mrc     p15, 0, r3, c0, c0, 5			// get our cpu id
#if (CONFIG_RESET_AFFINITY_ID == 1)
        and     r2, r3, #(0xFF << 8)			// mask cluster id
        and     r3, r3, #0xFF				// mask cpu id
        orr     r3, r2, lsr #6				// save cpu id
#else
        tst     r3, #0x4400				// check processor affinity
        orrne   r3, r3, #4				// mark cpu0 or cpu1
#endif
        and     r3, r3, #0xF				// save CPU id

        lsl     r2, r3, #2
        ldr     r4, [r0, r2]

        mrc     p15, 0, r1, c1, c0, 0			// Read control register
        orr     r1, r1, #0x1				// Enable MMU.
        orr     r1, r1, #0x1000				// Enable IC.
        orr     r1, r1, #0x4				// Enable DC.

        ldr     r0, =virtual_start
        bic     r0, #0xFF000000
        bic     r0, #0x00FF0000
        orr     r0, r0, r4
        cmp     r0, #0					// make sure no stall on "mov pc,r0" below

        mcr     p15, 0, r1, c1, c0, 0			// enable the mmu

        /* Jump to the virtual address of the 'virtual_start' label. */
        mov     pc, r0					//  jump to new virtual address
        nop
        nop
        nop

        /* mmu & caches now enabled. */
virtual_start:
        ldmfd   sp!, {r0-r4, lr}			// restore reg values

        mov     pc, lr
        b       .

#if 0
.global BurstZero
BurstZero:
        push    {r2-r9, lr}
        mvn     r2, r1
        mvn     r3, r1
        mvn     r4, r1
        mvn     r5, r1
        mvn     r6, r1
        mvn     r7, r1
        mvn     r8, r1
        mvn     r9, r1
        stmia   r0!, {r2-r9}
        pop     {r2-r9, pc}

.global BurstWrite
BurstWrite:
        push    {r2-r9, lr}
        mvn     r2, r1
        mvn     r3, r1
        mvn     r4, r1
        mvn     r5, r1
        mov     r6, r1
        mvn     r7, r1
        mvn     r8, r1
        mvn     r9, r1
        stmia   r0!, {r2-r9}
        pop     {r2-r9, pc}

.global BurstRead
BurstRead:
        push    {r2-r9, lr}
        ldmia   r0!, {r2-r9}
        stmia   r1!, {r2-r9}
        pop     {r2-r9, pc}
#endif


/*
 *  v7_flush_dcache_all()
 *
 *  Flush the whole D-cache.
 *
 *  Corrupted registers: r0-r7, r9-r11 (r6 only in Thumb mode)
 *
 *  - mm    - mm_struct describing address space
 */
.global v7_flush_dcache_all
v7_flush_dcache_all:
        dmb					// ensure ordering with previous memory accesses
        mrc     p15, 1, r0, c0, c0, 1		// read clidr
        ands    r3, r0, #0x7000000		// extract loc from clidr
        mov     r3, r3, lsr #23			// left align loc bit field
        beq     finished			// if loc is 0, then no need to clean
        mov     r10, #0				// start clean at cache level 0
loop1:
        add     r2, r10, r10, lsr #1		// work out 3x current cache level
        mov     r1, r0, lsr r2			// extract cache type bits from clidr
        and     r1, r1, #7			// mask of the bits for current cache only
        cmp     r1, #2				// see what cache we have at this level
        blt     skip				// skip if no cache, or just i-cache

#ifdef CONFIG_PREEMPT
        save_and_disable_irqs_notrace r9	// make cssr&csidr read atomic
#endif
        mcr     p15, 2, r10, c0, c0, 0		// select current cache level in cssr
        isb					// isb to sych the new cssr&csidr
        mrc     p15, 1, r1, c0, c0, 0		// read the new csidr
#ifdef CONFIG_PREEMPT
        restore_irqs_notrace r9
#endif
        and     r2, r1, #7			// extract the length of the cache lines
        add     r2, r2, #4			// add 4 (line length offset)
        ldr     r4, =0x3ff
        ands    r4, r4, r1, lsr #3		// find maximum number on the way size
        clz     r5, r4				// find bit position of way size increment
        ldr     r7, =0x7fff
        ands    r7, r7, r1, lsr #13		// extract max number of the index size
loop2:
        mov     r9, r4				// create working copy of max way size
loop3:
        orr     r11, r10, r9, lsl r5		// factor way and cache number into r11
        orr     r11, r11, r7, lsl r2		// factor index number into r11
        mcr     p15, 0, r11, c7, c14, 2		// clean & invalidate by set/way
        subs    r9, r9, #1			// decrement the way
        bge     loop3
        subs    r7, r7, #1			// decrement the index
        bge     loop2
skip:
        add     r10, r10, #2			// increment cache number
        cmp     r3, r10
        bgt     loop1
finished:
        mov     r10, #0				// swith back to cache level 0
        mcr     p15, 2, r10, c0, c0, 0		// select current cache level in cssr
        dsb
        isb
        mov     pc, lr

/*
 *  v7_flush_cache_all()
 *
 *  Flush the entire cache system.
 *  The data cache flush is now achieved using atomic clean / invalidates
 *  working outwards from L1 cache. This is done using Set/Way based cache
 *  maintenance instructions.
 *  The instruction cache can still be invalidated back to the point of
 *  unification in a single instruction.
 *
 */
.global v7_flush_cache_all
v7_flush_cache_all:
        stmfd   sp!, {r4-r5, r7, r9-r11, lr}
        bl      v7_flush_dcache_all
        mov     r0, #0
        mcr     p15, 0, r0, c7, c1, 0		// invalidate I-cache inner shareable
        mcr     p15, 0, r0, c7, c5, 0		// I+BTB cache invalidate
        ldmfd   sp!, {r4-r5, r7, r9-r11, lr}
        mov     pc, lr

        .align 2
        .long   0x12345678

.global g_suspend_ready
g_suspend_ready:
        .long   0x00000000

.global g_wakeup_ready
g_wakeup_ready:
        .long   0x00000000

.global g_store_stack
g_store_stack:
        .long   0x00000000

.global g_store_vaddr
g_store_vaddr:
        .long   0x00000000      //; CPU 0
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
        .long   0x00000000
