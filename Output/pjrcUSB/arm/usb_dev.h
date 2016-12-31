/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander 2014-2016
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

// Local Includes
#include "usb_mem.h"
#include "usb_desc.h"



// ----- Defines -----

#define usb_device_software_reset() SOFTWARE_RESET()



// ----- Variables -----

extern volatile uint8_t usb_configuration;

extern uint16_t usb_rx_byte_count_data[NUM_ENDPOINTS];



// ----- Functions -----

uint8_t usb_configured(); // is the USB port configured
uint8_t usb_init(); // Returns 1 on success, 0 if no cable is attached
void usb_reinit(); // Force restart USB interface, simulates disconnect

void usb_isr();
void usb_tx( uint32_t endpoint, usb_packet_t *packet );
void usb_tx_isr( uint32_t endpoint, usb_packet_t *packet );

uint8_t usb_resume();

uint32_t usb_tx_byte_count( uint32_t endpoint );
uint32_t usb_tx_packet_count( uint32_t endpoint );

usb_packet_t *usb_rx( uint32_t endpoint );

static inline uint32_t usb_rx_byte_count(uint32_t endpoint) __attribute__((always_inline));
static inline uint32_t usb_rx_byte_count(uint32_t endpoint)
{
	endpoint--;
	if ( endpoint >= NUM_ENDPOINTS )
		return 0;
	return usb_rx_byte_count_data[ endpoint ];
}

void usb_device_reload();
void usb_device_check();

extern void usb_serial_flush_callback();

