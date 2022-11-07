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
// Infinity Ergodox
//

// ----- Includes -----

// Project Includes
#include <Lib/gpio.h>

// Local Includes
#include "../device.h"
#include "../debug.h"



// ----- Defines -----

// ----- Variables -----

// Screen PWM
const GPIO_Pin red_chan_screen = gpio(C,1);

// Esc key strobe (S7)
const GPIO_Pin strobe_pin = gpio(D,0);
const GPIO_Pin sense_pin = gpio(D,1);



// ----- Functions -----

// Called early-on during ResetHandler
void Device_reset()
{
}

// Called during bootloader initialization
void Device_setup(bool *alt_device)
{
	// Set LCD backlight on ICED to Red
	GPIO_Ctrl( red_chan_screen, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( red_chan_screen, GPIO_Type_DriveLow, GPIO_Config_None );

	// Cols (strobe)
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveHigh, GPIO_Config_None );

	// Rows (sense)
	GPIO_Ctrl( sense_pin, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	// Check for S7 being pressed
	if ( GPIO_Ctrl( sense_pin, GPIO_Type_Read, GPIO_Config_Pulldown ) )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}
}

