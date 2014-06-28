/* Copyright (C) 2014 by Jacob Alexander
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

#include "uart_serial.h"
#include <Lib/OutputLib.h>
#include <Lib/Interrupts.h>
#include <string.h> // For memcpy

// ----- Variables -----

#define uart0_buffer_size 128 // 128 byte buffer
volatile uint8_t uart0_buffer_head = 0;
volatile uint8_t uart0_buffer_tail = 0;
volatile uint8_t uart0_buffer_items = 0;
volatile uint8_t uart0_buffer[uart0_buffer_size];

volatile uint8_t uart_configured = 0;



// ----- Interrupt Functions -----

void uart0_status_isr()
{
	cli(); // Disable Interrupts

	// UART0_S1 must be read for the interrupt to be cleared
	if ( UART0_S1 & ( UART_S1_RDRF | UART_S1_IDLE ) )
	{
		uint8_t available = UART0_RCFIFO;

		// If there was actually nothing
		if ( available == 0 )
		{
			// Cleanup
			available = UART0_D;
			UART0_CFIFO = UART_CFIFO_RXFLUSH;
			sei();
			return;
		}

		// Read UART0 into buffer until FIFO is empty
		while ( available-- > 0 )
		{
			uart0_buffer[uart0_buffer_tail++] = UART0_D;
			uart0_buffer_items++;

			// Wrap-around of tail pointer
			if ( uart0_buffer_tail >= uart0_buffer_size )
			{
				uart0_buffer_tail = 0;
			}

			// Make sure the head pointer also moves if circular buffer is overwritten
			if ( uart0_buffer_head == uart0_buffer_tail )
			{
				uart0_buffer_head++;
			}

			// Wrap-around of head pointer
			if ( uart0_buffer_head >= uart0_buffer_size )
			{
				uart0_buffer_head = 0;
			}
		}
	}

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

void uart_serial_setup()
{
	// Indication that the UART is not ready yet
	uart_configured = 0;

	// Setup the the UART interface for keyboard data input
	SIM_SCGC4 |= SIM_SCGC4_UART0; // Disable clock gating

	// Pin Setup for UART0
	PORTB_PCR16 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	PORTB_PCR17 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin

	// Setup baud rate - 115200 Baud
	// 48 MHz / ( 16 * Baud ) = BDH/L
	// Baud: 115200 -> 48 MHz / ( 16 * 115200 ) = 26.0416667
	// Thus baud setting = 26
	// NOTE: If finer baud adjustment is needed see UARTx_C4 -> BRFA in the datasheet
	uint16_t baud = 26; // Max setting of 8191
	UART0_BDH = (uint8_t)(baud >> 8);
	UART0_BDL = (uint8_t)baud;
	UART0_C4 = 0x02;

	// 8 bit, No Parity, Idle Character bit after stop
	UART0_C1 = UART_C1_ILT;

	// Interrupt notification watermark
	UART0_TWFIFO = 2;
	UART0_RWFIFO = 4;

	// TX FIFO Disabled, TX FIFO Size 1 (Max 8 datawords), RX FIFO Enabled, RX FIFO Size 1 (Max 8 datawords)
	// TX/RX FIFO Size:
	//  0x0 - 1 dataword
	//  0x1 - 4 dataword
	//  0x2 - 8 dataword
	UART0_PFIFO = UART_PFIFO_TXFE | UART_PFIFO_RXFE;

	// Reciever Inversion Disabled, LSBF
	// UART_S2_RXINV UART_S2_MSBF
	UART0_S2 |= 0x00;

	// Transmit Inversion Disabled
	// UART_C3_TXINV
	UART0_C3 |= 0x00;

	// TX Enabled, RX Enabled, RX Interrupt Enabled, Generate idles
	// UART_C2_TE UART_C2_RE UART_C2_RIE UART_C2_ILIE
	UART0_C2 = UART_C2_TE | UART_C2_RE | UART_C2_RIE | UART_C2_ILIE;

	// Add interrupt to the vector table
	NVIC_ENABLE_IRQ( IRQ_UART0_STATUS );

	// UART is now ready to use
	uart_configured = 1;
}


// Get the next character, or -1 if nothing received
int uart_serial_getchar()
{
	if ( !uart_configured )
		return -1;

	unsigned int value = -1;

	// Check to see if the FIFO has characters
	if ( uart0_buffer_items > 0 )
	{
		value = uart0_buffer[uart0_buffer_head++];
		uart0_buffer_items--;

		// Wrap-around of head pointer
		if ( uart0_buffer_head >= uart0_buffer_size )
		{
			uart0_buffer_head = 0;
		}
	}

	return value;
}


// Number of bytes available in the receive buffer
int uart_serial_available()
{
	return uart0_buffer_items;
}


// Discard any buffered input
void uart_serial_flush_input()
{
	uart0_buffer_head = 0;
	uart0_buffer_tail = 0;
	uart0_buffer_items = 0;
}


// Transmit a character.  0 returned on success, -1 on error
int uart_serial_putchar( uint8_t c )
{
	if ( !uart_configured )
		return -1;

	while ( !( UART0_SFIFO & UART_SFIFO_TXEMPT ) ); // Wait till there is room to send
	UART0_D = c;

	return 0;
}


int uart_serial_write( const void *buffer, uint32_t size )
{
	if ( !uart_configured )
		return -1;

	const uint8_t *data = (const uint8_t *)buffer;
	uint32_t position = 0;

	// While buffer is not empty and transmit buffer is
	while ( position < size )
	{
		while ( !( UART0_SFIFO & UART_SFIFO_TXEMPT ) ); // Wait till there is room to send
		UART0_D = data[position++];
	}

	return 0;
}


void uart_serial_flush_output()
{
	// Delay until buffer has been sent
	while ( !( UART0_SFIFO & UART_SFIFO_TXEMPT ) ); // Wait till there is room to send
}


void uart_device_reload()
{
	asm volatile("bkpt");
}

