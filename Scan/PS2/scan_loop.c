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
#include "./scan_loop.h"

// ----- Function Declarations -----

// CLI Functions
void cliFunc_ps2Verbose(char* args);

// ----- Variables -----
CLIDict_Entry(ps2Verbose, "Output the PS/2 Scancodes received.");

CLIDict_Def(scanCLIDict, "Scan Module Commands") = {
	CLIDict_Item(ps2Verbose),
	{ 0, 0, 0 }  // Null entry for dictionary end
};

static uint8_t verbose_state = 0;
// ----- Functions -----

uint8_t ps2data_read() {
#if defined(_teensy_3_)
	return (PS2_PORT_PDIR & (1 << PS2_DATA_PIN)) != 0;
#else
#warning "Not implemented"
#endif
}

void portb_isr() {
#if defined(_teensy_3_)
	PS2_PORT_ISFR = 0xFFFFFFFF;  // reset the interrupt flags.
	ps2interrupt();  // call the PS2 interrupt function.
#else
#warning "Not implemented"
#endif
}



void ScancodeDispatch() {
	static uint8_t state = 0x01;  // pressed
	static uint8_t is_extended = 0;
	while (1)
	{
		uint8_t c = get_scan_code();
		if (c)
		{
			if (verbose_state)
			{
				printHex(c);
				print(" ");
			}
			switch (c)
			{
				case 0xF0:  // F0, is release for next scancode.
					state = 0x03;  // release
					break;
				case 0xE0:  // extended scancode.
					is_extended = 1;
					break;
				default:
					// printHex(c);
					// printHex(state);
					// print("  ");
					if (is_extended) {
						// map the extended values to 0x80+c, this conveniently does not
						// collide with the non-extended set.
						c = 0x80 + c;
					}
				Macro_keyState(c, state);  // send the new state
				state = 0x01;  // default state is pressed
				is_extended = 0;  // default is non extended
			}
		}
		else
		{
			return;
		}
	}
}



// Setup
inline void Scan_setup() {
	// Register Scan CLI dictionary
	CLI_registerDictionary(scanCLIDict, scanCLIDictName);

#if defined(_teensy_3_)
	// Enable interrupts for port B.
	NVIC_ENABLE_IRQ(IRQ_PORTB);

	// Set pins to input:
	PS2_PORT_PDDR &= ~(1 << PS2_CLOCK_PIN);
	PS2_PORT_PDDR &= ~(1 << PS2_DATA_PIN);
	PS2_CLOCK_PCR = PORT_PCR_MUX(1);
	PS2_DATA_PCR = PORT_PCR_MUX(1);

	// Register pin PS2_CLOCK to trigger interrupt on port B:
	// falling mask is 0x0A, see pins_teensy.c
	// 16 is location in register, not pin
	uint32_t mask = (0x0A << 16) | 0x01000000;
	__disable_irq();
	PS2_CLOCK_PCR |= mask;
	__enable_irq();
#else
#warning "Not implemented"
#endif
}


// Main Detection Loop
inline uint8_t Scan_loop() {
	// uint8_t c =get_scan_code();
	// if (c != 0){
	// printHex(c);
	// }
	ScancodeDispatch();
	return 0;
}


// Signal from Macro Module that all keys have been processed (that it knows about)
inline void Scan_finishedWithMacro(uint8_t sentKeys) {
}


// Signal from Output Module that all keys have been processed (that it knows about)
inline void Scan_finishedWithOutput(uint8_t sentKeys) {
  // Reset scan loop indicator (resets each key debounce state)
  // TODO should this occur after USB send or Macro processing?
}


// Signal from the Output Module that the available current has changed
// current - mA
void Scan_currentChange(unsigned int current) {
	// Indicate to all submodules current change
}


void cliFunc_ps2Verbose(char* args) {
	verbose_state = !verbose_state;
}

