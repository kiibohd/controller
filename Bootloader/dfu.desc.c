// Originally Generated from MCHCK Toolkit
/* Copyright (c) Jacob Alexander 2014-2020 <haata@kiibohd.com>
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

#if defined(_sam_)
#undef LSB
#undef MSB
#endif

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
#if !defined(_sam_)
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
			.can_upload = 0,
			.can_upload = 1,
			.can_download = 1,
			.wDetachTimeOut = 0,
			.wTransferSize = USB_DFU_TRANSFER_SIZE,
			.bcdDFUVersion = { .maj = 1, .min = 1 }
		}
	},
#endif
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

// Enables Microsoft specefic setup requests with bmRequestType set to bMS_VendorCode
// LanguageID must be 0, not english (0x0409)
#if defined(_sam_)
static struct usb_desc_string_t msft_os_str_desc = {
	.bLength = 18,
	.bDescriptorType = USB_DESC_STRING,
	.bString = {
		'M', 'S', 'F', 'T', '1', '0', '0',	// qwSignature
		MS_VENDOR_CODE,				// bMS_VendorCode & bPad
	}
};
#endif

// Microsoft Compatible ID Feature Descriptor ---
// Requests the given driver if available
struct usb_desc_msft_compat_t msft_extended_compat_desc = {
	.dwLength = sizeof(struct usb_desc_msft_compat_t),
	.bcdVersion = 0x01,
	.wIndex = USB_CTRL_REQ_MSFT_COMPAT_ID,
	.bCount = 1,
	.bFirstInterfaceNumber = 0,
	.compatibleID = STR_WCID_DRIVER
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

// Initialize DFU USB descriptor
void dfu_usb_init()
{
	usb_init( &dfu_device );
}

// Poll USB for changes in DFU status
void dfu_usb_poll()
{
	usb_poll();
}

#if defined(_sam_)
#include <common/services/usb/usb_protocol.h>
#include <common/services/usb/udc/udd.h>

/*! \brief Example of extra USB string management
 * This feature is available for single or composite device
 * which want implement additional USB string than
 * Manufacture, Product and serial number ID.
 *
 * return true, if the string ID requested is know and managed by this functions
 */
bool main_extra_string()
{
	struct usb_desc_string_t *desc = NULL;
	uint8_t inreq = udd_g_ctrlreq.req.wValue & 0xff;

	// Microsoft looks for a special string at index 0xEE
	// If found it will be compared against known OS compatabilily strings
	// Once matched additional Microsoft-specific setup requests may be sent
	if (inreq  == 0xEE) {
		desc = &msft_os_str_desc;
		goto send_str_desc;
	}

	// Lookup request and send descriptor
	for ( uint8_t req = 0; dfu_device_str_desc[req] != NULL; req++ )
	{
		// Request matches
		if ( inreq == req )
		{
			desc = dfu_device_str_desc[req];
			goto send_str_desc;
		}
	}

	// No string found
	return false;

send_str_desc:
	udd_g_ctrlreq.payload = (uint8_t *)desc;
	udd_g_ctrlreq.payload_size = desc->bLength;

	// if the string is larger than request length, then cut it
	if (udd_g_ctrlreq.payload_size > udd_g_ctrlreq.req.wLength) {
		udd_g_ctrlreq.payload_size = udd_g_ctrlreq.req.wLength;
	}

	return true;
}
#endif

