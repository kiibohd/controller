/* Copyright (C) 2016 by Jacob Alexander
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

#pragma once

// ----- Includes -----

// Compiler Includes
#include <stdint.h>



// ----- Functions -----

// Functions to be called by main.c
void Scan_setup();
uint8_t Scan_loop();

// Call-backs
void Scan_finishedWithMacro( uint8_t sentKeys );  // Called by Macro Module
void Scan_finishedWithOutput( uint8_t sentKeys ); // Called by Output Module

void Scan_currentChange( unsigned int current ); // Called by Output Module


// ----- Capabilities -----

