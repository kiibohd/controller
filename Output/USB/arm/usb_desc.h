/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modified by Jacob Alexander (2013-2020)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

// ----- Includes -----

// Compiler Includes
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Project Includes
#include <Lib/mcu_compat.h>



// ----- Defines -----

#define ENDPOINT_UNUSED                 0x00
#define ENDPOINT_TRANSIMIT_ONLY         0x15
#define ENDPOINT_RECEIVE_ONLY           0x19
#define ENDPOINT_TRANSMIT_AND_RECEIVE   0x1D


#define DEVICE_CLASS            0x00 // Keep 0x00 to indicate each sub device will indicate what it is
#define DEVICE_SUBCLASS         0x00
#define DEVICE_PROTOCOL         0x00
#define EP0_SIZE                64
#define NUM_ENDPOINTS           6 // XXX Can save some space if this can be calculated using KLL
#define NUM_INTERFACES          5 // XXX Should be calculated at build time
#define NUM_USB_BUFFERS         30

// XXX Remember to update total interface count, if it isn't correct some OSs will not initialize USB
//     Linux warns in dmesg
//     Mac OSX login screen will not initialize
#define KEYBOARD_INTERFACES     3 // Boot, NKRO, SysCtrl
#define MOUSE_INTERFACES        1
#define RAWIO_INTERFACES        1


#define KEYBOARD_INTERFACE      0 // Keyboard
#define KEYBOARD_ENDPOINT       1
#define KEYBOARD_SIZE           8
#define KEYBOARD_INTERVAL       1
#define KEYBOARD_NAME           L"Boot Keyboard"

#define NKRO_KEYBOARD_INTERFACE 1 // NKRO Keyboard
#define NKRO_KEYBOARD_ENDPOINT  2
#define NKRO_KEYBOARD_SIZE      64
#define NKRO_KEYBOARD_INTERVAL  1
#define NKRO_KEYBOARD_NAME      L"NKRO Keyboard"

#define SYS_CTRL_INTERFACE      2 // Media Keys
#define SYS_CTRL_ENDPOINT       3
#define SYS_CTRL_SIZE           8
#define SYS_CTRL_INTERVAL       1
#define SYS_CTRL_NAME           L"Media Keys"

#define MOUSE_INTERFACE         3 // Mouse
#define MOUSE_ENDPOINT          4
#define MOUSE_SIZE              8
#define MOUSE_INTERVAL          1
#define MOUSE_NAME              L"Mouse"

#define RAWIO_INTERFACE         4 // RawIO
#define RAWIO_TX_ENDPOINT       5
#define RAWIO_TX_SIZE           64
#define RAWIO_TX_INTERVAL       1
#define RAWIO_RX_ENDPOINT       6
#define RAWIO_RX_SIZE           64
#define RAWIO_RX_INTERVAL       1
#define RAWIO_USAGE_PAGE        0xFF1C
#define RAWIO_USAGE             0x1100
#define RAWIO_NAME              L"HID-IO Interface"


// Descriptor sizes
#define BASE_DESC_SIZE            (9)
#define KEYBOARD_DESC_SIZE        (9+9+7)
#define NKRO_KEYBOARD_DESC_SIZE   (9+9+7)
#define SYS_CTRL_DESC_SIZE        (9+9+7)
#define MOUSE_DESC_SIZE           (9+9+7)
#define RAWIO_DESC_SIZE           (9+9+7+7)

// Descriptor offsets
#define KEYBOARD_DESC_BASE_OFFSET ( \
	BASE_DESC_SIZE + \
	9 \
)
#define RAWIO_DESC_BASE_OFFSET ( \
	BASE_DESC_SIZE + \
	KEYBOARD_DESC_TOTAL_OFFSET + \
	MOUSE_DESC_BASE_OFFSET + \
	9 \
)
#define MOUSE_DESC_BASE_OFFSET ( \
	BASE_DESC_SIZE + \
	KEYBOARD_DESC_TOTAL_OFFSET + \
	9 \
)


#define ENDPOINT1_CONFIG        ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT2_CONFIG        ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT3_CONFIG        ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT4_CONFIG        ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT5_CONFIG        ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT6_CONFIG        ENDPOINT_RECEIVE_ONLY

#if defined(_sam_)
#define USB_DEVICE_EP_CTRL_SIZE EP0_SIZE

#undef  USB_DEVICE_MAX_EP
#define USB_DEVICE_MAX_EP NUM_ENDPOINTS

#define UDI_HID_REPORT_IN_SIZE RAWIO_TX_SIZE
#define UDI_HID_REPORT_OUT_SIZE RAWIO_RX_SIZE
#define UDI_HID_GENERIC_EP_IN RAWIO_TX_ENDPOINT
#define UDI_HID_GENERIC_EP_OUT RAWIO_RX_ENDPOINT

#define UDI_HID_GENERIC_REPORT_OUT(ptr) HIDIO_rawio_rx_callback(ptr)
#define UDI_HID_GENERIC_ENABLE_EXT() HIDIO_rawio_enable()
#define UDI_HID_GENERIC_DISABLE_EXT() HIDIO_rawio_disable()
#endif

// ----- Structs -----

typedef struct {
	uint16_t        wValue;
	uint16_t        wIndex;
	const uint8_t   *addr;
	uint16_t        length;
} usb_descriptor_list_t;


struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wString[];
};



// ----- Variables -----

// NUM_ENDPOINTS = number of non-zero endpoints (0 to 15)
extern const uint8_t usb_endpoint_config_table[NUM_ENDPOINTS];

extern const usb_descriptor_list_t usb_descriptor_list[];

extern uint8_t *usb_bMaxPower;



// ----- Functions -----

void usb_set_config_descriptor_size();

#if defined(_sam_)
bool my_udi_hid_setup(void);
#endif

