/* Copyright (C) 2011-2017 by Jacob Alexander
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
#include <led.h>
#include <print.h>
#include <scan_loop.h>

// KLL
#include <kll_defs.h>
#include <kll.h>

// Local Includes
#include "output_com.h"



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

void cliFunc_kbdProtocol( char* args );
void cliFunc_outputDebug( char* args );
void cliFunc_readLEDs   ( char* args );
void cliFunc_sendKeys   ( char* args );
void cliFunc_setKeys    ( char* args );
void cliFunc_setMod     ( char* args );
void cliFunc_usbInitTime( char* args );



// ----- Variables -----

// Output Module command dictionary
CLIDict_Entry( kbdProtocol, "Keyboard Protocol Mode: 0 - Boot, 1 - OS/NKRO Mode" );
CLIDict_Entry( outputDebug, "Toggle Output Debug mode." );
CLIDict_Entry( readLEDs,    "Read LED byte:" NL "\t\t1 NumLck, 2 CapsLck, 4 ScrlLck, 16 Kana, etc." );
CLIDict_Entry( sendKeys,    "Send the prepared list of USB codes and modifier byte." );
CLIDict_Entry( setKeys,     "Prepare a space separated list of USB codes (decimal). Waits until \033[35msendKeys\033[0m." );
CLIDict_Entry( setMod,      "Set the modfier byte:" NL "\t\t1 LCtrl, 2 LShft, 4 LAlt, 8 LGUI, 16 RCtrl, 32 RShft, 64 RAlt, 128 RGUI" );
CLIDict_Entry( usbInitTime, "Displays the time in ms from usb_init() till the last setup call." );

CLIDict_Def( outputCLIDict, "USB Module Commands" ) = {
	CLIDict_Item( kbdProtocol ),
	CLIDict_Item( outputDebug ),
	CLIDict_Item( readLEDs ),
	CLIDict_Item( sendKeys ),
	CLIDict_Item( setKeys ),
	CLIDict_Item( setMod ),
	CLIDict_Item( usbInitTime ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Which modifier keys are currently pressed
// 1=left ctrl,    2=left shift,   4=left alt,    8=left gui
// 16=right ctrl, 32=right shift, 64=right alt, 128=right gui
uint8_t  USBKeys_Modifiers    = 0;
uint8_t  USBKeys_ModifiersCLI = 0; // Separate CLI send buffer

// Currently pressed keys, max is defined by USB_MAX_KEY_SEND
uint8_t  USBKeys_Keys   [USB_NKRO_BITFIELD_SIZE_KEYS];
uint8_t  USBKeys_KeysCLI[USB_NKRO_BITFIELD_SIZE_KEYS]; // Separate CLI send buffer
uint8_t  USBKeys_BitfieldSize = USB_NKRO_BITFIELD_SIZE_KEYS;

// System Control and Consumer Control 1KRO containers
uint8_t  USBKeys_SysCtrl;
uint16_t USBKeys_ConsCtrl;

// The number of keys sent to the usb in the array
uint8_t  USBKeys_Sent    = 0;
uint8_t  USBKeys_SentCLI = 0;

// 1=num lock, 2=caps lock, 4=scroll lock, 8=compose, 16=kana
volatile uint8_t  USBKeys_LEDs = 0;

// Currently pressed mouse buttons, bitmask, 0 represents no buttons pressed
volatile uint16_t USBMouse_Buttons = 0;

// Relative mouse axis movement, stores pending movement
volatile uint16_t USBMouse_Relative_x = 0;
volatile uint16_t USBMouse_Relative_y = 0;

// Protocol setting from the host.
// 0 - Boot Mode
// 1 - NKRO Mode (Default, unless set by a BIOS or boot interface)
volatile uint8_t  USBKeys_Protocol = USBProtocol_define;

// Indicate if USB should send update
// OS only needs update if there has been a change in state
volatile USBKeyChangeState USBKeys_Changed = USBKeyChangeState_None;

// Indicate if USB should send update
volatile USBMouseChangeState USBMouse_Changed = USBMouseChangeState_None;

// the idle configuration, how often we send the report to the
// host (ms * 4) even when it hasn't changed
// 0 - Disables
uint8_t  USBKeys_Idle_Config = 0;

// Count until idle timeout
uint32_t USBKeys_Idle_Expiry = 0;
uint8_t  USBKeys_Idle_Count = 0;

// Indicates whether the Output module is fully functional
// 0 - Not fully functional, 1 - Fully functional
// 0 is often used to show that a USB cable is not plugged in (but has power)
volatile uint8_t  Output_Available = 0;

// Debug control variable for Output modules
// 0 - Debug disabled (default)
// 1 - Debug enabled
uint8_t  Output_DebugMode = 0;

// mA - Set by outside module if not using USB (i.e. Interconnect)
// Generally set to 100 mA (low power) or 500 mA (high power)
uint16_t Output_ExtCurrent_Available = 0;

// mA - Set by USB module (if exists)
// Initially 100 mA, but may be negotiated higher (e.g. 500 mA)
uint16_t Output_USBCurrent_Available = 0;

// USB Init Time (ms) - usb_init()
volatile uint32_t USBInit_TimeStart;
volatile uint32_t USBInit_TimeEnd;
volatile uint16_t USBInit_Ticks;

// Callback function to host
// Output_Host_Callback( char* command, char* args ) return int
void *Output_Host_Callback;



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
	Output_flushBuffers();

	// Set the keyboard protocol to Boot Mode
	USBKeys_Protocol = 0;
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
	Output_flushBuffers();

	// Set the keyboard protocol to NKRO Mode
	USBKeys_Protocol = 1;
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
		Output_flushBuffers();

		// Toggle the keyboard protocol Mode
		USBKeys_Protocol = !USBKeys_Protocol;
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
		USBKeys_Changed |= USBKeyChangeState_Consumer;

	// Only send keypresses if press or hold state
	if ( stateType == 0x00 && state == 0x03 ) // Release state
	{
		USBKeys_ConsCtrl = 0;
		return;
	}

	// Set consumer control code
	USBKeys_ConsCtrl = *(uint16_t*)(&args[0]);
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
		USBKeys_Changed |= USBKeyChangeState_System;

	// Only send keypresses if press or hold state
	if ( stateType == 0x00 && state == 0x03 ) // Release state
	{
		USBKeys_SysCtrl = 0;
		return;
	}

	// Set system control code
	USBKeys_SysCtrl = args[0];
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
	uint8_t keyPress = 0; // Default to key release, only used for NKRO
	switch ( USBKeys_Protocol )
	{
	case 0: // Boot Mode
		// TODO Analog inputs
		// Only indicate USB has changed if either a press or release has occured
		if ( state == 0x01 || state == 0x03 )
			USBKeys_Changed = USBKeyChangeState_MainKeys;

		// Only send keypresses if press or hold state
		if ( stateType == 0x00 && state == 0x03 ) // Release state
			return;
		break;
	case 1: // NKRO Mode
		// Only send press and release events
		if ( stateType == 0x00 && state == 0x02 ) // Hold state
			return;

		// Determine if setting or unsetting the bitfield (press == set)
		if ( stateType == 0x00 && state == 0x01 ) // Press state
			keyPress = 1;
		break;
	}

	// Get the keycode from arguments
	uint8_t key = args[0];

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
			USBKeys_Modifiers |= 1 << (key ^ 0xE0); // Left shift 1 by key XOR 0xE0
		}
		// Normal USB Code
		else
		{
			// USB Key limit reached
			if ( USBKeys_Sent >= USB_BOOT_MAX_KEYS )
			{
				warn_print("USB Key limit reached");
				return;
			}

			// Make sure key is within the USB HID range
			if ( key <= 104 )
			{
				USBKeys_Keys[USBKeys_Sent++] = key;
			}
			// Invalid key
			else
			{
				warn_msg("USB Code above 104/0x68 in Boot Mode: ");
				printHex( key );
				print( NL );
			}
		}
		break;

	case 1: // NKRO Mode
		// Set the modifier bit if this key is a modifier
		if ( (key & 0xE0) == 0xE0 ) // AND with 0xE0 (Left Ctrl, first modifier)
		{
			if ( keyPress )
			{
				USBKeys_Modifiers |= 1 << (key ^ 0xE0); // Left shift 1 by key XOR 0xE0
			}
			else // Release
			{
				USBKeys_Modifiers &= ~(1 << (key ^ 0xE0)); // Left shift 1 by key XOR 0xE0
			}

			USBKeys_Changed |= USBKeyChangeState_Modifiers;
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

			USBKeys_Changed |= USBKeyChangeState_MainKeys;
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

			USBKeys_Changed |= USBKeyChangeState_SecondaryKeys;
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

			USBKeys_Changed |= USBKeyChangeState_TertiaryKeys;
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

			USBKeys_Changed |= USBKeyChangeState_QuartiaryKeys;
		}
		// Received 0x00
		// This is a special USB Code that internally indicates a "break"
		// It is used to send "nothing" in order to break up sequences of USB Codes
		else if ( key == 0x00 )
		{
			USBKeys_Changed |= USBKeyChangeState_MainKeys;

			// Also flush out buffers just in case
			Output_flushBuffers();
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
			USBKeys_Keys[bytePosition] |= (1 << byteShift);
			USBKeys_Sent++;
		}
		else // Release
		{
			USBKeys_Keys[bytePosition] &= ~(1 << byteShift);
			USBKeys_Sent++;
		}

		break;
	}
#endif
}

void Output_flashMode_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_flashMode()");
		return;
	}

	// Start flash mode
	Output_firmwareReload();
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

// Convenient wrapper for host callback
int Output_callback( char* command, char* args )
{
	int (*callback)(char* command, char* args) = (int(*)(char*, char*))(Output_Host_Callback);

	return callback( command, args );
}

// Flush Key buffers
void Output_flushBuffers()
{
	// Zero out USBKeys_Keys array
	for ( uint8_t c = 0; c < USB_NKRO_BITFIELD_SIZE_KEYS; c++ )
		USBKeys_Keys[ c ] = 0;

	// Zero out other key buffers
	USBKeys_ConsCtrl = 0;
	USBKeys_Modifiers = 0;
	USBKeys_SysCtrl = 0;
}


// USB Module Setup
inline void Output_setup()
{
	// Register USB Output CLI dictionary
	CLI_registerDictionary( outputCLIDict, outputCLIDictName );

	// Flush key buffers
	Output_flushBuffers();

#if enableRawIO_define == 1
	// Setup HID-IO
	HIDIO_setup();
#endif
}


// USB Data Send
inline void Output_send()
{
#if enableRawIO_define == 1
	// HID-IO Process
	HIDIO_process();
#endif

#if enableMouse_define == 1
	// Process mouse actions
	while ( USBMouse_Changed )
		Output_callback( "mouse_send", "" );
#endif

#if enableKeyboard_define == 1
	// Boot Mode Only, unset stale keys
	if ( USBKeys_Protocol == 0 )
		for ( uint8_t c = USBKeys_Sent; c < USB_BOOT_MAX_KEYS; c++ )
			USBKeys_Keys[c] = 0;

	// Send keypresses while there are pending changes
	while ( USBKeys_Changed )
		Output_callback( "keyboard_send", "" );

	// Clear keys sent
	USBKeys_Sent = 0;

	// Signal Scan Module we are finished
	switch ( USBKeys_Protocol )
	{
	case 0: // Boot Mode
		// Clear modifiers only in boot mode
		USBKeys_Modifiers = 0;
		Scan_finishedWithOutput( USBKeys_Sent <= USB_BOOT_MAX_KEYS ? USBKeys_Sent : USB_BOOT_MAX_KEYS );
		break;
	case 1: // NKRO Mode
		Scan_finishedWithOutput( USBKeys_Sent );
		break;
	}
#endif
}


// Sets the device into firmware reload mode
inline void Output_firmwareReload()
{
	Output_callback( "device_reload", "" );
}


// USB Input buffer available
inline unsigned int Output_availablechar()
{
#if enableVirtualSerialPort_define == 1
	return (unsigned int)Output_callback( "serial_available", "" );
#else
	return 0;
#endif
}


// USB Get Character from input buffer
inline int Output_getchar()
{
#if enableVirtualSerialPort_define == 1
	return Output_callback( "serial_read", "" );
#else
	return 0;
#endif
}


// USB Send Character to output buffer
inline int Output_putchar( char c )
{
#if enableVirtualSerialPort_define == 1
	char out[2] = { c, '\0' };
	return Output_callback( "serial_write", out );
#else
	return 0;
#endif
}


// USB Send String to output buffer, null terminated
inline int Output_putstr( char* str )
{
#if enableVirtualSerialPort_define == 1
	return Output_callback( "serial_write", str );
#else
	return 0;
#endif
}


// Soft Chip Reset
inline void Output_softReset()
{
	Output_callback( "restart", "" );
}


// USB RawIO buffer available
inline unsigned int Output_rawio_availablechar()
{
#if enableRawIO_define == 1
	return (unsigned int)Output_callback( "rawio_available", "" );
#else
	return 0;
#endif
}


// USB RawIO get buffer
// XXX Must be a 64 byte buffer
inline int Output_rawio_getbuffer( char* buffer )
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
inline int Output_rawio_sendbuffer( char* buffer )
{
#if enableRawIO_define == 1
	return Output_callback( "rawio_tx", buffer );
#else
	return 0;
#endif
}


// Update USB current (mA)
// Triggers power change event
void Output_update_usb_current( unsigned int current )
{
	// Only signal if changed
	if ( current == Output_USBCurrent_Available )
		return;

	// Update USB current
	Output_USBCurrent_Available = current;

	/* XXX Affects sleep states due to USB messages
	unsigned int total_current = Output_current_available();
	info_msg("USB Available Current Changed. Total Available: ");
	printInt32( total_current );
	print(" mA" NL);
	*/

	// Send new total current to the Scan Modules
	Scan_currentChange( Output_current_available() );
}


// Update external current (mA)
// Triggers power change event
void Output_update_external_current( unsigned int current )
{
	// Only signal if changed
	if ( current == Output_ExtCurrent_Available )
		return;

	// Update external current
	Output_ExtCurrent_Available = current;

	unsigned int total_current = Output_current_available();
	info_msg("External Available Current Changed. Total Available: ");
	printInt32( total_current );
	print(" mA" NL);

	// Send new total current to the Scan Modules
	Scan_currentChange( Output_current_available() );
}


// Power/Current Available
unsigned int Output_current_available()
{
	unsigned int total_current = 0;

	// Check for USB current source
	total_current += Output_USBCurrent_Available;

	// Check for external current source
	total_current += Output_ExtCurrent_Available;

	// XXX If the total available current is still 0
	// Set to 100 mA, which is generally a safe assumption at startup
	// before we've been able to determine actual available current
	if ( total_current == 0 )
	{
		total_current = 100;
	}

	return total_current;
}



// ----- CLI Command Functions -----

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
			USBKeys_Protocol = mode;
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


void cliFunc_outputDebug( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Default to 1 if no argument is given
	Output_DebugMode = 1;

	if ( arg1Ptr[0] != '\0' )
	{
		Output_DebugMode = (uint16_t)numToInt( arg1Ptr );
	}
}


void cliFunc_readLEDs( char* args )
{
	print( NL );
	info_msg("LED State: ");
	printInt8( USBKeys_LEDs );
}


void cliFunc_sendKeys( char* args )
{
	// Copy USBKeys_KeysCLI to USBKeys_Keys
	for ( uint8_t key = 0; key < USBKeys_SentCLI; ++key )
	{
		// TODO
		//USBKeys_Keys[key] = USBKeys_KeysCLI[key];
	}
	USBKeys_Sent = USBKeys_SentCLI;

	// Set modifier byte
	USBKeys_Modifiers = USBKeys_ModifiersCLI;
}


void cliFunc_setKeys( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Parse up to USBKeys_MaxSize args (whichever is least)
	for ( USBKeys_SentCLI = 0; USBKeys_SentCLI < USB_BOOT_MAX_KEYS; ++USBKeys_SentCLI )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Add the USB code to be sent
		// TODO
		//USBKeys_KeysCLI[USBKeys_SentCLI] = numToInt( arg1Ptr );
	}
}


void cliFunc_setMod( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	USBKeys_ModifiersCLI = numToInt( arg1Ptr );
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

