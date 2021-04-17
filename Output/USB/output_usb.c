/* Copyright (C) 2011-2019 by Jacob Alexander
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



// ----- Enumerations -----

typedef enum {
	OutputReset_None       = 0, // Do nothing
	OutputReset_Restart    = 1, // Clear USB stack and restart the firmware
	OutputReset_Bootloader = 2, // Clear USB stack and jump to bootloader
} OutputReset;



// ----- Function Declarations -----

void cliFunc_idle       ( char* args );
void cliFunc_kbdProtocol( char* args );
void cliFunc_readLEDs   ( char* args );
void cliFunc_usbAddr    ( char* args );
void cliFunc_usbConf    ( char* args );
void cliFunc_usbInitTime( char* args );
void cliFunc_usbErrors  ( char* args );



// ----- Variables -----

// Output Module command dictionary
CLIDict_Entry( idle,        "Show/set the HID Idle time (multiples of 4 ms)." );
CLIDict_Entry( kbdProtocol, "Keyboard Protocol Mode: 0 - Boot, 1 - OS/NKRO Mode." );
CLIDict_Entry( readLEDs,    "Read LED byte:" NL "\t\t1 NumLck, 2 CapsLck, 4 ScrlLck, 16 Kana, etc." );
CLIDict_Entry( usbAddr,     "Shows the negotiated USB unique Id, given to device by host." );
CLIDict_Entry( usbConf,     "Shows whether USB is configured or not." );
CLIDict_Entry( usbInitTime, "Displays the time in ms from usb_init() till the last setup call." );
CLIDict_Entry( usbErrors,   "Displays number of usb errors since startup." );

CLIDict_Def( usbCLIDict, "USB Module Commands" ) = {
	CLIDict_Item( idle ),
	CLIDict_Item( kbdProtocol ),
	CLIDict_Item( readLEDs ),
	CLIDict_Item( usbAddr ),
	CLIDict_Item( usbConf ),
	CLIDict_Item( usbInitTime ),
	CLIDict_Item( usbErrors ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// USBKeys Keyboard Buffer
volatile USBKeys USBKeys_primary; // Primary send buffer
volatile USBKeys USBKeys_idle;    // Idle timeout send buffer

// The number of keys sent to the usb in the array
volatile uint8_t  USBKeys_Sent;

// 1=num lock, 2=caps lock, 4=scroll lock, 8=compose, 16=kana
volatile uint8_t  USBKeys_LEDs;
volatile uint8_t  USBKeys_LEDs_prev;

// USBMouse Buffer
volatile USBMouse USBMouse_primary; // Primary mouse send buffer

// Protocol setting from the host.
// 0 - Boot Mode
// 1 - NKRO Mode (Default, unless set by a BIOS or boot interface)
volatile uint8_t  USBKeys_Protocol = USBProtocol_define;
volatile uint8_t  USBKeys_Protocol_New = USBProtocol_define;
volatile uint8_t  USBKeys_Protocol_Change; // New value to set to USBKeys_Protocol if _Change is set

// the idle configuration, how often we send the report to the
// host (ms * 4) even when it hasn't changed
// 0 - Disables
volatile uint8_t  USBKeys_Idle_Config = USBIdle_define;

// Count until idle timeout
volatile uint32_t USBKeys_Idle_Expiry;
volatile uint8_t  USBKeys_Idle_Count;

// USB Init Time (ms) - usb_init()
volatile uint32_t USBInit_TimeStart;
volatile uint32_t USBInit_TimeEnd;
volatile uint16_t USBInit_Ticks;

// USB Address - Set by host, unique to the bus
volatile uint8_t USBDev_Address;

// USB Errors
volatile uint32_t USBStatus_FrameErrors;

// Scheduled USB resets, used to clear USB packets before bringing down the USB stack
// This is useful for OSs like Windows where then OS doesn't clear the current state
// after the keyboard is disconnected (i.e. Ctrl keeps being held until Ctrl is pressed again).
volatile static uint8_t Output_reset_schedule;

// Latency measurement resource
static uint8_t outputPeriodicLatencyResource;
static uint8_t outputPollLatencyResource;



// ----- Capabilities -----

// Set Boot Keyboard Protocol
void Output_kbdProtocolBoot_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_kbdProtocolBoot()");
		return;
	default:
		return;
	}

	// Only set if necessary
	if ( USBKeys_Protocol == 0 )
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
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_kbdProtocolNKRO()");
		return;
	default:
		return;
	}

	// Only set if necessary
	if ( USBKeys_Protocol == 1 )
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
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Last:
		// Only use capability on release
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_toggleKbdProtocol()");
		return;
	default:
		return;
	}

	// Flush the key buffers
	USB_flushBuffers();

	// Toggle the keyboard protocol Mode
	USBKeys_Protocol_New = !USBKeys_Protocol;
	USBKeys_Protocol_Change = 1;
#endif
}


// Sends a Consumer Control code to the USB Output buffer
void Output_consCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Indicate changed
		USBKeys_primary.changed |= USBKeyChangeState_Consumer;
		break;
	case CapabilityState_Any:
		// Only set consumer code
		break;
	case CapabilityState_Last:
		// Clear consumer code
		USBKeys_primary.changed |= USBKeyChangeState_Consumer;
		USBKeys_primary.cons_ctrl = 0;
		return;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_consCtrlSend(consCode)");

		// Read arg if not set to 0
		if ( args != 0 )
		{
			uint16_t key = *(uint16_t*)(&args[0]);
			print(" -> ");
			printInt16( key );
		}
		return;
	default:
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
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Debug:
		// Display capability name
		print("Output_noneSend()");
		return;
	default:
		return;
	}

	// Nothing to do, because that's the point :P
}


// Sends a System Control code to the USB Output buffer
void Output_sysCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
#if enableKeyboard_define == 1
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Indicate changed
		USBKeys_primary.changed |= USBKeyChangeState_System;
		break;
	case CapabilityState_Any:
		// Only set consumer code
		break;
	case CapabilityState_Last:
		// Clear system code
		USBKeys_primary.changed |= USBKeyChangeState_System;
		USBKeys_primary.sys_ctrl = 0;
		return;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_sysCtrlSend(sysCode)");

		// Read arg if not set to 0
		if ( args != 0 )
		{
			uint8_t key = args[0];
			print(" -> ");
			printInt8( key );
		}
		return;
	default:
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
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	// Depending on which mode the keyboard is in the USB needs Press/Hold/Release events
	uint8_t keyPress = 0; // Default to key release

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Indicate changed
		keyPress = 1;
		break;
	case CapabilityState_Last:
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_usbCodeSend(usbCode)");

		// Read arg if not set to 0
		if ( args != 0 )
		{
			uint8_t key = args[0];
			print(" -> ");
			printInt8( key );
		}
		return;
	default:
		return;
	}

	// Get the keycode from arguments
	uint8_t key = args[0];

	// Extra USB Debug
	if ( Output_DebugMode > 1 )
	{
		print("\033[1;34mUSB\033[0m ");
		printInt8( key );
		print(" ");
		switch ( cstate )
		{
		case CapabilityState_Initial:
			print("\033[1;33mP\033[0m");
			break;
		case CapabilityState_Last:
			print("\033[1;35mR\033[0m");
			break;
		default:
			break;
		}
		print( NL );
	}

	// Depending on which mode the keyboard is in, USBKeys_Keys array is used differently
	// Boot mode - Maximum of 6 byte codes
	// NKRO mode - Each bit of the 26 byte corresponds to a key
	//  Bits   0 -   3                 unused
	//  Bits   4 - 164 (bytes  0 - 20) correspond to USB Codes   4 - 164 (Keyboard Section)
	//  Bits 165 - 175                 unused
	//  Bits 176 - 221 (bytes 22 - 27) correspond to USB Codes 176 - 221 (Keypad Section)
	//  Bits 222 - 223                 unused
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
					USBKeys_primary.changed = USBKeyChangeState_Keys;
					break;
				}
			}

			// USB Key limit reached
			if ( USBKeys_Sent >= USB_BOOT_MAX_KEYS )
			{
				warn_printNL("USB Key limit reached");
				break;
			}

			// Add key if not already found in the buffer
			if ( keyPress && !keyFound )
			{
				USBKeys_primary.keys[USBKeys_Sent++] = key;
				USBKeys_primary.changed = USBKeyChangeState_Keys;
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
		// Handle Keyboard and Keypad Sections
		else if ( key >= 1 && key <= 221 )
		{
			// Lookup (otherwise division or multiple checks are needed to do alignment)
			// Starting at 0th position, each byte has 8 bits
			switch ( key )
			{
				// Keyboard Section
				byteLookup( 0 );
				byteLookup( 1 );
				byteLookup( 2 );
				byteLookup( 3 );
				byteLookup( 4 );
				byteLookup( 5 );
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
				byteLookup( 20 );

				// Padding
				// XXX (HaaTa) Not necessary to include
				//byteLookup( 21 );

				// Keypad Section
				byteLookup( 22 );
				byteLookup( 23 );
				byteLookup( 24 );
				byteLookup( 25 );
				byteLookup( 26 );
				byteLookup( 27 );
			}

			USBKeys_primary.changed |= USBKeyChangeState_Keys;
		}
		// Received 0x00
		// This is a special USB Code that internally indicates a "break"
		// It is used to send "nothing" in order to break up sequences of USB Codes
		else if ( key == 0x00 )
		{
			USBKeys_primary.changed |= USBKeyChangeState_Keys;
			break;
		}
		// Invalid key
		else
		{
			warn_print("USB Code not within 4-155 (0x4-0x9B), 157-164 (0x9D-0xA4), 176-221 (0xB0-0xDD) or 224-231 (0xE0-0xE7) NKRO Mode: ");
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


void Output_usbCodeRelease_capability(TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args)
{
#if enableKeyboard_define == 1 && disable_usbCodeRelease_define == 0
	CapabilityState cstate = KLL_CapabilityState(state, stateType);

	// Release, only on initial trigger
	switch (cstate)
	{
	case CapabilityState_Initial:
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_usbCodeRelease(usbCode)");

		// Read arg if not set to 0
		if (args != 0)
		{
			uint8_t key = args[0];
			print(" -> ");
			printInt8(key);
		}
		return;
	case CapabilityState_Last:
	default:
		return;
	}

	// Force release
	state = ScheduleType_R;

	// Use capability to release key
	Output_usbCodeSend_capability(trigger, state, stateType, args);
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
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	// Determine which mouse button was sent
	// The USB spec defines up to a max of 0xFFFF buttons
	// The usual are:
	// 1 - Button 1 - (Primary)
	// 2 - Button 2 - (Secondary)
	// 3 - Button 3 - (Tertiary)
	uint16_t mouse_button = *(uint16_t*)(&args[0]);

	// X/Y Relative Axis
	int16_t mouse_x = *(int16_t*)(&args[2]);
	int16_t mouse_y = *(int16_t*)(&args[4]);

	// Adjust for bit shift
	uint16_t mouse_button_shift = mouse_button - 1;

	switch ( cstate )
	{
	case CapabilityState_Initial:
	case CapabilityState_Any:
		// Press/Hold
		if ( mouse_button )
		{
			USBMouse_primary.buttons |= (1 << mouse_button_shift);
		}

		if ( mouse_x )
		{
			USBMouse_primary.relative_x = mouse_x;
		}
		if ( mouse_y )
		{
			USBMouse_primary.relative_y = mouse_y;
		}
		break;
	case CapabilityState_Last:
		// Release
		if ( mouse_button )
		{
			USBMouse_primary.buttons &= ~(1 << mouse_button_shift);
		}
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_usbMouse(mouseButton,relX,relY)");
		return;
	default:
		return;
	}

	// Trigger updates
	if ( mouse_button )
	{
		USBMouse_primary.changed |= USBMouseChangeState_Buttons;
	}

	if ( mouse_x || mouse_y )
	{
		USBMouse_primary.changed |= USBMouseChangeState_Relative;
	}
}

// Sends a mouse wheel command over USB Output buffer
// XXX This function *will* be changing in the future
//     If you use it, be prepared that your .kll files will break in the future (post KLL 0.5)
// Argument #1: USB Vertical Wheel (8 bit)
// Argument #2: USB Horizontal Wheel (8 bit)
void Output_usbMouseWheel_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	// Vertical and horizontal mouse wheels
	int8_t wheel_vert = *(int8_t*)(&args[0]);
	int8_t wheel_hori = *(int8_t*)(&args[2]);

	switch ( cstate )
	{
	case CapabilityState_Initial:
	case CapabilityState_Any:
		// Press/Hold
		if ( wheel_vert )
		{
			USBMouse_primary.vertwheel = wheel_vert;
			USBMouse_primary.changed |= USBMouseChangeState_WheelVert;
		}

		if ( wheel_hori )
		{
			USBMouse_primary.horiwheel = wheel_hori;
			USBMouse_primary.changed |= USBMouseChangeState_WheelHori;
		}
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Output_usbMouseWheel(vert,hori)");
		return;
	default:
		return;
	}
}
#endif



// ----- Functions -----

// Flush Key buffers
void USB_flushBuffers()
{
	// Zero out USBKeys buffers
	memset( (void*)&USBKeys_primary, 0, sizeof( USBKeys ) );
	memset( (void*)&USBKeys_idle, 0, sizeof( USBKeys ) );

	// Clear idle timeout state
	USBKeys_Idle_Expiry = 0;
	USBKeys_Idle_Count = 0;

	// Reset USBKeys_Keys size
	USBKeys_Sent = 0;

	// Clear mouse state
	USBMouse_primary.buttons = 0;
	USBMouse_primary.relative_x = 0;
	USBMouse_primary.relative_y = 0;
	USBMouse_primary.vertwheel = 0;
	USBMouse_primary.horiwheel = 0;
	USBMouse_primary.changed = 0;

	// Make sure everything actually flushes
	USBKeys_primary.changed = 1;
	USBKeys_idle.changed = 1;
	USBMouse_primary.changed = 1;
}


// USB Module Setup
inline void USB_setup()
{
	// Reset frame error counter
	USBStatus_FrameErrors = 0;

	// Initialize the USB
	// If a USB connection does not exist, just ignore it
	// All usb related functions will non-fatally fail if called
	// If the USB initialization is delayed, then functionality will just be delayed
	usb_init();

	// Register USB Output CLI dictionary
	CLI_registerDictionary( usbCLIDict, usbCLIDictName );

	// USB Protocol Transition variable
	USBKeys_Protocol_Change = 0;

	// Clear USB LEDs (may be set by the OS rather quickly)
	USBKeys_LEDs_prev = 0;
	USBKeys_LEDs = 0;

	// Clear USB address
	USBDev_Address = 0;

	// Clear USB reset state
	Output_reset_schedule = OutputReset_None;

	// Flush key buffers
	USB_flushBuffers();

	// Check if we need to disable secure bootloader mode
	// This is done by setting both 32 bit Kiibohd specific VBAT secure register regions
#if SecureBootloader_define == 0
#if ( defined(_kii_v1_) || defined(_kii_v2_) )
	VBAT_SECURE1 = 0;
	VBAT_SECURE2 = 0;
#elif defined(_kii_v3_)
	GPBR_SECURE1 = 0;
	GPBR_SECURE2 = 0;
#endif
#endif

#if enableRawIO_define == 1
	// Setup HID-IO
	//HIDIO_setup();
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
	//HIDIO_process();
#endif

	// End latency measurement
	Latency_end_time( outputPollLatencyResource );
}


// Check if USB is ready
// Returns 1 if ready, 0 if not
uint8_t USB_ready()
{
#if !defined(_host_)
	return usb_configured();
#else
	return 1;
#endif
}


// Gather USB HID LED states
// Keeps track of previous state, and sends new state to PartialMap
void USB_indicator_update()
{
	// Check each bit of the indicator byte
	for ( uint8_t bit = 0; bit < LED_KANA_5; bit++ )
	{
		uint8_t id = bit + 1; // Conversion to USB HID Indicator code

		uint8_t cur = USBKeys_LEDs & (1 << bit);
		uint8_t prev = USBKeys_LEDs_prev & (1 << bit);

		// Detect if off
		if ( cur == 0 && cur == prev )
		{
			continue;
		}
		// Detect if on
		else if ( cur && cur == prev )
		{
			// On
			Macro_ledState( id, ScheduleType_On );
		}
		// Detect if press
		else if ( cur )
		{
			// TODO (HaaTa): Temporary Lock led control
#if Scan_KiraKeyboard_define == 1 && !defined(_host_)
			switch ( id )
			{
			case LED_NUM_LOCK_1:
				Scan_numlock(cur);
				break;
			case LED_CAPS_LOCK_2:
				Scan_capslock(cur);
				break;
			case LED_SCROLL_LOCK_3:
				Scan_scrolllock(cur);
				break;
			default:
				break;
			}
#endif
			// Activate
			Macro_ledState( id, ScheduleType_A );
		}
		// Detect if release
		else if ( prev )
		{
			// TODO (HaaTa): Temporary Lock led control
#if Scan_KiraKeyboard_define == 1 && !defined(_host_)
			switch ( id )
			{
			case LED_NUM_LOCK_1:
				Scan_numlock(cur);
				break;
			case LED_CAPS_LOCK_2:
				Scan_capslock(cur);
				break;
			case LED_SCROLL_LOCK_3:
				Scan_scrolllock(cur);
				break;
			default:
				break;
			}
#endif
			// Deactivate
			Macro_ledState( id, ScheduleType_D );
		}

	}

	// Update for next state comparison
	USBKeys_LEDs_prev = USBKeys_LEDs;
}


// Gather USB Suspend/Sleep status
// Send events accordingly to PartialMap depending on status
void USB_suspend_status_update()
{
	// TODO
	// usb_suspended() <- 1 if suspended
}


// USB Data Periodic
inline void USB_periodic()
{
	// Start latency measurement
	Latency_start_time( outputPeriodicLatencyResource );

	// Check to see if we need to reset the USB buffers
	switch ( Output_reset_schedule )
	{
	case OutputReset_Restart:
	case OutputReset_Bootloader:
		USB_flushBuffers();
		break;
	}

#if enableMouse_define == 1
	// Process mouse actions
	while ( USBMouse_primary.changed && USB_ready() )
	{
		usb_mouse_send();
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
	while ( USBKeys_primary.changed && USB_ready() )
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

	// Update HID LED states
	USB_indicator_update();

	// Monitor USB Suspend/Sleep State
	USB_suspend_status_update();
#endif

	// Check if a reset needs to be scheduled
	switch ( Output_reset_schedule )
	{
	case OutputReset_Restart:
		// Clear schedule
		Output_reset_schedule = OutputReset_None;

		// Restart firmware
		usb_device_software_reset();
		break;

	case OutputReset_Bootloader:
		// Clear schedule
		Output_reset_schedule = OutputReset_None;

		// Jump to bootloader
		usb_device_reload();
		break;
	}

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
	// Keyboard Section
	for ( uint8_t c = 0; c < 21; c++ )
	{
		printHex_op( buffer->keys[ c ], 2 );
	}
	print(" ");
	// Keypad Section
	for ( uint8_t c = 22; c < 28; c++ )
	{
		printHex_op( buffer->keys[ c ], 2 );
	}
	print( NL );
}


// Sets the device into firmware reload mode
inline void USB_firmwareReload()
{
	Output_reset_schedule = OutputReset_Bootloader;
}


// Soft Chip Reset
inline void USB_softReset()
{
	Output_reset_schedule = OutputReset_Restart;
}


// USB Input buffer available
inline unsigned int USB_availablechar()
{
	return 0;
}


// USB Get Character from input buffer
inline int USB_getchar()
{
	return 0;
}


// USB Send Character to output buffer
inline int USB_putchar( char c )
{
#if enableRawIO_define == 1
	if (HIDIO_VT_Connected) {
		return HIDIO_putchar( c );
	}
#endif
	return 0;
}


// USB Send String to output buffer, null terminated
inline int USB_putstr( char* str )
{
#if defined(_avr_at_) // AVR
	uint16_t count = 0;
#else
	uint32_t count = 0;
#endif
	// Count characters until NULL character, then send the amount counted
	while ( str[count] != '\0' )
		count++;

#if enableRawIO_define == 1
	if (HIDIO_VT_Connected) {
		return HIDIO_putstr( str, count );
	}
#endif

	return 0;
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
	return usb_rawio_rx( (void*)buffer, 100 );
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
	return usb_rawio_tx( (void*)buffer, 100 );
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
	info_print("USB Idle Config: ");
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
			info_print("Setting Keyboard Protocol to: ");
			printInt8( USBKeys_Protocol_New );
		}
	}
	else
	{
		info_print("Keyboard Protocol: ");
		printInt8( USBKeys_Protocol );
	}
}


void cliFunc_readLEDs( char* args )
{
	print( NL );
	info_print("LED State: ");
	printInt8( USBKeys_LEDs );
}


void cliFunc_usbAddr( char* args )
{
	print(NL);
	info_print("USB Address: ");
	printInt8( USBDev_Address );
}


void cliFunc_usbConf( char* args )
{
	print(NL);
	info_print("USB Configured: ");
#if !defined(_host_)
	printInt8( usb_configured() );
#endif
}


void cliFunc_usbInitTime( char* args )
{
	// Calculate overall USB initialization time
	// XXX A protocol analyzer will be more accurate, however, this is built-in and easier to collect data
	print(NL);
	info_print("USB Init Time: ");
	printInt32( USBInit_TimeEnd - USBInit_TimeStart );
	print(" ms - ");
	printInt16( USBInit_Ticks );
	print(" ticks");
}


void cliFunc_usbErrors( char* args )
{
	print(NL);
	info_print("USB Frame Errors: ");
	printInt32( USBStatus_FrameErrors );
}

