/* Copyright (C) 2014-2019 by Jacob Alexander
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

#if defined(_sam_)
#include <sam/drivers/pio/pio.h>
#endif



// ----- Defines -----

// ----- Macros -----

#define gpio( port, pin ) { GPIO_Port_##port, GPIO_Pin_##pin }
#define periph_io( port, pin, periph ) { GPIO_Port_##port, GPIO_Pin_##pin, GPIO_Peripheral_##periph }



// ----- Enums -----

// GPIO ports
typedef enum GPIO_Port {
	GPIO_Port_A = 0,
	GPIO_Port_B = 1,
	GPIO_Port_C = 2,
	GPIO_Port_D = 3,
	GPIO_Port_E = 4,
	GPIO_Port_F = 5,
} GPIO_Port;

// Each port has a possible 32 pins
typedef enum GPIO_Pin_Num {
	GPIO_Pin_0  = 0,
	GPIO_Pin_1  = 1,
	GPIO_Pin_2  = 2,
	GPIO_Pin_3  = 3,
	GPIO_Pin_4  = 4,
	GPIO_Pin_5  = 5,
	GPIO_Pin_6  = 6,
	GPIO_Pin_7  = 7,
	GPIO_Pin_8  = 8,
	GPIO_Pin_9  = 9,
	GPIO_Pin_10 = 10,
	GPIO_Pin_11 = 11,
	GPIO_Pin_12 = 12,
	GPIO_Pin_13 = 13,
	GPIO_Pin_14 = 14,
	GPIO_Pin_15 = 15,
	GPIO_Pin_16 = 16,
	GPIO_Pin_17 = 17,
	GPIO_Pin_18 = 18,
	GPIO_Pin_19 = 19,
	GPIO_Pin_20 = 20,
	GPIO_Pin_21 = 21,
	GPIO_Pin_22 = 22,
	GPIO_Pin_23 = 23,
	GPIO_Pin_24 = 24,
	GPIO_Pin_25 = 25,
	GPIO_Pin_26 = 26,
	GPIO_Pin_27 = 27,
	GPIO_Pin_28 = 28,
	GPIO_Pin_29 = 29,
	GPIO_Pin_30 = 30,
	GPIO_Pin_31 = 31,
} GPIO_Pin_Num;

// Type of pin
typedef enum GPIO_Type {
	GPIO_Type_DriveHigh,
	GPIO_Type_DriveLow,
	GPIO_Type_DriveToggle,
	GPIO_Type_DriveSetup,
	GPIO_Type_Read,
	GPIO_Type_ReadSetup,
} GPIO_Type;

// Sense/Strobe configuration
typedef enum GPIO_Config {
	GPIO_Config_None      = 0, // No configuration (ok for drive high/low)
	GPIO_Config_Pullup    = 1, // Internal pull-up
	GPIO_Config_Pulldown  = 2, // Internal pull-down
	GPIO_Config_Opendrain = 3, // External pull resistor
} GPIO_Config;

// Peripheral selection
typedef enum GPIO_Peripheral {
#if defined(_sam_)
	GPIO_Peripheral_A = PIO_PERIPH_A,
	GPIO_Peripheral_B = PIO_PERIPH_B,
	GPIO_Peripheral_C = PIO_PERIPH_C,
	GPIO_Peripheral_D = PIO_PERIPH_D,
	GPIO_Peripheral_Input = PIO_INPUT,
	GPIO_Peripheral_Output0 = PIO_OUTPUT_0,
	GPIO_Peripheral_Output1 = PIO_OUTPUT_1,
#else
	GPIO_Peripheral_A,
	GPIO_Peripheral_B,
	GPIO_Peripheral_C,
#endif
} GPIO_Peripheral;



// ----- Structs -----

// Struct container for defining Rows (Sense) and Columns (Strobes)
typedef struct GPIO_Pin {
	GPIO_Port    port;
	GPIO_Pin_Num pin;
} GPIO_Pin;

// Struct container for configuring a peripheral
typedef struct GPIO_ConfigPin {
	GPIO_Port       port;
	GPIO_Pin_Num    pin;
	GPIO_Peripheral peripheral;
} GPIO_ConfigPin;



// ----- Functions -----

uint8_t GPIO_Ctrl(GPIO_Pin gpio, GPIO_Type type, GPIO_Config config);
void PIO_Setup(GPIO_ConfigPin config);

