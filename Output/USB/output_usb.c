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

// Project Includes
#include <cli.h>
#include <hidio_com.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <scan_loop.h>

// USB Includes
#if defined(_avr_at_)
#include "avr/usb_keyboard_serial.h"
#elif defined(_kinetis_) || defined(_sam_)
#include "arm/usb_dev.h"
#include "arm/usb_keyboard.h"
#include "arm/usb_mouse.h"
#include "arm/usb_rawio.h"
#include "arm/usb_serial.h"
#endif

// KLL
#include <kll_defs.h>
#include <kll.h>

// Interface Includes
#include <output_com.h>



// ----- Macros -----

// Used to build a bitmap lookup table from a byte addressable array
#define byteLookup( byte ) \
	case (( byte ) * ( 8 )):         bytePosition = byte; byteShift = 0; break; \
	case (( byte ) * ( 8 ) + ( 1 )): bytePosition = byte; byteShift = 1; break; \
	case (( byte ) * ( 8 ) + ( 2 )): bytePosition = byte; byteShift = 2; break; \
	case (( byte ) * ( 8 ) + ( 3 )): bytePosition = byte; byteShift = 3; break; \
	case (( byte ) * ( 8 ) + ( 4 )): bytePosition = byte; byteShift = 4; break; \
	case (( byte ) * ( 8 ) + ( 5 )): bytePosition = byte; byteShift = 5; break; \
	case (( byte ) * ( 8 ) + ( 6 )): bytePosition = byte; byteShift = 6; break; \
	case (( byte ) * ( 8 ) + ( 7 )): bytePosition = byte; byteShift = 7; break



// ----- Function Declarations -----

void cliFunc_idle       ( char* args );
void cliFunc_kbdProtocol( char* args );
void cliFunc_readLEDs   ( char* args );
void cliFunc_usbInitTime( char* args );



// ----- Variables -----

// Output Module command dictionary
CLIDict_Entry( idle,        "Show/set the HID Idle time (multiples of 4 ms)." );
CLIDict_Entry( kbdProtocol, "Keyboard Protocol Mode: 0 - Boot, 1 - OS/NKRO Mode." );
CLIDict_Entry( readLEDs,    "Read LED byte:" NL "\t\t1 NumLck, 2 CapsLck, 4 ScrlLck, 16 Kana, etc." );
CLIDict_Entry( usbInitTime, "Displays the time in ms from usb_init() till the last setup call." );

CLIDict_Def( usbCLIDict, "USB Module Commands" ) = {
	CLIDict_Item( idle ),
	CLIDict_Item( kbdProtocol ),
	CLIDict_Item( readLEDs ),
	CLIDict_Item( usbInitTime ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// USBKeys Keyboard Buffer
volatile USBKeys USBKeys_primary; // Primary send buffer
volatile USBKeys USBKeys_idle;    // Idle timeout send buffer

// The number of keys sent to the usb in the array
volatile uint8_t  USBKeys_Sent;

// 1=num lock, 2=caps lock, 4=scroll lock, 8=compose, 16=kana
volatile uint8_t  USBKeys_LEDs = 0;
volatile uint8_t  USBKeys_LEDs_Changed;

// Currently pressed mouse buttons, bitmask, 0 represents no buttons pressed
volatile uint16_t USBMouse_Buttons = 0;

// Relative mouse axis movement, stores pending movement
volatile uint16_t USBMouse_Relative_x = 0;
volatile uint16_t USBMouse_Relative_y = 0;

// Protocol setting from the host.
// 0 - Boot Mode
// 1 - NKRO Mode (Default, unless set by a BIOS or boot interface)
volatile uint8_t  USBKeys_Protocol = USBProtocol_define;
volatile uint8_t  USBKeys_Protocol_New = USBProtocol_define;
volatile uint8_t  USBKeys_Protocol_Change; // New value to set to USBKeys_Protocol if _Change is set

// Indicate if USB should send update
USBMouseChangeState USBMouse_Changed = 0;

// the idle configuration, how often we send the report to the
// host (ms * 4) even when it hasn't changed
// 0 - Disables
volatile uint8_t  USBKeys_Idle_Config = USBIdle_define;

// Count until idle timeout
volatile uint32_t USBKeys_Idle_Expiry = 0;
volatile uint8_t  USBKeys_Idle_Count = 0;

// USB Init Time (ms) - usb_init()
volatile uint32_t USBInit_TimeStart;
volatile uint32_t USBInit_TimeEnd;
volatile uint16_t USBInit_Ticks;

// Latency measurement resource
static uint8_t outputPeriodicLatencyResource;
static uint8_t outputPollLatencyResource;



// ----- Capabilities -----

// Set Boot Keyboard Protocol
void Output_kbdProtocolBoot_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_kbdProtocolBoot()");
		return;
	}

	// Only set if necessary
	if ( USBKeys_Protocol == 0 )
		return;

	// TODO Analog inputs
	// Only set on key press
	if ( stateType != 0x01 )
		return;

	// Flush the key buffers
	USB_flushBuffers();

	// Set the keyboard protocol to Boot Mode
	USBKeys_Protocol_New = 0;
	USBKeys_Protocol_Change = 1;
#endif
}


// Set NKRO Keyboard Protocol
void Output_kbdProtocolNKRO_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_kbdProtocolNKRO()");
		return;
	}

	// Only set if necessary
	if ( USBKeys_Protocol == 1 )
		return;

	// TODO Analog inputs
	// Only set on key press
	if ( stateType != 0x01 )
		return;

	// Flush the key buffers
	USB_flushBuffers();

	// Set the keyboard protocol to NKRO Mode
	USBKeys_Protocol_New = 1;
	USBKeys_Protocol_Change = 1;
#endif
}


// Toggle Keyboard Protocol
void Output_toggleKbdProtocol_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_toggleKbdProtocol()");
		return;
	}

	// Only toggle protocol if release state
	if ( stateType == 0x00 && state == 0x03 )
	{
		// Flush the key buffers
		USB_flushBuffers();

		// Toggle the keyboard protocol Mode
		USBKeys_Protocol_New = !USBKeys_Protocol;
		USBKeys_Protocol_Change = 1;
	}
#endif
}


// Sends a Consumer Control code to the USB Output buffer
void Output_consCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_consCtrlSend(consCode)");
		return;
	}

	// TODO Analog inputs
	// Only indicate USB has changed if either a press or release has occured
	if ( state == 0x01 || state == 0x03 )
		USBKeys_primary.changed |= USBKeyChangeState_Consumer;

	// Only send keypresses if press or hold state
	if ( stateType == 0x00 && state == 0x03 ) // Release state
	{
		USBKeys_primary.cons_ctrl = 0;
		return;
	}

	// Set consumer control code
	USBKeys_primary.cons_ctrl = *(uint16_t*)(&args[0]);
#endif
}


// Ignores the given key status update
// Used to prevent fall-through, this is the None keyword in KLL
void Output_noneSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_noneSend()");
		return;
	}

	// Nothing to do, because that's the point :P
}


// Sends a System Control code to the USB Output buffer
void Output_sysCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_sysCtrlSend(sysCode)");
		return;
	}

	// TODO Analog inputs
	// Only indicate USB has changed if either a press or release has occured
	if ( state == 0x01 || state == 0x03 )
		USBKeys_primary.changed |= USBKeyChangeState_System;

	// Only send keypresses if press or hold state
	if ( stateType == 0x00 && state == 0x03 ) // Release state
	{
		USBKeys_primary.sys_ctrl = 0;
		return;
	}

	// Set system control code
	USBKeys_primary.sys_ctrl = args[0];
#endif
}


// Adds a single USB Code to the USB Output buffer
// Argument #1: USB Code
void Output_usbCodeSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_usbCodeSend(usbCode)");
		return;
	}

	// Depending on which mode the keyboard is in the USB needs Press/Hold/Release events
	uint8_t keyPress = 0; // Default to key release

	// Only send press and release events
	if ( stateType == 0x00 && state == 0x02 ) // Hold state
		return;

	// If press, send bit (NKRO) or byte (6KRO)
	if ( stateType == 0x00 && state == 0x01 ) // Press state
		keyPress = 1;

	// Get the keycode from arguments
	uint8_t key = args[0];

	// Extra USB Debug
	if ( Output_DebugMode > 1 )
	{
		print("\033[1;34mUSB\033[0m ");
		printInt8( key );
		print(" ");
		if ( state == 0x01 )
		{
			print("\033[1;33mP\033[0m");
		}
		else if ( state == 0x03 )
		{
			print("\033[1;35mR\033[0m");
		}
		print( NL );
	}

	// Depending on which mode the keyboard is in, USBKeys_Keys array is used differently
	// Boot mode - Maximum of 6 byte codes
	// NKRO mode - Each bit of the 26 byte corresponds to a key
	//  Bits   0 -  45 (bytes  0 -  5) correspond to USB Codes   4 -  49 (Main)
	//  Bits  48 - 161 (bytes  6 - 20) correspond to USB Codes  51 - 164 (Secondary)
	//  Bits 168 - 213 (bytes 21 - 26) correspond to USB Codes 176 - 221 (Tertiary)
	//  Bits 214 - 216                 unused
	uint8_t bytePosition = 0;
	uint8_t byteShift = 0;

	switch ( USBKeys_Protocol )
	{
	case 0: // Boot Mode
		// Set the modifier bit if this key is a modifier
		if ( (key & 0xE0) == 0xE0 ) // AND with 0xE0 (Left Ctrl, first modifier)
		{
			if ( keyPress )
			{
				USBKeys_primary.modifiers |= 1 << (key ^ 0xE0); // Left shift 1 by key XOR 0xE0
			}
			else // Release
			{
				USBKeys_primary.modifiers &= ~(1 << (key ^ 0xE0)); // Left shift 1 by key XOR 0xE0
			}

			USBKeys_primary.changed |= USBKeyChangeState_Modifiers;
		}
		// Normal USB Code
		else
		{
			// Determine if key was set
			uint8_t keyFound = 0;

			for ( uint8_t newkey = 0; newkey < USBKeys_Sent; newkey++ )
			{
				// On press, key already present, don't re-add
				if ( keyPress && USBKeys_primary.keys[newkey] == key )
				{
					keyFound = 1;
					break;
				}

				// On release, remove if found
				if ( !keyPress && USBKeys_primary.keys[newkey] == key )
				{
					// Shift keys over
					for ( uint8_t pos = newkey; pos < USBKeys_Sent - 1; pos++ )
					{
						USBKeys_primary.keys[pos] = USBKeys_primary.keys[pos + 1];
					}
					USBKeys_Sent--;
					keyFound = 1;
					USBKeys_primary.changed = USBKeyChangeState_MainKeys;
					break;
				}
			}

			// USB Key limit reached
			if ( USBKeys_Sent >= USB_BOOT_MAX_KEYS )
			{
				warn_print("USB Key limit reached");
				break;
			}

			// Add key if not already found in the buffer
			if ( keyPress && !keyFound )
			{
				USBKeys_primary.keys[USBKeys_Sent++] = key;
				USBKeys_primary.changed = USBKeyChangeState_MainKeys;
			}
		}
		break;

	case 1: // NKRO Mode
		// Set the modifier bit if this key is a modifier
		if ( (key & 0xE0) == 0xE0 ) // AND with 0xE0 (Left Ctrl, first modifier)
		{
			if ( keyPress )
			{
				USBKeys_primary.modifiers |= 1 << (key ^ 0xE0); // Left shift 1 by key XOR 0xE0
			}
			else // Release
			{
				USBKeys_primary.modifiers &= ~(1 << (key ^ 0xE0)); // Left shift 1 by key XOR 0xE0
			}

			USBKeys_primary.changed |= USBKeyChangeState_Modifiers;
			break;
		}
		// First 6 bytes
		else if ( key >= 4 && key <= 49 )
		{
			// Lookup (otherwise division or multiple checks are needed to do alignment)
			// Starting at 0th position, each byte has 8 bits, starting at 4th bit
			uint8_t keyPos = key + (0 * 8 - 4); // Starting position in array, Ignoring 4 keys
			switch ( keyPos )
			{
				byteLookup( 0 );
				byteLookup( 1 );
				byteLookup( 2 );
				byteLookup( 3 );
				byteLookup( 4 );
				byteLookup( 5 );
			}

			USBKeys_primary.changed |= USBKeyChangeState_MainKeys;
		}
		// Next 14 bytes
		else if ( key >= 51 && key <= 155 )
		{
			// Lookup (otherwise division or multiple checks are needed to do alignment)
			// Starting at 6th byte position, each byte has 8 bits, starting at 51st bit
			uint8_t keyPos = key + (6 * 8 - 51); // Starting position in array
			switch ( keyPos )
			{
				byteLookup( 6 );
				byteLookup( 7 );
				byteLookup( 8 );
				byteLookup( 9 );
				byteLookup( 10 );
				byteLookup( 11 );
				byteLookup( 12 );
				byteLookup( 13 );
				byteLookup( 14 );
				byteLookup( 15 );
				byteLookup( 16 );
				byteLookup( 17 );
				byteLookup( 18 );
				byteLookup( 19 );
			}

			USBKeys_primary.changed |= USBKeyChangeState_SecondaryKeys;
		}
		// Next byte
		else if ( key >= 157 && key <= 164 )
		{
			// Lookup (otherwise division or multiple checks are needed to do alignment)
			uint8_t keyPos = key + (20 * 8 - 157); // Starting position in array, Ignoring 6 keys
			switch ( keyPos )
			{
				byteLookup( 20 );
			}

			USBKeys_primary.changed |= USBKeyChangeState_TertiaryKeys;
		}
		// Last 6 bytes
		else if ( key >= 176 && key <= 221 )
		{
			// Lookup (otherwise division or multiple checks are needed to do alignment)
			uint8_t keyPos = key + (21 * 8 - 176); // Starting position in array
			switch ( keyPos )
			{
				byteLookup( 21 );
				byteLookup( 22 );
				byteLookup( 23 );
				byteLookup( 24 );
				byteLookup( 25 );
				byteLookup( 26 );
			}

			USBKeys_primary.changed |= USBKeyChangeState_QuartiaryKeys;
		}
		// Received 0x00
		// This is a special USB Code that internally indicates a "break"
		// It is used to send "nothing" in order to break up sequences of USB Codes
		else if ( key == 0x00 )
		{
			USBKeys_primary.changed |= USBKeyChangeState_MainKeys;
			break;
		}
		// Invalid key
		else
		{
			warn_msg("USB Code not within 4-49 (0x4-0x31), 51-155 (0x33-0x9B), 157-164 (0x9D-0xA4), 176-221 (0xB0-0xDD) or 224-231 (0xE0-0xE7) NKRO Mode: ");
			printHex( key );
			print( NL );
			break;
		}

		// Set/Unset
		if ( keyPress )
		{
			USBKeys_primary.keys[bytePosition] |= (1 << byteShift);
			USBKeys_Sent--;
		}
		else // Release
		{
			USBKeys_primary.keys[bytePosition] &= ~(1 << byteShift);
			USBKeys_Sent++;
		}

		break;
	}
#endif
}

#if enableMouse_define == 1
// Sends a mouse command over the USB Output buffer
// XXX This function *will* be changing in the future
//     If you use it, be prepared that your .kll files will break in the future (post KLL 0.5)
// Argument #1: USB Mouse Button (16 bit)
// Argument #2: USB X Axis (16 bit) relative
// Argument #3: USB Y Axis (16 bit) relative
void Output_usbMouse_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_usbMouse(mouseButton,relX,relY)");
		return;
	}

	// Determine which mouse button was sent
	// The USB spec defines up to a max of 0xFFFF buttons
	// The usual are:
	// 1 - Button 1 - (Primary)
	// 2 - Button 2 - (Secondary)
	// 3 - Button 3 - (Tertiary)
	uint16_t mouse_button = *(uint16_t*)(&args[0]);

	// X/Y Relative Axis
	uint16_t mouse_x = *(uint16_t*)(&args[2]);
	uint16_t mouse_y = *(uint16_t*)(&args[4]);

	// Adjust for bit shift
	uint16_t mouse_button_shift = mouse_button - 1;

	// Only send mouse button if in press or hold state
	if ( stateType == 0x00 && state == 0x03 ) // Release state
	{
		// Release
		if ( mouse_button )
			USBMouse_Buttons &= ~(1 << mouse_button_shift);
	}
	else
	{
		// Press or hold
		if ( mouse_button )
			USBMouse_Buttons |= (1 << mouse_button_shift);

		if ( mouse_x )
			USBMouse_Relative_x = mouse_x;
		if ( mouse_y )
			USBMouse_Relative_y = mouse_y;
	}

	// Trigger updates
	if ( mouse_button )
		USBMouse_Changed |= USBMouseChangeState_Buttons;

	if ( mouse_x || mouse_y )
		USBMouse_Changed |= USBMouseChangeState_Relative;
}
#endif



// ----- Functions -----

// Flush Key buffers
void USB_flushBuffers()
{
	// Zero out USBKeys buffers
	memset( (void*)&USBKeys_primary, 0, sizeof( USBKeys ) );
	memset( (void*)&USBKeys_idle, 0, sizeof( USBKeys ) );

	// Reset USBKeys_Keys size
	USBKeys_Sent = 0;

	// Set USBKeys_LEDs_Changed to indicate that we should update LED status
	USBKeys_LEDs_Changed = 1;
}


// USB Module Setup
inline void USB_setup()
{
	// Initialize the USB
	// If a USB connection does not exist, just ignore it
	// All usb related functions will non-fatally fail if called
	// If the USB initialization is delayed, then functionality will just be delayed
	usb_init();

	// Register USB Output CLI dictionary
	CLI_registerDictionary( usbCLIDict, usbCLIDictName );

	// USB Protocol Transition variable
	USBKeys_Protocol_Change = 0;

	// Flush key buffers
	USB_flushBuffers();

	// Check if we need to disable secure bootloader mode
	// This is done by setting both 32 bit Kiibohd specific VBAT secure register regions
#if ( defined(_kii_v1_) || defined(_kii_v2_) ) && SecureBootloader_define == 0
	VBAT_SECURE1 = 0;
	VBAT_SECURE2 = 0;
#endif

#if enableRawIO_define == 1
	// Setup HID-IO
	HIDIO_setup();
#endif

	// Latency resource allocation
	outputPeriodicLatencyResource = Latency_add_resource("USBOutputPeri", LatencyOption_Ticks);
	outputPollLatencyResource = Latency_add_resource("USBOutputPoll", LatencyOption_Ticks);
}


// USB Data Poll
inline void USB_poll()
{
	// Start latency measurement
	Latency_start_time( outputPollLatencyResource );

	// USB status checks
	// Non-standard USB state manipulation, usually does nothing
	usb_device_check();

	// Re-send last usb keyboard state if we've passed the expiry time
	// And the HID IDLE is set
	usb_keyboard_idle_update();

#if enableRawIO_define == 1
	// HID-IO Process
	HIDIO_process();
#endif

	// End latency measurement
	Latency_end_time( outputPollLatencyResource );
}


// USB Data Periodic
inline void USB_periodic()
{
	// Start latency measurement
	Latency_start_time( outputPeriodicLatencyResource );

#if enableMouse_define == 1
	// Process mouse actions
	while ( USBMouse_Changed )
		usb_mouse_send();
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

	// End latency measurement
	Latency_end_time( outputPeriodicLatencyResource );
}


// Packet buffer debug for System Control codes
void USB_SysCtrlDebug( USBKeys *buffer )
{
	print("\033[1;34mSysCtrl\033[0m[");
	printHex_op( buffer->sys_ctrl, 2 );
	print( "] " NL );
}


// Packet buffer debug for Consumer Control codes
void USB_ConsCtrlDebug( USBKeys *buffer )
{
	print("\033[1;34mConsCtrl\033[0m[");
	printHex_op( buffer->cons_ctrl, 2 );
	print( "] " NL );
}


// Packet buffer debug for 6kro/boot mode USB Keyboard codes
void USB_6KRODebug( USBKeys *buffer )
{
	print("\033[1;34m6KRO\033[0m ");
	printHex_op( buffer->modifiers, 2 );
	print(" ");
	printHex( 0 );
	print(" ");
	printHex_op( buffer->keys[0], 2 );
	printHex_op( buffer->keys[1], 2 );
	printHex_op( buffer->keys[2], 2 );
	printHex_op( buffer->keys[3], 2 );
	printHex_op( buffer->keys[4], 2 );
	printHex_op( buffer->keys[5], 2 );
	print( NL );
}


// Packet buffer debug for nkro USB Keyboard codes
void USB_NKRODebug( USBKeys *buffer )
{
	print("\033[1;34mNKRO\033[0m ");
	printHex_op( buffer->modifiers, 2 );
	print(" ");
	for ( uint8_t c = 0; c < 6; c++ )
		printHex_op( buffer->keys[ c ], 2 );
	print(" ");
	for ( uint8_t c = 6; c < 20; c++ )
		printHex_op( buffer->keys[ c ], 2 );
	print(" ");
	printHex_op( buffer->keys[20], 2 );
	print(" ");
	for ( uint8_t c = 21; c < 27; c++ )
		printHex_op( buffer->keys[ c ], 2 );
	print( NL );
}


// Sets the device into firmware reload mode
inline void USB_firmwareReload()
{
	usb_device_reload();
}


// Soft Chip Reset
inline void USB_softReset()
{
	usb_device_software_reset();
}


// USB Input buffer available
inline unsigned int USB_availablechar()
{
#if enableVirtualSerialPort_define == 1
	return usb_serial_available();
#else
	return 0;
#endif
}


// USB Get Character from input buffer
inline int USB_getchar()
{
#if enableVirtualSerialPort_define == 1
	// XXX Make sure to check output_availablechar() first! Information is lost with the cast (error codes) (AVR)
	return (int)usb_serial_getchar();
#else
	return 0;
#endif
}


// USB Send Character to output buffer
inline int USB_putchar( char c )
{
#if enableVirtualSerialPort_define == 1
	return usb_serial_putchar( c );
#else
	return 0;
#endif
}


// USB Send String to output buffer, null terminated
inline int USB_putstr( char* str )
{
#if enableVirtualSerialPort_define == 1
#if defined(_avr_at_) // AVR
	uint16_t count = 0;
#else
	uint32_t count = 0;
#endif
	// Count characters until NULL character, then send the amount counted
	while ( str[count] != '\0' )
		count++;

	return usb_serial_write( str, count );
#else
	return 0;
#endif
}


// USB RawIO buffer available
unsigned int USB_rawio_availablechar()
{
#if enableRawIO_define == 1
	return usb_rawio_available();
#else
	return 0;
#endif
}


// USB RawIO get buffer
// XXX Must be a 64 byte buffer
int USB_rawio_getbuffer( char* buffer )
{
#if enableRawIO_define == 1
	// No timeout, fail immediately
	return usb_rawio_rx( (void*)buffer, 0 );
#else
	return 0;
#endif
}


// USB RawIO send buffer
// XXX Must be a 64 byte buffer
int USB_rawio_sendbuffer( char* buffer )
{
#if enableRawIO_define == 1
	// No timeout, fail immediately
	return usb_rawio_tx( (void*)buffer, 0 );
#else
	return 0;
#endif
}



// ----- CLI Command Functions -----

void cliFunc_idle( char* args )
{
	print( NL );

	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set Idle count
	if ( arg1Ptr[0] != '\0' )
	{
		uint8_t idle = (uint8_t)numToInt( arg1Ptr );
		USBKeys_Idle_Config = idle;
	}

	// Show Idle count
	info_msg("USB Idle Config: ");
	printInt16( 4 * USBKeys_Idle_Config );
	print(" ms - ");
	printInt8( USBKeys_Idle_Config );
}


void cliFunc_kbdProtocol( char* args )
{
	print( NL );

	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	if ( arg1Ptr[0] != '\0' )
	{
		uint8_t mode = (uint8_t)numToInt( arg1Ptr );

		// Do nothing if the argument was wrong
		if ( mode == 0 || mode == 1 )
		{
			USBKeys_Protocol_New = mode;
			USBKeys_Protocol_Change = 1;
			info_msg("Setting Keyboard Protocol to: ");
			printInt8( USBKeys_Protocol );
		}
	}
	else
	{
		info_msg("Keyboard Protocol: ");
		printInt8( USBKeys_Protocol );
	}
}


void cliFunc_readLEDs( char* args )
{
	print( NL );
	info_msg("LED State: ");
	printInt8( USBKeys_LEDs );
}


void cliFunc_usbInitTime( char* args )
{
	// Calculate overall USB initialization time
	// XXX A protocol analyzer will be more accurate, however, this is built-in and easier to collect data
	print(NL);
	info_msg("USB Init Time: ");
	printInt32( USBInit_TimeEnd - USBInit_TimeStart );
	print(" ms - ");
	printInt16( USBInit_Ticks );
	print(" ticks");
}

