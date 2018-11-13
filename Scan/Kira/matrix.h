/* Copyright (C) 2014-2018 by Jacob Alexander
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

// NOTE:
// Before using a pin, make sure it supports being a GPIO *and* doesn't have a default pull-up/pull-down
// Checking this is completely on the ownness of the user

// Define Rows (Sense) and Columns (Strobes)
GPIO_Pin Matrix_cols[] = { gpio(B,1), gpio(B,2), gpio(B,3), gpio(A,18), gpio(A,19), gpio(A,23), gpio(A,20), gpio(A,11), gpio(A,8), gpio(A,7), gpio(A,6), gpio(A,5), gpio(A,27), gpio(A,28), gpio(A,29), gpio(A,30), gpio(A,2), gpio(A,1), gpio(A,0) };
GPIO_Pin Matrix_rows[] = { gpio(A,26), gpio(A,25), gpio(A,24), gpio(A,13), gpio(A,14), gpio(A,31) };

// Define type of scan matrix
GPIO_Config Matrix_type = GPIO_Config_Pulldown;

