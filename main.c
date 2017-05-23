/* Copyright (C) 2011-2016 by Jacob Alexander
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
#include <led.h>
#include <print.h>



// ----- Functions -----

int main()
{
	// AVR - Teensy Set Clock speed to 16 MHz
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_)
	CLKPR = 0x80;
	CLKPR = 0x00;
#endif

	// Enable CLI
	CLI_init();

	// Setup Modules
	Output_setup();
	Macro_setup();
	Scan_setup();

	// Main Detection Loop
	while ( 1 )
	{
		// Process CLI
		CLI_process();

		// Acquire Key Indices
		// Loop continuously until scan_loop returns 0
		while ( Scan_loop() );

		// Run Macros over Key Indices and convert to USB Keys
		Macro_process();

		// Sends USB data only if changed
		Output_send();
	}
}

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

	return 1;
}

int Host_cli_process()
{
	// Process CLI
	return CLI_process();
}

int Host_process()
{
	// Acquire Key Indices
	// Loop continuously until scan_loop returns 0
	while ( Scan_loop() );

	// Run Macros over Key Indices and convert to USB Keys
	Macro_process();

	// Sends USB data only if changed
	Output_send();

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

