/*
 * Copyright (C) 2016	Nexell Co., Ltd. All Rights Reserved.
 * Nexell Co. Proprietary & Confidential
 *
 * NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
 * AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * Author: Sangjong, Han <hans@nexell.co.kr>
 */

#define NULL	0
#define false	0
#define true	1

#include <nx_chip.h>
#include <nx_gpio.h>
#include <nx_clkpwr.h>
#include <nx_clkgen.h>
#include <nx-vip-primitive.h>

#include "printf.h"

void reset_con(unsigned int device_num, int enable);

static void udelay(unsigned int utime)
{
	register volatile unsigned int i;
	utime /= 21;
	if (utime == 0)
		utime = 1;
	for (i = 0; i < 53 * utime; i++);
}

enum devicephyindex {
	diGPIOA,
	diVIP0,
	diVIP0_CLKGEN,
	diVIP1,
	diVIP1_CLKGEN,
	diVIP2,
	diVIP2_CLKGEN
};
static unsigned int ioVIRBASEADDR[] = {
	0xC001A000,	// GPIOA
	0xC0063000,	// vip1
	0xC00C2000,	// vip1 clkgen
	0xC0064000,	// vip0
	0xC00C1000,	// vip0 clkgen
#ifdef USE_VIP2
	0xC0099000,	// vip2
	0xC009a000	// vip2 clkgen
#endif
};

struct portinfo {
	unsigned char gpg;      // group
	unsigned char gpn;      // pad num
	unsigned char gpa;      // alt
	unsigned char gppen;    // pull en
	unsigned char gpud;     // 1: pullup, 0: pulldown
	unsigned char gpio;     // 1: input, 0: output
	unsigned char gpol;     // 0: low, 1: high
};
static struct portinfo dpcameractrlport[] = {
	// g group	,pad,al,pu,ud,io,lh
	{NX_GPIO_GROUP_E,  4, 0, 0, 0, 0, 1},	// viclk0
	{NX_GPIO_GROUP_E,  5, 0, 0, 0, 0, 1},	// vihsync0
	{NX_GPIO_GROUP_E,  6, 0, 0, 0, 0, 1},	// vivsync0

	{NX_GPIO_GROUP_A, 28, 0, 0, 0, 0, 1},	// viclk1
	{NX_GPIO_GROUP_E, 13, 0, 0, 0, 0, 1},	// vihsync1
	{NX_GPIO_GROUP_E,  7, 0, 0, 0, 0, 1},	// vivsync1

#ifdef USE_VIP2
	{NX_GPIO_GROUP_C, 14, 1, 0, 0, 0, 1},	// viclk2
	{NX_GPIO_GROUP_C, 15, 1, 0, 0, 0, 1},	// vihsync2
	{NX_GPIO_GROUP_C, 16, 1, 0, 0, 0, 1}	// vivsync2
#endif
};


static int port_init(void)
{
	int i, cpn = sizeof(dpcameractrlport) / sizeof(struct portinfo);
	volatile struct NX_GPIO_RegisterSet * pgpa =
	//	(volatile struct NX_GPIO_RegisterSet *)ioVIRBASEADDR[diGPIOA];
		(volatile struct NX_GPIO_RegisterSet *)ioVIRBASEADDR[diGPIOA];

	for (i = 0; i < cpn; i++) {
		struct portinfo *ppi = &dpcameractrlport[i];
		volatile struct NX_GPIO_RegisterSet * pgp = pgpa + ppi->gpg;

		pgp->GPIOxALTFN[ppi->gpn >> 4] &= ~(3 << ((ppi->gpn & 0xf) * 2));
		pgp->GPIOxALTFN[ppi->gpn >> 4] |= (ppi->gpa & 3) << ((ppi->gpn & 0xf) * 2);
		if (ppi->gpud) {
			pgp->GPIOx_PULLSEL |= 1 << ppi->gpn;	// pull up/down
			pgp->GPIOx_PULLSEL_DISABLE_DEFAULT |= 1 << ppi->gpn;	//
		} else
			pgp->GPIOx_PULLSEL &= ~(1 << ppi->gpn);	// pull up/down
		if (ppi->gppen) {
			pgp->GPIOx_PULLENB |= 1 << ppi->gpn;	// pull enable
			pgp->GPIOx_PULLENB_DISABLE_DEFAULT |= 1 << ppi->gpn;	// pull enable
		} else
			pgp->GPIOx_PULLENB &= ~(1 << ppi->gpn);	// pull enable
		pgp->GPIOx_DRV0 |= 1 << ppi->gpn;		// drv strength max
		pgp->GPIOx_DRV1 |= 1 << ppi->gpn;
		pgp->GPIOx_DRV0_DISABLE_DEFAULT |= 1 << ppi->gpn;
		pgp->GPIOx_DRV1_DISABLE_DEFAULT |= 1 << ppi->gpn;
		pgp->GPIOx_SLEW &= ~(1 << ppi->gpn);		// slew rate min
		pgp->GPIOx_SLEW_DISABLE_DEFAULT |= 1 << ppi->gpn;

		if (ppi->gpol)
			pgp->GPIOxOUT |= 1 << ppi->gpn;		// out value
		else
			pgp->GPIOxOUT &= ~(1 << ppi->gpn);	// out value
		if (!ppi->gpio & 1)
			pgp->GPIOxOUTENB |= 1 << ppi->gpn;	// in/out
		else
			pgp->GPIOxOUTENB &= ~(1 << ppi->gpn);	// in/out
	}

	return 1;
}

struct ot_clipper_info {
	unsigned int module;
	unsigned int interface_type;
	unsigned int external_sync;
	unsigned int padclk_sel;
	unsigned int h_syncpolarity;
	unsigned int v_syncpolarity;
	unsigned int h_frontporch;
	unsigned int h_syncwidth;
	unsigned int h_backporch;
	unsigned int v_frontporch;
	unsigned int v_backporch;
	unsigned int v_syncwidth;
	unsigned int clock_invert;
	unsigned int port;
	unsigned int interlace;
	unsigned int bus_fmt;
	unsigned int width;
	unsigned int height;
	unsigned int format;
	unsigned int lu_stride;
	unsigned int cb_stride;
};
static struct ot_clipper_info Everif = {
	0,	// module
	0,	// interface_type	// parallel
	1,	// external_sync
	0,	// padclk_sel ?
	0,	// h_syncpolarity ?
	0,	// v_syncpolarity ?
	0,	// h_frontporch
	480,	// h_syncwidth
	0,	// h_backporch
	0,	// v_frontporch
	0,	// v_backporch
	480,	// v_syncwidth
	0,	// clock_invert
	0,	// port
	0,	// interlace	0: no interlace
	nx_vip_dataorder_y0cby1cr,	// bus_fmt
	448,	// width
	1040,	// height
	nx_vip_format_l422,	// format
	0,	// lu_stride
	0,	// cb_stride
};

static struct ot_clipper_info *psensorinfo = &Everif;

#define ALIGN(X, N)             ((X+N-1) & (~(N-1)))
#define YUV_STRIDE_ALIGN_FACTOR                 64
#define YUV_Y_STRIDE(w)         ALIGN(w, YUV_STRIDE_ALIGN_FACTOR)
#define YUV_CB_STRIDE(w)        ALIGN(w, YUV_STRIDE_ALIGN_FACTOR / 2)
#define YUV_CR_STRIDE(w)        ALIGN(w, YUV_STRIDE_ALIGN_FACTOR / 2)

enum FRAME_KIND {
	Y,
	CB,
	CR
};

static int stride_cal(int width, enum FRAME_KIND type)
{
	int stride;

	switch (type) {
	case Y:
		stride = YUV_Y_STRIDE(width);
		break;
	case CB:
		stride = YUV_CB_STRIDE(width / 2);
		break;
	case CR:
		stride = YUV_CR_STRIDE(width / 2);
		break;

	}

	return stride;
}

static void vip_init(void)
{
	nx_vip_set_base_address(0, (void*)ioVIRBASEADDR[diVIP0]);
	nx_vip_set_base_address(1, (void*)ioVIRBASEADDR[diVIP1]);
#ifdef USE_VIP2
	nx_vip_set_base_address(2, (void*)ioVIRBASEADDR[diVIP2]);
#endif
	struct NX_CLKGEN_RegisterSet *pvip0 =
		(struct NX_CLKGEN_RegisterSet *)ioVIRBASEADDR[diVIP0_CLKGEN];
	struct NX_CLKGEN_RegisterSet *pvip1 =
		(struct NX_CLKGEN_RegisterSet *)ioVIRBASEADDR[diVIP1_CLKGEN];
#ifdef USE_VIP2
	struct NX_CLKGEN_RegisterSet *pvip2 =
		(struct NX_CLKGEN_RegisterSet *)ioVIRBASEADDR[diVIP2_CLKGEN];
#endif
	pvip0->CLKENB = 1 << 2 | 3 << 0;
	pvip1->CLKENB = 1 << 2 | 3 << 0;
#ifdef USE_VIP2
	pvip2->CLKENB = 1 << 2 | 3 << 0;
#endif
	udelay(10);
	reset_con(RESETINDEX_OF_VIP0_MODULE_i_nRST, 0);
	reset_con(RESETINDEX_OF_VIP1_MODULE_i_nRST, 0);
#ifdef USE_VIP2
	reset_con(RESETINDEX_OF_VIP2_MODULE_i_nRST, 0);
#endif
	nx_vip_clear_interrupt_pending_all(0);	// clear all pending int
	nx_vip_clear_interrupt_pending_all(1);
#ifdef USE_VIP2
	nx_vip_clear_interrupt_pending_all(2);
#endif
	nx_vip_set_interrupt_enable_all(0, 0);	// disable all vip int
	nx_vip_set_interrupt_enable_all(1, 0);
#ifdef USE_VIP2
	nx_vip_set_interrupt_enable_all(2, 0);
#endif
	nx_vip_set_input_port(0, psensorinfo->port);		// input at bt601
	nx_vip_set_input_port(1, psensorinfo->port);
#ifdef USE_VIP2
	nx_vip_set_input_port(2, psensorinfo->port);
#endif
}

static void vip_set_capture(void)
{
	nx_vip_clear_interrupt_pending_all(0);	// clear all pending int
	nx_vip_clear_interrupt_pending_all(1);
#ifdef USE_VIP2
	nx_vip_clear_interrupt_pending_all(2);
#endif
	nx_vip_set_interrupt_enable_all(0, 0);	// disable all vip int
	nx_vip_set_interrupt_enable_all(1, 0);
#ifdef USE_VIP2
	nx_vip_set_interrupt_enable_all(2, 0);
#endif
	nx_vip_set_input_port(0, psensorinfo->port);		// input at bt601
	nx_vip_set_input_port(1, psensorinfo->port);
#ifdef USE_VIP2
	nx_vip_set_input_port(2, psensorinfo->port);
#endif
	nx_vip_set_field_mode(0, false, nx_vip_fieldsel_bypass,
			psensorinfo->interlace, false);	// field:0, no interlace, bypass
	nx_vip_set_field_mode(1, false, nx_vip_fieldsel_bypass,
			psensorinfo->interlace, false);
#ifdef USE_VIP2
	nx_vip_set_field_mode(2, false, nx_vip_fieldsel_bypass,
			psensorinfo->interlace, false);
#endif
	nx_vip_set_data_mode(0, psensorinfo->bus_fmt, 8);	// order: 2, width:8
	nx_vip_set_data_mode(1, psensorinfo->bus_fmt, 8);
#ifdef USE_VIP2
	nx_vip_set_data_mode(2, psensorinfo->bus_fmt, 8);
#endif
	nx_vip_set_dvalid_mode(0, false, false, false);	// dvalid:0, bypass, ext sync
	nx_vip_set_dvalid_mode(1, false, false, false);
#ifdef USE_VIP2
	nx_vip_set_dvalid_mode(2, false, false, false);
#endif
	nx_vip_set_hvsync(0,
			psensorinfo->external_sync,
			psensorinfo->width * 2,
			psensorinfo->interlace ?
				psensorinfo->height >> 1 : psensorinfo->height,
			psensorinfo->padclk_sel,
			psensorinfo->h_syncpolarity,
			psensorinfo->v_syncpolarity,
			psensorinfo->h_syncwidth,
			psensorinfo->h_frontporch,
			psensorinfo->h_backporch,
			psensorinfo->v_syncwidth,
			psensorinfo->v_frontporch,
			psensorinfo->v_backporch);
	nx_vip_set_hvsync(1,
			psensorinfo->external_sync,
			psensorinfo->width * 2,
			psensorinfo->interlace ?
				psensorinfo->height >> 1 : psensorinfo->height,
			psensorinfo->padclk_sel,
			psensorinfo->h_syncpolarity,
			psensorinfo->v_syncpolarity,
			psensorinfo->h_syncwidth,
			psensorinfo->h_frontporch,
			psensorinfo->h_backporch,
			psensorinfo->v_syncwidth,
			psensorinfo->v_frontporch,
			psensorinfo->v_backporch);
#ifdef USE_VIP2
	nx_vip_set_hvsync(2,
			psensorinfo->external_sync,
			psensorinfo->width * 2,
			psensorinfo->interlace ?
				psensorinfo->height >> 1 : psensorinfo->height,
			psensorinfo->padclk_sel,
			psensorinfo->h_syncpolarity,
			psensorinfo->v_syncpolarity,
			psensorinfo->h_syncwidth,
			psensorinfo->h_frontporch,
			psensorinfo->h_backporch,
			psensorinfo->v_syncwidth,
			psensorinfo->v_frontporch,
			psensorinfo->v_backporch);
#endif
	nx_vip_set_fiforeset_mode(0, nx_vip_fiforeset_all);
	nx_vip_set_fiforeset_mode(1, nx_vip_fiforeset_all);
#ifdef USE_VIP2
	nx_vip_set_fiforeset_mode(2, nx_vip_fiforeset_all);
#endif
	nx_vip_set_clip_region(0, 0, 0, psensorinfo->width,
			psensorinfo->interlace ?
			psensorinfo->height >> 1 : psensorinfo->height);
	nx_vip_set_clip_region(1, 0, 0, psensorinfo->width,
			psensorinfo->interlace ?
			psensorinfo->height >> 1 : psensorinfo->height);
#ifdef USE_VIP2
	nx_vip_set_clip_region(2, 0, 0, psensorinfo->width,
			psensorinfo->interlace ?
			psensorinfo->height >> 1 : psensorinfo->height);
#endif
	nx_vip_set_clipper_format(0, psensorinfo->format);	// l422
	nx_vip_set_clipper_format(1, psensorinfo->format);
#ifdef USE_VIP2
	nx_vip_set_clipper_format(2, psensorinfo->format);
#endif
	psensorinfo->lu_stride = stride_cal(psensorinfo->width, Y);
	psensorinfo->cb_stride = stride_cal(psensorinfo->width, CB);

	if (psensorinfo->format == nx_vip_format_l422)
		psensorinfo->lu_stride *= 2;

	nx_vip_set_clipper_format(0, psensorinfo->format);	// l422
	nx_vip_set_clipper_format(1, psensorinfo->format);
#ifdef USE_VIP2
	nx_vip_set_clipper_format(2, psensorinfo->format);
#endif
	nx_vip_set_clipper_addr(0,
			psensorinfo->format,
			psensorinfo->width,
			psensorinfo->height,
			0x40000000,
			0, 0,
			psensorinfo->lu_stride,
			psensorinfo->cb_stride);
	nx_vip_set_clipper_addr(1,
			psensorinfo->format,
			psensorinfo->width,
			psensorinfo->height,
			0x40400000,
			0, 0,
			psensorinfo->lu_stride,
			psensorinfo->cb_stride);
#ifdef USE_VIP2
	nx_vip_set_clipper_addr(2,
			psensorinfo->format,
			psensorinfo->width,
			psensorinfo->height,
			0x40800000,
			0, 0,
			psensorinfo->lu_stride,
			psensorinfo->cb_stride);
#endif
	nx_vip_clear_interrupt_pending_all(0);
	nx_vip_clear_interrupt_pending_all(1);
#ifdef USE_VIP2
	nx_vip_clear_interrupt_pending_all(2);
#endif
	nx_vip_set_vipenable(0, true, true, true, false);
	nx_vip_set_vipenable(1, true, true, true, false);
#ifdef USE_VIP2
	nx_vip_set_vipenable(2, true, true, true, false);
#endif
	nx_vip_set_interrupt_enable(0, nx_vip_int_done, true);	// vsync int enable
	nx_vip_set_interrupt_enable(1, nx_vip_int_done, true);
#ifdef USE_VIP2
	nx_vip_set_interrupt_enable(2, nx_vip_int_done, true);
#endif
}

static void vip_sync_gen(void)
{
	register volatile struct NX_GPIO_RegisterSet * pgpa =
		(volatile struct NX_GPIO_RegisterSet *)ioVIRBASEADDR[diGPIOA];
	register volatile struct NX_GPIO_RegisterSet * pgpe =
		(volatile struct NX_GPIO_RegisterSet *)(ioVIRBASEADDR[diGPIOA] + 4*0x1000);
	register volatile struct NX_GPIO_RegisterSet * pgpc
		= (volatile struct NX_GPIO_RegisterSet *)(ioVIRBASEADDR[diGPIOA] + 2*0x1000);
	register unsigned int p, h, pclk, hsync, vsync, pclkh, hsynch, vsynch;

	/* vip 0 sync */
	pclk = pgpe->GPIOxOUT;
	hsync = pgpe->GPIOxOUT;
	vsync = pgpe->GPIOxOUT;
	pclkh = pgpe->GPIOxOUT | 1 << 4;
	hsynch = pgpe->GPIOxOUT | 1 << 5;
	vsynch = pgpe->GPIOxOUT | 1 << 6 | 1 << 5;

	pgpe->GPIOxOUT = vsynch;
		pgpe->GPIOxOUT = hsynch;
			pgpa->GPIOxOUT = pclkh;
			pgpa->GPIOxOUT = pclk;
		pgpe->GPIOxOUT = hsync;
	pgpe->GPIOxOUT = vsync;

	for (p = 0; p < 896 * 2; p++) {
		pgpe->GPIOxOUT = pclkh;
		pgpe->GPIOxOUT = pclk;
	}
	pgpe->GPIOxOUT = vsynch;
		pgpe->GPIOxOUT = hsynch;
			pgpe->GPIOxOUT = pclkh;
			pgpe->GPIOxOUT = pclk;
		pgpe->GPIOxOUT = hsync;
	pgpe->GPIOxOUT = vsync;

	/* vip 1 sync */
	pclk = pgpa->GPIOxOUT;
	hsync = pgpe->GPIOxOUT;
	vsync = pgpe->GPIOxOUT;
	pclkh = pgpa->GPIOxOUT | 1 << 28;
	hsynch = pgpe->GPIOxOUT | 1 << 13;
	vsynch = pgpe->GPIOxOUT | 1 << 7 | 1 << 13;

	pgpe->GPIOxOUT = vsynch;
		pgpe->GPIOxOUT = hsynch;
			pgpa->GPIOxOUT = pclkh;
			pgpa->GPIOxOUT = pclk;
		pgpe->GPIOxOUT = hsync;
	pgpe->GPIOxOUT = vsync;

	for (p = 0; p < 896 * 2; p++) {
		pgpa->GPIOxOUT = pclkh;
		pgpa->GPIOxOUT = pclk;
	}
	pgpe->GPIOxOUT = vsynch;
		pgpe->GPIOxOUT = hsynch;
			pgpa->GPIOxOUT = pclkh;
			pgpa->GPIOxOUT = pclk;
		pgpe->GPIOxOUT = hsync;
	pgpe->GPIOxOUT = vsync;

#ifdef USE_VIP2
	/* vip 2 sync */
	pclk = pgpc->GPIOxOUT;
	hsync = pgpc->GPIOxOUT;
	vsync = pgpc->GPIOxOUT;
	pclkh = pgpc->GPIOxOUT | 1 << 14;
	hsynch = pgpc->GPIOxOUT | 1 << 15;
	vsynch = pgpc->GPIOxOUT | 1 << 16 | 1 << 15;

	pgpc->GPIOxOUT = vsynch;
		pgpc->GPIOxOUT = hsynch;
			pgpc->GPIOxOUT = pclkh;
			pgpc->GPIOxOUT = pclk;
		pgpc->GPIOxOUT = hsync;
	pgpc->GPIOxOUT = vsync;
	for (p = 0; p < 896 * 2; p++) {
		pgpc->GPIOxOUT = pclkh;
		pgpc->GPIOxOUT = pclk;
	}
	pgpc->GPIOxOUT = vsynch;
		pgpc->GPIOxOUT = hsynch;
			pgpc->GPIOxOUT = pclkh;
			pgpc->GPIOxOUT = pclk;
		pgpc->GPIOxOUT = hsync;
	pgpc->GPIOxOUT = vsync;
#endif
}

static void vip_capture(void)
{
	vip_set_capture();
	vip_sync_gen();
}

#include <nx_wdt.h>
static struct NX_WDT_RegisterSet *const pReg_WDT =
	(struct NX_WDT_RegisterSet * const)PHY_BASEADDR_WDT_MODULE;
static void wdt_reset(unsigned short ms)
{
	/* Enable watchdog to eliminate booting failuire */
	reset_con(RESETINDEX_OF_WDT_MODULE_PRESETn, 0);	// reset off
	udelay(10);
	reset_con(RESETINDEX_OF_WDT_MODULE_nPOR, 0);	// reset off

	pReg_WDT->WTCON =
			156 << 8 |	// prescaler value
			0x03 << 3 |	// division factor (3:128)
			0x01 << 2;	// watchdog reset enable
	// 200MHz/157/128 = 9952.229229, 100.48us, ms*10*100us
	pReg_WDT->WTCNT = ms * 10;
	pReg_WDT->WTCON =
			156 << 8 |	// prescaler value
			0x01 << 5 |	// watchdog timer enable
			0x03 << 3 |	// division factor (3:128)
			0x01 << 2;	// watchdog reset enable
	printf("wdt\r\n");
}

static void plat_init(void)
{
	port_init();
	vip_init();
}

void capture_emul(void)
{
	printf("vip reset\r\n");
	plat_init();
	vip_capture();
	wdt_reset(10);
}
