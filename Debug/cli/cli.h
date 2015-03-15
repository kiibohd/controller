/* Copyright (C) 2014-2015 by Jacob Alexander
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
#define CLIMaxDictionaries   10
#define CLIEntryTabAlign     13



// ----- Macros -----

// AVR CLI Dictionary definitions (has to deal with the annoying PROGMEM
// Only using PROGMEM with descriptions (all the string comparison tools need to be re-written otherwise)
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
#define CLIDict_Def(name,description) \
	const PROGMEM char name##Name[] = description; \
	const CLIDictItem name[]

#define CLIDict_Item(name) \
	{ #name, name##CLIDict_DescEntry, (const void (*)(char*))cliFunc_##name }

#define CLIDict_Entry(name,description) \
	const PROGMEM char name##CLIDict_DescEntry[] = description;

// ARM is easy :P
#elif defined(_mk20dx128_) || defined(_mk20dx128vlf5_) || defined(_mk20dx256_) // ARM
#define CLIDict_Def(name,description) \
	const char name##Name[] = description; \
	const CLIDictItem name[]

#define CLIDict_Item(name) \
	{ #name, name##CLIDict_DescEntry, (const void (*)(char*))cliFunc_##name }

#define CLIDict_Entry(name,description) \
	const char name##CLIDict_DescEntry[] = description;
#endif



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

uint8_t CLILEDState;
uint8_t CLIHexDebugMode;



// ----- Functions and Corresponding Function Aliases -----

void CLI_init();
void CLI_process();
void CLI_registerDictionary( const CLIDictItem *cmdDict, const char* dictName );
void CLI_argumentIsolation( char* string, char** first, char** second );

void CLI_commandLookup();
void CLI_tabCompletion();

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

