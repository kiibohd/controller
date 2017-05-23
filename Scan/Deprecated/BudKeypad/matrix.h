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
#define scanMode scanCol



// ----- Key Settings -----

#define KEYBOARD_KEYS 16 // # of keys
#define MAX_ROW_SIZE  16 // # of keys in the largest row
#define MAX_COL_SIZE   1 // # of keys in the largest column



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

// See Scan/matrix/matrix_template.h for more details


  { scanMode, pinF0, pinF4, pinB7, pinD3, pinF5, pinF1, pinD1, pinD2, pinF6, pinF7, pinB2, pinD0, pinB0, pinB6, pinB1, pinB3 },
  { pinNULL,  1,     2,     3,     4,     5,     6,     7,     8,     9,     10,    11,    12,    13,    14,    15,    16    },



};

