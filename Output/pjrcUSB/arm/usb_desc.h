/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
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

#ifndef _usb_desc_h_
#define _usb_desc_h_

// This header is NOT meant to be included when compiling
// user sketches in Arduino.  The low-level functions
// provided by usb_dev.c are meant to be called only by
// code which provides higher-level interfaces to the user.

#include <stdint.h>
#include <stddef.h>
#include "output_com.h"

#define ENDPOINT_UNUSED			0x00
#define ENDPOINT_TRANSIMIT_ONLY		0x15
#define ENDPOINT_RECEIVE_ONLY		0x19
#define ENDPOINT_TRANSMIT_AND_RECEIVE	0x1D

// Some operating systems, especially Windows, may cache USB device
// info.  Changes to the device name may not update on the same
// computer unless the vendor or product ID numbers change, or the
// "bcdDevice" revision code is increased.

#define DEVICE_CLASS		0xEF
#define DEVICE_SUBCLASS	0x02
#define DEVICE_PROTOCOL	0x01
#define EP0_SIZE		64
#define NUM_ENDPOINTS		15
#define NUM_INTERFACE		4
#define CDC_IAD_DESCRIPTOR	1
#define CDC_STATUS_INTERFACE	0
#define CDC_DATA_INTERFACE	1	// Serial
#define CDC_ACM_ENDPOINT	2
#define CDC_RX_ENDPOINT       3
#define CDC_TX_ENDPOINT       4
#define CDC_ACM_SIZE          16
#define CDC_RX_SIZE           64
#define CDC_TX_SIZE           64
#define KEYBOARD_INTERFACE    2	// Keyboard
#define KEYBOARD_ENDPOINT     1
#define KEYBOARD_SIZE         8
#define KEYBOARD_INTERVAL     1
#define MOUSE_INTERFACE       3	// Mouse
#define MOUSE_ENDPOINT        5
#define MOUSE_SIZE            8
#define MOUSE_INTERVAL        2
#define KEYBOARD_DESC_OFFSET	(9+8 + 9+5+5+4+5+7+9+7+7 + 9)
#define MOUSE_DESC_OFFSET	(9+8 + 9+5+5+4+5+7+9+7+7 + 9+9+7 + 9)
#define CONFIG_DESC_SIZE	(9+8 + 9+5+5+4+5+7+9+7+7 + 9+9+7 + 9+9+7)
#define ENDPOINT1_CONFIG	ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT2_CONFIG	ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT3_CONFIG	ENDPOINT_RECEIVE_ONLY
#define ENDPOINT4_CONFIG	ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT5_CONFIG	ENDPOINT_TRANSIMIT_ONLY



// NUM_ENDPOINTS = number of non-zero endpoints (0 to 15)
extern const uint8_t usb_endpoint_config_table[NUM_ENDPOINTS];

typedef struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint16_t	length;
} usb_descriptor_list_t;

extern const usb_descriptor_list_t usb_descriptor_list[];


#endif

