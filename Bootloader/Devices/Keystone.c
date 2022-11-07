/* Copyright (C) 2017-2022 by Jacob Alexander
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

//
// Kira
//

// ----- Includes -----

// Project Includes
#include <Lib/gpio.h>

// Local Includes
#include "../device.h"
#include "../debug.h"



// ----- Defines -----

// ----- Variables -----

static uint8_t prev_btn_state = 1;

// Esc key strobe
const GPIO_Pin strobe_pin = gpio(B,0);
const GPIO_Pin sense_pin = gpio(A,17); // Row 1

// Debug led
const GPIO_Pin debug_led = gpio(A,15);

// Cfg Pins
const GPIO_Pin cfg1_pin = gpio(C,25);
const GPIO_Pin cfg2_pin = gpio(C,26);

// Model Check Pin
const GPIO_Pin model_check_strobe = gpio(A,28);

// Disable strobes
const GPIO_Pin Matrix_cols[] = {
	gpio(B,0),
	gpio(B,1),
	gpio(B,2),
	gpio(B,3),
	gpio(B,4),
	gpio(B,5),
	gpio(B,14),
	gpio(A,0),
	gpio(A,1),
	gpio(A,2),
	gpio(A,5),
	gpio(A,6),
	gpio(A,7),
	gpio(A,8),
	gpio(A,9),
	gpio(A,10),
	gpio(A,23),
	gpio(A,24),
	gpio(A,25),
	gpio(A,26),
	gpio(A,27),
	gpio(A,28),
};

// Convenience Macros
#define Matrix_colsNum sizeof(Matrix_cols) / sizeof(GPIO_Pin)



// ----- Functions -----

// Called early-on during ResetHandler
void Device_reset()
{
	// Make sure debug LED is off
	GPIO_Ctrl( debug_led, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( debug_led, GPIO_Type_DriveHigh, GPIO_Config_None );
}

// Called during bootloader initialization
void Device_setup(bool *alt_device)
{
	// Enable Debug LED
	GPIO_Ctrl( debug_led, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( debug_led, GPIO_Type_DriveLow, GPIO_Config_None );

	// Disable all strobes (to prevent voltage regulator from overheating on prototypes)
	for (uint8_t c = 0; c < Matrix_colsNum; c++)
	{
		GPIO_Ctrl(Matrix_cols[c], GPIO_Type_DriveSetup, GPIO_Config_Pulldown);
		GPIO_Ctrl(Matrix_cols[c], GPIO_Type_DriveHigh, GPIO_Config_Pulldown);
	}

	// Detect which configuration this pcb is in
	GPIO_Ctrl( cfg1_pin, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );
	GPIO_Ctrl( cfg2_pin, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );
	uint8_t cfg1 = GPIO_Ctrl( cfg1_pin, GPIO_Type_Read, GPIO_Config_Pulldown ) != 0;
	uint8_t cfg2 = GPIO_Ctrl( cfg2_pin, GPIO_Type_Read, GPIO_Config_Pulldown ) != 0;

	// Compute layout
	// Default to ANSI
	if ( !cfg1 && cfg2 )
	{
		// TODO ISO
	}
	else if ( cfg1 && !cfg2 )
	{
		// TODO JIS
	}
	else
	{
		// TODO ANSI
	}

	// Check Fullsize vs TKL using fullsize-only strobes
	// Strobes are wired to a 1k pullup, so if the pin is high, it's fullsize
	if (GPIO_Ctrl( model_check_strobe, GPIO_Type_ReadSetup, GPIO_Config_Pulldown ) != 0)
	{
		// Fullsize
		print("Fullsize detected" NL);
		*alt_device = false;
	}
	else
	{
		// TKL
		print("TKL detected" NL);
		*alt_device = true;
	}

	// TODO Basic hall scanning for keypress (is GPIO possible?)
	// Setup scanning for S1
	PMC->PMC_PCER0 = (1 << ID_PIOA) | (1 << ID_PIOB);

	// Cols (strobe)
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveLow, GPIO_Config_None );

	// Rows (sense)
	GPIO_Ctrl( sense_pin, GPIO_Type_ReadSetup, GPIO_Config_Opendrain );
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	uint8_t cur_btn_state;

	// Check for S1 being pressed
	cur_btn_state = GPIO_Ctrl( sense_pin, GPIO_Type_Read, GPIO_Config_Opendrain ) != 0;

	// Rising edge = press
	if ( cur_btn_state && !prev_btn_state )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}

	prev_btn_state = cur_btn_state;
}

