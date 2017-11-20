/* Copyright (C) 2014-2016 by Jacob Alexander
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
#include <Lib/MainLib.h>

// Project Includes
#include <output_com.h>


// ----- Defines -----

#define CLILineBufferMaxSize 100
#define CLIMaxDictionaries   10
#define CLIEntryTabAlign     13
#define CLIMaxHistorySize    10


// ----- Macros -----

// AVR CLI Dictionary definitions (has to deal with the annoying PROGMEM
// Only using PROGMEM with descriptions (all the string comparison tools need to be re-written otherwise)
#if defined(_avr_at) // AVR
#define CLIDict_Def(name,description) \
	const PROGMEM char name##Name[] = description; \
	const CLIDictItem name[]

#define CLIDict_Item(name) \
	{ #name, name##CLIDict_DescEntry, (const void (*)(char*))cliFunc_##name }

#define CLIDict_Entry(name,description) \
	const PROGMEM char name##CLIDict_DescEntry[] = description;

// ARM like nearly everything else is easy :P
#else
#define CLIDict_Def(name,description) \
	const char name##Name[] = description; \
	const CLIDictItem name[]

#define CLIDict_Item(name) \
	{ #name, name##CLIDict_DescEntry, (const void (*)(char*))cliFunc_##name }

#define CLIDict_Entry(name,description) \
	const char name##CLIDict_DescEntry[] = description;

#endif

#define RING_PREV(i) CLI_wrap(i - 1, 0, CLIMaxHistorySize - 1)
#define RING_NEXT(i) CLI_wrap(i + 1, 0, CLIMaxHistorySize - 1)


// ----- Structs -----

// Each item has a name, description, and function pointer with an argument for arguments
typedef struct CLIDictItem {
	const char*  name;
	const char*  description;
	const void (*function)(char*);
} CLIDictItem;



// ----- Variables -----

char    CLILineBuffer[CLILineBufferMaxSize+1]; // +1 for an additional NULL
uint8_t CLILineBufferCurrent;

// Main command dictionary
CLIDictItem *CLIDict     [CLIMaxDictionaries];
char*        CLIDictNames[CLIMaxDictionaries];
uint8_t      CLIDictionariesUsed;

// History
char CLIHistoryBuffer[CLIMaxHistorySize][CLILineBufferMaxSize];
uint8_t CLIHistoryHead;
uint8_t CLIHistoryTail;
int8_t CLIHistoryCurrent;

// Debug
uint8_t CLILEDState;
uint8_t CLIHexDebugMode;



// ----- Functions and Corresponding Function Aliases -----

void CLI_init();
int CLI_process();
void CLI_registerDictionary( const CLIDictItem *cmdDict, const char* dictName );
void CLI_argumentIsolation( char* string, char** first, char** second );

int CLI_wrap( int x, int low, int high );
void CLI_commandLookup();
void CLI_tabCompletion();
void CLI_saveHistory( char *buff );
void CLI_retreiveHistory( int index );

// CLI Command Functions
void cliFunc_clear    ( char* args );
void cliFunc_cliDebug ( char* args );
void cliFunc_colorTest( char* args );
void cliFunc_exit     ( char* args );
void cliFunc_help     ( char* args );
void cliFunc_latency  ( char* args );
void cliFunc_led      ( char* args );
void cliFunc_periodic ( char* args );
void cliFunc_rand     ( char* args );
void cliFunc_reload   ( char* args );
void cliFunc_reset    ( char* args );
void cliFunc_restart  ( char* args );
void cliFunc_tick     ( char* args );
void cliFunc_version  ( char* args );

