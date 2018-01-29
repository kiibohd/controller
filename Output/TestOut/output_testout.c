/* Copyright (C) 2011-2018 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/OutputLib.h>
#include <string.h>

// Project Includes
#include <hidio_com.h>
#include <output_com.h>
#include <output_usb.h>
#include <print.h>

// KLL
#include <kll.h>

// Local Includes
#include "output_testout.h"



// ----- Macros -----

// ----- Function Declarations -----

// ----- Variables -----

// Needed to automatically determine buffer sizes for shared library
uint8_t  USBKeys_BitfieldSize = USB_NKRO_BITFIELD_SIZE_KEYS;

// Callback function to host
// Output_Host_Callback( char* command, char* args ) return int
void *Output_Host_Callback;



// ----- Capabilities -----

// ----- Functions -----

// Convenient wrapper for host callback
int Output_callback( char* command, char* args )
{
	int (*callback)(char* command, char* args) = (int(*)(char*, char*))(Output_Host_Callback);

	return callback( command, args );
}

// USB Module Setup
inline void TestOut_setup()
{
	// Initialize Interface module
	OutputGen_setup();

	// USB Protocol Transition variable
	USBKeys_Protocol_Change = 0;

	// Flush key buffers
	USB_flushBuffers();

#if enableRawIO_define == 1
	// Setup HID-IO
	HIDIO_setup();
#endif
}


// USB Data Poll
inline void TestOut_poll()
{
#if enableRawIO_define == 1
	// HID-IO Process
	HIDIO_process();
#endif
}


// USB Data Periodic
inline void TestOut_periodic()
{
#if enableMouse_define == 1
	// Process mouse actions
	while ( USBMouse_Changed )
	{
		Output_callback( "mouse_send", "" );
	}
#endif

#if enableKeyboard_define == 1
	// Determine if we need to change the Kbd Protocol
	if ( USBKeys_Protocol_Change )
	{
		// Clear current interface
		usb_keyboard_clear( USBKeys_Protocol );

		// Set new protocol
		USBKeys_Protocol = USBKeys_Protocol_New;
		USBKeys_Protocol_Change = 0;
	}

	// Boot Mode Only, unset stale keys
	if ( USBKeys_Protocol == 0 )
	{
		for ( uint8_t c = USBKeys_Sent; c < USB_BOOT_MAX_KEYS; c++ )
		{
			USBKeys_primary.keys[c] = 0;
		}
	}

	// Send keypresses while there are pending changes
	while ( USBKeys_primary.changed )
	{
		usb_keyboard_send( (USBKeys*)&USBKeys_primary, USBKeys_Protocol );
		Output_callback( "keyboard_send", "" );
	}

	// Signal Scan Module we are finished
	switch ( USBKeys_Protocol )
	{
	case 0: // Boot Mode
		Scan_finishedWithOutput( USBKeys_Sent <= USB_BOOT_MAX_KEYS ? USBKeys_Sent : USB_BOOT_MAX_KEYS );
		break;
	case 1: // NKRO Mode
		Scan_finishedWithOutput( USBKeys_Sent );
		break;
	}
#endif
}


// Sets the device into firmware reload mode
inline void TestOut_firmwareReload()
{
	Output_callback( "device_reload", "" );
}


// USB Input buffer available
inline unsigned int TestOut_availablechar()
{
#if enableVirtualSerialPort_define == 1
	return (unsigned int)Output_callback( "serial_available", "" );
#else
	return 0;
#endif
}


// USB Get Character from input buffer
inline int TestOut_getchar()
{
#if enableVirtualSerialPort_define == 1
	return Output_callback( "serial_read", "" );
#else
	return 0;
#endif
}


// USB Send Character to output buffer
inline int TestOut_putchar( char c )
{
#if enableVirtualSerialPort_define == 1
	char out[2] = { c, '\0' };
	return Output_callback( "serial_write", out );
#else
	return 0;
#endif
}


// USB Send String to output buffer, null terminated
inline int TestOut_putstr( char* str )
{
#if enableVirtualSerialPort_define == 1
	return Output_callback( "serial_write", str );
#else
	return 0;
#endif
}


// Soft Chip Reset
inline void TestOut_softReset()
{
	Output_callback( "restart", "" );
}


// USB RawIO buffer available
unsigned int TestOut_rawio_availablechar()
{
#if enableRawIO_define == 1
	return (unsigned int)Output_callback( "rawio_available", "" );
#else
	return 0;
#endif
}


// USB RawIO get buffer
// XXX Must be a 64 byte buffer
int TestOut_rawio_getbuffer( char* buffer )
{
#if enableRawIO_define == 1
	// TODO
	return Output_callback( "rawio_rx", buffer );
#else
	return 0;
#endif
}


// USB RawIO send buffer
// XXX Must be a 64 byte buffer
int TestOut_rawio_sendbuffer( char* buffer )
{
#if enableRawIO_define == 1
	return Output_callback( "rawio_tx", buffer );
#else
	return 0;
#endif
}



// ----- USB Dummy Functions -----

void usb_init() {}

void usb_device_check() {}
void usb_device_reload() {}
void usb_device_software_reset() {}

void usb_keyboard_clear( uint8_t protocol ) {}
void usb_keyboard_idle_update() {}
void usb_keyboard_send( USBKeys *buffer, uint8_t protocol )
{
	// Only show debug information
	if ( Output_DebugMode )
	{
		// SysCtrl
		if ( buffer->changed & USBKeyChangeState_System )
		{
			USB_SysCtrlDebug( buffer );
			buffer->changed &= ~USBKeyChangeState_System; // Mark sent
		}

		// ConsCtrl
		if ( buffer->changed & USBKeyChangeState_Consumer )
		{
			USB_ConsCtrlDebug( buffer );
			buffer->changed &= ~USBKeyChangeState_Consumer; // Mark sent
		}

		// USB
		switch ( protocol )
		{
		// Send boot keyboard interrupt packet(s)
		case 0:
			if ( buffer->changed )
			{
				USB_6KRODebug( buffer );
			}
			break;

		// Send nkro keyboard interrupt packets(s)
		case 1:
			if ( buffer->changed )
			{
				USB_NKRODebug( buffer );
			}
			break;
		}
	}

	buffer->changed = USBKeyChangeState_None;
}

void usb_mouse_send() {}

int usb_serial_available() { return 0; }
int usb_serial_getchar() { return 0; }
int usb_serial_putchar( uint8_t c ) { return 0; }
int usb_serial_write( const void *buffer, uint32_t size ) { return 0; }

uint32_t usb_rawio_available() { return 0; }
int32_t usb_rawio_rx( void *buf, uint32_t timeout ) { return 0; }
int32_t usb_rawio_tx( const void *buf, uint32_t timeout ) { return 0; }

