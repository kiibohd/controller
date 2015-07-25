/* Copyright (C) 2015 by Jacob Alexander
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


// ----- Variables -----

// A bit mask determining which LEDs are enabled in the ISSI chip
// Infinity ErgoDox full mask
// 0x00 -> 0x11
/*
const uint8_t LED_ledEnableMask[] = {
0xE8, // I2C address
0x00, // Starting register address
0x37, 0x37, // C1-1 -> C1-16
0xBF, 0xBF, // C2-1 -> C2-16
0xFF, 0xFF, // C3-1 -> C3-16
0xEF, 0xEF, // C4-1 -> C4-16
0xF7, 0xF7, // C5-1 -> C5-16
0xFF, 0xFF, // C6-1 -> C6-16
0xF3, 0xF3, // C7-1 -> C7-16
0x6C, 0x6C, // C8-1 -> C8-16
0x24, 0x24, // C9-1 -> C9-16
};
*/
/*
const uint8_t LED_ledEnableMask[] = {
0xE8, // I2C address
0x00, // Starting register address
0x00, 0x00, // C1-1 -> C1-16
//0xEC, 0xEC, // C1-1 -> C1-16
//0xFD, 0xFD, // C2-1 -> C2-16
0x00, 0x00, // C3-1 -> C3-16
0x00, 0x00, // C4-1 -> C4-16
0x00, 0x00, // C5-1 -> C5-16
0x00, 0x00, // C6-1 -> C6-16
0x00, 0x00, // C7-1 -> C7-16
0x00, 0x00, // C8-1 -> C8-16
0x00, 0x00, // C9-1 -> C9-16
};
*/
const uint8_t LED_ledEnableMask[] = {
0xE8, // I2C address
0x00, // Starting register address
0xFF, 0xFF, // C1-1 -> C1-16
0xFF, 0xFF, // C2-1 -> C2-16
0xFF, 0xFF, // C3-1 -> C3-16
0xFF, 0xFF, // C4-1 -> C4-16
0xFF, 0xFF, // C5-1 -> C5-16
0xFF, 0xFF, // C6-1 -> C6-16
0xFF, 0xFF, // C7-1 -> C7-16
0xFF, 0xFF, // C8-1 -> C8-16
0xFF, 0xFF, // C9-1 -> C9-16
};

