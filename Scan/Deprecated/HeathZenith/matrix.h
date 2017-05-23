/* Copyright (C) 2011,2015 by Jacob Alexander
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
#define scanMode scanDual



// ----- Key Settings -----

#define KEYBOARD_KEYS 63 // # of keys
#define MAX_ROW_SIZE  12 // # of keys in the largest row
#define MAX_COL_SIZE   9 // # of keys in the largest column



// ----- Matrix Configuration -----
static const uint8_t matrix_pinout[][MAX_ROW_SIZE + 1] = {


// Bread-board debug pinout
// Note: Pins 49 and 60 are connected together, by row AND column, why? dunno...(shift)
  { scanMode, pinC6, pinC5, pinC4, pinC3, pinC2, pinE1, pinC0, pinC1, pinD7, pinE0, pinD6, pinC7 },
  { pinF3,    1,     2,     3,     4,     5,     6,     7,     8,     0,     0,     0,     0     },
  { pinE7,    16,    15,    14,    13,    12,    11,    10,    9,     0,     0,     0,     0     },
  { pinB4,    17,    18,    19,    20,    21,    22,    23,    24,    0,     0,     0,     0     },
  { pinB0,    32,    31,    30,    29,    28,    27,    26,    25,    0,     0,     0,     0     },
  { pinB2,    35,    36,    37,    38,    39,    40,    41,    42,    0,     0,     0,     0     },
  { pinB1,    47,    61,    46,    45,    44,    43,    58,    0,     0,     0,     0,     0     },
  { pinB5,    50,    51,    52,    53,    54,    55,    56,    57,    0,     0,     0,     0     },
  { pinE6,    62,    63,    0,     0,     59,    0,     0,     0,     0,     0,     0,     0     },
  { pinB6,    0,     0,     0,     0,     0,     0,     0,     0,     33,    34,    48,    49    },



};

