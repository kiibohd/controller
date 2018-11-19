/* Copyright (C) 2017-2018 by Jacob Alexander
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
void Device_setup()
{
	// Set LCD backlight on ICED to Red
	GPIO_Ctrl( red_chan_screen, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( red_chan_screen, GPIO_Type_DriveHigh, GPIO_Config_None );

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

