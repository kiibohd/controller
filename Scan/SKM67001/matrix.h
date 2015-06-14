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

#define KEYBOARD_KEYS 85 // # of keys (It actually has 66, but there are markings up to 80 on the PCB); 85 due to there being 5 "switch" keys, that have no numbers
#define MAX_ROW_SIZE   9 // # of keys in the largest row
#define MAX_COL_SIZE   9 // # of keys in the largest column



// ----- Matrix Configuration -----
static const uint8_t matrix_pinout[][MAX_ROW_SIZE + 1] = {


// SKM Typewriter PCB Matrix
// Note: Pins 50, 51, and 52 are connected together (LShift, RShift, and Lock)
// Board Pins: 13      5     12      6     11      9      8      7     10
  { scanMode, pinC0, pinC7, pinC4, pinC2, pinC6, pinC5, pinC3, pinE1, pinC1, },
  { pinE6,    71,    72,    73,    74,    75,    76,    80,    55,    53,    }, //  1 - White
  { pinF7,    43,    81,    45,    41,    54,    44,    46,    58,    42,    }, //  2 - Red
  { pinF4,    37,    82,    39,    35,    34,    38,    33,    36,    40,    }, //  3 - Pink
  { pinF5,    31,    83,    25,    28,    27,    32,    26,    30,    29,    }, //  4 - Black
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, //  5 - Blue
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, //  6 - Red / Blue
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, //  7 - White / Green
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, //  8 - Grey / Pink
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, //  9 - Brown / Green
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, // 10 - Brown / Grey
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, // 11 - White / Grey
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, // 12 - Yellow / White
//{ pinXX,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, // 13 - Brown / Yellow
  { pinF2,    0,     0,     0,     0,     0,     0,     0,     0,     0,     }, // 14 - Yellow
  { pinF0,    23,    84,    19,    20,    64,    24,    57,    22,    21,    }, // 15 - Purple
  { pinF1,    17,    85,    12,    14,    13,    11,    18,    16,    15,    }, // 16 - Brown
  { pinF3,    62,    52,    49,    1,     47,    61,    48,    3,     2,     }, // 17 - Green
  { pinF6,    4,     63,    6,     8,     7,     5,     56,    9,     10,    }, // 18 - Grey



};

