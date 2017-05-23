/* Copyright (C) 2012,2015 by Jacob Alexander
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



// ----- Scan Mode Setting (See matrix_scan.h for more details) -----
#define scanMode scanCol_powrRow



// ----- Key Settings -----

#define KEYBOARD_KEYS 90 // # of keys (It actually has 78, but there are markings up to 81 on the PCB and scan lines enough for 90
#define MAX_ROW_SIZE   6 // # of rows
#define MAX_COL_SIZE  15 // # of columns



// ----- Matrix Configuration -----
static const uint8_t matrix_pinout[][MAX_COL_SIZE + 1] = {


// GND Pins
// Board Pins: 2, 8, 14, 20, 24
// OO2 Labels: 3, 7, 13, 19, 25

// IBM Convertible PCB Matrix
// Board Pins labeled as ()
// Board Pins: 26     25     23     22     21     19     17     15     13     11      9      7      5      3      1
// OO2 Labels:  1      2      4      5      6      8     10     12     14     16     18     20     22     24     26
//             C1     C2     C3     C4     C5     C6     C7     C8     C9     C10    C11    C12    C13    C14    C15
  { scanMode, pinC0, pinC1, pinC2, pinC3, pinC4, pinC5, pinC6, pinC7, pinF0, pinF1, pinF2, pinF3, pinF4, pinF5, pinF6 },
  { pinE1,    80,    79,    78,    81,    77,    76,    75,    74,    73,    72,    71,    70,    69,    68,    67,   }, // R1 -  9 (18)
  { pinE2,    14,    13,    12,    15,    11,    10,    9,     8,     7,     6,     5,     4,     3,     2,     1,    }, // R2 - 11 (16)
  { pinE3,    0,     28,    27,    0,     26,    25,    24,    23,    22,    21,    20,    19,    18,    17,    16,   }, // R3 - 15 (12)
  { pinE4,    43,    0,     41,    0,     40,    39,    38,    37,    36,    35,    34,    33,    32,    31,    30,   }, // R4 - 17 (10)
  { pinE5,    57,    0,     56,    0,     55,    54,    53,    52,    51,    50,    49,    48,    47,    46,    44,   }, // R5 - 21 (6)
  { pinE6,    65,    0,     64,    66,    63,    62,    0,     0,     0,     61,    0,     0,     60,    59,    58,   }, // R6 - 23 (4)



};

