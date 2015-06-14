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



// ----- Scan Mode Setting (See matrix_scan.h for more details) -----
#define scanMode scanDual



// ----- Key Settings -----

// -- Example for scanCol --
/*
#define KEYBOARD_KEYS 16 // # of keys
#define MAX_ROW_SIZE  16 // # of keys in the largest row
#define MAX_COL_SIZE   1 // # of keys in the largest column
*/


// -- Example for scanRow --
/*
#define KEYBOARD_KEYS 16 // # of keys
#define MAX_ROW_SIZE   1 // # of keys in the largest row
#define MAX_COL_SIZE  16 // # of keys in the largest column
*/


// -- Example for scanRow_powrCol, scanCol_powrRow, and scanDual --
/*
#define KEYBOARD_KEYS 69 // # of keys
#define MAX_ROW_SIZE   8 // # of keys in the largest row
#define MAX_COL_SIZE   9 // # of keys in the largest column
*/



// ----- Matrix Configuration -----
static const uint8_t matrix_pinout[][MAX_ROW_SIZE + 1] = {



// Just layout the matrix by rows and columns
// Usually you'll want to set the scanMode above to scanDual or scanCol_powrRow/scanRow_powrCol
// The mode allows for optimization in the kind of scanning algorithms that are done
//
// The key numbers are used to translate into the keymap table (array) (and always start from 1, not 0).
// Thus if a row doesn't use all the key positions, you can denote it as 0, which will be ignored/skipped on each scan
// See the keymap.h file for the various preconfigured arrays.

// Scan Mode | Col 1 | Col 2 | Col 3 | Col 4 | Col 5 | ...
// -------------------------------------------------------
//     Row 1 | Key 1   Key 7   Key32    ...
//     Row 2 | Key 3   Key92    ...
//     Row 3 | Key23    ...
//     Row 4 |  ...
//     Row 5 |
//      ...  |


// -- scanCol Example --
/*
  { scanMode, pinF0, pinF4, pinB7, pinD3, pinF5, pinF1, pinD1, pinD2, pinF6, pinF7, pinB2, pinD0, pinB0, pinB6, pinB1, pinB3 },
  { pinNULL,  1,     2,     3,     4,     5,     6,     7,     8,     9,     10,    11,    12,    13,    14,    15,    16    },
*/


// -- scanRow Example --
/*
  { scanMode, pinNULL },
  { pinF0,    1       },
  { pinF4,    2       },
  { pinB7,    3       },
  { pinD3,    4       },
  { pinF5,    5       },
  { pinF1,    6       },
  { pinD1,    7       },
  { pinD2,    8       },
  { pinF6,    9       },
  { pinF7,    10      },
  { pinB2,    11      },
  { pinD0,    12      },
  { pinB0,    13      },
  { pinB6,    14      },
  { pinB1,    15      },
  { pinB3,    16      },
*/


// -- scanRow_powrCol Example and scanCol_powrRow Example --
// The example is the same, as the difference is whether the row or col is powered, and the other is used to detect the signal
/*
  { scanMode, pinF0, pinF4, pinB7, pinD3, pinF5, pinF1, pinD1, pinD2 },
  { pinF6,    1,     2,     3,     4,     5,     6,     7,     8     },
  { pinF7,    9,     10,    11,    12,    13,    14,    15,    16    },
  { pinB2,    17,    20,    30,    40,    50,    60,    59,    38    },
  { pinD0,    18,    21,    31,    41,    51,    61,    67,    39    },
  { pinB0,    19,    22,    32,    42,    52,    62,    68,    47    },
  { pinB6,    27,    23,    33,    43,    53,    63,    69,    48    },
  { pinB1,    28,    24,    34,    44,    54,    64,    0,     49    }, // 0 signifies no key at that location
  { pinB3,    29,    25,    35,    45,    55,    65,    0,     57    },
  { pinA0,    37,    26,    36,    46,    56,    66,    0,     58    },
*/


// -- scanDual Example --
// The example is the same as the previous one, but uses both columns and rows to power and detect, needed for non-NKRO matrices.
/*
  { scanMode, pinF0, pinF4, pinB7, pinD3, pinF5, pinF1, pinD1, pinD2 },
  { pinF6,    1,     2,     3,     4,     5,     6,     7,     8     },
  { pinF7,    9,     10,    11,    12,    13,    14,    15,    16    },
  { pinB2,    17,    20,    30,    40,    50,    60,    59,    38    },
  { pinD0,    18,    21,    31,    41,    51,    61,    67,    39    },
  { pinB0,    19,    22,    32,    42,    52,    62,    68,    47    },
  { pinB6,    27,    23,    33,    43,    53,    63,    69,    48    },
  { pinB1,    28,    24,    34,    44,    54,    64,    0,     49    }, // 0 signifies no key at that location
  { pinB3,    29,    25,    35,    45,    55,    65,    0,     57    },
  { pinA0,    37,    26,    36,    46,    56,    66,    0,     58    },
*/


};

