// Originally Generated from MCHCK Toolkit
/* Copyright (c) Jacob Alexander 2014-2022 <haata@kiibohd.com>
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

#pragma once

// ----- Local Includes -----

#include "device.h"


// ----- Defines -----

// We can set this to anything we want
#define MS_VENDOR_CODE 0x20


// ----- Variables -----

struct usb_desc_msft_compat_t {
	uint32_t dwLength;
	uint16_t bcdVersion;
	uint16_t wIndex;
	uint8_t  bCount;
	uint8_t  reserved1[7];
	uint8_t  bFirstInterfaceNumber;
	uint8_t  reserved2;
	uint8_t  compatibleID[8];
	uint8_t  subCompatibleID[8];
	uint8_t  reserved3[6];
};
CTASSERT_SIZE_BYTE(struct usb_desc_msft_compat_t, 40);

extern struct usb_desc_msft_compat_t msft_extended_compat_desc;
extern struct usb_desc_string_t * const dfu_device_str_desc[];

usbd_init_fun_t init_usb_bootloader;



// ----- Structs -----

struct usb_config_1 {
	struct usb_desc_config_t config;
	struct dfu_function_desc usb_function_0;
};



// ----- Functions -----

void dfu_usb_init(bool alt_device);
void dfu_usb_poll();

