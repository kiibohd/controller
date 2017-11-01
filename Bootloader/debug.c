/* Copyright (C) 2015-2017 by Jacob Alexander
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

// Project Includes
#include <Lib/mcu_compat.h>

// Local Includes
#include "mchck.h"



// ----- Defines -----

// UART Configuration
#if defined(_kii_v2_) // UART2 Debug
#define UART_BDH    UART2_BDH
#define UART_BDL    UART2_BDL
#define UART_C1     UART2_C1
#define UART_C2     UART2_C2
#define UART_C3     UART2_C3
#define UART_C4     UART2_C4
#define UART_CFIFO  UART2_CFIFO
#define UART_D      UART2_D
#define UART_PFIFO  UART2_PFIFO
#define UART_RCFIFO UART2_RCFIFO
#define UART_RWFIFO UART2_RWFIFO
#define UART_S1     UART2_S1
#define UART_S2     UART2_S2
#define UART_SFIFO  UART2_SFIFO
#define UART_TWFIFO UART2_TWFIFO

#define SIM_SCGC4_UART  SIM_SCGC4_UART2
#define IRQ_UART_STATUS IRQ_UART2_STATUS

#elif defined(_kii_v1_) // UART0 Debug
#define UART_BDH    UART0_BDH
#define UART_BDL    UART0_BDL
#define UART_C1     UART0_C1
#define UART_C2     UART0_C2
#define UART_C3     UART0_C3
#define UART_C4     UART0_C4
#define UART_CFIFO  UART0_CFIFO
#define UART_D      UART0_D
#define UART_PFIFO  UART0_PFIFO
#define UART_RCFIFO UART0_RCFIFO
#define UART_RWFIFO UART0_RWFIFO
#define UART_S1     UART0_S1
#define UART_S2     UART0_S2
#define UART_SFIFO  UART0_SFIFO
#define UART_TWFIFO UART0_TWFIFO

#define SIM_SCGC4_UART  SIM_SCGC4_UART0
#define IRQ_UART_STATUS IRQ_UART0_STATUS

#else
#error "Bootloader UART Debug unsupported"
#endif



// ----- Functions -----

void uart_serial_setup()
{
	// Setup the the UART interface for keyboard data input
	SIM_SCGC4 |= SIM_SCGC4_UART; // Disable clock gating

// Kiibohd-dfu
#if defined(_kii_v2_)
	// Pin Setup for UART2
	PORTD_PCR3 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin
#elif defined(_kii_v1_)
	// Pin Setup for UART0
	PORTA_PCR2 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin
#endif


#if defined(_kii_v2_) // UART2 Debug
	// Setup baud rate - 115200 Baud
	// Uses Bus Clock
	// 36 MHz / ( 16 * Baud ) = BDH/L
	// Baud: 115200 -> 36 MHz / ( 16 * 115200 ) = 19.53125
	// Thus baud setting = 19
	// NOTE: If finer baud adjustment is needed see UARTx_C4 -> BRFA in the datasheet
	uint16_t baud = 19; // Max setting of 8191
	UART_BDH = (uint8_t)(baud >> 8);
	UART_BDL = (uint8_t)baud;
	UART_C4 = 0x11;

#elif defined(_kii_v1_)
	// Setup baud rate - 115200 Baud
	// 48 MHz / ( 16 * Baud ) = BDH/L
	// Baud: 115200 -> 48 MHz / ( 16 * 115200 ) = 26.0416667
	// Thus baud setting = 26
	// NOTE: If finer baud adjustment is needed see UARTx_C4 -> BRFA in the datasheet
	uint16_t baud = 26; // Max setting of 8191
	UART_BDH = (uint8_t)(baud >> 8);
	UART_BDL = (uint8_t)baud;
	UART_C4 = 0x02;

#endif

	// 8 bit, No Parity, Idle Character bit after stop
	UART_C1 = UART_C1_ILT;

	// TX FIFO Enabled, TX FIFO Size 1 (Max 8 datawords)
	// TX/RX FIFO Size:
	//  0x0 - 1 dataword
	//  0x1 - 4 dataword
	//  0x2 - 8 dataword
	UART_PFIFO = UART_PFIFO_TXFE;

	// TX Enabled, RX Enabled, RX Interrupt Enabled, Generate idles
	// UART_C2_TE UART_C2_RE UART_C2_RIE UART_C2_ILIE
	UART_C2 = UART_C2_TE | UART_C2_ILIE;
}


int uart_serial_write( const void *buffer, uint32_t size )
{
	const uint8_t *data = (const uint8_t *)buffer;
	uint32_t position = 0;

	// While buffer is not empty and transmit buffer is
	while ( position < size )
	{
		while ( !( UART_SFIFO & UART_SFIFO_TXEMPT ) ); // Wait till there is room to send
		UART_D = data[position++];
	}

	return 0;
}


int Output_putstr( char* str )
{
	uint32_t count = 0;

	// Count characters until NULL character, then send the amount counted
	while ( str[count] != '\0' )
		count++;

	return uart_serial_write( str, count );
}


uint16_t lenStr( char* in )
{
	// Iterator
	char *pos;

	// Loop until null is found
	for ( pos = in; *pos; pos++ );

	// Return the difference between the pointers of in and pos (which is the string length)
	return (pos - in);
}


void revsStr( char* in )
{
	// Iterators
	int i, j;

	// Temp storage
	char c;

	// Loop through the string, and reverse the order of the characters
	for ( i = 0, j = lenStr( in ) - 1; i < j; i++, j-- )
	{
		c = in[i];
		in[i] = in[j];
		in[j] = c;
	}
}


void hexToStr_op( uint32_t in, char* out, uint8_t op )
{
	// Position container
	uint32_t pos = 0;

	// Evaluate through digits as hex
	do
	{
		uint32_t cur = in % 16;
		out[pos++] = cur + (( cur < 10 ) ? '0' : 'A' - 10);
	}
	while ( (in /= 16) > 0 );

	// Output formatting options
	switch ( op )
	{
	case 1: // Add 0x
		out[pos++] = 'x';
		out[pos++] = '0';
		break;
	case 2: //  8-bit padding
	case 4: // 16-bit padding
	case 8: // 32-bit padding
		while ( pos < op )
			out[pos++] = '0';
		break;
	}

	// Append null
	out[pos] = '\0';

	// Reverse the string to the correct order
	revsStr( out );
}


void printHex_op( uint32_t in, uint8_t op )
{
	// With an op of 1, the max number of characters is 6 + 1 for null
	// e.g. "0xFFFF\0"
	// op 2 and 4 require fewer characters (2+1 and 4+1 respectively)
	char tmpStr[7];

	// Convert number
	hexToStr_op( in, tmpStr, op );

	// Print number
	Output_putstr( tmpStr );
}

