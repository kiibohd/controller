/* Copyright (C) 2012,2014,2016 by Jacob Alexander
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

// ----- Includes -----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <kll.h>
#include <kll_defs.h>
#include <led.h>
#include <macro.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"



// ----- Defines -----

// Pinout Defines
#define HOLD_PORT PORTD
#define HOLD_DDR   DDRD
#define HOLD_PIN      3


// ----- Macros -----



// ----- Enums -----

// Keypress States
typedef enum KeyPosition {
	KeyState_Off     = 0,
	KeyState_Press   = 1,
	KeyState_Hold    = 2,
	KeyState_Release = 3,
	KeyState_Invalid,
} KeyPosition;



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;
volatile uint8_t KeyIndex_Add_InputSignal; // Used to pass the (click/input value) to the keyboard for the clicker


// Buffer Signals
volatile uint8_t BufferReadyToClear;



// ----- Function Declarations -----

void processKeyValue( uint8_t keyValue );
void  removeKeyValue( uint8_t keyValue );



// ----- Interrupt Functions -----

// USART Receive Buffer Full Interrupt
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
ISR(USART1_RX_vect)
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
void uart0_status_isr()
#endif
{
	cli(); // Disable Interrupts

	uint8_t keyValue = 0x00;
	uint8_t keyState = 0x00;

#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	// Read the scancode packet from the USART (1st to 8th bits)
	keyValue = UDR1;

	// Read the release/press bit (9th bit) XXX Unnecessary, and wrong it seems, parity bit? or something else?
	keyState = UCSR1B & 0x02;
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
	// UART0_S1 must be read for the interrupt to be cleared
	if ( UART0_S1 & UART_S1_RDRF )
	{
		// Only doing single byte FIFO here
		keyValue = UART0_D;
	}
#endif

	// High bit of keyValue, also represents press/release
	keyState = keyValue & 0x80 ? 0x00 : 0x02;

	// Debug
	char tmpStr[6];
	hexToStr( keyValue & 0x7F, tmpStr );

	// Process the scancode
	switch ( keyState )
	{
	case 0x00: // Released
		dPrintStrs( tmpStr, "R  " ); // Debug
		break;

	case 0x02: // Pressed
		dPrintStrs( tmpStr, "P " ); // Debug
		break;
	}

	// Add key event to macro key buffer
	TriggerGuide guide = {
		.type     = 0x00,
		.state    = keyState == 0x02 ? 0x01 : 0x03,
		.scanCode = keyValue & 0x7F,
	};
	Macro_pressReleaseAdd( &guide );

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

// Setup
inline void Scan_setup()
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
{
	// Setup the the USART interface for keyboard data input
	// NOTE: The input data signal needs to be inverted for the Teensy USART to properly work

	// Setup baud rate
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud <- 0.823284 ms per bit, thus 1000 / 0.823284 = 1214.65004 -> 823.2824
	// Thus baud setting = 823
	uint16_t baud = 823; // Max setting of 4095
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;

	// Enable the receiver, and RX Complete Interrupt as well as 9 bit data
	UCSR1B = 0x94;

	// The transmitter is only to be enabled when needed
	// Set the pin to be pull-up otherwise (use the lowered voltage inverter in order to sink)
	HOLD_DDR  &= ~(1 << HOLD_PIN);
	HOLD_PORT |=  (1 << HOLD_PIN);

	// Set frame format: 9 data, 1 stop bit, no parity
	// Asynchrounous USART mode
	UCSR1C = 0x06;

	// Initially buffer doesn't need to be cleared (it's empty...)
	BufferReadyToClear = 0;

	// InputSignal is off by default
	KeyIndex_Add_InputSignal = 0x00;

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
{
	// Setup the the UART interface for keyboard data input
	SIM_SCGC4 |= SIM_SCGC4_UART0; // Disable clock gating

	// Pin Setup for UART0
	PORTB_PCR16 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	PORTB_PCR17 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin

	// Setup baud rate - 1205 Baud
	// 48 MHz / ( 16 * Baud ) = BDH/L
	// Baud: 1215 -> 48 MHz / ( 16 * 1215 ) = 2469.1358
	// Thus baud setting = 2469
	// NOTE: If finer baud adjustment is needed see UARTx_C4 -> BRFA in the datasheet
	uint16_t baud = 2469; // Max setting of 8191
	UART0_BDH = (uint8_t)(baud >> 8);
	UART0_BDL = (uint8_t)baud;

	// 8 bit, Even Parity, Idle Character bit after stop
	// NOTE: For 8 bit with Parity you must enable 9 bit transmission (pg. 1065)
	//       You only need to use UART0_D for 8 bit reading/writing though
	// UART_C1_M UART_C1_PE UART_C1_PT UART_C1_ILT
	UART0_C1 = UART_C1_M | UART_C1_PE | UART_C1_ILT;

	// Number of bytes in FIFO before TX Interrupt
	UART0_TWFIFO = 1;

	// Number of bytes in FIFO before RX Interrupt
	UART0_RWFIFO = 1;

	// TX FIFO Disabled, TX FIFO Size 1 (Max 8 datawords), RX FIFO Enabled, RX FIFO Size 1 (Max 8 datawords)
	// TX/RX FIFO Size:
	//  0x0 - 1 dataword
	//  0x1 - 4 dataword
	//  0x2 - 8 dataword
	//UART0_PFIFO = UART_PFIFO_TXFE | /*TXFIFOSIZE*/ (0x0 << 4) | UART_PFIFO_RXFE | /*RXFIFOSIZE*/ (0x0);

	// Reciever Inversion Disabled, LSBF
	// UART_S2_RXINV UART_S2_MSBF
	UART0_S2 |= 0x00;

	// Transmit Inversion Disabled
	// UART_C3_TXINV
	UART0_C3 |= 0x00;

	// TX Disabled, RX Enabled, RX Interrupt Enabled
	// UART_C2_TE UART_C2_RE UART_C2_RIE
	UART0_C2 = UART_C2_RE | UART_C2_RIE | UART_C2_TE;

	// Add interrupt to the vector table
	NVIC_ENABLE_IRQ( IRQ_UART0_STATUS );

	// Reset the keyboard before scanning, we might be in a wierd state
	Scan_resetKeyboard();
}
#endif


// Main Detection Loop
// Not needed for the BETKB, this is just a busy loop
inline uint8_t Scan_loop()
{
	return 0;
}

// Send data
uint8_t scan_sendData( uint8_t dataPayload )
{
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	// Enable the USART Transmitter
	UCSR1B |=  (1 << 3);
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
#endif

	// Debug
	char tmpStr[6];
	hexToStr( dataPayload, tmpStr );
	info_dPrint( "Sending - ", tmpStr );

#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	UDR1 = dataPayload;
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
	UART0_D = dataPayload;
#endif

	// Wait for the payload
	_delay_us( 800 );

#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	// Disable the USART Transmitter
	UCSR1B &= ~(1 << 3);
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
#endif

	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
void Scan_finishedWithMacro( uint8_t sentKeys )
{
}

// Signal that the keys have been properly sent over USB
void Scan_finishedWithOutput( uint8_t sentKeys )
{
}

// Reset/Hold keyboard
// NOTE: Does nothing with the BETKB
void Scan_lockKeyboard()
{
}

// NOTE: Does nothing with the BETKB
void Scan_unlockKeyboard()
{
}

// Reset Keyboard
void Scan_resetKeyboard()
{
	// Not a calculated valued...
	_delay_ms( 50 );
}

// NOTE: Does nothing with the BETKB
void Scan_currentChange( unsigned int current )
{
}

