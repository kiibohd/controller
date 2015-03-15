/* Copyright (C) 2014 by Jacob Alexander
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

#ifndef __macro_h
#define __macro_h

// ----- Includes -----

// Compiler Includes
#include <stdint.h>



// ----- Capabilities -----

void Macro_layerState_capability( uint8_t state, uint8_t stateType, uint8_t *args );
void Macro_layerLatch_capability( uint8_t state, uint8_t stateType, uint8_t *args );
void Macro_layerLock_capability( uint8_t state, uint8_t stateType, uint8_t *args );
void Macro_layerShift_capability( uint8_t state, uint8_t stateType, uint8_t *args );



// ----- Functions -----

void Macro_analogState( uint8_t scanCode, uint8_t state );
void Macro_keyState( uint8_t scanCode, uint8_t state );
void Macro_ledState( uint8_t ledCode, uint8_t state );
void Macro_triggerState( void *triggers, uint8_t num ); // triggers is of type TriggerGuide, void* for circular dependencies
void Macro_process();
void Macro_setup();

#endif

