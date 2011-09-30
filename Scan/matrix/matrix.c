/* Copyright (C) 2011 by Jacob Alexander
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

// ----- Includes -----

// AVR Includes
#include <avr/io.h>

// Local Includes
#include "matrix.h"



// ----- Macros -----

#define REG_SET(reg)	reg |= (1 << ( matrix[row][col] % 10 ) )
			
#define PIN_SET_COL(pin) \
			switch ( scanMode ) { \
			case scanCol: \
			case scanCol_powrRow: \
			case scanDual: \
				REG_SET(port##pin); break; \
			case scanRow_powrCol: REG_SET(ddr##pin); REG_SET(port##pin); break; \
			} \
			break

#define PIN_SET_ROW(pin) \
			switch ( scanMode ) { \
			case scanRow: \
			case scanRow_powrCol: \
			case scanDual: \
				REG_SET(port##pin); break; \
			case scanCol_powrRow: REG_SET(ddr##pin); REG_SET(port##pin); break; \
			} \
			break

#define PIN_CASE(pinLetter) \
			case pin##pinLetter##0: \
			case pin##pinLetter##1: \
			case pin##pinLetter##2: \
			case pin##pinLetter##3: \
			case pin##pinLetter##4: \
			case pin##pinLetter##5: \
			case pin##pinLetter##6: \
			case pin##pinLetter##7

#define PIN_TEST_COL(pin) \
			if ( !( pin & ( 1 << ( matrix[0][col] % 10 ) ) \
				detectArray[matrix[row][col]]++; \
			break



// ----- Variables -----

uint8_t KeyIndex_Array[KEYBOARD_SIZE + 1];



// ----- Functions -----

void matrix_pinSetup( uint8_t *matrix )
{
	// Setup the variables
	uint8_t portA = 0x00;
	uint8_t portB = 0x00;
	uint8_t portC = 0x00;
	uint8_t portD = 0x00;
	uint8_t portE = 0x00;
	uint8_t portF = 0x00;

	uint8_t ddrA = 0x00;
	uint8_t ddrB = 0x00;
	uint8_t ddrC = 0x00;
	uint8_t ddrD = 0x00;
	uint8_t ddrE = 0x00;
	uint8_t ddrF = 0x00;

	// Loop through all the pin assignments, for the initial pin settings
	//int row, col;

	// Rows
	/*
	for ( row = 1; row < sizeof(matrix); row++ ) {
		switch ( matrix[row][col] ) {
		PIN_CASE(A):
			PIN_SET_ROW(A);
		PIN_CASE(B):
			PIN_SET_ROW(B);
		PIN_CASE(C):
			PIN_SET_ROW(C);
		PIN_CASE(D):
			PIN_SET_ROW(D);
		PIN_CASE(E):
			PIN_SET_ROW(E);
		PIN_CASE(F):
			PIN_SET_ROW(F);

		default:
			continue;
		}
	}

	// Columns
	for ( col = 1; col < sizeof(matrix[0]); row++ ) {
		switch ( matrix[row][col] ) {
		PIN_CASE(A):
			PIN_SET_COL(A);
		PIN_CASE(B):
			PIN_SET_COL(B);
		PIN_CASE(C):
			PIN_SET_COL(C);
		PIN_CASE(D):
			PIN_SET_COL(D);
		PIN_CASE(E):
			PIN_SET_COL(E);
		PIN_CASE(F):
			PIN_SET_COL(F);

		default:
			continue;
		}
	}
	*/

	// Setting the pins
	DDRA = ddrA;
	DDRB = ddrB;
	DDRC = ddrC;
	DDRD = ddrD;
	DDRE = ddrE;
	DDRF = ddrF;

	PORTA = portA;
	PORTB = portB;
	PORTC = portC;
	PORTD = portD;
	PORTE = portE;
	PORTF = portF;
}

// TODO Proper matrix scanning
void matrix_scan( uint8_t *matrix, uint8_t *detectArray )
{
	// Column Scan
#if scanMode == scanCol
	/*
	uint8_t col = 1;
	uint8_t row = 1;
	for ( ; col < sizeof(matrix[1]); col++ ) {
		switch ( matrix[0][col] / 10 ) {
		case 0: // PINA
			PIN_TEST_COL(PINA);
		case 1: // PINB
			PIN_TEST_COL(PINB);
		case 2: // PINC
			PIN_TEST_COL(PINC);
		case 3: // PIND
			PIN_TEST_COL(PIND);
		case 4: // PINE
			PIN_TEST_COL(PINE);
		case 5: // PINF
			PIN_TEST_COL(PINF);
		}
	}
	*/
#endif

	// Row Scan
#if scanMode == scanRow
#endif

	// Column Scan, Power Row
#if scanMode == scanCol_powrRow
#endif

	// Row Scan, Power Column
#if scanMode == scanRow_powrCol
#endif

	// Dual Scan
#if scanMode == scanDual
#endif
}

