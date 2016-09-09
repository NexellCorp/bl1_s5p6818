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

#include <nx_usb20otg.h>
#include <iUSBBOOT.h>
#include "nx_bootheader.h"

#if 0 //DEVMSG_ON
#define dev_msg(x, ...) printf(x, ...)
#else
#define dev_msg(x, ...)
#endif

extern U32 iget_fcs(U32 fcs, U32 data);
extern void Decrypt(U32 *SrcAddr, U32 *DestAddr, U32 Size);

void ResetCon(U32 devicenum, CBOOL en);

static NX_USB20OTG_APB_RegisterSet *const pUSB20OTGAPBReg =
    (NX_USB20OTG_APB_RegisterSet *)PHY_BASEADDR_USB20OTG_MODULE_APB;
static struct NX_USB_OTG_RegisterSet *const pUOReg =
    (struct NX_USB_OTG_RegisterSet *)PHY_BASEADDR_USB20OTG_MODULE_AHBS0;

static U8 __attribute__((aligned(4)))
gs_DeviceDescriptorFS[DEVICE_DESCRIPTOR_SIZE] = {
    18,				 //  0 desc size
    (U8)(DESCRIPTORTYPE_DEVICE), //  1 desc type (DEVICE)
    (U8)(FULL_USB_VER % 0x100),  //  2 USB release
    (U8)(FULL_USB_VER / 0x100),  //  3 => 1.00
    0xFF,			 //  4 class
    0xFF,			 //  5 subclass
    0xFF,			 //  6 protocol
    (U8)FULL_MAX_PKT_SIZE_EP0,   //  7 max pack size
    (U8)(VENDORID % 0x100),      //  8 vendor ID LSB
    (U8)(VENDORID / 0x100),      //  9 vendor ID MSB
    (U8)(PRODUCTID % 0x100),     // 10 product ID LSB    (second product)
    (U8)(PRODUCTID / 0x100),     // 11 product ID MSB
    0x00,			 // 12 device release LSB
    0x00,			 // 13 device release MSB
    0x00,			 // 14 manufacturer string desc index
    0x00,			 // 15 product string desc index
    0x00,			 // 16 serial num string desc index
    0x01			 // 17 num of possible configurations
};

static U8 __attribute__((aligned(4)))
gs_DeviceDescriptorHS[DEVICE_DESCRIPTOR_SIZE] = {
    18,				 //  0 desc size
    (U8)(DESCRIPTORTYPE_DEVICE), //  1 desc type (DEVICE)
    (U8)(HIGH_USB_VER % 0x100),  //  2 USB release
    (U8)(HIGH_USB_VER / 0x100),  //  3 => 1.00
    0xFF,			 //  4 class
    0xFF,			 //  5 subclass
    0xFF,			 //  6 protocol
    (U8)HIGH_MAX_PKT_SIZE_EP0,   //  7 max pack size
    (U8)(VENDORID % 0x100),      //  8 vendor ID LSB
    (U8)(VENDORID / 0x100),      //  9 vendor ID MSB
    (U8)(PRODUCTID % 0x100),     // 10 product ID LSB    (second product)
    (U8)(PRODUCTID / 0x100),     // 11 product ID MSB
    0x00,			 // 12 device release LSB
    0x00,			 // 13 device release MSB
    0x00,			 // 14 manufacturer string desc index
    0x00,			 // 15 product string desc index
    0x00,			 // 16 serial num string desc index
    0x01			 // 17 num of possible configurations
};

static const U8 __attribute__((aligned(4)))
gs_ConfigDescriptorFS[CONFIG_DESCRIPTOR_SIZE] = {
    //--------------------------------------------------------------------------
    // Configuration Descriptor
    0x09,				// [ 0] desc size
    (U8)(DESCRIPTORTYPE_CONFIGURATION), // [ 1] desc type (CONFIGURATION)
    (U8)(CONFIG_DESCRIPTOR_SIZE %
	 0x100), // [ 2] total length of data returned LSB
    (U8)(CONFIG_DESCRIPTOR_SIZE /
	 0x100), // [ 3] total length of data returned MSB
    0x01,	// [ 4] num of interfaces
    0x01,	// [ 5] value to select config (1 for now)
    0x00,	// [ 6] index of string desc ( 0 for now)
    CONF_ATTR_DEFAULT | CONF_ATTR_SELFPOWERED, // [ 7] bus powered
    25,					       // [ 8] max power, 50mA for now

    //--------------------------------------------------------------------------
    // Interface Decriptor
    0x09,			    // [ 0] desc size
    (U8)(DESCRIPTORTYPE_INTERFACE), // [ 1] desc type (INTERFACE)
    0x00,			    // [ 2] interface index.
    0x00,			    // [ 3] value for alternate setting
    0x02, // [ 4] bNumEndpoints (number endpoints used, excluding EP0)
    0xFF, // [ 5]
    0xFF, // [ 6]
    0xFF, // [ 7]
    0x00, // [ 8] string index,

    //--------------------------------------------------------------------------
    // Endpoint descriptor (EP 1 Bulk IN)
    0x07,			   // [ 0] desc size
    (U8)(DESCRIPTORTYPE_ENDPOINT), // [ 1] desc type (ENDPOINT)
    BULK_IN_EP | EP_ADDR_IN,       // [ 2] endpoint address: endpoint 1, IN
    EP_ATTR_BULK,		   // [ 3] endpoint attributes: Bulk
    (U8)(FULL_MAX_PKT_SIZE_EP1 % 0x100), // [ 4] max packet size LSB
    (U8)(FULL_MAX_PKT_SIZE_EP1 / 0x100), // [ 5] max packet size MSB
    0x00, // [ 6] polling interval (4ms/bit=time,500ms)

    //--------------------------------------------------------------------------
    // Endpoint descriptor (EP 2 Bulk OUT)
    0x07,			   // [ 0] desc size
    (U8)(DESCRIPTORTYPE_ENDPOINT), // [ 1] desc type (ENDPOINT)
    BULK_OUT_EP | EP_ADDR_OUT,     // [ 2] endpoint address: endpoint 2, OUT
    EP_ATTR_BULK,		   // [ 3] endpoint attributes: Bulk
    (U8)(FULL_MAX_PKT_SIZE_EP2 % 0x100), // [ 4] max packet size LSB
    (U8)(FULL_MAX_PKT_SIZE_EP2 / 0x100), // [ 5] max packet size MSB
    0x00 // [ 6] polling interval (4ms/bit=time,500ms)
};

static const U8 __attribute__((aligned(4)))
gs_ConfigDescriptorHS[CONFIG_DESCRIPTOR_SIZE] = {
    //--------------------------------------------------------------------------
    // Configuration Descriptor
    0x09,				// [ 0] desc size
    (U8)(DESCRIPTORTYPE_CONFIGURATION), // [ 1] desc type (CONFIGURATION)
    (U8)(CONFIG_DESCRIPTOR_SIZE %
	 0x100), // [ 2] total length of data returned LSB
    (U8)(CONFIG_DESCRIPTOR_SIZE /
	 0x100), // [ 3] total length of data returned MSB
    0x01,	// [ 4] num of interfaces
    0x01,	// [ 5] value to select config (1 for now)
    0x00,	// [ 6] index of string desc ( 0 for now)
    CONF_ATTR_DEFAULT | CONF_ATTR_SELFPOWERED, // [ 7] bus powered
    25,					       // [ 8] max power, 50mA for now

    //--------------------------------------------------------------------------
    // Interface Decriptor
    0x09,			    // [ 0] desc size
    (U8)(DESCRIPTORTYPE_INTERFACE), // [ 1] desc type (INTERFACE)
    0x00,			    // [ 2] interface index.
    0x00,			    // [ 3] value for alternate setting
    0x02, // [ 4] bNumEndpoints (number endpoints used, excluding EP0)
    0xFF, // [ 5]
    0xFF, // [ 6]
    0xFF, // [ 7]
    0x00, // [ 8] string index,

    //--------------------------------------------------------------------------
    // Endpoint descriptor (EP 1 Bulk IN)
    0x07,			   // [ 0] desc size
    (U8)(DESCRIPTORTYPE_ENDPOINT), // [ 1] desc type (ENDPOINT)
    BULK_IN_EP | EP_ADDR_IN,       // [ 2] endpoint address: endpoint 1, IN
    EP_ATTR_BULK,		   // [ 3] endpoint attributes: Bulk
    (U8)(HIGH_MAX_PKT_SIZE_EP1 % 0x100), // [ 4] max packet size LSB
    (U8)(HIGH_MAX_PKT_SIZE_EP1 / 0x100), // [ 5] max packet size MSB
    0x00, // [ 6] polling interval (4ms/bit=time,500ms)

    //--------------------------------------------------------------------------
    // Endpoint descriptor (EP 2 Bulk OUT)
    0x07,			   // [ 0] desc size
    (U8)(DESCRIPTORTYPE_ENDPOINT), // [ 1] desc type (ENDPOINT)
    BULK_OUT_EP | EP_ADDR_OUT,     // [ 2] endpoint address: endpoint 2, OUT
    EP_ATTR_BULK,		   // [ 3] endpoint attributes: Bulk
    (U8)(HIGH_MAX_PKT_SIZE_EP2 % 0x100), // [ 4] max packet size LSB
    (U8)(HIGH_MAX_PKT_SIZE_EP2 / 0x100), // [ 5] max packet size MSB
    0x00 // [ 6] polling interval (4ms/bit=time,500ms)
};

static void nx_usb_write_in_fifo(U32 ep, U8 *buf, S32 num)
{
	S32 i;
	U32 *dwbuf = (U32 *)buf; /* assume all data ptr is 4 bytes aligned */
	for (i = 0; i < (num + 3) / 4; i++)
		pUOReg->EPFifo[ep][0] = dwbuf[i];
}

/*
 * CRC Check for modifications.
 * A global variable for a CRC check.
 * */
static struct NX_SecondBootInfo *g_TBI;
static USBBOOTSTATUS *g_USBBootStatus;
static unsigned int g_fcs = 0;

static void nx_usb_read_out_fifo(U32 ep, U8 *buf, S32 num)
{
	S32 i;
	U32 *dwbuf = (U32 *)buf;

	for (i = 0; i < (num + 3) / 4; i++) {
		dwbuf[i] = pUOReg->EPFifo[ep][0];
		/* Data other than the HEADER. */
		if (CTRUE == g_USBBootStatus->bHeaderReceived)
			g_fcs = iget_fcs(g_fcs, dwbuf[i]);
	}
	/* Save the CRC Check Value  */
	//g_TBI->DBI.SDMMCBI.CRC32 = g_fcs;
}

static void nx_usb_ep0_int_hndlr(USBBOOTSTATUS *pUSBBootStatus)
{
	U32 buf[2];
	SetupPacket *pSetupPacket = (SetupPacket *)buf;
	U16 addr;

	dev_msg("Event EP0\r\n");

	if (pUSBBootStatus->ep0_state == EP0_STATE_INIT) {

		buf[0] = pUOReg->EPFifo[CONTROL_EP][0];
		buf[1] = pUOReg->EPFifo[CONTROL_EP][0];

		dev_msg("Req:%02X %02X %04X %04X %04X\r\n",
			pSetupPacket->bmRequestType, pSetupPacket->bRequest,
			pSetupPacket->wValue, pSetupPacket->wIndex,
			pSetupPacket->wLength);
		switch (pSetupPacket->bRequest) {
		case STANDARD_SET_ADDRESS:
			/* Set Address Update bit */
			addr = (pSetupPacket->wValue & 0xFF);
			dev_msg("STANDARD_SET_ADDRESS: %02X\r\n", addr);
			pUOReg->DCSR.DCFG =
			    1 << 18 | addr << 4 | pUSBBootStatus->speed << 0;
			pUSBBootStatus->ep0_state = EP0_STATE_INIT;
			break;

		case STANDARD_SET_DESCRIPTOR:
			dev_msg("STANDARD_SET_DESCRIPTOR\r\n");
			break;

		case STANDARD_SET_CONFIGURATION:
			dev_msg("STANDARD_SET_CONFIGURATION\r\n");
			/* Configuration value in configuration descriptor */
			pUSBBootStatus->CurConfig = pSetupPacket->wValue;
			pUSBBootStatus->ep0_state = EP0_STATE_INIT;
			break;

		case STANDARD_GET_CONFIGURATION:
			dev_msg("STANDARD_GET_CONFIGURATION\r\n");
			pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPTSIZ =
			    (1 << 19) | (1 << 0);
			/*ep0 enable, clear nak, next ep0, 8byte */
			pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL =
			    EPEN_CNAK_EP0_8;
			pUOReg->EPFifo[CONTROL_EP][0] =
			    pUSBBootStatus->CurConfig;
			pUSBBootStatus->ep0_state = EP0_STATE_INIT;
			break;

		case STANDARD_GET_DESCRIPTOR:
			dev_msg("STANDARD_GET_DESCRIPTOR :");
			pUSBBootStatus->Remain_size =
			    (U32)pSetupPacket->wLength;
			switch (pSetupPacket->wValue >> 8) {
			case DESCRIPTORTYPE_DEVICE:
				pUSBBootStatus->Current_ptr =
				    (U8 *)pUSBBootStatus->DeviceDescriptor;
				pUSBBootStatus->Current_Fifo_Size =
				    pUSBBootStatus->ctrl_max_pktsize;
				if (pUSBBootStatus->Remain_size >
				    DEVICE_DESCRIPTOR_SIZE)
					pUSBBootStatus->Remain_size =
					    DEVICE_DESCRIPTOR_SIZE;
				pUSBBootStatus->ep0_state = EP0_STATE_GET_DSCPT;
				break;

			case DESCRIPTORTYPE_CONFIGURATION:
				pUSBBootStatus->Current_ptr =
				    (U8 *)pUSBBootStatus->ConfigDescriptor;
				pUSBBootStatus->Current_Fifo_Size =
				    pUSBBootStatus->ctrl_max_pktsize;
				if (pUSBBootStatus->Remain_size >
				    CONFIG_DESCRIPTOR_SIZE)
					pUSBBootStatus->Remain_size =
					    CONFIG_DESCRIPTOR_SIZE;
				pUSBBootStatus->ep0_state = EP0_STATE_GET_DSCPT;
				break;

			//            case DESCRIPTORTYPE_STRING :
			//            case DESCRIPTORTYPE_INTERFACE:
			//            case DESCRIPTORTYPE_ENDPOINT:
			default:
				pUOReg->DCSR.DEPIR[0].DIEPCTL |= DEPCTL_STALL;
				break;
			}
			break;

		case STANDARD_CLEAR_FEATURE:
			dev_msg("STANDARD_CLEAR_FEATURE :");
			break;

		case STANDARD_SET_FEATURE:
			dev_msg("STANDARD_SET_FEATURE :");
			break;

		case STANDARD_GET_STATUS:
			dev_msg("STANDARD_GET_STATUS :");
			pUSBBootStatus->ep0_state = EP0_STATE_GET_STATUS;
			break;

		case STANDARD_GET_INTERFACE:
			dev_msg("STANDARD_GET_INTERFACE\r\n");
			pUSBBootStatus->ep0_state = EP0_STATE_GET_INTERFACE;
			break;

		case STANDARD_SET_INTERFACE:
			dev_msg("STANDARD_SET_INTERFACE\r\n");
			pUSBBootStatus->CurInterface = pSetupPacket->wValue;
			pUSBBootStatus->CurSetting = pSetupPacket->wValue;
			pUSBBootStatus->ep0_state = EP0_STATE_INIT;
			break;

		case STANDARD_SYNCH_FRAME:
			dev_msg("STANDARD_SYNCH_FRAME\r\n");
			pUSBBootStatus->ep0_state = EP0_STATE_INIT;
			break;

		default:
			break;
		}
	}

	pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPTSIZ =
	    (1 << 19) | (pUSBBootStatus->ctrl_max_pktsize << 0);

	if (pUSBBootStatus->speed == USB_HIGH) {
		/*clear nak, next ep0, 64byte */
		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL =
		    ((1 << 26) | (CONTROL_EP << 11) | (0 << 0));
	} else {
		/*clear nak, next ep0, 8byte */
		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL =
		    ((1 << 26) | (CONTROL_EP << 11) | (3 << 0));
	}
}

static void nx_usb_transfer_ep0(USBBOOTSTATUS *pUSBBootStatus)
{
	switch (pUSBBootStatus->ep0_state) {
	case EP0_STATE_INIT:
		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPTSIZ = (1 << 19) | (0 << 0);
		/*ep0 enable, clear nak, next ep0, 8byte */
		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL = EPEN_CNAK_EP0_8;
		dev_msg("EP0_STATE_INIT\r\n");
		break;

	/* GET_DESCRIPTOR:DEVICE */
	case EP0_STATE_GET_DSCPT:
		dev_msg("EP0_STATE_GD_DEV_0 :");
		if (pUSBBootStatus->speed == USB_HIGH) {
			/*ep0 enable, clear nak, next ep0, max 64byte */
			pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL =
			    EPEN_CNAK_EP0_64;
		} else {
			pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL =
			    EPEN_CNAK_EP0_8;
		}
		if (pUSBBootStatus->Current_Fifo_Size >=
		    pUSBBootStatus->Remain_size) {
			pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPTSIZ =
			    (1 << 19) | (pUSBBootStatus->Remain_size << 0);
			nx_usb_write_in_fifo(CONTROL_EP,
					     pUSBBootStatus->Current_ptr,
					     pUSBBootStatus->Remain_size);
			pUSBBootStatus->ep0_state = EP0_STATE_INIT;
		} else {
			pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPTSIZ =
			    (1 << 19) |
			    (pUSBBootStatus->Current_Fifo_Size << 0);
			nx_usb_write_in_fifo(CONTROL_EP,
					     pUSBBootStatus->Current_ptr,
					     pUSBBootStatus->Current_Fifo_Size);
			pUSBBootStatus->Remain_size -=
			    pUSBBootStatus->Current_Fifo_Size;
			pUSBBootStatus->Current_ptr +=
			    pUSBBootStatus->Current_Fifo_Size;
		}
		break;

	case EP0_STATE_GET_INTERFACE:
	case EP0_STATE_GET_CONFIG:
	case EP0_STATE_GET_STATUS:
		dev_msg("EP0_STATE_INTERFACE_GET\r\n");
		dev_msg("EP0_STATE_GET_STATUS\r\n");

		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPTSIZ = (1 << 19) | (1 << 0);
		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL = EPEN_CNAK_EP0_8;

		if (pUSBBootStatus->ep0_state == EP0_STATE_GET_INTERFACE)
			pUOReg->EPFifo[CONTROL_EP][0] =
			    pUSBBootStatus->CurInterface;
		else if (pUSBBootStatus->ep0_state == EP0_STATE_GET_CONFIG)
			pUOReg->EPFifo[CONTROL_EP][0] =
			    pUSBBootStatus->CurConfig;
		else
			pUOReg->EPFifo[CONTROL_EP][0] = 0;
		pUSBBootStatus->ep0_state = EP0_STATE_INIT;
		break;

	default:
		break;
	}
}

static void nx_usb_int_bulkin(USBBOOTSTATUS *pUSBBootStatus)
{
	U8 *bulkin_buf;
	U32 remain_cnt;

	dev_msg("Bulk In Function\r\n");

	bulkin_buf = (U8 *)pUSBBootStatus->up_ptr;
	remain_cnt = pUSBBootStatus->up_size -
		     (U32)(pUSBBootStatus->up_ptr - pUSBBootStatus->up_addr);

	if (remain_cnt > pUSBBootStatus->bulkin_max_pktsize) {
		pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPTSIZ =
		    (1 << 19) | (pUSBBootStatus->bulkin_max_pktsize << 0);

		/*ep1 enable, clear nak, bulk, usb active, next ep2, max pkt
		 * 64*/
		pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPCTL =
		    1u << 31 | 1 << 26 | 2 << 18 | 1 << 15 |
		    pUSBBootStatus->bulkin_max_pktsize << 0;

		nx_usb_write_in_fifo(BULK_IN_EP, bulkin_buf,
				     pUSBBootStatus->bulkin_max_pktsize);

		pUSBBootStatus->up_ptr += pUSBBootStatus->bulkin_max_pktsize;

	} else if (remain_cnt > 0) {
		pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPTSIZ =
		    (1 << 19) | (remain_cnt << 0);

		/*ep1 enable, clear nak, bulk, usb active, next ep2, max pkt
		 * 64*/
		pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPCTL =
		    1u << 31 | 1 << 26 | 2 << 18 | 1 << 15 |
		    pUSBBootStatus->bulkin_max_pktsize << 0;

		nx_usb_write_in_fifo(BULK_IN_EP, bulkin_buf, remain_cnt);

		pUSBBootStatus->up_ptr += remain_cnt;

	} else { /*remain_cnt = 0*/
		pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPCTL =
		    (DEPCTL_SNAK | DEPCTL_BULK_TYPE);
	}
}


static void header_check(USBBOOTSTATUS *pUSBBootStatus,
			 struct nx_bootheader *ptbh)
{
	struct nx_tbbinfo *tbi= &ptbh->tbbi;

	/* decrypt header */
	if (pReg_ClkPwr->SYSRSTCONFIG & 1<<14)
		Decrypt((U32 *)tbi, (U32 *)tbi, sizeof(*tbi));

	if (tbi->signature == HEADER_ID) { /* "NSIH" */
		pUSBBootStatus->bHeaderReceived = CTRUE;
		pUSBBootStatus->RxBuffAddr =
			(U8 *)((U8 *)(tbi->loadaddr) + sizeof(*tbi));
		pUSBBootStatus->RxBuffAddr_save = pUSBBootStatus->RxBuffAddr;
		pUSBBootStatus->iRxSize = 512 +	/* rsa header */
			tbi->loadsize +
			tbi->dbi[0].usbbi.split_size +
			tbi->dbi[1].usbbi.split_size;
		pUSBBootStatus->iRxSize_save = pUSBBootStatus->iRxSize;
		SYSMSG("USB Load Address = 0x%016X Launch "
		       "Address = 0x%016X, size = %08X bytes\r\n",
		       (MPTRS)tbi->loadaddr,
		       (MPTRS)tbi->startaddr,
		       (int32_t)pUSBBootStatus->iRxSize);
	} else {
		pUSBBootStatus->iRxHeaderSize = 0;
		pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPCTL |= DEPCTL_STALL;
	}
}

static void nx_usb_int_bulkout(USBBOOTSTATUS *pUSBBootStatus,
			       struct NX_SecondBootInfo *pTBI,
			       U32 fifo_cnt_byte)
{
	/* get header */
	if (CTRUE != pUSBBootStatus->bHeaderReceived) {
		struct nx_bootheader *ptbh = (struct nx_bootheader *)pTBI;
		U32 *pdwBuffer = (U32 *)pTBI;

		nx_usb_read_out_fifo(BULK_OUT_EP,
			(U8 *)&pdwBuffer[pUSBBootStatus->iRxHeaderSize / 4],
			fifo_cnt_byte);

		if ((fifo_cnt_byte & 3) == 0) {
			pUSBBootStatus->iRxHeaderSize += fifo_cnt_byte;
		} else {
			printf("ERROR : Header Packet Size must be aligned on "
			       "32-bits.\r\n");
			pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPCTL |= DEPCTL_STALL;
		}

		if (512 <= pUSBBootStatus->iRxHeaderSize) /* why 512 <= ??? */
			/* mark header received in function */
			header_check(pUSBBootStatus, ptbh);
	/* get body */
	} else {
		NX_ASSERT((pUSBBootStatus->iRxSize) > 0);
		NX_ASSERT(0 == ((MPTRS)pUSBBootStatus->RxBuffAddr & 3));
		nx_usb_read_out_fifo(BULK_OUT_EP,
				     (U8 *)pUSBBootStatus->RxBuffAddr,
				     fifo_cnt_byte);
#if (0)
		dev_msg("Bin Packet Size = %d => 0x%08X, %d\r\n", iRxSize,
			pUSBBootStatus->RxBuffAddr, pUSBBootStatus->iRxSize);
#endif

		pUSBBootStatus->RxBuffAddr += fifo_cnt_byte;
		pUSBBootStatus->iRxSize -= fifo_cnt_byte;

		if (pUSBBootStatus->iRxSize <= 0) {
			SYSMSG("Download completed!\r\n");

			pUSBBootStatus->bDownLoading = CFALSE;
			pUSBBootStatus->bHeaderReceived = CFALSE;
		}
	}

	pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPTSIZ =
	    (1 << 19) | (pUSBBootStatus->bulkout_max_pktsize << 0);

	/*ep2 enable, clear nak, bulk, usb active, next ep2, max pkt 64*/
	pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPCTL =
	    1u << 31 | 1 << 26 | 2 << 18 | 1 << 15 |
	    pUSBBootStatus->bulkout_max_pktsize << 0;
}


static void nx_usb_reset(USBBOOTSTATUS *pUSBBootStatus)
{
	U32 i;
	/* set all out ep nak */
	for (i = 0; i < 16; i++)
		pUOReg->DCSR.DEPOR[i].DOEPCTL |= DEPCTL_SNAK;

	pUSBBootStatus->ep0_state = EP0_STATE_INIT;
	pUOReg->DCSR.DAINTMSK = ((1 << BULK_OUT_EP) | (1 << CONTROL_EP)) << 16 |
				((1 << BULK_IN_EP) | (1 << CONTROL_EP));
	pUOReg->DCSR.DOEPMSK =
	    CTRL_OUT_EP_SETUP_PHASE_DONE | AHB_ERROR | TRANSFER_DONE;
	pUOReg->DCSR.DIEPMSK =
	    INTKN_TXFEMP | NON_ISO_IN_EP_TIMEOUT | AHB_ERROR | TRANSFER_DONE;

	/* Rx FIFO Size */
	pUOReg->GCSR.GRXFSIZ = RX_FIFO_SIZE;

	/* Non Periodic Tx FIFO Size */
	pUOReg->GCSR.GNPTXFSIZ =
	    NPTX_FIFO_SIZE << 16 | NPTX_FIFO_START_ADDR << 0;

	/* clear all out ep nak */
	for (i = 0; i < 16; i++)
		pUOReg->DCSR.DEPOR[i].DOEPCTL |= (DEPCTL_EPENA | DEPCTL_CNAK);

	/*clear device address */
	pUOReg->DCSR.DCFG &= ~(0x7F << 4);
}

static S32 nx_usb_set_init(USBBOOTSTATUS *pUSBBootStatus)
{
	U32 status = pUOReg->DCSR.DSTS; /* System status read */
	union {
		U32 AID;
		U16 SID[2];
		U8 BID[4];
	} USBID;

	pUSBBootStatus->bHeaderReceived = CFALSE;
	pUSBBootStatus->iRxHeaderSize = 0;

	/* Set if Device is High speed or Full speed */
	if (((status & 0x6) >> 1) == USB_HIGH) {
		pUSBBootStatus->speed = USB_HIGH;
		SYSMSG("High Speed Connected\r\n");
	} else if (((status & 0x6) >> 1) == USB_FULL) {
		pUSBBootStatus->speed = USB_FULL;
		SYSMSG("Full Speed Connected\r\n");
	} else {
		printf("**** Error:Neither High_Speed nor Full_Speed\r\n");
		return CFALSE;
	}

	/* set endpoint */
	/* Unmask NX_OTG_DAINT source */
	pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPINT = 0xFF;
	pUOReg->DCSR.DEPOR[CONTROL_EP].DOEPINT = 0xFF;
	pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPINT = 0xFF;
	pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPINT = 0xFF;

	/* Init For Ep0*/
	if (pUSBBootStatus->speed == USB_HIGH) {
		pUSBBootStatus->ctrl_max_pktsize = HIGH_MAX_PKT_SIZE_EP0;
		pUSBBootStatus->bulkin_max_pktsize = HIGH_MAX_PKT_SIZE_EP1;
		pUSBBootStatus->bulkout_max_pktsize = HIGH_MAX_PKT_SIZE_EP2;
		pUSBBootStatus->DeviceDescriptor = gs_DeviceDescriptorHS;
		pUSBBootStatus->ConfigDescriptor = gs_ConfigDescriptorHS;

		/*MPS:64bytes */
		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL =
		    ((1 << 26) | (CONTROL_EP << 11) | (0 << 0));
		/*ep0 enable, clear nak */
		pUOReg->DCSR.DEPOR[CONTROL_EP].DOEPCTL =
		    (1u << 31) | (1 << 26) | (0 << 0);
	} else {
		pUSBBootStatus->ctrl_max_pktsize = FULL_MAX_PKT_SIZE_EP0;
		pUSBBootStatus->bulkin_max_pktsize = FULL_MAX_PKT_SIZE_EP1;
		pUSBBootStatus->bulkout_max_pktsize = FULL_MAX_PKT_SIZE_EP2;
		pUSBBootStatus->DeviceDescriptor = gs_DeviceDescriptorFS;
		pUSBBootStatus->ConfigDescriptor = gs_ConfigDescriptorFS;

		/*MPS:8bytes */
		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPCTL =
		    ((1 << 26) | (CONTROL_EP << 11) | (3 << 0));
		/*ep0 enable, clear nak */
		pUOReg->DCSR.DEPOR[CONTROL_EP].DOEPCTL =
		    (1u << 31) | (1 << 26) | (3 << 0);
	}

	//--------------------------------------------------------------------------
	// get VID & PID for USBD
	//--------------------------------------------------------------------------
	USBID.AID = ReadIO32(&pReg_ECID->ECID[3]);

	dev_msg("USBD VID = %04X, PID = %04X\r\n", USBID.SID[1], USBID.SID[0]);

	if ((USBID.SID[0] != 0) && (USBID.SID[1] != 0)) {
		pUSBBootStatus->DeviceDescriptor[8] =
		    (U8)(USBID.BID[2]); //  8 vendor ID LSB
		pUSBBootStatus->DeviceDescriptor[9] =
		    (U8)(USBID.BID[3]); //  9 vendor ID MSB
		pUSBBootStatus->DeviceDescriptor[10] =
		    (U8)(USBID.BID[0]); // 10 product ID LSB    (second product)
		pUSBBootStatus->DeviceDescriptor[11] =
		    (U8)(USBID.BID[1]); // 11 product ID MSB
	}

	/* set_opmode */
	pUOReg->GCSR.GINTMSK = INT_RESUME | INT_OUT_EP | INT_IN_EP |
			       INT_ENUMDONE | INT_RESET | INT_SUSPEND |
			       INT_RX_FIFO_NOT_EMPTY;

	pUOReg->GCSR.GAHBCFG = MODE_SLAVE | BURST_SINGLE | GBL_INT_UNMASK;

	pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPTSIZ =
	    (1 << 19) | (pUSBBootStatus->bulkout_max_pktsize << 0);

	pUOReg->DCSR.DEPOR[BULK_IN_EP].DOEPTSIZ = (1 << 19) | (0 << 0);

	/*bulk out ep enable, clear nak, bulk, usb active, next ep2, max pkt */
	pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPCTL =
	    1u << 31 | 1 << 26 | 2 << 18 | 1 << 15 |
	    pUSBBootStatus->bulkout_max_pktsize << 0;

	/*bulk in ep enable, clear nak, bulk, usb active, next ep1, max pkt */
	pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPCTL =
	    0u << 31 | 1 << 26 | 2 << 18 | 1 << 15 |
	    pUSBBootStatus->bulkin_max_pktsize << 0;

	return CTRUE;
}

static void nx_usb_pkt_receive(USBBOOTSTATUS *pUSBBootStatus,
			       struct NX_SecondBootInfo *pTBI)
{
	U32 rx_status;
	U32 fifo_cnt_byte;

	rx_status = pUOReg->GCSR.GRXSTSP;
	/* CRC Check for Global Boot Status */
	g_USBBootStatus = pUSBBootStatus;

	if ((rx_status & (0xf << 17)) == SETUP_PKT_RECEIVED) {
		dev_msg("SETUP_PKT_RECEIVED\r\n");
		nx_usb_ep0_int_hndlr(pUSBBootStatus);
	} else if ((rx_status & (0xf << 17)) == OUT_PKT_RECEIVED) {
		fifo_cnt_byte = (rx_status & 0x7ff0) >> 4;
		dev_msg("OUT_PKT_RECEIVED\r\n");

		if ((rx_status & BULK_OUT_EP) && (fifo_cnt_byte)) {
			nx_usb_int_bulkout(pUSBBootStatus, pTBI, fifo_cnt_byte);
			pUOReg->GCSR.GINTMSK =
			    INT_RESUME | INT_OUT_EP | INT_IN_EP | INT_ENUMDONE |
			    INT_RESET | INT_SUSPEND | INT_RX_FIFO_NOT_EMPTY;
			return;
		}
	} else if ((rx_status & (0xf << 17)) == GLOBAL_OUT_NAK) {
		dev_msg("GLOBAL_OUT_NAK\r\n");
	} else if ((rx_status & (0xf << 17)) == OUT_TRNASFER_COMPLETED) {
		dev_msg("OUT_TRNASFER_COMPLETED\r\n");
	} else if ((rx_status & (0xf << 17)) == SETUP_TRANSACTION_COMPLETED) {
		dev_msg("SETUP_TRANSACTION_COMPLETED\r\n");
	} else {
		dev_msg("Reserved\r\n");
	}
}

static void nx_usb_transfer(USBBOOTSTATUS *pUSBBootStatus)
{
	U32 ep_int;
	U32 ep_int_status;

	ep_int = pUOReg->DCSR.DAINT;

	if (ep_int & (1 << CONTROL_EP)) {
		ep_int_status = pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPINT;

		if (ep_int_status & INTKN_TXFEMP) {

			while ((pUOReg->GCSR.GNPTXSTS & 0xFFFF) <
			       pUSBBootStatus->ctrl_max_pktsize)
				;

			nx_usb_transfer_ep0(pUSBBootStatus);
		}

		pUOReg->DCSR.DEPIR[CONTROL_EP].DIEPINT = ep_int_status;
	}

	if (ep_int & ((1 << CONTROL_EP) << 16)) {
		ep_int_status = pUOReg->DCSR.DEPOR[CONTROL_EP].DOEPINT;

		pUOReg->DCSR.DEPOR[CONTROL_EP].DOEPTSIZ =
		    (1 << 29) | (1 << 19) | (8 << 0);
		pUOReg->DCSR.DEPOR[CONTROL_EP].DOEPCTL =
		    1u << 31 | 1 << 26; /*ep0 enable, clear nak */

		pUOReg->DCSR.DEPOR[CONTROL_EP].DOEPINT =
		    ep_int_status; /* Interrupt Clear */
	}

	if (ep_int & (1 << BULK_IN_EP)) {
		ep_int_status = pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPINT;

		pUOReg->DCSR.DEPIR[BULK_IN_EP].DIEPINT =
		    ep_int_status; /* Interrupt Clear */

		if (ep_int_status & INTKN_TXFEMP)
			nx_usb_int_bulkin(pUSBBootStatus);
	}

	if (ep_int & ((1 << BULK_OUT_EP) << 16)) {
		ep_int_status = pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPINT;
		pUOReg->DCSR.DEPOR[BULK_OUT_EP].DOEPINT =
		    ep_int_status; /* Interrupt Clear */
	}
}

static void nx_udc_int_hndlr(USBBOOTSTATUS *pUSBBootStatus,
			     struct NX_SecondBootInfo *pTBI)
{
	U32 int_status;
	S32 tmp;

	int_status = pUOReg->GCSR.GINTSTS; /* Core Interrupt Register */

	if (int_status & INT_RESET) {
		dev_msg("INT_RESET\r\n");

		nx_usb_reset(pUSBBootStatus);
	}

	if (int_status & INT_ENUMDONE) {
		dev_msg("INT_ENUMDONE :");

		tmp = nx_usb_set_init(pUSBBootStatus);
		if (tmp == CFALSE) {
			pUOReg->GCSR.GINTSTS = int_status; /* Interrupt Clear */
			return;
		}
	}

	if (int_status & INT_RESUME) {
		dev_msg("INT_RESUME\r\n");
	}

	if (int_status & INT_SUSPEND) {
		dev_msg("INT_SUSPEND\r\n");
	}

	if (int_status & INT_RX_FIFO_NOT_EMPTY) {
		dev_msg("INT_RX_FIFO_NOT_EMPTY\r\n");
		/* Read only register field */

		pUOReg->GCSR.GINTMSK = INT_RESUME | INT_OUT_EP | INT_IN_EP |
				       INT_ENUMDONE | INT_RESET | INT_SUSPEND;
		nx_usb_pkt_receive(pUSBBootStatus, pTBI);
		pUOReg->GCSR.GINTMSK = INT_RESUME | INT_OUT_EP | INT_IN_EP |
				       INT_ENUMDONE | INT_RESET | INT_SUSPEND |
				       INT_RX_FIFO_NOT_EMPTY;
	}

	if ((int_status & INT_IN_EP) || (int_status & INT_OUT_EP)) {
		dev_msg("INT_IN or OUT_EP\r\n");
		/* Read only register field */

		nx_usb_transfer(pUSBBootStatus);
	}
	pUOReg->GCSR.GINTSTS = int_status; /* Interrupt Clear */
}
void udelay(U32 utime)
{
	register volatile U32 i;
	for (; utime > 0; utime--)
		for (i = 106; i > 0; i--)
			;
}

/* Copy memory to memory */
static void memcpy(void *dst, const void *src, uint32_t cnt)
{
	int i = 0;
	U32 *d = (U32 *)dst;
	U32 *s = (U32 *)src;

	for (i = 0; i < (cnt >> 2); i++)
		*d++ = *s++;
	for (i = 0; i < (cnt & 3); i++)
		((U8 *)d)[i] = ((U8 *)s)[i];
}

void post_process(USBBOOTSTATUS *pUSBBootStatus,
		  struct NX_SecondBootInfo *pTBI)
{
	struct nx_bootheader *psbh = (struct nx_bootheader *)pSBI;
	struct asymmetrickey *sb_rsa = &psbh->rsa_public;
	struct nx_bootheader *ptbh = (struct nx_bootheader *)pTBI;
	struct nx_tbbinfo *tbi = &ptbh->tbbi;
	U8 *tb_rsa = pUSBBootStatus->RxBuffAddr_save;
	U8 *next = tb_rsa + sizeof(*sb_rsa);

	/* Decrypt loader body */
	if (pReg_ClkPwr->SYSRSTCONFIG & 1<<14)
		Decrypt((U32*)next, (U32*)next, ptbh->tbbi.loadsize);
	next += ptbh->tbbi.loadsize;

	/* copy loader header */
	memcpy(tb_rsa - sizeof(*tbi), tbi, sizeof(*tbi));
	/* copy rsapublickey part. */
	memcpy(((struct asymmetrickey *)tb_rsa)->rsapublickey,
	       &sb_rsa->rsapublickey,
	       sizeof(sb_rsa->rsapublickey));

	/* split load : header + body */
	memcpy(tbi->dbi[0].usbbi.split_loadaddr - sizeof(struct nx_bootheader),
	       next,
	       tbi->dbi[0].usbbi.split_size);
	next += tbi->dbi[0].usbbi.split_size;
	memcpy(tbi->dbi[1].usbbi.split_loadaddr - sizeof(struct nx_bootheader),
	       next,
	       tbi->dbi[1].usbbi.split_size);
}


CBOOL iUSBBOOT(struct NX_SecondBootInfo *pTBI)
{
	struct nx_tbbinfo *tbi = (struct nx_tbbinfo *)pTBI;
	USBBOOTSTATUS USBBootStatus;
	USBBOOTSTATUS *pUSBBootStatus = &USBBootStatus;
	ResetCon(RESETINDEX_OF_USB20OTG_MODULE_i_nRST, CTRUE);  // reset on
	ResetCon(RESETINDEX_OF_USB20OTG_MODULE_i_nRST, CFALSE); // reset negate

	/* CRC Check for variables */
	g_TBI = pTBI;

	pReg_Tieoff->TIEOFFREG[12] &=
	    ~0x3; // scale mode ( 0: real silicon, 1: test simul, 2, 3)
	pReg_Tieoff->TIEOFFREG[14] |=
	    3 << 8; // 8: enable, 9:phy word interface (0: 8 bit, 1: 16 bit)
	pReg_Tieoff->TIEOFFREG[13] =
	    0xA3006C00; // VBUSVLDEXT=1,VBUSVLDEXTSEL=1,POR=0
	pReg_Tieoff->TIEOFFREG[13] = 0xA3006C80; // POR_ENB=1
	udelay(40);				 // 40us delay need.

	pReg_Tieoff->TIEOFFREG[13] = 0xA3006C88; // nUtmiResetSync : 00000001
	udelay(1);				 // 10 clock need

	pReg_Tieoff->TIEOFFREG[13] = 0xA3006C8C; // nResetSync : 00000001
	udelay(1);				 // 10 clock need

	/* usb core soft reset */
	pUOReg->GCSR.GRSTCTL = CORE_SOFT_RESET;
	while (!(pUOReg->GCSR.GRSTCTL & AHB_MASTER_IDLE))
		;

	/* init_core */
	pUOReg->GCSR.GAHBCFG = PTXFE_HALF | NPTXFE_HALF | MODE_SLAVE |
			       BURST_SINGLE | GBL_INT_UNMASK;
	pUOReg->GCSR.GUSBCFG =
	    (0 << 15) | /* PHY Low Power Clock sel */
	    (1 << 14) | /* Non-Periodic TxFIFO Rewind Enable */
	    (5 << 10) | /* Turnaround time */
	    (0 << 9) |  /* 0:HNP disable, 1:HNP enable */
	    (0 << 8) |  /* 0:SRP disable, 1:SRP enable */
	    (0 << 7) |  /* ULPI DDR sel */
	    (0 << 6) |  /* 0: high speed utmi+, 1: full speed serial */
	    (0 << 4) |  /* 0: utmi+, 1:ulpi */
	    (1 << 3) |  /* phy i/f  0:8bit, 1:16bit */
	    (7 << 0);   /* HS/FS Timeout**/

	/* check current mode */
	if ((pUOReg->GCSR.GINTSTS & 0x1) == INT_DEV_MODE) {
		/* soft disconnect on */
		pUOReg->DCSR.DCTL |= SOFT_DISCONNECT;
		udelay(10);
		/* soft disconnect off */
		pUOReg->DCSR.DCTL &= ~SOFT_DISCONNECT;

		/* usb init device */
		pUOReg->DCSR.DCFG = 1 << 18; // | pUSBBootStatus->speed<<0; /*
					     // [][1: full speed(30Mhz) 0:high
					     // speed]*/
		pUOReg->GCSR.GINTMSK = INT_RESUME | INT_OUT_EP | INT_IN_EP |
				       INT_ENUMDONE | INT_RESET | INT_SUSPEND |
				       INT_RX_FIFO_NOT_EMPTY;
	}

	pUSBBootStatus->CurConfig = 0;
	pUSBBootStatus->CurInterface = 0;
	pUSBBootStatus->CurSetting = 0;
	pUSBBootStatus->speed = USB_HIGH;
	pUSBBootStatus->ep0_state = EP0_STATE_INIT;

	SYSMSG("USB Boot Ready!\r\n");

	pUSBBootStatus->bDownLoading = CTRUE;
	while (pUSBBootStatus->bDownLoading) {
		if (pUOReg->GCSR.GINTSTS &
		    (WkUpInt | OEPInt | IEPInt | EnumDone | USBRst | USBSusp |
		     RXFLvl)) {
			nx_udc_int_hndlr(pUSBBootStatus, pTBI);
			pUOReg->GCSR.GINTSTS = 0xFFFFFFFF;
		}
	}
	/* usb core soft reset */
	pUOReg->GCSR.GRSTCTL = CORE_SOFT_RESET;
	while (!(pUOReg->GCSR.GRSTCTL & AHB_MASTER_IDLE));

	pReg_Tieoff->TIEOFFREG[13] &= ~(1 << 3); // nUtmiResetSync = 0
	pReg_Tieoff->TIEOFFREG[13] &= ~(1 << 2); // nResetSync = 0
	pReg_Tieoff->TIEOFFREG[13] |= 3 << 7; // POR_ENB=1, POR=1
	ResetCon(RESETINDEX_OF_USB20OTG_MODULE_i_nRST, CTRUE); // reset on

	/* post process */
	post_process(pUSBBootStatus, pTBI);

	SYSMSG("\r\n\nusb image download is done!\r\n\n");

	SYSMSG("USB Load Address = 0x%016X Launch Address = 0x%016X, size = %08X bytes\r\n",
	       (MPTRS)tbi->loadaddr,
	       (MPTRS)tbi->startaddr,
	       (int32_t)pUSBBootStatus->iRxSize_save);

	return CTRUE;
}
