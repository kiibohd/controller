/* Copyright (C) 2014-2017 by Jacob Alexander
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
#include <string.h> // For memcpy

// Project Includes
#include <Lib/OutputLib.h>
#include <Lib/Interrupts.h>
#include <print.h>
#include <kll_defs.h>

// Local Includes
#include "uart_serial.h"



// ----- Defines -----

// UART Configuration
#if defined(_kii_v1_) || defined(_teensy_3_) // UART0 Debug
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

#elif defined(_kii_v2_) // UART2 Debug
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

#elif defined(_kii_v3_)
#define UART_BAUD   115200
#define UART_PERIPH UART0
#define UART_IRQ    UART0_IRQn
#define UART_PMC    ID_UART0
#define UART_PIO    PIOA
#define UART_RX_PIN 9
#define UART_TX_PIN 10

#endif



// ----- Variables -----

#define uart_buffer_size 128 // 128 byte buffer
volatile uint8_t uart_buffer_head = 0;
volatile uint8_t uart_buffer_tail = 0;
volatile uint8_t uart_buffer_items = 0;
volatile uint8_t uart_buffer[uart_buffer_size];

volatile uint8_t uart_configured = 0;



// ----- Interrupt Functions -----

void uart_buffer_append(char c);

#if defined(_kii_v1_) || defined(_teensy_3_) // UART0 Debug
void uart0_status_isr()
#elif defined(_kii_v2_) // UART2 Debug
void uart2_status_isr()
#elif defined(_kii_v3_)
void UART0_Handler()
#endif
{
	cli(); // Disable Interrupts

#if defined(_kinetis_)
	// UART0_S1 must be read for the interrupt to be cleared
	if ( UART_S1 & ( UART_S1_RDRF | UART_S1_IDLE ) )
	{
		uint8_t available = UART_RCFIFO;

		// If there was actually nothing
		if ( available == 0 )
		{
			// Cleanup
			available = UART_D;
			UART_CFIFO = UART_CFIFO_RXFLUSH;
			goto done;
		}

		// Read UART0 into buffer until FIFO is empty
		while ( available-- > 0 )
		{
			uart_buffer_append(UART_D);
		}
	}
done:

#elif defined(_sam_)
	// UART_SR must be read for the interrupt to be cleared
	if ( UART_PERIPH->UART_SR & UART_SR_RXRDY )
	{
		uart_buffer_append(UART_PERIPH->UART_RHR);
	}
#endif

	sei(); // Re-enable Interrupts
}



// ----- Functions -----

void uart_serial_setup()
{
	// Indication that the UART is not ready yet
	uart_configured = 0;

	// Setup the the UART interface for keyboard data input
#if defined(_kinetis_)
	SIM_SCGC4 |= SIM_SCGC4_UART; // Disable clock gating
#endif

// MCHCK / Kiibohd-dfu
#if defined(_kii_v1_)
	// Pin Setup for UART0
	PORTA_PCR1 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(2); // RX Pin
	PORTA_PCR2 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(2); // TX Pin

// Kiibohd-dfu
#elif defined(_kii_v2_)
	// Pin Setup for UART2
	PORTD_PCR2 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	PORTD_PCR3 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin

// SAM dev kit
#elif defined(_kii_v3_)
	//Select Peripheral A for UART pins
	UART_PIO->PIO_ABCDSR[0] &= ~((1 << UART_RX_PIN) | (1 << UART_TX_PIN));
	UART_PIO->PIO_ABCDSR[1] &= ~((1 << UART_RX_PIN) | (1 << UART_TX_PIN));
	UART_PIO->PIO_PDR = (1 << UART_RX_PIN) | (1 << UART_TX_PIN);

	//Enable UART peripheral clock
	PMC->PMC_PCER0 = (1 << UART_PMC);

// Teensy
#elif defined(_teensy_)
	// Pin Setup for UART0
	PORTB_PCR16 = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3); // RX Pin
	PORTB_PCR17 = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3); // TX Pin
#else
#warning "Unknown chip"
#endif


#if defined(_kii_v1_) || defined(_teensy_3_) // UART0 Debug
	// Setup baud rate - 115200 Baud
	// 48 MHz / ( 16 * Baud ) = BDH/L
	// Baud: 115200 -> 48 MHz / ( 16 * 115200 ) = 26.0416667
	// Thus baud setting = 26
	// NOTE: If finer baud adjustment is needed see UARTx_C4 -> BRFA in the datasheet
	uint16_t baud = 26; // Max setting of 8191
	UART_BDH = (uint8_t)(baud >> 8);
	UART_BDL = (uint8_t)baud;
	UART_C4 = 0x02;

#elif defined(_kii_v2_) // UART2 Debug
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

#elif defined(_kii_v3_)
	uint16_t div = F_CPU / (UART_BAUD*16);
	UART_PERIPH->UART_BRGR = UART_BRGR_CD(div);
#endif

#if defined(_kinetis_)
	// 8 bit, No Parity, Idle Character bit after stop
	UART_C1 = UART_C1_ILT;
#elif defined(_sam_)
	// 8 bit, No Parity, 1 stop bit
	UART_PERIPH->UART_MR = UART_MR_PAR_NO | UART_MR_CHMODE_NORMAL;
#endif

	// Interrupt notification watermarks
#if defined(_kii_v1_) || defined(_teensy_3_) // UART0 Debug
	UART_TWFIFO = 2;
	UART_RWFIFO = 4;
#elif defined(_kii_v2_) // UART2 Debug
	// UART2 has a single byte FIFO
	UART_TWFIFO = 1;
	UART_RWFIFO = 1;
#endif

	// TX FIFO Enabled, TX FIFO Size 1 (Max 8 datawords), RX FIFO Enabled, RX FIFO Size 1 (Max 8 datawords)
	// TX/RX FIFO Size:
	//  0x0 - 1 dataword
	//  0x1 - 4 dataword
	//  0x2 - 8 dataword
#if defined(_kinetis_)
	UART_PFIFO = UART_PFIFO_TXFE | UART_PFIFO_RXFE;
#endif

	// Reciever Inversion Disabled, LSBF
	// UART_S2_RXINV UART_S2_MSBF
#if defined(_kinetis_)
	UART_S2 |= 0x00;
#endif

	// Transmit Inversion Disabled
	// UART_C3_TXINV
#if defined(_kinetis_)
	UART_C3 |= 0x00;
#endif

	// TX Enabled, RX Enabled, RX Interrupt Enabled, Generate idles
	// UART_C2_TE UART_C2_RE UART_C2_RIE UART_C2_ILIE
#if defined(_kinetis_)
	UART_C2 = UART_C2_TE | UART_C2_RE | UART_C2_RIE | UART_C2_ILIE;
#elif defined(_sam_)
	UART_PERIPH->UART_CR = UART_CR_TXEN | UART_CR_RXEN;
	UART_PERIPH->UART_IER = UART_IER_RXRDY;
#endif

	// Add interrupt to the vector table (slightly higher than USB)
#if defined(_kinetis_)
	NVIC_SET_PRIORITY( IRQ_UART_STATUS, UART_Priority_define );
	NVIC_ENABLE_IRQ( IRQ_UART_STATUS );
#elif defined(_sam_)
	NVIC_SetPriority( UART_IRQ, UART_Priority_define );
	NVIC_EnableIRQ( UART_IRQ );
#endif

	// UART is now ready to use
	uart_configured = 1;
}

void uart_buffer_append(char c)
{
	uart_buffer[uart_buffer_tail++] = c;
	uart_buffer_items++;

	// Wrap-around of tail pointer
	if ( uart_buffer_tail >= uart_buffer_size )
	{
		uart_buffer_tail = 0;
	}

	// Make sure the head pointer also moves if circular buffer is overwritten
	if ( uart_buffer_head == uart_buffer_tail )
	{
		uart_buffer_head++;
	}

	// Wrap-around of head pointer
	if ( uart_buffer_head >= uart_buffer_size )
	{
		uart_buffer_head = 0;
	}
}

// Get the next character, or -1 if nothing received
int uart_serial_getchar()
{
	if ( !uart_configured )
		return -1;

	unsigned int value = -1;

	// Check to see if the FIFO has characters
	if ( uart_buffer_items > 0 )
	{
		value = uart_buffer[uart_buffer_head++];
		uart_buffer_items--;

		// Wrap-around of head pointer
		if ( uart_buffer_head >= uart_buffer_size )
		{
			uart_buffer_head = 0;
		}
	}

	return value;
}


// Number of bytes available in the receive buffer
int uart_serial_available()
{
	return uart_buffer_items;
}


// Discard any buffered input
void uart_serial_flush_input()
{
	uart_buffer_head = 0;
	uart_buffer_tail = 0;
	uart_buffer_items = 0;
}


// Transmit a character.  0 returned on success, -1 on error
int uart_serial_putchar( uint8_t c )
{
	if ( !uart_configured )
		return -1;

#if defined(_kinetis_)
	while ( !( UART_SFIFO & UART_SFIFO_TXEMPT ) ); // Wait till there is room to send
	UART_D = c;
#elif defined(_sam_)
	while ( !(UART_PERIPH->UART_SR & UART_SR_TXRDY) ); // Wait till tx ready
	UART0->UART_THR = c;
#endif

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
#if defined(_kinetis_)
		while ( !( UART_SFIFO & UART_SFIFO_TXEMPT ) ); // Wait till there is room to send
		UART_D = data[position++];
#elif defined(_sam_)
		while ( !( UART_PERIPH->UART_SR & UART_SR_TXRDY ) ); //Wait till tx ready
		UART0->UART_THR = data[position++];
#endif
	}

	return 0;
}


void uart_serial_flush_output()
{
	// Delay until buffer has been sent
#if defined(_kinetis_)
	while ( !( UART_SFIFO & UART_SFIFO_TXEMPT ) ); // Wait till there is room to send
#elif defined(_sam_)
	while ( !( UART_PERIPH->UART_SR & UART_SR_TXEMPTY ) );
#endif
}


void uart_device_reload()
{
	if ( flashModeEnabled_define == 0 )
	{
		print( NL );
		warn_printNL("flashModeEnabled not set, cancelling firmware reload...");
		info_print("Set flashModeEnabled to 1 in your kll configuration.");
		return;
	}

// MCHCK
#if defined(_kii_v1_)

	// MCHCK Kiibohd Variant
	// Check to see if PTA3 (has a pull-up) is connected to GND (usually via jumper)
	// Only allow reload if the jumper is present (security)
	GPIOA_PDDR &= ~(1<<3); // Input
	PORTA_PCR3 = PORT_PCR_PFE | PORT_PCR_MUX(1); // Internal pull-up

	// Check for jumper
	if ( GPIOA_PDIR & (1<<3) && flashModeEnabled_define != 0 )
	{
		print( NL );
		warn_printNL("Security jumper not present, cancelling firmware reload...");
		info_print("Replace jumper on middle 2 pins, or manually press the firmware reload button.");
	}
	else
	{
		// Copies variable into the VBAT register, must be identical to the variable in the bootloader to jump to the bootloader flash mode
		for ( int pos = 0; pos < sizeof(sys_reset_to_loader_magic); pos++ )
			(&VBAT)[ pos ] = sys_reset_to_loader_magic[ pos ];
		SOFTWARE_RESET();
	}

// Kiibohd mk20dx256vlh7
#elif defined(_kii_v2_)
	// Copies variable into the VBAT register, must be identical to the variable in the bootloader to jump to the bootloader flash mode
	for ( int pos = 0; pos < sizeof(sys_reset_to_loader_magic); pos++ )
		(&VBAT)[ pos ] = sys_reset_to_loader_magic[ pos ];
	SOFTWARE_RESET();

#elif defined(_kii_v3_)
	SOFTWARE_RESET();

// Teensy 3.0 and 3.1
#else
	asm volatile("bkpt");
#endif
}

