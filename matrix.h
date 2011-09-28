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

#ifndef __MATRIX_H
#define __MATRIX_H

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
#define scanRow_powrCol 2  // NKRO supported (simple detection)
#define scanCol_powrRow 3  // Opposite of scanRow_powrCol
#define scanDual        4  // Typical ~2KRO matrix



// ----- Scan Mode Setting -----
#define scanMode scanCol



// ----- Key Settings -----
#define keyboardSize 16  // # of keys



// ----- Matrix Configuration -----
static uint8_t matrix_pinout[][] = {



// Just layout the matrix by rows and columns
// Usually you'll want to set the scanMode above to scanDual or scanCol_powrRow/scanRow_powrCol
// The mode allows for optimization in the kind of scanning algorithms that are done
// 
// The key numbers are used to translate into the keymap table (array) (and always start from 1, not 0).
// See the keymap.h file for the various preconfigured arrays.

// Scan Mode | Col 1 | Col 2 | Col 3 | Col 4 | Col 4 | ...
// -------------------------------------------------------
//     Row 1 | Key 1   Key 7   Key32    ...
//     Row 2 | Key 3   Key92    ...
//     Row 3 | Key23    ...
//     Row 4 |  ...
//     Row 5 |
//      ...  |


  { scanMode, pinF4, pinA6, pinA1, pinA3, pinF5, pinA5, pinA2, pinF0, pinF6, pinA7, pinA0, pinF1, pinF3, pinF7, pinA4, pinF2 },
  { pinNULL,  1,     2,     3,     4,     5,     6,     7,     8,     9,     10,    11,    12,    13,    14,    15,    16    },


// Example Rows
//{ pinE0,    1,     2,     3,     4,     5,     6,     7,     8,     9,     10,    11,    12,    13,    14,    15,    16    },
//{ pinE1,   21,    22,    23,    24,    25,    26,    27,    28,    29,     30,    31,    32,    33,    34,                 },


};


// ----- Variables -----

// NOTE: Highest Bit: Valid keypress (0x80 is valid keypress)
//        Other Bits: Pressed state sample counter
uint8_t keyboardDetectArray[keyboardSize + 1];



// ----- Functions -----


#endif // __MATRIX_H


