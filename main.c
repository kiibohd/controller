/* Copyright (C) 2011-2014 by Jacob Alexander
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

extern volatile uint32_t systick_millis_count;


// ----- Functions -----

int main()
{
	/*
	GPIOA_PDDR |= (1<<5);
	// Setup pin - A5 - See Lib/pin_map.mchck for more details on pins
	PORTA_PCR5 = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);
	while( 1 )
	{
		GPIOA_PTOR |= (1<<5);
		for (uint32_t d = 0; d < 720000; d++ );
	}
	*/

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
		cli();
		while ( Scan_loop() );
		sei();

		// Run Macros over Key Indices and convert to USB Keys
		Macro_process();

		// Sends USB data only if changed
		Output_send();
	}
}

