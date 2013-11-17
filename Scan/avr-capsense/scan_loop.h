/* Copyright (C) 2013 by Jacob Alexander
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SCAN_LOOP_H
#define __SCAN_LOOP_H

// ----- Includes -----

// Compiler Includes
#include <stdint.h>

// Local Includes



// ----- Defines -----

#define KEYBOARD_KEYS 0xFF // TODO Determine max number of keys
#define KEYBOARD_BUFFER 24 // Max number of key signals to buffer
                           // This limits the NKRO-ability, so at 24, the keyboard is 24KRO
                           // The buffer is really only needed for converter modules
                           // An alternative macro module could be written for matrix modules and still work well



// ----- Variables -----

extern volatile     uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
extern volatile     uint8_t KeyIndex_BufferUsed;



// ----- Functions -----

// Functions used by main.c
void scan_setup( void );
uint8_t scan_loop( void );


// Functions available to macro.c
uint8_t scan_sendData( uint8_t dataPayload );

void scan_finishedWithBuffer( uint8_t sentKeys );
void scan_finishedWithUSBBuffer( uint8_t sentKeys );
void scan_lockKeyboard( void );
void scan_unlockKeyboard( void );
void scan_resetKeyboard( void );


#endif // __SCAN_LOOP_H

