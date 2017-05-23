/* Copyright (C) 2011,2014 by Jacob Alexander
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
#include <util/delay.h>

// Project Includes
#include <print.h>

// Local Includes
#include "matrix_scan.h"

// Matrix Configuration
#include <matrix.h>



// ----- Macros -----

// -- pinSetup Macros --
#define REG_SET(reg)    reg |=  (1 << ( matrix[row*(MAX_ROW_SIZE+1)+col] % 10 ) ) // Modulo 10 for the define offset for each pin set 12 or 32 -> shift of 2
#define REG_UNSET(reg)  reg &= ~(1 << ( matrix[row*(MAX_ROW_SIZE+1)+col] % 10 ) )

#define PIN_SET(pin,scan,direction) \
			switch ( direction ) { \
			case columnSet: PIN_SET_COL(pin,scan); \
			case rowSet:    PIN_SET_ROW(pin,scan); \
			} \
			break

// TODO Only scanCol_powrRow Tested (and powrRow)
#define PIN_SET_COL(pin,scan) \
			switch ( scan ) { \
			case scanCol: \
			case scanRow_powrCol: \
			case scanDual: \
				REG_SET(port##pin); break; \
			case scanCol_powrRow: REG_UNSET(ddr##pin); REG_UNSET(DDR##pin); \
		                              REG_SET(port##pin);  REG_SET(PORT##pin); break; \
			case powrRow: break; \
			case powrCol: REG_SET(ddr##pin);  REG_SET(DDR##pin); \
			              REG_SET(port##pin); REG_SET(PORT##pin); break; \
			} \
			break

// TODO Only scanCol_powrRow Tested (and powrRow)
#define PIN_SET_ROW(pin,scan) \
			switch ( scan ) { \
			case scanRow_powrCol: REG_UNSET(ddr##pin); REG_SET(port##pin); break; \
			case scanRow: \
			case scanDual: \
				REG_SET(port##pin); break; \
			case scanCol_powrRow: REG_SET(ddr##pin);    REG_SET(DDR##pin); \
					      REG_UNSET(port##pin); REG_UNSET(PORT##pin); break; \
			case powrRow: REG_SET(ddr##pin);  REG_SET(DDR##pin); \
				      REG_SET(port##pin); REG_SET(PORT##pin); break; \
			case powrCol: break; \
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

// -- Column Scan Macros --
#define PIN_TEST_COL(pin) \
			scanCode = matrix[row*(MAX_ROW_SIZE+1)+col]; \
			if ( scanCode && !( pin & ( 1 << ( matrix[0*(MAX_ROW_SIZE+1)+col] % 10 ) ) ) ) \
			{ \
				detectArray[scanCode]++; \
			} \
			break

// -- Row Scan Macros --
#define PIN_TEST_ROW(pin) \
			scanCode = matrix[row*(MAX_ROW_SIZE+1)+col]; \
			if ( scanCode && !( pin & ( 1 << ( matrix[row*(MAX_ROW_SIZE+1)+0] % 10 ) ) ) ) \
			{ \
				detectArray[scanCode]++; \
			} \
			break

// -- Scan Dual Macros --
#define PIN_DUALTEST_ROW(pin) \
			scanCode = matrix[row*(MAX_ROW_SIZE+1)+col]; \
			if ( scanCode \
			  && !( pin & ( 1 << ( matrix[row*(MAX_ROW_SIZE+1)+0] % 10 ) ) ) \
			  && detectArray[scanCode] & 0x01 ) \
			{ \
				detectArray[scanCode]++; \
			} \
			else \
			{ \
				if ( detectArray[scanCode] & 0x01 ) \
					detectArray[scanCode]--; \
			} \
			break



// ----- Variables -----
uint8_t showDebug = 0;

// Debug Variables for GPIO setting
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


// ----- Functions -----
void matrix_debugPins(void);

// Pin Setup Debug
inline void matrix_debugPins()
{
	char tmpStr[6];
	info_print("Initial Matrix Pin Setup");
	info_print(" ddrA  ddrB  ddrC  ddrD  ddrE  ddrF");
	print("      ");
	hexToStr_op( ddrA, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( ddrB, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( ddrC, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( ddrD, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( ddrE, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( ddrF, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	print("\n");
	info_print("portA portB portC portD portE portF");
	print("      ");
	hexToStr_op( portA, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( portB, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( portC, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( portD, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( portE, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	hexToStr_op( portF, tmpStr, 2 ); dPrintStrs( "  0x", tmpStr );
	print("\n");

	showDebug++;
}


// Column Setup
inline void matrix_columnSet( uint8_t *matrix, uint8_t scanType, uint16_t startIndex, uint16_t colsToIterate )
{
	// Calculate the number of pins to iterate over
	uint8_t maxColumns = startIndex + colsToIterate - 1;
	if ( maxColumns > MAX_COL_SIZE )
		maxColumns = MAX_COL_SIZE;

	uint16_t row, col;

	// Columns
	for ( col = startIndex, row = 0; col <= maxColumns; col++ )
	{
		// We can't pass 2D arrays, so just point to the first element and calculate directly
		switch ( matrix[row*(MAX_ROW_SIZE+1)+col] )
		{
#if defined(__AVR_AT90USB1286__)
		PIN_CASE(A):
			PIN_SET(A, scanType, columnSet);
#endif
		PIN_CASE(B):
			PIN_SET(B, scanType, columnSet);
		PIN_CASE(C):
			PIN_SET(C, scanType, columnSet);
		PIN_CASE(D):
			PIN_SET(D, scanType, columnSet);
		PIN_CASE(E):
			PIN_SET(E, scanType, columnSet);
		PIN_CASE(F):
			PIN_SET(F, scanType, columnSet);

		default:
			continue;
		}
	}
}

// Row Setup
inline void matrix_rowSet( uint8_t *matrix, uint8_t scanType, uint16_t startIndex, uint8_t rowsToIterate )
{
	// Calculate the number of pins to iterate over
	uint16_t maxRows = startIndex + rowsToIterate - 1;
	if ( maxRows > MAX_ROW_SIZE )
		maxRows = MAX_ROW_SIZE;

	uint16_t row, col;

	// Rows
	for ( col = 0, row = startIndex; row <= maxRows; row++ )
	{
		// We can't pass 2D arrays, so just point to the first element and calculate directly
		switch ( matrix[row*(MAX_ROW_SIZE+1)+col] )
		{
#if defined(__AVR_AT90USB1286__)
		PIN_CASE(A):
			PIN_SET(A, scanType, rowSet);
#endif
		PIN_CASE(B):
			PIN_SET(B, scanType, rowSet);
		PIN_CASE(C):
			PIN_SET(C, scanType, rowSet);
		PIN_CASE(D):
			PIN_SET(D, scanType, rowSet);
		PIN_CASE(E):
			PIN_SET(E, scanType, rowSet);
		PIN_CASE(F):
			PIN_SET(F, scanType, rowSet);

		default:
			continue;
		}
	}
}


// Goes through the defined matrix and matrix mode, and sets the initial state of all of the available pins
void matrix_pinSetup( uint8_t *matrix, uint8_t scanType )
{
	// Loop through all the pin assignments, for the initial pin settings
	matrix_rowSet   ( matrix, scanType, 1, MAX_ROW_SIZE );
	matrix_columnSet( matrix, scanType, 1, MAX_COL_SIZE );

	// Pin Status
	if ( showDebug == 0 ) // Only show once
	{
		matrix_debugPins();
	}	
}

// Scans the given matrix determined by the scanMode method
inline void matrix_scan( uint8_t *matrix, uint8_t *detectArray )
{
	// Loop variables for all modes
	uint16_t col = 1;
	uint16_t row = 1;
	uint16_t scanCode = 0;


	// TODO Only scanCol_powrRow tested
	// Column Scan and Column Scan, Power Row
#if scanMode == scanCol || scanMode == scanCol_powrRow
	for ( ; row <= MAX_ROW_SIZE; row++ )
	{
		// Power each row separately
		matrix_rowSet( matrix, powrRow, row, 1 );

		for ( col = 1; col <= MAX_COL_SIZE; col++ )
		{
			// Scan over the pins for each of the columns, and using the pin alias to determine which pin to set
			// (e.g. / 10 is for the pin name (A,B,C,etc.) and % 10 is for the position of the pin (A1,A2,etc.))
			switch ( matrix[0*(MAX_ROW_SIZE+1)+col] / 10 )
			{
#if defined(__AVR_AT90USB1286__)
			case 0: // PINA
				PIN_TEST_COL(PINA);
#endif
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

		// Unset the row power
		matrix_rowSet( matrix, scanMode, row, 1 );
	}
#endif // scanMode


	// Row Scan and Row Scan, Power Row
#if scanMode == scanRow || scanMode == scanRow_powrCol
	for ( ; col <= MAX_COL_SIZE; col++ )
	{
		// Power each column separately
		matrix_columnSet( matrix, powrCol, col, 1 );

		for ( row = 1; row <= MAX_ROW_SIZE; row++ )
		{
			// Scan over the pins for each of the rows, and using the pin alias to determine which pin to set
			// (e.g. / 10 is for the pin name (A,B,C,etc.) and % 10 is for the position of the pin (A1,A2,etc.))
			switch ( matrix[row*(MAX_ROW_SIZE+1)+0] / 10 )
			{
#if defined(__AVR_AT90USB1286__)
			case 0: // PINA
				PIN_TEST_ROW(PINA);
#endif
			case 1: // PINB
				PIN_TEST_ROW(PINB);
			case 2: // PINC
				PIN_TEST_ROW(PINC);
			case 3: // PIND
				PIN_TEST_ROW(PIND);
			case 4: // PINE
				PIN_TEST_ROW(PINE);
			case 5: // PINF
				PIN_TEST_ROW(PINF);
			}
		}

		// Unset the column power
		matrix_columnSet( matrix, scanMode, col, 1 );
	}
#endif // scanMode


	// Dual Scan
#if scanMode == scanDual
	// First do a scan of all of the columns, marking each one
	//matrix_pinSetup( matrix, scanCol_powrRow, 0, MAX_ROW_SIZE, MAX_COL_SIZE ); TODO
	_delay_us( 1 );
	for ( ; row < (MAX_COL_SIZE+1); row++ ) for ( ; col < (MAX_ROW_SIZE+1); col++ )
	{
		// Scan over the pins for each of the columns, and using the pin alias to determine which pin to set
		// (e.g. / 10 is for the pin name (A,B,C,etc.) and % 10 is for the position of the pin (A1,A2,etc.))
		switch ( matrix[0*(MAX_ROW_SIZE+1)+col] / 10 )
		{
#if defined(__AVR_AT90USB1286__)
		case 0: // PINA
			PIN_TEST_COL(PINA);
#endif
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

	// Next, do a scan of all of the rows, clearing any "vague" keys (only detected on row, but not column, or vice-versa)
	// And marking any keys that are detected on the row and column
	//matrix_pinSetup( matrix, scanRow_powrCol, 0, MAX_ROW_SIZE, MAX_COL_SIZE ); TODO
	_delay_us( 1 );
	col = 1;
	row = 1;
	for ( ; col < (MAX_ROW_SIZE+1); col++ ) for ( ; row < (MAX_COL_SIZE+1); row++ )	
	{
		// Scan over the pins for each of the rows, and using the pin alias to determine which pin to set
		// (e.g. / 10 is for the pin name (A,B,C,etc.) and % 10 is for the position of the pin (A1,A2,etc.))
		switch ( matrix[row*(MAX_ROW_SIZE+1)+0] / 10 )
		{
#if defined(__AVR_AT90USB1286__)
		case 0: // PINA
			PIN_DUALTEST_ROW(PINA);
#endif
		case 1: // PINB
			PIN_DUALTEST_ROW(PINB);
		case 2: // PINC
			PIN_DUALTEST_ROW(PINC);
		case 3: // PIND
			PIN_DUALTEST_ROW(PIND);
		case 4: // PINE
			PIN_DUALTEST_ROW(PINE);
		case 5: // PINF
			PIN_DUALTEST_ROW(PINF);
		}
	}
#endif
}

