/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander 2013-2018
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

#include <kll_defs.h>
#if enableKeyboard_define == 1

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
#define TX_TIMEOUT_MS 50



// ----- Variables -----

static uint8_t transmit_previous_timeout = 0;



// ----- Functions -----

// Re-send the contents of the keyboard buffer, if exceeding the expiry timer
void usb_keyboard_idle_update()
{
	// Ignore if set to 0
	if ( USBKeys_Idle_Config != 0 )
	{
		// Check if we need to send an update
		// USBKeys_Idle_Expiry is updated on usb_tx
		if ( USBKeys_Idle_Expiry + USBKeys_Idle_Config * 4 >= systick_millis_count )
		{
			USBKeys_idle.changed = USBKeyChangeState_All;

			// Send packets for each of the keyboard interfaces
			while ( USBKeys_idle.changed )
			{
				usb_keyboard_send( (USBKeys*)&USBKeys_idle, USBKeys_Protocol );
			}
		}
	}
}


// Send usb clear packet
// Useful when switching between 6kro and nkro
void usb_keyboard_clear( uint8_t protocol )
{
	// Prepare empty buffer
	USBKeys buffer = {
		.modifiers = 0,
		.keys = { 0 },
		.sys_ctrl = 0,
		.cons_ctrl = 0,
		.changed = USBKeyChangeState_All,
	};

	// Send updates
	usb_keyboard_send( &buffer, protocol );
}


// Send the contents of keyboard_keys and keyboard_modifier_keys
void usb_keyboard_send( USBKeys *buffer, uint8_t protocol )
{
	Time start = Time_now();
	usb_packet_t *tx_packet;

	// Wait till ready
	while ( 1 )
	{
		if ( !usb_configuration )
		{
			erro_print("USB not configured...");
			return;
		}

		// Try to wake up the host if it's asleep
		if ( usb_resume() )
		{
			// Drop packet
			buffer->changed = USBKeyChangeState_None;
			return;
		}

		if ( protocol == 0 ) // Boot Mode
		{
			if ( usb_tx_packet_count( KEYBOARD_ENDPOINT ) < TX_PACKET_LIMIT )
			{
				tx_packet = usb_malloc();
				if ( tx_packet )
					break;
			}
		}
		else if ( protocol == 1 ) // NKRO Mode
		{
			if ( usb_tx_packet_count( NKRO_KEYBOARD_ENDPOINT ) < TX_PACKET_LIMIT )
			{
				tx_packet = usb_malloc();
				if ( tx_packet )
					break;
			}
		}
		else if ( buffer->changed &
			( USBKeyChangeState_System | USBKeyChangeState_Consumer )
		)
		{
			if ( usb_tx_packet_count( SYS_CTRL_ENDPOINT ) < TX_PACKET_LIMIT )
			{
				tx_packet = usb_malloc();
				if ( tx_packet )
					break;
			}
		}

		// USB Timeout, drop the packet, and potentially try something more drastic to re-enable the bus
		if ( Time_duration_ms( start ) > TX_TIMEOUT_MS || transmit_previous_timeout )
		{
			transmit_previous_timeout = 1;
			buffer->changed = USBKeyChangeState_None; // Indicate packet lost
			#if enableDeviceRestartOnUSBTimeout == 1
			warn_print("USB Transmit Timeout...restarting device");
			usb_device_software_reset();
			#else
			warn_print("USB Transmit Timeout...auto-restart disabled");
			#endif
			// Try to wakeup
			return;
		}

		yield();
	}

	transmit_previous_timeout = 0;

	// Pointer to USB tx packet buffer
	uint8_t *tx_buf = tx_packet->buf;

	// Check system control keys
	if ( buffer->changed & USBKeyChangeState_System )
	{
		if ( Output_DebugMode )
		{
			USB_SysCtrlDebug( buffer );
		}

		// Store update for idle packet
		USBKeys_idle.sys_ctrl = buffer->sys_ctrl;

		*tx_buf++ = 0x02; // ID
		*tx_buf   = buffer->sys_ctrl;
		tx_packet->len = 2;

		// Send USB Packet
		usb_tx( SYS_CTRL_ENDPOINT, tx_packet );
		buffer->changed &= ~USBKeyChangeState_System; // Mark sent
		return;
	}

	// Check consumer control keys
	if ( buffer->changed & USBKeyChangeState_Consumer )
	{
		if ( Output_DebugMode )
		{
			USB_ConsCtrlDebug( buffer );
		}

		// Store update for idle packet
		USBKeys_idle.cons_ctrl = buffer->cons_ctrl;

		*tx_buf++ = 0x03; // ID
		*tx_buf++ = (uint8_t)(buffer->cons_ctrl & 0x00FF);
		*tx_buf   = (uint8_t)(buffer->cons_ctrl >> 8);
		tx_packet->len = 3;

		// Send USB Packet
		usb_tx( SYS_CTRL_ENDPOINT, tx_packet );
		buffer->changed &= ~USBKeyChangeState_Consumer; // Mark sent
		return;
	}

	switch ( protocol )
	{
	// Send boot keyboard interrupt packet(s)
	case 0:
		// USB Boot Mode debug output
		if ( Output_DebugMode )
		{
			USB_6KRODebug( buffer );
		}

		// Store update for idle packet
		memcpy( (void*)&USBKeys_idle.keys, buffer->keys, USB_BOOT_MAX_KEYS );
		USBKeys_idle.modifiers = buffer->modifiers;

		// Boot Mode
		*tx_buf++ = buffer->modifiers;
		*tx_buf++ = 0;
		memcpy( tx_buf, buffer->keys, USB_BOOT_MAX_KEYS );
		tx_packet->len = 8;

		// Send USB Packet
		usb_tx( KEYBOARD_ENDPOINT, tx_packet );
		buffer->changed = USBKeyChangeState_None;
		break;

	// Send NKRO keyboard interrupts packet(s)
	case 1:
		if ( Output_DebugMode )
		{
			dbug_msg("NKRO USB: ");
		}

		// Standard HID Keyboard
		if ( buffer->changed )
		{
			// USB NKRO Debug output
			if ( Output_DebugMode )
			{
				USB_NKRODebug( buffer );
			}

			// Store update for idle packet
			memcpy( (void*)&USBKeys_idle.keys, buffer->keys, USB_NKRO_BITFIELD_SIZE_KEYS );
			USBKeys_idle.modifiers = buffer->modifiers;

			// Clear packet length
			tx_packet->len = 0;

			// Modifiers
			*tx_buf++ = 0x01; // ID
			*tx_buf++ = buffer->modifiers;
			tx_packet->len += 2;

			// 4-49 (first 6 bytes)
			memcpy( tx_buf, buffer->keys, 6 );
			tx_buf += 6;
			tx_packet->len += 6;

			// 51-155 (Middle 14 bytes)
			memcpy( tx_buf, buffer->keys + 6, 14 );
			tx_buf += 14;
			tx_packet->len += 14;

			// 157-164 (Next byte)
			memcpy( tx_buf, buffer->keys + 20, 1 );
			tx_buf += 1;
			tx_packet->len += 1;

			// 176-221 (last 6 bytes)
			memcpy( tx_buf, buffer->keys + 21, 6 );
			tx_packet->len += 6;

			// Send USB Packet
			usb_tx( NKRO_KEYBOARD_ENDPOINT, tx_packet );
			buffer->changed = USBKeyChangeState_None; // Mark sent
		}

		break;
	}

	return;
}

#endif

