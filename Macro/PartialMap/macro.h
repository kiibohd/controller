/* Copyright (C) 2014-2017 by Jacob Alexander
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

void Macro_analogState( uint16_t scanCode, uint8_t state );
void Macro_animationState( uint16_t animationIndex, uint8_t state );
void Macro_keyState( uint16_t scanCode, uint8_t state );
void Macro_ledState( uint16_t ledCode, uint8_t state );

void Macro_periodic();
void Macro_poll();
void Macro_setup();

uint8_t Macro_pressReleaseAdd( void *trigger ); // triggers is of type TriggerGuide, void* for circular dependencies

