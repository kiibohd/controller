/* Copyright (C) 2013,2014,2016 by Jacob Alexander
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
#include <kll_defs.h>
#include <led.h>
#include <macro.h>
#include <print.h>

// Local Includes
#include "scan_loop.h"



// ----- Defines -----


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



// ----- Function Declarations -----

void processKeyValue( uint8_t valueType );
void  removeKeyValue( uint8_t keyValue );



// ----- Interrupt Functions -----

// UART Receive Buffer Full Interrupt
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
ISR(USART1_RX_vect)
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
void uart0_status_isr(void)
#endif
{
	cli(); // Disable Interrupts

	// Variable for UART data read
	uint8_t keyValue = 0x00;

#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	keyValue = UDR1;
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
	// UART0_S1 must be read for the interrupt to be cleared
	if ( UART0_S1 & UART_S1_RDRF )
	{
		// Only doing single byte FIFO here
		keyValue = UART0_D;
	}
#endif

	// Debug
	char tmpStr[6];
	hexToStr( keyValue, tmpStr );
	dPrintStrs( tmpStr, " " ); // Debug

	// Decipher scan value
	processKeyValue( keyValue );

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

// Reset Keyboard
void Scan_resetKeyboard( void )
{
	// Not a calculated valued...
	_delay_ms( 50 );

	KeyIndex_BufferUsed = 0;
}

// Setup
inline void Scan_setup()
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

	// Enable the receiver, and RX Complete Interrupt
	UCSR1B = 0x90;

	// Set frame format: 8 data, 1 stop bit, even parity
	// Asynchrounous USART mode
	UCSR1C = 0x26;

	// Reset the keyboard before scanning, we might be in a wierd state
	Scan_resetKeyboard();
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
	// Baud: 1205 -> 48 MHz / ( 16 * 1205 ) = 2489.6266
	// Thus baud setting = 2490
	// NOTE: If finer baud adjustment is needed see UARTx_C4 -> BRFA in the datasheet
	uint16_t baud = 2490; // Max setting of 8191
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
	UART0_C2 = UART_C2_RE | UART_C2_RIE;

	// Add interrupt to the vector table
	NVIC_ENABLE_IRQ( IRQ_UART0_STATUS );

	// Reset the keyboard before scanning, we might be in a wierd state
	Scan_resetKeyboard();
}
#endif


// Main Detection Loop
inline uint8_t Scan_loop()
{
	return 0;
}

void processKeyValue( uint8_t keyValue )
{
	// XXX NOTE: The key processing is not complete for this keyboard
	//           Mostly due to laziness, and that the keyboard can't really be useful on a modern computer
	//           Basic typing will work, but some of the keys and the Graph mode changes things around

	// Add key(s) to processing buffer
	// First split out Shift and Ctrl
	//  Reserved Codes:
	//   Shift - 0xF5
	//   Ctrl  - 0xF6
	switch ( keyValue )
	{
	// - Ctrl Keys -
	// Exception keys
	case 0x08: // ^H
	case 0x09: // ^I
	case 0x0D: // ^M
	case 0x1B: // ^[
		Macro_keyState( keyValue, KeyState_Press );
		break;
	// 0x40 Offset Keys
	// Add Ctrl key and offset to the lower alphabet
	case 0x00: // ^@
	case 0x1C: // "^\"
	case 0x1D: // ^]
	case 0x1E: // ^^
	case 0x1F: // ^_
		Macro_keyState( 0xF6, KeyState_Press );
		Macro_keyState( keyValue + 0x40, KeyState_Press );
		break;

	// - Add Shift key and offset to non-shifted key -
	// 0x10 Offset Keys
	case 0x21: // !
	case 0x23: // #
	case 0x24: // $
	case 0x25: // %
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue + 0x10, KeyState_Press );
		break;
	// 0x11 Offset Keys
	case 0x26: // &
	case 0x28: // (
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue + 0x11, KeyState_Press );
		break;
	// 0x07 Offset Keys
	case 0x29: // )
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue + 0x07, KeyState_Press );
		break;
	// -0x0E Offset Keys
	case 0x40: // @
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue - 0x0E, KeyState_Press );
		break;
	// 0x0E Offset Keys
	case 0x2A: // *
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue + 0x0E, KeyState_Press );
		break;
	// 0x12 Offset Keys
	case 0x2B: // +
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue + 0x12, KeyState_Press );
		break;
	// 0x05 Offset Keys
	case 0x22: // "
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue + 0x05, KeyState_Press );
		break;
	// 0x01 Offset Keys
	case 0x3A: // :
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue + 0x01, KeyState_Press );
		break;
	// -0x10 Offset Keys
	case 0x3C: // <
	case 0x3E: // >
	case 0x3F: // ?
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue - 0x10, KeyState_Press );
		break;
	// -0x28 Offset Keys
	case 0x5E: // ^
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue - 0x28, KeyState_Press );
		break;
	// -0x32 Offset Keys
	case 0x5F: // _
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue - 0x32, KeyState_Press );
		break;
	// -0x20 Offset Keys
	case 0x7B: // {
	case 0x7C: // |
	case 0x7D: // }
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue - 0x20, KeyState_Press );
		break;
	// -0x1E Offset Keys
	case 0x7E: // ~
		Macro_keyState( 0xF5, KeyState_Press );
		Macro_keyState( keyValue - 0x1E, KeyState_Press );
		break;
	// All other keys
	default:
		// Ctrl Characters are from 0x00 to 0x1F, excluding:
		//  0x08 - Backspace
		//  0x09 - [Horizontal] Tab
		//  0x0D - [Carriage] Return
		//  0x1B - Escape
		//  0x7F - Delete (^?) (Doesn't need to be split out)

		// 0x60 Offset Keys
		// Add Ctrl key and offset to the lower alphabet
		if ( keyValue >= 0x00 && keyValue <= 0x1F )
		{
			Macro_keyState( 0xF6, KeyState_Press );
			Macro_keyState( keyValue + 0x60, KeyState_Press );
		}

		// Shift Characters are from 0x41 to 0x59
		//  No exceptions here :D
		// Add Shift key and offset to the lower alphabet
		else if ( keyValue >= 0x41 && keyValue <= 0x5A )
		{
			Macro_keyState( 0xF5, KeyState_Press );
			Macro_keyState( keyValue + 0x20, KeyState_Press );
		}

		// Everything else
		else
		{
			Macro_keyState( keyValue, KeyState_Press );
		}
		break;
	}
}

// Send data
// NOTE: Example only, MBC-55X cannot receive user data
uint8_t Scan_sendData( uint8_t dataPayload )
{
	// Debug
	char tmpStr[6];
	hexToStr( dataPayload, tmpStr );
	info_dPrint( "Sending - ", tmpStr );

#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_) // AVR
	UDR1 = dataPayload;
#elif defined(_mk20dx128_) || defined(_mk20dx256_) // ARM
	UART0_D = dataPayload;
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
	cli(); // Disable Interrupts

	// Reset the buffer counter
	KeyIndex_BufferUsed = 0;

	sei(); // Re-enable Interrupts
}

// Reset/Hold keyboard
// NOTE: Does nothing with the MBC-55x
void Scan_lockKeyboard( void )
{
}

// NOTE: Does nothing with the MBC-55x
void Scan_unlockKeyboard( void )
{
}

// NOTE: Does nothing with the MBC-55x
void Scan_currentChange( unsigned int current )
{
}

