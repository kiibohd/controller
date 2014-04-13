/* Copyright (C) 2014 by Jacob Alexander
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
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"



// ----- Defines -----



// ----- Macros -----



// ----- Function Declarations -----

void cliFunc_dac    ( char* args );
void cliFunc_dacVref( char* args );
void cliFunc_echo   ( char* args );



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;


// Scan Module command dictionary
char*       scanCLIDictName = "ADC Test Module Commands";
CLIDictItem scanCLIDict[] = {
#if defined(_mk20dx256_) // DAC is only supported on Teensy 3.1
	{ "dac",     "Set DAC output value, from 0 to 4095 (1/4096 Vref to Vref).", cliFunc_dac },
	{ "dacVref", "Set DAC Vref. 0 is 1.2V. 1 is 3.3V.", cliFunc_dacVref },
#endif
	{ "echo",    "Example command, echos the arguments.", cliFunc_echo },
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Functions -----

// Setup
inline void Scan_setup()
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( scanCLIDict, scanCLIDictName );
}
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( scanCLIDict, scanCLIDictName );

#if defined(_mk20dx256_) // DAC is only supported on Teensy 3.1
	// DAC Setup
	SIM_SCGC2 |= SIM_SCGC2_DAC0;
	DAC0_C0 = DAC_C0_DACEN | DAC_C0_DACRFS; // 3.3V VDDA is DACREF_2
#endif
}
#endif


// Main Detection Loop
inline uint8_t Scan_loop()
{
	return 0;
}


// Signal KeyIndex_Buffer that it has been properly read
void Scan_finishedWithBuffer( uint8_t sentKeys )
{
}


// Signal that the keys have been properly sent over USB
void Scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
}

// Reset Keyboard
void Scan_resetKeyboard()
{
}


// ----- CLI Command Functions -----

// XXX Just an example command showing how to parse arguments (more complex than generally needed)
void cliFunc_echo( char* args )
{
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	print( NL ); // No \n by default after the command is entered

	// Parse args until a \0 is found
	while ( 1 )
	{
		curArgs = arg2Ptr; // Use the previous 2nd arg pointer to separate the next arg from the list
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Print out the arg
		dPrint( arg1Ptr );
	}
}

void cliFunc_dac( char* args )
{
#if defined(_mk20dx256_) // DAC is only supported on Teensy 3.1
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	int dacOut = decToInt( arg1Ptr );

	// Make sure the value is between 0 and 4096, otherwise ignore
	if ( dacOut >= 0 && dacOut <= 4095 )
	{
		*(int16_t *) &(DAC0_DAT0L) = dacOut;
	}
#endif
}

void cliFunc_dacVref( char* args )
{
#if defined(_mk20dx256_) // DAC is only supported on Teensy 3.1
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	switch ( decToInt( arg1Ptr ) )
	{
	case 0:
		// XXX Doesn't seem to work...
		DAC0_C0 = DAC_C0_DACEN; // 1.2V ref is DACREF_1
		break;
	case 1:
		DAC0_C0 = DAC_C0_DACEN | DAC_C0_DACRFS; // 3.3V VDDA is DACREF_2
		break;
	}
#endif
}

