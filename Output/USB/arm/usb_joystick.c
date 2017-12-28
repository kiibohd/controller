/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modified by Jacob Alexander (2015-2017)
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

#if enableJoystick_define == 1

// ----- Includes -----

// Compiler Includes
#include <string.h> // for memcpy()

// Project Includes
#include <Lib/OutputLib.h>
#include <print.h>

// Local Includes
#include "usb_dev.h"
#include "usb_joystick.h"



// ----- Defines -----

// Maximum number of transmit packets to queue so we don't starve other endpoints for memory
#define TX_PACKET_LIMIT 3

// When the PC isn't listening, how long do we wait before discarding data?
#define TX_TIMEOUT_MS 30



// ----- Variables -----

uint32_t usb_joystick_data[3];


static uint8_t transmit_previous_timeout = 0;



// ----- Functions -----

int usb_joystick_send()
{
	Time start = Time_now();
	usb_packet_t *tx_packet;

	//serial_print("send");
	//serial_print("\n");
	while ( 1 )
	{
		if ( !usb_configuration )
		{
			//serial_print("error1\n");
			return -1;
		}
		if ( usb_tx_packet_count( JOYSTICK_ENDPOINT ) < TX_PACKET_LIMIT )
		{
			tx_packet = usb_malloc();
			if ( tx_packet )
			{
				break;
			}
		}
		if ( Time_duration_ms( start ) > TX_TIMEOUT_MS || transmit_previous_timeout )
		{
			transmit_previous_timeout = 1;
			//serial_print("error2\n");
			return -1;
		}
		yield();
	}
	transmit_previous_timeout = 0;
	memcpy( tx_packet->buf, usb_joystick_data, 12 );
	tx_packet->len = 12;
	usb_tx( JOYSTICK_ENDPOINT, tx_packet );
	//serial_print("ok\n");
	return 0;
}

#endif

