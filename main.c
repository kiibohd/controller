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



// ----- Functions -----

int main()
{
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

