/* Copyright (C) 2017 by Jacob Alexander
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
// WhiteFox
//

// ----- Includes -----

// Project Includes
#include <Lib/gpio.h>

// Local Includes
#include "../device.h"
#include "../debug.h"



// ----- Defines -----

// ----- Variables -----

// Esc key strobe
const GPIO_Pin strobe_pin = gpio(B,2);
const GPIO_Pin sense_pin = gpio(D,0);



// ----- Functions -----

// Called early-on during ResetHandler
void Device_reset()
{
}

// Called during bootloader initialization
void Device_setup()
{
	// Setup scanning for S1
	// Col 1 (strobe)
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveSetup, GPIO_Config_None );
	GPIO_Ctrl( strobe_pin, GPIO_Type_DriveHigh, GPIO_Config_None );

	// Row 1 (sense)
	GPIO_Ctrl( sense_pin, GPIO_Type_ReadSetup, GPIO_Config_Pulldown );
}

// Called during each loop of the main bootloader sequence
void Device_process()
{
	// Check for S1 being pressed
	if ( GPIO_Ctrl( sense_pin, GPIO_Type_Read, GPIO_Config_Pulldown ) )
	{
		print( "Reset key pressed." NL );
		SOFTWARE_RESET();
	}
}

