/*
  The MIT License (MIT)
  Copyright (c) 2017 Ivor Wanders
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#pragma once

// ----- Includes -----

// Compiler Includes
#include <stdint.h>
#include <cli.h>
#include <print.h>
#include "PS2Keyboard.h"

// ----- Defines -----

// Specify which GPIO pins to use.

// Use port B:
// Set the PDDR, PDIR and ISFR defines to their respective port B ones.
#define PS2_PORT_PDDR GPIOB_PDDR
#define PS2_PORT_PDIR GPIOB_PDIR
#define PS2_PORT_ISFR PORTB_ISFR

// Clock is on Port B, pin 17.
#define PS2_CLOCK_PIN 17
#define PS2_CLOCK_PCR PORTB_PCR17

// Data is on port B, pin 16.
#define PS2_DATA_PIN 16
#define PS2_DATA_PCR PORTB_PCR16



// ----- Functions to used by PS2Keyboard -----
uint8_t ps2data_read();

// ----- Functions -----

// Functions to be called by main.c
void Scan_setup();
uint8_t Scan_loop();


// Call-backs
void Scan_finishedWithMacro(uint8_t sentKeys);  // Called by Macro Module
void Scan_finishedWithOutput(uint8_t sentKeys);  // Called by Output Module

void Scan_currentChange(unsigned int current);  // Called by Output Module

