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

#include <stdint.h>

// KLL Includes
#include <kll_defs.h>

// Module Includes
#if defined(Output_UARTEnabled_define)
#include <output_uart.h>
#endif

#if defined(Output_USBEnabled_define)
#include <output_usb.h>
#endif

#if defined(Output_TestOutEnabled_define)
#include <output_testout.h>
#endif



// ----- Defines -----

// ----- Enumerations -----

// ----- Structs -----

// ----- Variables -----

extern volatile uint8_t  Output_Available; // 0 - Output module not fully functional, 1 - Output module working

extern          uint8_t  Output_DebugMode; // 0 - Debug disabled, 1 - Debug enabled, 2 - Extra debug

extern          uint16_t Output_ExtCurrent_Available; // mA - Set by outside module if not using USB (i.e. Interconnect)



// ----- Functions -----

void OutputGen_setup(); // Initialize Generic Interface Module
void Output_setup();    // Initialize module
void Output_poll();     // Poll routine for module
void Output_periodic(); // Periodic routine for module

void Output_firmwareReload(); // Request firmware reload
void Output_softReset();      // Request soft reset

unsigned int Output_availablechar();

int Output_getchar();
int Output_putchar( char c );
int Output_putstr( char* str );

unsigned int Output_rawio_availablechar();
int Output_rawio_getbuffer( char* buffer );
int Output_rawio_sendbuffer( char* buffer );

// Returns the total mA available (total, if used in a chain, each device will have to use a slice of it)
unsigned int Output_current_available();

void Output_update_external_current( unsigned int current ); // Callback to update current from interconnect
void Output_update_usb_current( unsigned int current );      // Callback to update current

