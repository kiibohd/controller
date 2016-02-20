/* Copyright (C) 2014-2016 by Jacob Alexander
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

// Project Includes
#include <matrix_setup.h>



// ----- Matrix Definition -----

// Freescale ARM MK20's support GPIO PTA, PTB, PTC, PTD and PTE 0..31
// Not all chips have access to all of these pins (most don't have 160 pins :P)
//
// NOTE:
// Before using a pin, make sure it supports being a GPIO *and* doesn't have a default pull-up/pull-down
// Checking this is completely on the ownness of the user

// MD1
//
// Columns (Strobe)
//  PTB0..3,16,17
//  PTC4,5
//  PTD0
//
// Rows (Sense)
//  PTD1..7

// Define Rows (Sense) and Columns (Strobes)
GPIO_Pin Matrix_cols[] = { gpio(C,0), gpio(C,1), gpio(C,2), gpio(C,3), gpio(C,4), gpio(C,5), gpio(C,6), gpio(C,7), gpio(D,0) };
GPIO_Pin Matrix_rows[] = { gpio(D,1), gpio(D,2), gpio(D,3), gpio(D,4), gpio(D,5), gpio(D,6), gpio(D,7) };

// Define type of scan matrix
Config Matrix_type = Config_Pulldown;

