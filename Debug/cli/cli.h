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
#include <Lib/MainLib.h>

// Project Includes
#include <output_com.h>


// ----- Defines -----

#define CLILineBufferMaxSize 100
#define CLIMaxDictionaries   5
#define CLIEntryTabAlign     13


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

uint8_t CLILEDState;
uint8_t CLIHexDebugMode;



// ----- Functions and Corresponding Function Aliases -----

void init_cli();
void process_cli();
void registerDictionary_cli( CLIDictItem *cmdDict );
void argumentIsolation_cli( char* string, char** first, char** second );

void commandLookup_cli();

// CLI Command Functions
void cliFunc_arch    ( char* args );
void cliFunc_chip    ( char* args );
void cliFunc_cliDebug( char* args );
void cliFunc_device  ( char* args );
void cliFunc_help    ( char* args );
void cliFunc_led     ( char* args );
void cliFunc_reload  ( char* args );
void cliFunc_reset   ( char* args );
void cliFunc_restart ( char* args );
void cliFunc_version ( char* args );


#endif

