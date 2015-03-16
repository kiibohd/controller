/* Copyright (c) 2011,2012 Simon Schubert <2@0x2c.org>.
 * Modifications by Jacob Alexander 2014 <haata@kiibohd.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _USB_DFU_H
#define _USB_DFU_H

// ----- Compiler Includes -----

#include <sys/types.h>



// ----- Defines -----

#define USB_FUNCTION_DFU_IFACE_COUNT 1


#ifndef USB_DFU_TRANSFER_SIZE
#define USB_DFU_TRANSFER_SIZE   FLASH_SECTOR_SIZE
#endif

#define USB_FUNCTION_DESC_DFU_DECL                         \
	struct dfu_function_desc

#define USB_FUNCTION_DFU_IFACE_COUNT    1
#define USB_FUNCTION_DFU_RX_EP_COUNT    0
#define USB_FUNCTION_DFU_TX_EP_COUNT    0



// ----- Macros -----

#define USB_FUNCTION_DESC_DFU(state...)                                 \
	{                                                               \
		.iface = {                                              \
			.bLength = sizeof(struct usb_desc_iface_t),     \
			.bDescriptorType = USB_DESC_IFACE,              \
			.bInterfaceNumber = USB_FUNCTION_IFACE(0, state), \
			.bAlternateSetting = 0,                         \
			.bNumEndpoints = 0,                             \
			.bInterfaceClass = USB_DEV_CLASS_APP,           \
			.bInterfaceSubClass = USB_DEV_SUBCLASS_APP_DFU, \
			.bInterfaceProtocol = USB_DEV_PROTO_DFU_DFU,    \
			.iInterface = 0,                                \
		},                                                      \
			.dfu = {                                        \
			.bLength = sizeof(struct dfu_desc_functional),  \
			.bDescriptorType = {                            \
				.id = 0x1,                              \
				.type_type = USB_DESC_TYPE_CLASS        \
			},                                              \
			.will_detach = 1,                               \
			.manifestation_tolerant = 0,                    \
			.can_upload = 0,                                \
			.can_download = 1,                              \
			.wDetachTimeOut = 0,                            \
			.wTransferSize = USB_DFU_TRANSFER_SIZE,         \
			.bcdDFUVersion = { .maj = 1, .min = 1 }         \
		}                                                       \
	}



// ----- Enumerations -----

enum dfu_dev_subclass {
	USB_DEV_SUBCLASS_APP_DFU = 0x01
};

enum dfu_dev_proto {
	USB_DEV_PROTO_DFU_APP = 0x01,
	USB_DEV_PROTO_DFU_DFU = 0x02
};

enum dfu_ctrl_req_code {
	USB_CTRL_REQ_DFU_DETACH = 0,
	USB_CTRL_REQ_DFU_DNLOAD = 1,
	USB_CTRL_REQ_DFU_UPLOAD = 2,
	USB_CTRL_REQ_DFU_GETSTATUS = 3,
	USB_CTRL_REQ_DFU_CLRSTATUS = 4,
	USB_CTRL_REQ_DFU_GETSTATE = 5,
	USB_CTRL_REQ_DFU_ABORT = 6
};

enum dfu_status {
	DFU_STATUS_async = 0xff,
	DFU_STATUS_OK = 0x00,
	DFU_STATUS_errTARGET = 0x01,
	DFU_STATUS_errFILE = 0x02,
	DFU_STATUS_errWRITE = 0x03,
	DFU_STATUS_errERASE = 0x04,
	DFU_STATUS_errCHECK_ERASED = 0x05,
	DFU_STATUS_errPROG = 0x06,
	DFU_STATUS_errVERIFY = 0x07,
	DFU_STATUS_errADDRESS = 0x08,
	DFU_STATUS_errNOTDONE = 0x09,
	DFU_STATUS_errFIRMWARE = 0x0a,
	DFU_STATUS_errVENDOR = 0x0b,
	DFU_STATUS_errUSBR = 0x0c,
	DFU_STATUS_errPOR = 0x0d,
	DFU_STATUS_errUNKNOWN = 0x0e,
	DFU_STATUS_errSTALLEDPKT = 0x0f
};

enum dfu_state {
	DFU_STATE_appIDLE = 0,
	DFU_STATE_appDETACH = 1,
	DFU_STATE_dfuIDLE = 2,
	DFU_STATE_dfuDNLOAD_SYNC = 3,
	DFU_STATE_dfuDNBUSY = 4,
	DFU_STATE_dfuDNLOAD_IDLE = 5,
	DFU_STATE_dfuMANIFEST_SYNC = 6,
	DFU_STATE_dfuMANIFEST = 7,
	DFU_STATE_dfuMANIFEST_WAIT_RESET = 8,
	DFU_STATE_dfuUPLOAD_IDLE = 9,
	DFU_STATE_dfuERROR = 10
};



// ----- Structs -----

struct dfu_status_t {
	enum dfu_status bStatus : 8;
	uint32_t bwPollTimeout : 24;
	enum dfu_state bState : 8;
	uint8_t iString;
} __packed;
CTASSERT_SIZE_BYTE(struct dfu_status_t, 6);


typedef enum dfu_status (*dfu_setup_write_t)(size_t off, size_t len, void **buf);
typedef enum dfu_status (*dfu_finish_write_t)(void *, size_t off, size_t len);
typedef void (*dfu_detach_t)(void);

struct dfu_ctx {
	struct usbd_function_ctx_header header;
	enum dfu_state state;
	enum dfu_status status;
	dfu_setup_write_t setup_write;
	dfu_finish_write_t finish_write;
	size_t off;
	size_t len;
};


struct dfu_desc_functional {
	uint8_t bLength;
	struct usb_desc_type_t bDescriptorType; /* = class DFU/0x1 FUNCTIONAL */
	union {
		struct {
			uint8_t can_download : 1;
			uint8_t can_upload : 1;
			uint8_t manifestation_tolerant : 1;
			uint8_t will_detach : 1;
			uint8_t _rsvd0 : 4;
		};
		uint8_t bmAttributes;
	};
	uint16_t wDetachTimeOut;
	uint16_t wTransferSize;
	struct usb_bcd_t bcdDFUVersion;
} __packed;
CTASSERT_SIZE_BYTE(struct dfu_desc_functional, 9);

struct dfu_function_desc {
	struct usb_desc_iface_t iface;
	struct dfu_desc_functional dfu;
};


extern const struct usbd_function dfu_function;
extern const struct usbd_function dfu_app_function;



// ----- Functions -----

void dfu_write_done(enum dfu_status, struct dfu_ctx *ctx);
void dfu_init(dfu_setup_write_t setup_write, dfu_finish_write_t finish_write, struct dfu_ctx *ctx);
void dfu_app_init(dfu_detach_t detachcb);

#endif
