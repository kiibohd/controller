/* Copyright (C) 2014-2017 by Jacob Alexander
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
#include <led.h>
#include <print.h>
#include <scan_loop.h>

// USB Includes
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_)
#elif defined(_mk20dx128_) || defined(_mk20dx128vlf5_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
#include "arm/uart_serial.h"
#endif

// KLL Include
#include <kll.h>

// Local Includes
#include "output_com.h"



// ----- Function Declarations -----

void cliFunc_kbdProtocol( char* args );
void cliFunc_readLEDs   ( char* args );
void cliFunc_sendKeys   ( char* args );
void cliFunc_setKeys    ( char* args );
void cliFunc_setMod     ( char* args );



// ----- Variables -----

// Output Module command dictionary
CLIDict_Entry( kbdProtocol, "Keyboard Protocol Mode: 0 - Boot, 1 - OS/NKRO Mode" );
CLIDict_Entry( readLEDs,    "Read LED byte:" NL "\t\t1 NumLck, 2 CapsLck, 4 ScrlLck, 16 Kana, etc." );
CLIDict_Entry( sendKeys,    "Send the prepared list of USB codes and modifier byte." );
CLIDict_Entry( setKeys,     "Prepare a space separated list of USB codes (decimal). Waits until \033[35msendKeys\033[0m." );
CLIDict_Entry( setMod,      "Set the modfier byte:" NL "\t\t1 LCtrl, 2 LShft, 4 LAlt, 8 LGUI, 16 RCtrl, 32 RShft, 64 RAlt, 128 RGUI" );

CLIDict_Def( outputCLIDict, "USB Module Commands" ) = {
	CLIDict_Item( kbdProtocol ),
	CLIDict_Item( readLEDs ),
	CLIDict_Item( sendKeys ),
	CLIDict_Item( setKeys ),
	CLIDict_Item( setMod ),
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

// System Control and Consumer Control 1KRO containers
	uint8_t  USBKeys_SysCtrl;
	uint16_t USBKeys_ConsCtrl;

// The number of keys sent to the usb in the array
	uint8_t  USBKeys_Sent    = 0;
	uint8_t  USBKeys_SentCLI = 0;

// 1=num lock, 2=caps lock, 4=scroll lock, 8=compose, 16=kana
volatile uint8_t  USBKeys_LEDs = 0;

// Protocol setting from the host.
// 0 - Boot Mode
// 1 - NKRO Mode (Default, unless set by a BIOS or boot interface)
volatile uint8_t  USBKeys_Protocol = 0;

// Indicate if USB should send update
// OS only needs update if there has been a change in state
USBKeyChangeState USBKeys_Changed = USBKeyChangeState_None;

// the idle configuration, how often we send the report to the
// host (ms * 4) even when it hasn't changed
	uint8_t  USBKeys_Idle_Config = 125;

// count until idle timeout
	uint8_t  USBKeys_Idle_Count = 0;

// Indicates whether the Output module is fully functional
// 0 - Not fully functional, 1 - Fully functional
// 0 is often used to show that a USB cable is not plugged in (but has power)
volatile uint8_t  Output_Available = 0;



// ----- Capabilities -----

// Ignored capabilities
void Output_kbdProtocolBoot_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_kbdProtocolNKRO_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_toggleKbdProtocol_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_consCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_noneSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_sysCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_usbCodeSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}
void Output_usbMouse_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}

void Output_flashMode_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_flashMode(usbCode)");
		return;
	}

	// Start flash mode
	Output_firmwareReload();
}



// ----- Functions -----

// USB Module Setup
inline void Output_setup()
{
	// Setup UART
	uart_serial_setup();

	// Register USB Output CLI dictionary
	CLI_registerDictionary( outputCLIDict, outputCLIDictName );
}


// USB Data Send
inline void Output_send(void)
{
	// TODO
}


// Sets the device into firmware reload mode
inline void Output_firmwareReload()
{
	uart_device_reload();
}


// USB Input buffer available
inline unsigned int Output_availablechar()
{
	return uart_serial_available();
}


// USB Get Character from input buffer
inline int Output_getchar()
{
	// XXX Make sure to check output_availablechar() first! Information is lost with the cast (error codes) (AVR)
	return (int)uart_serial_getchar();
}


// USB Send Character to output buffer
inline int Output_putchar( char c )
{
	return uart_serial_putchar( c );
}


// USB Send String to output buffer, null terminated
inline int Output_putstr( char* str )
{
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	uint16_t count = 0;
#elif defined(_mk20dx128_) || defined(_mk20dx128vlf5_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // ARM
	uint32_t count = 0;
#endif
	// Count characters until NULL character, then send the amount counted
	while ( str[count] != '\0' )
		count++;

	return uart_serial_write( str, count );
}


// Soft Chip Reset
inline void Output_softReset()
{
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
#elif defined(_mk20dx128_) || defined(_mk20dx128vlf5_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // ARM
	SOFTWARE_RESET();
#endif
}


// ----- CLI Command Functions -----

void cliFunc_kbdProtocol( char* args )
{
	print( NL );
	info_msg("Keyboard Protocol: ");
	printInt8( USBKeys_Protocol );
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

