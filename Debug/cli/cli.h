/* Copyright (C) 2014 by Jacob Alexander
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

#ifndef cli_h__
#define cli_h__

// ----- Includes -----

// Compiler Includes
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_)

#elif defined(_mk20dx128_)

#include "arm/usb_serial.h"

#endif



// ----- Defines -----

#define CLILineBufferMaxSize 100
#define CLIMaxDictionaries   5


// ----- Structs -----

// Each item has a name, description, and function pointer with an argument for arguments
typedef struct CLIDictItem {
	char*  name;
	char*  description;
	void (*function)(char*);
} CLIDictItem;



// ----- Variables -----

char    CLILineBuffer[CLILineBufferMaxSize+1]; // +1 for an additional NULL
uint8_t CLILineBufferCurrent;

// Main command dictionary
CLIDictItem *CLIDict[CLIMaxDictionaries];
uint8_t CLIDictionariesUsed;




// ----- Functions and Corresponding Function Aliases -----

void init_cli();
void process_cli();
void registerDictionary_cli( CLIDictItem *cmdDict );
void argumentIsolation_cli( char* string, char** first, char** second );

void commandLookup_cli();

// CLI Command Functions
void cliFunc_help   ( char* args );
void cliFunc_version( char* args );


#endif

