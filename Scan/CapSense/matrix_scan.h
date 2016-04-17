/* Copyright (C) 2016 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// ----- Includes -----

// KLL Generated Defines
#include <kll_defs.h>



// ----- Defines -----

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

// Depending on the microcontroller, it can have 1 or more ADCs
typedef enum ADC {
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_)
	ADC_0 = 0,
#elif defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
	ADC_0 = 0,
	ADC_1 = 1,
#endif
} ADC;

// ADC Register offset map
unsigned int *ADC_reg_offset_map[] = {
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_)
	(unsigned int*)(&ADC0_SC1A),
#elif defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
	(unsigned int*)(&ADC0_SC1A),
	(unsigned int*)(&ADC1_SC1A),
#endif
};

// Each ADC has a possible 32 channels
typedef enum Channel {
	Channel_0  = 0,
	Channel_1  = 1,
	Channel_2  = 2,
	Channel_3  = 3,
	Channel_4  = 4,
	Channel_5  = 5,
	Channel_6  = 6,
	Channel_7  = 7,
	Channel_8  = 8,
	Channel_9  = 9,
	Channel_10 = 10,
	Channel_11 = 11,
	Channel_12 = 12,
	Channel_13 = 13,
	Channel_14 = 14,
	Channel_15 = 15,
	Channel_16 = 16,
	Channel_17 = 17,
	Channel_18 = 18,
	Channel_19 = 19,
	Channel_20 = 20,
	Channel_21 = 21,
	Channel_22 = 22,
	Channel_23 = 23,
	Channel_24 = 24,
	Channel_25 = 25,
	Channel_26 = 26,
	Channel_27 = 27,
	Channel_28 = 28,
	Channel_29 = 29,
	Channel_30 = 30,
	Channel_31 = 31,
} Channel;

// Type of pin
typedef enum Type {
	Type_StrobeOn,
	Type_StrobeOff,
	Type_StrobeSetup,
	Type_Sense,
	Type_SenseSetup,
} Type;

// Keypress States
typedef enum KeyPosition {
	KeyState_Off     = 0,
	KeyState_Press   = 1,
	KeyState_Hold    = 2,
	KeyState_Release = 3,
	KeyState_Invalid,
} KeyPosition;



// ----- Structs -----

// Struct container for defining Strobe pins
typedef struct GPIO_Pin {
	Port port;
	Pin  pin;
} GPIO_Pin;

// Struct container for defining Sense pins
typedef struct ADC_Pin {
	Port    port;
	Pin     pin;
	ADC     adc;
	Channel ch;
} ADC_Pin;



// ----- Functions -----

void Matrix_setup();
void Matrix_scan( uint16_t scanNum );

void Matrix_currentChange( unsigned int current );

