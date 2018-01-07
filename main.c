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
#include <Lib/MainLib.h>

// Project Includes
#include <macro.h>
#include <scan_loop.h>
#include <output_com.h>

#include <cli.h>
#include <latency.h>
#include <led.h>
#include <print.h>

#include <Lib/periodic.h>



// ----- Enumerations -----

typedef enum PeriodicStage {
	PeriodicStage_Scan,
	PeriodicStage_Macro,
	PeriodicStage_Output,
} PeriodicStage;



// ----- Variables -----

// Periodic Stage Tracker
static volatile PeriodicStage stage_tracker;



// ----- Functions -----

// Run periodically at a consistent time rate
// Used to process events that need to be run at regular intervals
// And have negative effect being delayed or stretched too much
//
// Returns 1 if full rotation has completed
// Returns 0 otherwise
int main_periodic()
{
	// Scan module periodic routines
	switch ( stage_tracker )
	{
	case PeriodicStage_Scan:
		// Returns non-zero if ready to process macros
		if ( Scan_periodic() )
		{
			stage_tracker = PeriodicStage_Macro;
		}
		break;

	case PeriodicStage_Macro:
		// Run Macros over Key Indices and convert to USB Keys
		Macro_periodic();
		stage_tracker = PeriodicStage_Output;
		break;

	case PeriodicStage_Output:
		// Send periodic USB results
		Output_periodic();
		stage_tracker = PeriodicStage_Scan;

		// Full rotation
		return 1;
	}

	return 0;
}

// ----- MCU-only Functions -----
#if !defined(_host_)

int main()
{
	// AVR - Teensy Set Clock speed to 16 MHz
#if defined(_avr_at_)
	CLKPR = 0x80;
	CLKPR = 0x00;
#endif
	// Setup Latency Measurements
	Latency_init();

	// Enable CLI
	CLI_init();

	// Setup periodic timer function
	Periodic_function( &main_periodic );

	// Setup Modules
	Output_setup();
	Macro_setup();
	Scan_setup();

	// Start scanning on first periodic loop
	stage_tracker = PeriodicStage_Scan;

	// Main Detection Loop
	while ( 1 )
	{
		// Run constantly
		// Used to process things such as the cli and output module (i.e. USB).
		// Should not be used to run things that require consistent timing.
		// While counter-intuitive, things such as LED/Display modules should be run as poll
		// as they need to run as quickly as possible, in case there needs to be frame drops

		// Process CLI
		CLI_process();

		// Scan module poll routines
		Scan_poll();

		// Macro module poll routines
		Macro_poll();

		// Output module poll routines
		Output_poll();
	}
}

#endif

// ----- Host-only Functions -----
#if defined(_host_)

int Host_init()
{
	// Enable CLI
	CLI_init();

	// Setup Modules
	Output_setup();
	Macro_setup();
	Scan_setup();

	// Start scanning on first periodic loop
	stage_tracker = PeriodicStage_Scan;

	return 1;
}

int Host_cli_process()
{
	// Process CLI
	return CLI_process();
}

int Host_periodic()
{
	return main_periodic();
}

int Host_poll()
{
	// Run constantly
	// Used to process things such as the cli and output module (i.e. USB).
	// Should not be used to run things that require consistent timing.
	// While counter-intuitive, things such as LED/Display modules should be run as poll
	// as they need to run as quickly as possible, in case there needs to be frame drops

	// Process CLI
	CLI_process();

	// Scan module poll routines
	Scan_poll();

	// Macro module poll routines
	Macro_poll();

	// Output module poll routines
	Output_poll();

	return 1;
}

int Host_process()
{
	// Run periodic loop for a full rotation
	while ( !Host_periodic() );

	// Then a single poll loop
	Host_poll();

	return 1;
}

// Register callback for system calls
int Host_register_callback( void* func )
{
	Output_Host_Callback = func;
	return 1;
}

// Change the value of systick (milliseconds)
volatile uint32_t systick_millis_count;
int Host_set_systick( uint32_t systick_ms )
{
	systick_millis_count = systick_ms;
	return 1;
}

// Change the nanosecs since last systick
volatile uint32_t ns_since_systick_count;
int Host_set_nanosecs_since_systick( uint32_t systick_ns )
{
	ns_since_systick_count = systick_ns;
	return 1;
}

// Test function to validate library
int Host_callback_test()
{
	// Print twice to validate callback
	print("Test 1 ");
	print("Test 2 ");
	print( NL );
	print("Systick Millisecond: ");
	printHex32( systick_millis_count );
	print( NL );
	return 9;
}

#endif

