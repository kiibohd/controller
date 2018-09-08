/**
 * \file
 *
 * \brief Default descriptors for a USB Device with a single interface DFU Atmel
 *
 * Copyright (c) 2011-2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "conf_usb.h"
#include "udd.h"
#include "udi.h"
#include "udc_desc.h"
#include "udi_dfu_atmel.h"

/**
 * \defgroup udi_dfu_atmel_group_single_desc USB device descriptors for a single interface
 *
 * The following structures provide the USB device descriptors required for
 * USB Device with a single interface DFU Atmel Class.
 *
 * It is ready to use and do not require more definition.
 * @{
 */

//! Only one interface for this device
#define  USB_DEVICE_NB_INTERFACE       1

//! USB Device Descriptor
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE usb_dev_desc_t udc_device_desc = {
	.bLength                   = sizeof(usb_dev_desc_t),
	.bDescriptorType           = USB_DT_DEVICE,
	.bcdUSB                    = LE16(USB_V2_0),
	.bDeviceClass              = NO_CLASS,
	.bDeviceSubClass           = NO_SUBCLASS,
	.bDeviceProtocol           = NO_PROTOCOL,
	.bMaxPacketSize0           = USB_DEVICE_EP_CTRL_SIZE,
	.idVendor                  = LE16(USB_DEVICE_VENDOR_ID),
	.idProduct                 = LE16(USB_DEVICE_PRODUCT_ID),
	.bcdDevice                 = LE16((USB_DEVICE_MAJOR_VERSION << 8)
		| USB_DEVICE_MINOR_VERSION),
#ifdef USB_DEVICE_MANUFACTURE_NAME
	.iManufacturer             = 1,
#else
	.iManufacturer             = 0,  // No manufacture string
#endif
#ifdef USB_DEVICE_PRODUCT_NAME
	.iProduct                  = 2,
#else
	.iProduct                  = 0,  // No product string
#endif
#ifdef USB_DEVICE_SERIAL_NAME
	.iSerialNumber             = 3,
#else
	.iSerialNumber             = 0,  // No serial string
#endif
	.bNumConfigurations        = 1
};


//! Structure for USB Device Configuration Descriptor
COMPILER_PACK_SET(1)
typedef struct {
	usb_conf_desc_t conf;
	udi_dfu_atmel_desc_t udi_dfu_atmel;
} udc_desc_t;
COMPILER_PACK_RESET()

#define FLASH_SECTOR_SIZE (128*IFLASH0_PAGE_SIZE)
//#define USB_DFU_TRANSFER_SIZE (FLASH_SECTOR_SIZE-1)
#define USB_DFU_TRANSFER_SIZE (8*IFLASH0_PAGE_SIZE) //4096

//! USB Device Configuration Descriptor filled for FS
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE udc_desc_t udc_desc_fs = {
	.conf.bLength              = sizeof(usb_conf_desc_t),
	.conf.bDescriptorType      = USB_DT_CONFIGURATION,
	.conf.wTotalLength         = LE16(sizeof(udc_desc_t)),
	.conf.bNumInterfaces       = USB_DEVICE_NB_INTERFACE,
	.conf.bConfigurationValue  = 1,
	.conf.iConfiguration       = 5,
	.conf.bmAttributes         = USB_CONFIG_ATTR_MUST_SET | USB_DEVICE_ATTR,
	.conf.bMaxPower            = USB_CONFIG_MAX_POWER(USB_DEVICE_POWER),
	.udi_dfu_atmel             = {
		.iface = {
			.bLength             = sizeof(usb_iface_desc_t),
			.bDescriptorType     = USB_DT_INTERFACE,
			.bInterfaceNumber    = UDI_DFU_ATMEL_IFACE_NUMBER,
			.bAlternateSetting   = 0,
			.bNumEndpoints       = 0,
			.bInterfaceClass     = USB_DEV_CLASS_APP,
			.bInterfaceSubClass  = USB_DEV_SUBCLASS_APP_DFU,
			.bInterfaceProtocol  = USB_DEV_PROTO_DFU_DFU,
			.iInterface          = 4
		},
		.dfu = {
			.bLength = sizeof(struct dfu_desc_functional),
			.bDescriptorType = {
				.id = 0x1,
				.type_type = USB_DESC_TYPE_CLASS
			},
			.will_detach = 1,
			.manifestation_tolerant = 0,
			.can_upload = 0,
			.can_upload = 1,
			.can_download = 1,
			.wDetachTimeOut = 0,
			.wTransferSize = USB_DFU_TRANSFER_SIZE,
			.bcdDFUVersion = { .maj = 1, .min = 1 }
		}
	}
};

#ifdef USB_DEVICE_HS_SUPPORT
#  error DFU Atmel for USB High Speed not implemented
#endif


/**
 * \name UDC structures which contains all USB Device definitions
 */
//@{

//! Associate an UDI for each USB interface
UDC_DESC_STORAGE udi_api_t *udi_apis[USB_DEVICE_NB_INTERFACE] = {
	&udi_api_dfu_atmel,
};

//! Add UDI with USB Descriptors FS
UDC_DESC_STORAGE udc_config_speed_t   udc_config_lsfs[1] = {{
	.desc          = (usb_conf_desc_t UDC_DESC_STORAGE*)&udc_desc_fs,
	.udi_apis      = udi_apis,
}};

//! Add all information about USB Device in global structure for UDC
UDC_DESC_STORAGE udc_config_t udc_config = {
	.confdev_lsfs = &udc_device_desc,
	.conf_lsfs = udc_config_lsfs,
};

//@}
//@}
