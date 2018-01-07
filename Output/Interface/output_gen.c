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
#include <print.h>

// KLL
#include <kll.h>

// Interface Includes
#include <output_com.h>



// ----- Macros -----

// ----- Function Declarations -----

void cliFunc_current    ( char* args );
void cliFunc_outputDebug( char* args );



// ----- Variables -----

// Output Module command dictionary
CLIDict_Entry( current,     "Shows the current negotiated current." );
CLIDict_Entry( outputDebug, "Toggle Output Debug mode." );

CLIDict_Def( outputCLIDict, "Output Module Commands" ) = {
	CLIDict_Item( current ),
	CLIDict_Item( outputDebug ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Indicates whether the Output module is fully functional
// 0 - Not fully functional, 1 - Fully functional
// 0 is often used to show that a USB cable is not plugged in (but has power)
volatile uint8_t  Output_Available;

// Debug control variable for Output modules
// 0 - Debug disabled (default)
// 1 - Debug enabled
uint8_t  Output_DebugMode;

// mA - Set by outside module if not using USB (i.e. Interconnect)
// Generally set to 100 mA (low power) or 500 mA (high power)
uint16_t Output_ExtCurrent_Available;

// mA - Set by USB module (if exists)
// Initially 100 mA, but may be negotiated higher (e.g. 500 mA)
uint16_t Output_USBCurrent_Available;



// ----- Capabilities -----

// Ignored capabilities
void Output_ignored_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args ) {}

#if !defined(_APPLE_)
void Output_kbdProtocolBoot_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
	__attribute__ ((weak, alias("Output_ignored_capability")));
void Output_kbdProtocolNKRO_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
	__attribute__ ((weak, alias("Output_ignored_capability")));
void Output_toggleKbdProtocol_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
	__attribute__ ((weak, alias("Output_ignored_capability")));
void Output_consCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
	__attribute__ ((weak, alias("Output_ignored_capability")));
void Output_noneSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
	__attribute__ ((weak, alias("Output_ignored_capability")));
void Output_sysCtrlSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
	__attribute__ ((weak, alias("Output_ignored_capability")));
void Output_usbCodeSend_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
	__attribute__ ((weak, alias("Output_ignored_capability")));
void Output_usbMouse_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
	__attribute__ ((weak, alias("Output_ignored_capability")));
#endif

// Jump to bootloader capability
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



// ----- Functions -----

// Output Module Setup
inline void OutputGen_setup()
{
	// Register USB Output CLI dictionary
	CLI_registerDictionary( outputCLIDict, outputCLIDictName );

	// Initialize variables
	Output_Available = 0;
	Output_DebugMode = 0;
	Output_ExtCurrent_Available = 0;
	Output_USBCurrent_Available = 0;
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

void cliFunc_current( char* args )
{
	print( NL );
	info_msg("Current available: ");
	printInt16( Output_current_available() );
	print(" mA");
}


void cliFunc_outputDebug( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Default to 1 if no argument is given
	Output_DebugMode = Output_DebugMode ? 0 : 1;

	if ( arg1Ptr[0] != '\0' )
	{
		Output_DebugMode = (uint16_t)numToInt( arg1Ptr );
	}
}

