/* Copyright (C) 2013-2018 by Jacob Alexander
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

#pragma once

// ----- Includes -----

// Compiler Includes
#include <stdint.h>



// ----- Defines -----

// ----- Enumerations -----

// ----- Structs -----

// ----- Variables -----

// Variables used to communciate to the output module
extern uint8_t  USBKeys_BitfieldSize;
extern void*    Output_Host_Callback; // Callback function to host



// ----- Functions -----

// General functions
void TestOut_setup();
void TestOut_poll();
void TestOut_periodic();

// Basic Output module functionality
void TestOut_firmwareReload();
void TestOut_softReset();

// Debug Serial Output
unsigned int TestOut_availablechar();

int TestOut_getchar();
int TestOut_putchar( char c );
int TestOut_putstr( char* str );

// RawIO Interface
unsigned int TestOut_rawio_availablechar();
int TestOut_rawio_getbuffer( char* buffer );
int TestOut_rawio_sendbuffer( char* buffer );



// ----- USB Dummy Functions -----

void usb_init();

void usb_device_check();
void usb_device_reload();
void usb_device_software_reset();

void usb_keyboard_clear( uint8_t protocol );
void usb_keyboard_idle_update();
void usb_keyboard_send( USBKeys *buffer, uint8_t protocol );

void usb_mouse_send();

int usb_serial_available();
int usb_serial_getchar();
int usb_serial_putchar( uint8_t c );
int usb_serial_write( const void *buffer, uint32_t size );

uint32_t usb_rawio_available();
int32_t usb_rawio_rx( void *buf, uint32_t timeout );
int32_t usb_rawio_tx( const void *buf, uint32_t timeout );

