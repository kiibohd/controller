// Originally Generated from MCHCK Toolkit
/* Copyright (c) Jacob Alexander 2014-2017 <haata@kiibohd.com>
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

// ----- Local Includes -----

#include "dfu.desc.h"



// ----- Generated Includes -----

#include <buildvars.h>



// ----- Macros -----

#define LSB(n) ((n) & 255)
#define MSB(n) (((n) >> 8) & 255)



// ----- Structs -----

static const struct usb_config_1 usb_config_1 = {
	.config = {
		.bLength = sizeof(struct usb_desc_config_t),
		.bDescriptorType = USB_DESC_CONFIG,
		.wTotalLength = sizeof(struct usb_config_1),
		.bNumInterfaces = 1,
		.bConfigurationValue = 1,
		.iConfiguration = 5,
		.one = 1,
		.bMaxPower = 50
	},
	.usb_function_0 = {
		.iface = {
			.bLength = sizeof(struct usb_desc_iface_t),
			.bDescriptorType = USB_DESC_IFACE,
			.bInterfaceNumber = 0,
			.bAlternateSetting = 0,
			.bNumEndpoints = 0,
			.bInterfaceClass = USB_DEV_CLASS_APP,
			.bInterfaceSubClass = USB_DEV_SUBCLASS_APP_DFU,
			.bInterfaceProtocol = USB_DEV_PROTO_DFU_DFU,
			.iInterface = 4
		},

	.dfu = {
		.bLength = sizeof(struct dfu_desc_functional),
		.bDescriptorType = {
			.id = 0x1,
			.type_type = USB_DESC_TYPE_CLASS
		},
		.will_detach = 1,
		.manifestation_tolerant = 0,
#if defined(_mk20dx128vlf5_) // Kiibohd-dfu / McHCK
		.can_upload = 0,
#elif defined(_mk20dx256vlh7_) // Kiibohd-dfu
		.can_upload = 1,
#endif
		.can_download = 1,
		.wDetachTimeOut = 0,
		.wTransferSize = USB_DFU_TRANSFER_SIZE,
		.bcdDFUVersion = { .maj = 1, .min = 1 }
	}
},

};

static const struct usbd_config usbd_config_1 = {
	.init = init_usb_bootloader,
	.suspend = NULL,
	.resume = NULL,
	.desc = &usb_config_1.config,
	.function = {&dfu_function},
};

static const struct usb_desc_dev_t dfu_device_dev_desc = {
	.bLength = sizeof(struct usb_desc_dev_t),
	.bDescriptorType = USB_DESC_DEV,
	.bcdUSB = { .maj = 2 },
	.bDeviceClass = USB_DEV_CLASS_SEE_IFACE,
	.bDeviceSubClass = USB_DEV_SUBCLASS_SEE_IFACE,
	.bDeviceProtocol = USB_DEV_PROTO_SEE_IFACE,
	.bMaxPacketSize0 = EP0_BUFSIZE,
	.idVendor = VENDOR_ID,
	.idProduct = PRODUCT_ID,
	.bcdDevice = { .maj = MSB( BCD_VERSION ), .min = LSB( BCD_VERSION ) },
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

struct usb_desc_string_t * const dfu_device_str_desc[] = {
	USB_DESC_STRING_LANG_ENUS,
	USB_DESC_STRING(STR_MANUFACTURER),
	USB_DESC_STRING(STR_PRODUCT),
	USB_DESC_STRING(STR_SERIAL),
	USB_DESC_STRING(STR_ALTNAME),
	USB_DESC_STRING(STR_CONFIG_NAME),
	NULL
};

const struct usbd_device dfu_device = {
	.dev_desc = &dfu_device_dev_desc,
	.string_descs = dfu_device_str_desc,
	.configs = {
		&usbd_config_1,
		NULL
	}
};

