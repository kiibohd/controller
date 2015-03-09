/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander 2013-2015
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

// ----- Includes -----

// Compiler Includes
#include <string.h> // for memcpy()

// Project Includes
#include <Lib/OutputLib.h>
#include <print.h>

// Local Includes
#include "usb_dev.h"
#include "usb_keyboard.h"



// ----- Defines -----

// Maximum number of transmit packets to queue so we don't starve other endpoints for memory
#define TX_PACKET_LIMIT 4

// When the PC isn't listening, how long do we wait before discarding data?
#define TX_TIMEOUT_MSEC 50

#if F_CPU == 96000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 596)
#elif F_CPU == 48000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 428)
#elif F_CPU == 24000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 262)
#endif



// ----- Variables -----

static uint8_t transmit_previous_timeout = 0;



// ----- Functions -----

// send the contents of keyboard_keys and keyboard_modifier_keys
void usb_keyboard_send()
{
	uint32_t wait_count = 0;
	usb_packet_t *tx_packet;

	// Wait till ready
	while ( 1 )
	{
		if ( !usb_configuration )
		{
			erro_print("USB not configured...");
			return;
		}

		if ( USBKeys_Protocol == 0 ) // Boot Mode
		{
			if ( usb_tx_packet_count( NKRO_KEYBOARD_ENDPOINT ) < TX_PACKET_LIMIT )
			{
				tx_packet = usb_malloc();
				if ( tx_packet )
					break;
			}
		}
		else if ( USBKeys_Protocol == 1 ) // NKRO Mode
		{
			if ( usb_tx_packet_count( KEYBOARD_ENDPOINT ) < TX_PACKET_LIMIT )
			{
				tx_packet = usb_malloc();
				if ( tx_packet )
					break;
			}
		}

		if ( ++wait_count > TX_TIMEOUT || transmit_previous_timeout )
		{
			transmit_previous_timeout = 1;
			warn_print("USB Transmit Timeout...");
			return;
		}
		yield();
	}

	// Pointer to USB tx packet buffer
	uint8_t *tx_buf = tx_packet->buf;

	switch ( USBKeys_Protocol )
	{
	// Send boot keyboard interrupt packet(s)
	case 0:
		// USB Boot Mode debug output
		if ( Output_DebugMode )
		{
			dbug_msg("Boot USB: ");
			printHex_op( USBKeys_Modifiers, 2 );
			print(" ");
			printHex( 0 );
			print(" ");
			printHex_op( USBKeys_Keys[0], 2 );
			printHex_op( USBKeys_Keys[1], 2 );
			printHex_op( USBKeys_Keys[2], 2 );
			printHex_op( USBKeys_Keys[3], 2 );
			printHex_op( USBKeys_Keys[4], 2 );
			printHex_op( USBKeys_Keys[5], 2 );
			print( NL );
		}

		// Boot Mode
		*tx_buf++ = USBKeys_Modifiers;
		*tx_buf++ = 0;
		memcpy( tx_buf, USBKeys_Keys, USB_BOOT_MAX_KEYS );
		tx_packet->len = 8;

		// Send USB Packet
		usb_tx( KEYBOARD_ENDPOINT, tx_packet );
		USBKeys_Changed = USBKeyChangeState_None;
		break;

	// Send NKRO keyboard interrupts packet(s)
	case 1:
		if ( Output_DebugMode )
		{
			dbug_msg("NKRO USB: ");
		}

		// Check system control keys
		if ( USBKeys_Changed & USBKeyChangeState_System )
		{
			if ( Output_DebugMode )
			{
				print("SysCtrl[");
				printHex_op( USBKeys_SysCtrl, 2 );
				print( "] " NL );
			}

			*tx_buf++ = 0x02; // ID
			*tx_buf   = USBKeys_SysCtrl;
			tx_packet->len = 2;

			// Send USB Packet
			usb_tx( NKRO_KEYBOARD_ENDPOINT, tx_packet );
			USBKeys_Changed &= ~USBKeyChangeState_System; // Mark sent
		}

		// Check consumer control keys
		if ( USBKeys_Changed & USBKeyChangeState_Consumer )
		{
			if ( Output_DebugMode )
			{
				print("ConsCtrl[");
				printHex_op( USBKeys_ConsCtrl, 2 );
				print( "] " NL );
			}

			*tx_buf++ = 0x03; // ID
			*tx_buf++ = (uint8_t)(USBKeys_ConsCtrl & 0x00FF);
			*tx_buf   = (uint8_t)(USBKeys_ConsCtrl >> 8);
			tx_packet->len = 3;

			// Send USB Packet
			usb_tx( NKRO_KEYBOARD_ENDPOINT, tx_packet );
			USBKeys_Changed &= ~USBKeyChangeState_Consumer; // Mark sent
		}

		// Standard HID Keyboard
		if ( USBKeys_Changed )
		{
			// USB NKRO Debug output
			if ( Output_DebugMode )
			{
				printHex_op( USBKeys_Modifiers, 2 );
				print(" ");
				for ( uint8_t c = 0; c < 6; c++ )
					printHex_op( USBKeys_Keys[ c ], 2 );
				print(" ");
				for ( uint8_t c = 6; c < 20; c++ )
					printHex_op( USBKeys_Keys[ c ], 2 );
				print(" ");
				printHex_op( USBKeys_Keys[20], 2 );
				print(" ");
				for ( uint8_t c = 21; c < 27; c++ )
					printHex_op( USBKeys_Keys[ c ], 2 );
				print( NL );
			}

			tx_packet->len = 0;

			// Modifiers
			*tx_buf++ = 0x01; // ID
			*tx_buf++ = USBKeys_Modifiers;
			tx_packet->len += 2;

			// 4-49 (first 6 bytes)
			memcpy( tx_buf, USBKeys_Keys, 6 );
			tx_buf += 6;
			tx_packet->len += 6;

			// 51-155 (Middle 14 bytes)
			memcpy( tx_buf, USBKeys_Keys + 6, 14 );
			tx_buf += 14;
			tx_packet->len += 14;

			// 157-164 (Next byte)
			memcpy( tx_buf, USBKeys_Keys + 20, 1 );
			tx_buf += 1;
			tx_packet->len += 1;

			// 176-221 (last 6 bytes)
			memcpy( tx_buf, USBKeys_Keys + 21, 6 );
			tx_packet->len += 6;

			// Send USB Packet
			usb_tx( NKRO_KEYBOARD_ENDPOINT, tx_packet );
			USBKeys_Changed = USBKeyChangeState_None; // Mark sent
		}

		break;
	}

	return;
}

