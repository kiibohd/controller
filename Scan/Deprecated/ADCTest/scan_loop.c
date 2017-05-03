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

// ADC Clock divisor settings (F_BUS == 48000000)
#define ADC_CFG1_6MHZ  ADC_CFG1_ADIV(2) + ADC_CFG1_ADICLK(1)
#define ADC_CFG1_12MHZ ADC_CFG1_ADIV(1) + ADC_CFG1_ADICLK(1)
#define ADC_CFG1_24MHZ ADC_CFG1_ADIV(0) + ADC_CFG1_ADICLK(1)



// ----- Macros -----



// ----- Function Declarations -----

void cliFunc_adc    ( char* args );
void cliFunc_adcInit( char* args );
void cliFunc_dac    ( char* args );
void cliFunc_dacVref( char* args );
void cliFunc_echo   ( char* args );



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;


// Scan Module command dictionary
char scanCLIDictName[] = "ADC Test Module Commands";
const CLIDictItem scanCLIDict[] = {
#if defined(_mk20dx128_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // ARM
	{ "adc",     "Read the specified number of values from the ADC at the given pin: <pin> [# of reads]"
		  NL "\t\t See \033[35mLib/pin_map.teensy3\033[0m for ADC0 channel number.", cliFunc_adc },
	{ "adcInit", "Intialize/calibrate ADC: <ADC Resolution> <Vref> <Hardware averaging samples>"
		  NL "\t\tADC Resolution -> 8, 10, 12, 16 (bit)"
		  NL "\t\t          Vref -> 0 (1.2 V), 1 (External)"
		  NL "\t\tHw Avg Samples -> 0 (disabled), 4, 8, 16, 32", cliFunc_adcInit },
#endif
#if defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // DAC is only supported on Teensy 3.1
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
#elif defined(_mk20dx128_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // ARM
{
	// Register Scan CLI dictionary
	CLI_registerDictionary( scanCLIDict, scanCLIDictName );

	// ADC Setup
	VREF_TRM = 0x60;
	VREF_SC  = 0xE1; // Enable 1.2V Vref

#if defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // DAC is only supported on Teensy 3.1
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

	// Parse args until a \0 is found
	while ( 1 )
	{
		print( NL ); // No \r\n by default after the command is entered

		curArgs = arg2Ptr; // Use the previous 2nd arg pointer to separate the next arg from the list
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Print out the arg
		dPrint( arg1Ptr );
	}
}

void cliFunc_adc( char* args )
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
{
}
#elif defined(_mk20dx128_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // ARM
{
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set the ADC Channel
	uint8_t channel = numToInt( arg1Ptr );
	__disable_irq();
	ADC0_SC1A = channel;
	__enable_irq();

	// Number of ADC samples to display
	CLI_argumentIsolation( arg2Ptr, &arg1Ptr, &arg2Ptr );

	int displayedADC = 1; // Default to 1 read
	if ( arg1Ptr ) // If there is an argument, use that instead
	{
		displayedADC = numToInt( arg1Ptr );
	}

	// Poll ADC until it gets a value, making sure to serve interrupts on each attempt
	while ( displayedADC > 0 )
	{
		__disable_irq();

		// ADC Sample is ready
		if ( (ADC0_SC1A & ADC_SC1_COCO) )
		{
			int result = ADC0_RA;
			print( NL );
			printInt32( result );
			displayedADC--;

			// Prepare for another read
			if ( displayedADC > 0 )
			{
				ADC0_SC1A = channel;
			}
		}

		__enable_irq();
		yield(); // Make sure interrupts actually get serviced
	}
}
#endif

void cliFunc_adcInit( char* args )
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
{
}
#elif defined(_mk20dx128_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // ARM
{
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Make sure calibration has stopped
	ADC0_SC3 = 0;

	// Select bit resolution
	int bitResolution = numToInt( arg1Ptr );
	switch ( bitResolution )
	{
	case 8: // 8-bit
		ADC0_CFG1 = ADC_CFG1_24MHZ + ADC_CFG1_MODE(0);
		ADC0_CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(3);
		break;

	case 10: // 10-bit
		ADC0_CFG1 = ADC_CFG1_12MHZ + ADC_CFG1_MODE(2) + ADC_CFG1_ADLSMP;
		ADC0_CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(3);
		break;

	case 12: // 12-bit
		ADC0_CFG1 = ADC_CFG1_12MHZ + ADC_CFG1_MODE(1) + ADC_CFG1_ADLSMP;
		ADC0_CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(2);
		break;

	case 16: // 16-bit
		ADC0_CFG1 = ADC_CFG1_12MHZ + ADC_CFG1_MODE(3) + ADC_CFG1_ADLSMP;
		ADC0_CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(2);
		break;

	default: return; // Do nothing, invalid arg
	}

	// Select Vref
	CLI_argumentIsolation( arg2Ptr, &arg1Ptr, &arg2Ptr );
	int vRef = numToInt( arg1Ptr );
	switch ( vRef )
	{
	case 0: // 1.2V internal Vref
		ADC0_SC2 = ADC_SC2_REFSEL(1);
		break;

	case 1: // Vcc/Ext Vref
		ADC0_SC2 = ADC_SC2_REFSEL(0);
		break;

	default: return; // Do nothing, invalid arg
	}

	// Hardware averaging (and start calibration)
	CLI_argumentIsolation( arg2Ptr, &arg1Ptr, &arg2Ptr );
	int hardwareAvg = numToInt( arg1Ptr );
	switch ( hardwareAvg )
	{
	case 0:  // No hardware averaging
		ADC0_SC3 = ADC_SC3_CAL; // Just start calibration
		break;

	case 4:  // 4 sample averaging
		ADC0_SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(0);
		break;

	case 8:  // 8 sample averaging
		ADC0_SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(1);
		break;

	case 16: // 16 sample averaging
		ADC0_SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(2);
		break;

	case 32: // 32 sample averaging
		ADC0_SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(3);
		break;

	default: return; // Do nothing, invalid arg
	}

	// Wait for calibration
	while ( ADC0_SC3 & ADC_SC3_CAL );

	// Set calibration
	uint16_t sum;

	// XXX Why is PJRC doing this? Is the self-calibration not good enough? -HaaTa
	// ADC Plus-Side Gain Register
	__disable_irq(); // Disable interrupts
	sum = ADC0_CLPS + ADC0_CLP4 + ADC0_CLP3 + ADC0_CLP2 + ADC0_CLP1 + ADC0_CLP0;
	sum = (sum / 2) | 0x8000;
	ADC0_PG = sum;

	print( NL );
	info_msg("Calibration ADC0_PG (Plus-Side Gain Register)  set to: ");
	printInt16( sum );

	// ADC Minus-Side Gain Register
	// XXX I don't think this is necessary when doing single-ended (as opposed to differential) -HaaTa
	//     K20P64M72SF1RM.pdf 31.3.10 pg. 666
	sum = ADC0_CLMS + ADC0_CLM4 + ADC0_CLM3 + ADC0_CLM2 + ADC0_CLM1 + ADC0_CLM0;
	sum = (sum / 2) | 0x8000;
	ADC0_MG = sum;

	print( NL );
	info_msg("Calibration ADC0_MG (Minus-Side Gain Register) set to: ");
	printInt16( sum );
	__enable_irq(); // Re-enable interrupts
}
#endif

void cliFunc_dac( char* args )
{
#if defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // DAC is only supported on Teensy 3.1
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	int dacOut = numToInt( arg1Ptr );

	// Make sure the value is between 0 and 4096, otherwise ignore
	if ( dacOut >= 0 && dacOut <= 4095 )
	{
		*(int16_t *) &(DAC0_DAT0L) = dacOut;
	}
#endif
}

void cliFunc_dacVref( char* args )
{
#if defined(_mk20dx256_) || defined(_mk20dx256vlh7_) // DAC is only supported on Teensy 3.1
	// Parse code from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	switch ( numToInt( arg1Ptr ) )
	{
	case 0:
		DAC0_C0 = DAC_C0_DACEN; // 1.2V Vref is DACREF_1
		break;
	case 1:
		DAC0_C0 = DAC_C0_DACEN | DAC_C0_DACRFS; // 3.3V VDDA is DACREF_2
		break;
	}
#endif
}

