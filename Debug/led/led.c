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
#include <kll_defs.h>
#include <Lib/gpio.h>
#include "led.h"



// ----- Variables -----

static const GPIO_Pin led_pin = DebugLED_ErrorPin_define;



// ----- Functions -----

// Error LED Setup
inline void init_errorLED()
{
	// Enable output
	GPIO_Ctrl( led_pin, GPIO_Type_DriveSetup, GPIO_Config_None );
}

// Error LED Control
inline void errorLED( uint8_t on )
{
#if DebugLED_Inverse_define == 0
	// Drive high to enable LED
	// Error LED On
	if (on)
	{
		GPIO_Ctrl( led_pin, GPIO_Type_DriveHigh, GPIO_Config_None );
	}
	// Error LED Off
	else
	{
		GPIO_Ctrl( led_pin, GPIO_Type_DriveLow, GPIO_Config_None );
	}
#else
	// Pull low to enable LED
	// Error LED On
	if (on)
	{
		GPIO_Ctrl( led_pin, GPIO_Type_DriveLow, GPIO_Config_None );
	}
	// Error LED Off
	else
	{
		GPIO_Ctrl( led_pin, GPIO_Type_DriveHigh, GPIO_Config_None );
	}
#endif
}

// Error LED Toggle
inline void errorLEDToggle()
{
	GPIO_Ctrl( led_pin, GPIO_Type_DriveToggle, GPIO_Config_None );
}

