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

// ----- Includes -----

// Compiler Includes
#include <Lib/OutputLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <output_com.h>
#include <print.h>

// KLL
#include <kll_defs.h>
#include <kll.h>

// Local Includes
#include "hidio_com.h"



// ----- Macros -----

// ----- Function Declarations -----

// ----- Variables -----

CLIDict_Def( hidioCLIDict, "HID-IO Module Commands" ) = {
	{ 0, 0, 0 } // Null entry for dictionary end
};



// ----- Capabilities -----

// ----- Functions -----

// HID-IO Module Setup
inline void HIDIO_setup()
{
	// Register Output CLI dictionary
	CLI_registerDictionary( hidioCLIDict, hidioCLIDictName );

	// TODO
}

// HID-IO Processing Loop
inline void HIDIO_process()
{
	// TODO
}



// ----- CLI Command Functions -----

