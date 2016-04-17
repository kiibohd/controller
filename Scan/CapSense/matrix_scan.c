/* Copyright (C) 2016 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <kll_defs.h>
#include <led.h>
#include <print.h>
#include <macro.h>
#include <Lib/delay.h>

// Local Includes
#include "matrix_scan.h"

// Matrix Configuration
//#include <matrix.h>



// ----- Defines -----

// ----- Function Declarations -----

// CLI Functions
void cliFunc_matrixDebug( char* args );
void cliFunc_matrixInfo( char* args );
void cliFunc_matrixState( char* args );



// ----- Variables -----

// Scan Module command dictionary
CLIDict_Entry( matrixDebug,  "Enables matrix debug mode, prints out each scan code." NL "\t\tIf argument \033[35mT\033[0m is given, prints out each scan code state transition." );
CLIDict_Entry( matrixInfo,   "Print info about the configured matrix." );
CLIDict_Entry( matrixState,  "Prints out the current scan table N times." NL "\t\t \033[1mO\033[0m - Off, \033[1;33mP\033[0m - Press, \033[1;32mH\033[0m - Hold, \033[1;35mR\033[0m - Release, \033[1;31mI\033[0m - Invalid" );

CLIDict_Def( matrixCLIDict, "Matrix Module Commands" ) = {
	CLIDict_Item( matrixDebug ),
	CLIDict_Item( matrixInfo ),
	CLIDict_Item( matrixState ),
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Functions -----

// TODO
// - Support multiple ADCs
// - Channel/Mux setup
void ADC_setup( ADC adc )
{

	// Enable ADC clock
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_)
	SIM_SCGC6 |= SIM_SCGC6_ADC0;
#elif defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
	SIM_SCGC6 |= SIM_SCGC6_ADC0;
	SIM_SCGC3 |= SIM_SCGC3_ADC1;
#endif

	// Lookup base ADC register
	volatile unsigned int *ADC_SC1A = (unsigned int*)(&ADC_reg_offset_map[adc]);

	// Calculate Register offsets
	volatile unsigned int *ADC_CFG1 = (unsigned int*)(&ADC_SC1A) + 0x08;
	volatile unsigned int *ADC_CFG2 = (unsigned int*)(&ADC_SC1A) + 0x0C;
	volatile unsigned int *ADC_SC2  = (unsigned int*)(&ADC_SC1A) + 0x20;
	volatile unsigned int *ADC_SC3  = (unsigned int*)(&ADC_SC1A) + 0x24;
	volatile unsigned int *ADC_PG   = (unsigned int*)(&ADC_SC1A) + 0x2C;
	volatile unsigned int *ADC_CLPS = (unsigned int*)(&ADC_SC1A) + 0x38;
	volatile unsigned int *ADC_CLP4 = (unsigned int*)(&ADC_SC1A) + 0x3C;
	volatile unsigned int *ADC_CLP3 = (unsigned int*)(&ADC_SC1A) + 0x40;
	volatile unsigned int *ADC_CLP2 = (unsigned int*)(&ADC_SC1A) + 0x44;
	volatile unsigned int *ADC_CLP1 = (unsigned int*)(&ADC_SC1A) + 0x48;
	volatile unsigned int *ADC_CLP0 = (unsigned int*)(&ADC_SC1A) + 0x4C;

	// Make sure calibration has stopped
	*ADC_SC3 = 0;

	// - CFG1 -
	// ADIV:   (input)/2 divider
	// ADICLK:   (bus)/2 divider
	// MODE:   16-bit
	// ADLSMP: Long sample
	//ADC_CFG1 = ADC_CFG1_ADIV(1) | ADC_CFG1_ADICLK(1) | ADC_CFG1_MODE(3) | ADC_CFG1_ADLSMP;
	// ADIV:   (input)/8 divider
	*ADC_CFG1 = ADC_CFG1_ADIV(3) | ADC_CFG1_ADICLK(1) | ADC_CFG1_MODE(3) | ADC_CFG1_ADLSMP;

	// - CFG2 -
	// ADLSTS: 6 extra ADCK cycles; 10 ADCK cycles total sample time
	//ADC_CFG2 = ADC_CFG2_ADLSTS(2);
	// ADLSTS: 20 extra ADCK cycles; 24 ADCK cycles total sample time
	*ADC_CFG2 = ADC_CFG2_ADLSTS(0);

	// - SC2 -
	// REFSEL: Use default 3.3V reference
	*ADC_SC2 = ADC_SC2_REFSEL(0);
	/*
	// Setup VREF to 1.2 V
	VREF_TRM = 0x60;
	VREF_SC = 0xE1; // Enable 1.2 volt ref
	// REFSEL: Use 1.2V reference VREF
	*ADC_SC2 = ADC_SC2_REFSEL(1);
	*/

	// - SC3 -
	// CAL:  Start calibration
	// AVGE: Enable hardware averaging
	// AVGS: 32 samples averaged
	// 32 sample averaging
	*ADC_SC3 = ADC_SC3_CAL | ADC_SC3_AVGE | ADC_SC3_AVGS(3);

	// Wait for calibration
	while ( *ADC_SC3 & ADC_SC3_CAL );

	// Apply computed calibration offset
	// XXX Note, for single-ended, only the plus side offsets have to be applied
	//     For differential the minus side also has to be set as well

	__disable_irq(); // Disable interrupts while reading/setting offsets

	// Set calibration
	// ADC Plus-Side Gain Register
	// See Section 31.4.7 in the datasheet (mk20dx256vlh7) for details
	uint16_t sum = *ADC_CLPS + *ADC_CLP4 + *ADC_CLP3 + *ADC_CLP2 + *ADC_CLP1 + *ADC_CLP0;
	sum = (sum / 2) | 0x8000;
	*ADC_PG = sum;

	__enable_irq(); // Re-enable interrupts

	// Start ADC reading loop
	// - SC1A -
	// ADCH: Channel DAD0 (A10)
	// AIEN: Enable interrupt
	//*ADC_SC1A = ADC_SC1_AIEN | ADC_SC1_ADCH(0);

	// Enable ADC0 IRQ Vector
	//NVIC_ENABLE_IRQ( IRQ_ADC0 );
}

// TODO
// - Enable/Disable strobe detection (IBM)
// - Setup strobe matrix
void Strobe_setup()
{
}

// TODO
// - Setup ADCs
// - Setup ADC muxes
// - Setup voltage stab
void Sense_setup()
{
}

void Matrix_setup()
{
	// Register Matrix CLI dictionary
	CLI_registerDictionary( matrixCLIDict, matrixCLIDictName );

	// Setup sense
	Sense_setup();

	// Setup strobes
	Strobe_setup();
}

// Scan the matrix for keypresses
// NOTE: scanNum should be reset to 0 after a USB send (to reset all the counters)
void Matrix_scan( uint16_t scanNum )
{
}


// Called by parent scan module whenever the available current changes
// current - mA
void Matrix_currentChange( unsigned int current )
{
	// TODO - Any potential power savings?
}



// ----- CLI Command Functions -----

void cliFunc_matrixInfo( char* args )
{
}

void cliFunc_matrixDebug( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set the matrix debug flag depending on the argument
	// If no argument, set to scan code only
	// If set to T, set to state transition
	switch ( arg1Ptr[0] )
	{
	// T as argument
	case 'T':
	case 't':
		break;

	// No argument
	case '\0':
		break;

	// Invalid argument
	default:
		return;
	}
}

void cliFunc_matrixState( char* args )
{
}

