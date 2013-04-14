/* Copyright (C) 2013 by Jacob Alexander
 * 
 * dfj, put whatever license here you want
 * This file will probably be removed though.
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

