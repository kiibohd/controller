/* Copyright (C) 2014-2017 by Jacob Alexander
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

// KLL Generated Defines
#include <kll_defs.h>



// ----- Defines -----

#define DebounceCounter uint8_t
#define DebounceDivThreshold 0xFF

#if   ( MinDebounceTime_define > 0xFF )
#error "MinDebounceTime is a maximum of 255 ms"
#elif ( MinDebounceTime_define < 0x00 )
#error "MinDebounceTime is a minimum 0 ms"
#endif



// ----- Enums -----

// Freescale MK20s have GPIO ports A...E
typedef enum Port {
	Port_A = 0,
	Port_B = 1,
	Port_C = 2,
	Port_D = 3,
	Port_E = 4,
} Port;

// Each port has a possible 32 pins
typedef enum Pin {
	Pin_0  = 0,
	Pin_1  = 1,
	Pin_2  = 2,
	Pin_3  = 3,
	Pin_4  = 4,
	Pin_5  = 5,
	Pin_6  = 6,
	Pin_7  = 7,
	Pin_8  = 8,
	Pin_9  = 9,
	Pin_10 = 10,
	Pin_11 = 11,
	Pin_12 = 12,
	Pin_13 = 13,
	Pin_14 = 14,
	Pin_15 = 15,
	Pin_16 = 16,
	Pin_17 = 17,
	Pin_18 = 18,
	Pin_19 = 19,
	Pin_20 = 20,
	Pin_21 = 21,
	Pin_22 = 22,
	Pin_23 = 23,
	Pin_24 = 24,
	Pin_25 = 25,
	Pin_26 = 26,
	Pin_27 = 27,
	Pin_28 = 28,
	Pin_29 = 29,
	Pin_30 = 30,
	Pin_31 = 31,
} Pin;

// Type of pin
typedef enum Type {
	Type_StrobeOn,
	Type_StrobeOff,
	Type_StrobeSetup,
	Type_Sense,
	Type_SenseSetup,
} Type;

// Sense/Strobe configuration
typedef enum Config {
	Config_Pullup,    // Internal pull-up
	Config_Pulldown,  // Internal pull-down
	Config_Opendrain, // External pull resistor
} Config;

// Keypress States
typedef enum KeyPosition {
	KeyState_Off     = 0,
	KeyState_Press   = 1,
	KeyState_Hold    = 2,
	KeyState_Release = 3,
	KeyState_Invalid,
} KeyPosition;



// ----- Structs -----

// Struct container for defining Rows (Sense) and Columns (Strobes)
typedef struct GPIO_Pin {
	Port port;
	Pin  pin;
} GPIO_Pin;

// Debounce Element
typedef struct KeyState {
	DebounceCounter activeCount;
	DebounceCounter inactiveCount;
	KeyPosition     prevState;
	KeyPosition     curState;
	uint32_t        prevDecisionTime;
} KeyState;



// ----- Functions -----

void Matrix_setup();
void Matrix_start();

uint8_t Matrix_single_scan();
uint8_t Matrix_totalColumns();

void Matrix_currentChange( unsigned int current );

