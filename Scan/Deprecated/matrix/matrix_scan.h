/* Copyright (C) 2011,2014-2015 by Jacob Alexander
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

// Local Includes



// ----- Defines -----

// ----- Quick Map (don't change) -----
#define pinA0  0
#define pinA1  1
#define pinA2  2
#define pinA3  3
#define pinA4  4
#define pinA5  5
#define pinA6  6
#define pinA7  7

#define pinB0 10
#define pinB1 11
#define pinB2 12
#define pinB3 13
#define pinB4 14
#define pinB5 15
#define pinB6 16
#define pinB7 17

#define pinC0 20
#define pinC1 21
#define pinC2 22
#define pinC3 23
#define pinC4 24
#define pinC5 25
#define pinC6 26
#define pinC7 27

#define pinD0 30
#define pinD1 31
#define pinD2 32
#define pinD3 33
#define pinD4 34
#define pinD5 35
#define pinD6 36
#define pinD7 37

#define pinE0 40
#define pinE1 41
#define pinE2 42
#define pinE3 43
#define pinE4 44
#define pinE5 45
#define pinE6 46
#define pinE7 47

#define pinF0 50
#define pinF1 51
#define pinF2 52
#define pinF3 53
#define pinF4 54
#define pinF5 55
#define pinF6 56
#define pinF7 57

#define pinNULL 128



// ----- Scan Mode (usually dual-scan) -----
// Ordered by increasing memory/CPU usage
#define scanRow         0  // Needed for powered switches (Hall-Effect)
#define scanCol         1  // Opposite of scanRow
#define scanRow_powrCol 2  // NKRO supported matrix (simple detection)
#define scanCol_powrRow 3  // Opposite of scanRow_powrCol
#define scanDual        4  // Typical ~2KRO matrix

#define powrRow         5  // Matrix setup for powering a row, initially the row would be set low
#define powrCol         6  // Like powrRow but for columns


// ----- Direction -----
#define columnSet       0  // PIN_SET_COL for PIN_SET
#define rowSet          1  // PIN_SET_ROW for PIN_SET



// ----- Variables -----



// ----- Functions -----

void matrix_pinSetup( uint8_t *matrix, uint8_t scanType );
void matrix_scan( uint8_t *matrix, uint8_t *detectArray );

