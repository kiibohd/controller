/* Copyright (C) 2013 by Jacob Alexander
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
#include <led.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"



// ----- Defines -----


// ----- Macros -----

// Make sure we haven't overflowed the buffer
#define bufferAdd(byte) \
		if ( KeyIndex_BufferUsed < KEYBOARD_BUFFER ) \
			KeyIndex_Buffer[KeyIndex_BufferUsed++] = byte



// ----- Variables -----

// Buffer used to inform the macro processing module which keys have been detected as pressed
volatile uint8_t KeyIndex_Buffer[KEYBOARD_BUFFER];
volatile uint8_t KeyIndex_BufferUsed;



// ----- Function Declarations -----

void processKeyValue( uint8_t valueType );
void  removeKeyValue( uint8_t keyValue );



// ----- Interrupt Functions -----

// UART Receive Buffer Full Interrupt
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
ISR(USART1_RX_vect)
#elif defined(_mk20dx128_) // ARM
void uart0_status_isr(void)
#endif
{
	cli(); // Disable Interrupts

	// Read part of the scan code (3 8bit chunks) from USART
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	uint8_t tmp = UDR1;
#elif defined(_mk20dx128_) // ARM
	// Exit out if nothing to do
	/*
	if ( !(UART0_S1 & UART_S1_RDRF ) )
	{
		sei();
		return;
	}
	*/

	// Only doing single byte FIFO here
	uint8_t tmp = UART0_D;
	print("YAYA");
#endif

	// Debug
	char tmpStr[6];
	hexToStr( tmp, tmpStr );
	dPrintStrsNL( tmpStr, " " ); // Debug

	// TODO

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

// Setup
inline void scan_setup()
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
{
	// Setup the the USART interface for keyboard data input

	// Setup baud rate - 1205 Baud
	// 16 MHz / ( 16 * Baud ) = UBRR
	// Baud: 1205 -> 16 MHz / ( 16 * 1205 ) = 829.8755
	// Thus baud setting = 830
	uint16_t baud = 830; // Max setting of 4095
	UBRR1H = (uint8_t)(baud >> 8);
	UBRR1L = (uint8_t)baud;

	// Enable the receiver, transmitter, and RX Complete Interrupt
	// TODO - Only receiver, and rx interrupt
	UCSR1B = 0x98;

	// Set frame format: 8 data, 1 stop bit, odd parity
	// Asynchrounous USART mode
	// TODO - Even parity
	UCSR1C = 0x36;

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}
#elif defined(_mk20dx128_) // ARM
{
	// Setup the the UART interface for keyboard data input
	SIM_SCGC4 |= SIM_SCGC4_UART0; // Disable clock gating

	// Pin Setup for UART0
	PORTB_PCR16 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	PORTB_PCR17 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin

	// Setup baud rate - 1205 Baud
	// 48 MHz / ( 16 * Baud ) = BDH/L
	// Baud: 1205 -> 48 MHz / ( 16 * 1205 ) = 2489.6266
	// Thus baud setting = 2490
	// NOTE: If finer baud adjustment is needed see UARTx_C4 -> BRFA in the datasheet
	uint16_t baud = 2490; // Max setting of 8191
	UART0_BDH = (uint8_t)(baud >> 8);
	UART0_BDL = (uint8_t)baud;

	// 8 bit, Even Parity, Idle Character bit after stop
	UART0_C1 = ~UART_C1_M | ~UART_C1_PE | UART_C1_PT | UART_C1_ILT;

	// Number of bytes in FIFO before TX Interrupt
	UART0_TWFIFO = 1;

	// Number of bytes in FIFO before RX Interrupt
	UART0_RWFIFO = 1;

	// TX FIFO Disabled, TX FIFO Size 1 (Max 8 datawords), RX FIFO Enabled, RX FIFO Size 1 (Max 8 datawords)
	// TX/RX FIFO Size:
	//  0x0 - 1 dataword
	//  0x1 - 4 dataword
	//  0x2 - 8 dataword
	UART0_PFIFO = ~UART_PFIFO_TXFE | /*TXFIFOSIZE*/ (0x0 << 4) | ~UART_PFIFO_RXFE | /*RXFIFOSIZE*/ (0x0);

	// Reciever Inversion Disabled, LSBF
	UART0_S2 = ~UART_S2_RXINV | UART_S2_MSBF;

	// Transmit Inversion Disabled
	UART0_C3 = ~UART_S2_TXINV;

	// TX Disabled, RX Enabled, RX Interrupt Enabled
	UART0_C2 = UART_C2_TE | UART_C2_RE | UART_C2_RIE;

	// Add interrupt to the vector table
	NVIC_ENABLE_IRQ( IRQ_UART0_STATUS );

	// Reset the keyboard before scanning, we might be in a wierd state
	scan_resetKeyboard();
}
#endif


// Main Detection Loop
inline uint8_t scan_loop()
{
	UART0_D = 0x56;
	UART0_D = 0x1C;
	_delay_ms( 100 );
	return 0;
}

void processKeyValue( uint8_t keyValue )
{
	// TODO Process ASCII

	// Make sure the key isn't already in the buffer
	for ( uint8_t c = 0; c < KeyIndex_BufferUsed + 1; c++ )
	{
		// Key isn't in the buffer yet
		if ( c == KeyIndex_BufferUsed )
		{
			bufferAdd( keyValue );
			break;
		}

		// Key already in the buffer
		if ( KeyIndex_Buffer[c] == keyValue )
			break;
	}
}

void removeKeyValue( uint8_t keyValue )
{
	// Check for the released key, and shift the other keys lower on the buffer
	uint8_t c;
	for ( c = 0; c < KeyIndex_BufferUsed; c++ )
	{
		// Key to release found
		if ( KeyIndex_Buffer[c] == keyValue )
		{
			// Shift keys from c position
			for ( uint8_t k = c; k < KeyIndex_BufferUsed - 1; k++ )
				KeyIndex_Buffer[k] = KeyIndex_Buffer[k + 1];

			// Decrement Buffer
			KeyIndex_BufferUsed--;

			break;
		}
	}

	// Error case (no key to release)
	if ( c == KeyIndex_BufferUsed + 1 )
	{
		errorLED( 1 );
		char tmpStr[6];
		hexToStr( keyValue, tmpStr );
		erro_dPrint( "Could not find key to release: ", tmpStr );
	}
}

// Send data
// NOTE: Example only, MBC-55X cannot receive user data
uint8_t scan_sendData( uint8_t dataPayload )
{
	// Debug
	char tmpStr[6];
	hexToStr( dataPayload, tmpStr );
	info_dPrint( "Sending - ", tmpStr );

#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	UDR1 = dataPayload;
#elif defined(_mk20dx128_) // ARM
	UART0_D = dataPayload;
#endif

	return 0;
}

// Signal KeyIndex_Buffer that it has been properly read
void scan_finishedWithBuffer( uint8_t sentKeys )
{
}

// Signal that the keys have been properly sent over USB
void scan_finishedWithUSBBuffer( uint8_t sentKeys )
{
}

// Reset/Hold keyboard
// NOTE: Does nothing with the FACOM6684
void scan_lockKeyboard( void )
{
}

// NOTE: Does nothing with the FACOM6684
void scan_unlockKeyboard( void )
{
}

// Reset Keyboard
void scan_resetKeyboard( void )
{
	// Not a calculated valued...
	_delay_ms( 50 );

	KeyIndex_BufferUsed = 0;
}

