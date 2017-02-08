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

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <print.h>
#include <macro.h>

// Local Includes
#include "scan_loop.h"

// ----- Function Declarations -----

// ----- Variables -----


// ----- Functions -----

uint8_t ps2data_read(){
  return (PS2_PORT_PDIR & (1<<PS2_DATA_PIN)) != 0;
}

void portb_isr(){
  PS2_PORT_ISFR = 0xFFFFFFFF; // reset the interrupt flags.
  ps2interrupt(); // call the PS2 interrupt function.
}

// Setup
inline void Scan_setup() {
  // Enable interrupts for port B.
  NVIC_ENABLE_IRQ(IRQ_PORTB);

  // Set pins to input:
  PS2_PORT_PDDR &= ~(1<<PS2_CLOCK_PIN);
  PS2_PORT_PDDR &= ~(1<<PS2_DATA_PIN);
  PS2_CLOCK_PCR = PORT_PCR_MUX(1);
  PS2_DATA_PCR = PORT_PCR_MUX(1);

  // Register pin 16 to trigger interrupt on port B:
  // falling mask is 0x0A, see pins_teensy.c
  uint32_t mask = (0x0A << 16) | 0x01000000; // 16 is location in register, not pin
    __disable_irq();
  PS2_CLOCK_PCR |= mask;
  __enable_irq();
}


// Main Detection Loop
inline uint8_t Scan_loop() {
        uint8_t c =get_scan_code();
        if (c != 0){
          printHex(c);
        }
	return 0;

}


// Signal from Macro Module that all keys have been processed (that it knows about)
inline void Scan_finishedWithMacro( uint8_t sentKeys ) {
}


// Signal from Output Module that all keys have been processed (that it knows about)
inline void Scan_finishedWithOutput( uint8_t sentKeys ) {
	// Reset scan loop indicator (resets each key debounce state)
	// TODO should this occur after USB send or Macro processing?
}


// Signal from the Output Module that the available current has changed
// current - mA
void Scan_currentChange( unsigned int current )
{
    // Indicate to all submodules current change
}

