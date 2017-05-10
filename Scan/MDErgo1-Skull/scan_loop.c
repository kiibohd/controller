/* Copyright (C) 2014-2015 by Jacob Alexander
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
#include <lcd_scan.h>
#include <led.h>
#include <led_scan.h>
#include <print.h>
#include <matrix_scan.h>
#include <macro.h>
#include <output_com.h>

// Local Includes
#include "scan_loop.h"



// ----- Variables -----

// Number of scans since the last USB send
uint16_t Scan_scanCount = 0;



// ----- Functions -----

// Setup
inline void Scan_setup()
{
	// Setup UART Connect, if Output_Available, this is the master node
	Connect_setup( Output_Available );

	// Setup GPIO pins for matrix scanning
	Matrix_setup();

	// Setup ISSI chip to control the leds
	LED_setup();

	// Setup the ST/NHD lcd display
	LCD_setup();

	// Reset scan count
	Scan_scanCount = 0;
}


// Main Detection Loop
inline uint8_t Scan_loop()
{
	// Scan Matrix
	Matrix_scan( Scan_scanCount++ );

	// Process any interconnect commands
	Connect_scan();

	// Process any LED events
	LED_scan();

	// Process any LCD events
	LCD_scan();

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

