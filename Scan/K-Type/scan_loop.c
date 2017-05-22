/* Copyright (C) 2015-2017 by Jacob Alexander
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
#include <connect_scan.h>
#include <led.h>
#include <led_scan.h>
#include <print.h>
#include <matrix_scan.h>
#include <macro.h>
#include <output_com.h>
#include <port_scan.h>
#include <pixel.h>

// Local Includes
#include "scan_loop.h"



// ----- Function Declarations -----

// ----- Variables -----

// Number of scans since the last USB send
uint16_t Scan_scanCount = 0;

uint8_t Scan_strobe_position;



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup Port Swap module
	Port_setup();

	// Setup UART Connect, if Output_Available, this is the master node
	Connect_setup( Output_Available, 1 );

	// Setup GPIO pins for matrix scanning
	Matrix_setup();

	// Setup ISSI chip to control the leds
	LED_setup();

	// Setup Pixel Map
	Pixel_setup();

	// Reset scan count
	Scan_scanCount = 0;

	// Reset starting strobe position
	Scan_strobe_position = 0;
}


// Main Detection Loop
inline uint8_t Scan_loop()
{
	// Port Swap detection
	Port_scan();

	// Scan Matrix
	Matrix_scan( Scan_scanCount, &Scan_strobe_position, 4 );

	// Process any interconnect commands
	Connect_scan();

	// Prepare any LED events
	Pixel_process();

	// Process any LED events
	LED_scan();

	// Check if we are ready roll ovr the strobe position
	if ( Scan_strobe_position >= Matrix_totalColumns() - 1 )
	{
		Scan_strobe_position = 0;
		Scan_scanCount++;
	}

	return 0;
}


// Signal from Macro Module that all keys have been processed (that it knows about)
inline void Scan_finishedWithMacro( uint8_t sentKeys )
{
}


// Signal from Output Module that all keys have been processed (that it knows about)
inline void Scan_finishedWithOutput( uint8_t sentKeys )
{
	// Reset scan loop indicator (resets each key debounce state)
	// TODO should this occur after USB send or Macro processing?
	Scan_scanCount = 0;
}


#if defined(Pixel_MapEnabled_define) && defined(animation_test_layout_define)
uint8_t Pixel_addDefaultAnimation( uint32_t index );
#endif

// Signal from the Output Module that the available current has changed
// current - mA
void Scan_currentChange( unsigned int current )
{
	// Indicate to all submodules current change
	Connect_currentChange( current );
	Matrix_currentChange( current );
	LED_currentChange( current );

	if ( current == 500 )
	{
		// TODO REMOVEME once default animations can be set
		Pixel_addDefaultAnimation( Animation__rainbow_wave );
	}
}

