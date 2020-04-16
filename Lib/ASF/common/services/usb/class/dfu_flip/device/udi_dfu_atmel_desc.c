/**
 * \file
 *
 * \brief Default descriptors for a USB Device with a single interface DFU Atmel
 *
 * Copyright (c) 2011-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "conf_usb.h"
#include <common/services/usb/udc/udd.h>
#include <common/services/usb/udc/udi.h>
#include <common/services/usb/udc/udc_desc.h>
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
	.bcdDevice                 = LE16(USB_DEVICE_VERSION),
	.iManufacturer             = 1,
	.iProduct                  = 2,
	.iSerialNumber             = 3,
	.bNumConfigurations        = 1
};


//! Structure for USB Device Configuration Descriptor
COMPILER_PACK_SET(1)
typedef struct {
	usb_conf_desc_t conf;
	udi_dfu_atmel_desc_t udi_dfu_atmel;
#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
	udi_dfu_atmel_desc_t udi_dfu_atmel2;
#endif
} udc_desc_t;
COMPILER_PACK_RESET()

#define FLASH_SECTOR_SIZE (128*IFLASH0_PAGE_SIZE)
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
			.iInterface          = 4,
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
		},
	},
#if DFU_EXTRA_BLE_SWD_SUPPORT == 1
	.udi_dfu_atmel2            = {
		.iface = {
			.bLength             = sizeof(usb_iface_desc_t),
			.bDescriptorType     = USB_DT_INTERFACE,
			.bInterfaceNumber    = UDI_DFU_ATMEL_IFACE_NUMBER,
			.bAlternateSetting   = 1,
			.bNumEndpoints       = 0,
			.bInterfaceClass     = USB_DEV_CLASS_APP,
			.bInterfaceSubClass  = USB_DEV_SUBCLASS_APP_DFU,
			.bInterfaceProtocol  = USB_DEV_PROTO_DFU_DFU,
			.iInterface          = 6,
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
		},
	},
#endif
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

//! Needed to fix lsusb "Resource temporarily unavailable"
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE usb_dev_qual_desc_t udc_device_qual = {
	.bLength                   = 1,
};

//! Needed to fix lsusb "Resource temporarily unavailable"
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE usb_dev_debug_desc_t udc_device_debug = {
	.bLength                   = 1,
};

//! Add all information about USB Device in global structure for UDC
UDC_DESC_STORAGE udc_config_t udc_config = {
	.confdev_lsfs = &udc_device_desc,
	.conf_lsfs = udc_config_lsfs,
	.qualifier = &udc_device_qual,
	.debug = &udc_device_debug,
};

//@}
//@}
