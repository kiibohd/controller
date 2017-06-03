/*
 * Copyright (C) 2014 Jan Rychter
 * Modifications (C) 2015-2017 Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files ( the "Software" ), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// ----- Includes ----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <print.h>
#include <kll_defs.h>

// Local Includes
#include "i2c.h"



// ----- Variables -----

volatile I2C_Channel i2c_channels[ISSI_I2C_Buses_define];

uint32_t i2c_offset[] = {
	0x0,    // Bus 0
	0x1000, // Bus 1
};



// ----- Functions -----

void i2c_setup()
{
	for ( uint8_t ch = 0; ch < ISSI_I2C_Buses_define; ch++ )
	{
		volatile uint8_t *I2C_F   = (uint8_t*)(&I2C0_F) + i2c_offset[ch];
		volatile uint8_t *I2C_FLT = (uint8_t*)(&I2C0_FLT) + i2c_offset[ch];
		volatile uint8_t *I2C_C1  = (uint8_t*)(&I2C0_C1) + i2c_offset[ch];
		volatile uint8_t *I2C_C2  = (uint8_t*)(&I2C0_C2) + i2c_offset[ch];

		switch ( ch )
		{
		case 0:
			// Enable I2C internal clock
			SIM_SCGC4 |= SIM_SCGC4_I2C0; // Bus 0

			// External pull-up resistor
			PORTB_PCR0 = PORT_PCR_ODE | PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(2);
			PORTB_PCR1 = PORT_PCR_ODE | PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(2);

			break;

#if defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
		case 1:
			// Enable I2C internal clock
			SIM_SCGC4 |= SIM_SCGC4_I2C1; // Bus 1

			// External pull-up resistor
			PORTC_PCR10 = PORT_PCR_ODE | PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(2);
			PORTC_PCR11 = PORT_PCR_ODE | PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(2);

			break;
#endif
		}

		// SCL Frequency Divider
#if ISSI_Chip_31FL3731_define == 1
		// 0x40 -> 36 MHz / (4 * 28) = 321.428 kBaud
		// 0x80 => mul(4)
		// 0x00 => ICL(28)
		*I2C_F = 0x84;
		*I2C_FLT = 0x03;
#elif ISSI_Chip_31FL3732_define == 1 || ISSI_Chip_31FL3733_define
		/*
		// Works
		// 0x40 -> 36 MHz / (4 * 28) = 321.428 kBaud
		// 0x80 => mul(4)
		// 0x00 => ICL(28)
		*I2C_F = 0x84;
		*I2C_FLT = 0x02;

		// Also works, 80 fps, no errors (flicker?)
		// 0x40 -> 36 MHz / (1 * 44) = 818.181 kBaud
		// 0x80 => mul(1)
		// 0x0C => ICL(44)
		*I2C_F = 0x0C;
		*I2C_FLT = 0x02; // Glitch protection, reduce if you see bus errors
		*/

		// Also works, 86 fps, no errors, using frame delay of 50 us
		// 0x40 -> 36 MHz / (2 * 20) = 900 kBaud
		// 0x40 => mul(2)
		// 0x00 => ICL(20)
		*I2C_F = 0x40;
		*I2C_FLT = 0x02;
#endif
		*I2C_C1 = I2C_C1_IICEN;
		*I2C_C2 = I2C_C2_HDRS; // High drive select

		switch ( ch )
		{
		case 0:
			// Enable I2C Interrupt
			NVIC_ENABLE_IRQ( IRQ_I2C0 );
			break;

#if defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
		case 1:

			// Enable I2C Interrupt
			NVIC_ENABLE_IRQ( IRQ_I2C1 );
			break;
#endif
		}
	}
}

void i2c_reset()
{
	// Cleanup after an I2C error
	for ( uint8_t ch = 0; ch < ISSI_I2C_Buses_define; ch++ )
	{
		volatile I2C_Channel *channel = &( i2c_channels[ch] );
		channel->status = I2C_AVAILABLE;
	}

	i2c_setup();
}

uint8_t i2c_busy( uint8_t ch )
{
	volatile I2C_Channel *channel = &( i2c_channels[ch] );
	if ( channel->status == I2C_BUSY )
	{
		return 1;
	}

	return 0;
}

uint8_t i2c_any_busy()
{
	for ( uint8_t ch = 0; ch < ISSI_I2C_Buses_define; ch++ )
	{
		if ( i2c_busy( ch ) )
			return 1;
	}
	return 0;
}


// These are here for readability and correspond to bit 0 of the address byte.
#define I2C_WRITING 0
#define I2C_READING 1

int32_t i2c_send_sequence(
	uint8_t ch,
	uint16_t *sequence,
	uint32_t sequence_length,
	uint8_t *received_data,
	void ( *callback_fn )( void* ),
	void *user_data
) {
	volatile I2C_Channel *channel = &( i2c_channels[ch] );

	volatile uint8_t *I2C_C1  = (uint8_t*)(&I2C0_C1) + i2c_offset[ch];
	volatile uint8_t *I2C_S   = (uint8_t*)(&I2C0_S) + i2c_offset[ch];
	volatile uint8_t *I2C_D   = (uint8_t*)(&I2C0_D) + i2c_offset[ch];

	int32_t result = 0;
	uint8_t status;

	if ( channel->status == I2C_BUSY )
	{
		return -1;
	}

	// Debug
	/*
	for ( uint8_t c = 0; c < sequence_length; c++ )
	{
		printHex( sequence[c] );
		print(" ");
	}
	print(NL);
	*/

	channel->sequence = sequence;
	channel->sequence_end = sequence + sequence_length;
	channel->received_data = received_data;
	channel->status = I2C_BUSY;
	channel->txrx = I2C_WRITING;
	channel->callback_fn = callback_fn;
	channel->user_data = user_data;

	// reads_ahead does not need to be initialized

	// Acknowledge the interrupt request, just in case
	*I2C_S |= I2C_S_IICIF;
	*I2C_C1 = ( I2C_C1_IICEN | I2C_C1_IICIE );

	// Generate a start condition and prepare for transmitting.
	*I2C_C1 |= ( I2C_C1_MST | I2C_C1_TX );

	status = *I2C_S;
	if ( status & I2C_S_ARBL )
	{
		warn_print("Arbitration lost");
		result = -1;
		goto i2c_send_sequence_cleanup;
	}

	// Write the first (address) byte.
	*I2C_D = *channel->sequence++;

	// Everything is OK.
	return result;

i2c_send_sequence_cleanup:
	*I2C_C1 &= ~( I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TX );
	channel->status = I2C_ERROR;
	return result;
}


void i2c_isr( uint8_t ch )
{
	volatile I2C_Channel* channel = &i2c_channels[ch];

	volatile uint8_t *I2C_C1  = (uint8_t*)(&I2C0_C1) + i2c_offset[ch];
	volatile uint8_t *I2C_S   = (uint8_t*)(&I2C0_S) + i2c_offset[ch];
	volatile uint8_t *I2C_D   = (uint8_t*)(&I2C0_D) + i2c_offset[ch];

	uint16_t element;
	uint8_t status;

	status = *I2C_S;

	// Acknowledge the interrupt request
	*I2C_S |= I2C_S_IICIF;

	// Arbitration problem
	if ( status & I2C_S_ARBL )
	{
		warn_msg("Arbitration error. Bus: ");
		printHex( ch );
		print(NL);

		*I2C_S |= I2C_S_ARBL;
		goto i2c_isr_error;
	}

	if ( channel->txrx == I2C_READING )
	{

		switch( channel->reads_ahead )
		{
		// All the reads in the sequence have been processed ( but note that the final data register read still needs to
		// be done below! Now, the next thing is either a restart or the end of a sequence. In any case, we need to
		// switch to TX mode, either to generate a repeated start condition, or to avoid triggering another I2C read
		// when reading the contents of the data register.
		case 0:
			*I2C_C1 |= I2C_C1_TX;

			// Perform the final data register read now that it's safe to do so.
			*channel->received_data++ = *I2C_D;

			// Do we have a repeated start?
			if ( ( channel->sequence < channel->sequence_end ) && ( *channel->sequence == I2C_RESTART ) )
			{

				// Generate a repeated start condition.
				*I2C_C1 |= I2C_C1_RSTA;

				// A restart is processed immediately, so we need to get a new element from our sequence. This is safe, because
				// a sequence cannot end with a RESTART: there has to be something after it. Note that the only thing that can
				// come after a restart is an address write.
				channel->txrx = I2C_WRITING;
				channel->sequence++;
				element = *channel->sequence;
				*I2C_D = element;
			}
			else
			{
				goto i2c_isr_stop;
			}
			break;

		case 1:
			// do not ACK the final read
			*I2C_C1 |= I2C_C1_TXAK;
			*channel->received_data++ = *I2C_D;
			break;

		default:
			*channel->received_data++ = *I2C_D;
			break;
		}

		channel->reads_ahead--;

	}
	// channel->txrx == I2C_WRITING
	else
	{
		// First, check if we are at the end of a sequence.
		if ( channel->sequence == channel->sequence_end )
			goto i2c_isr_stop;

		// We received a NACK. Generate a STOP condition and abort.
		if ( status & I2C_S_RXAK )
		{
			warn_print("NACK Received");
			goto i2c_isr_error;
		}

		// check next thing in our sequence
		element = *channel->sequence;

		// Do we have a restart? If so, generate repeated start and make sure TX is on.
		if ( element == I2C_RESTART )
		{
			*I2C_C1 |= I2C_C1_RSTA | I2C_C1_TX;

			// A restart is processed immediately, so we need to get a new element from our sequence.
			// This is safe, because a sequence cannot end with a RESTART: there has to be something after it.
			channel->sequence++;
			element = *channel->sequence;

			// Note that the only thing that can come after a restart is a write.
			*I2C_D = element;
		}
		else
		{
			if ( element == I2C_READ ) {
				channel->txrx = I2C_READING;
				// How many reads do we have ahead of us ( not including this one )?
				// For reads we need to know the segment length to correctly plan NACK transmissions.
				// We already know about one read
				channel->reads_ahead = 1;
				while (
					(  ( channel->sequence + channel->reads_ahead ) < channel->sequence_end ) &&
					( *( channel->sequence + channel->reads_ahead ) == I2C_READ )
				) {
					channel->reads_ahead++;
				}

				// Switch to RX mode.
				*I2C_C1 &= ~I2C_C1_TX;

				// do not ACK the final read
				if ( channel->reads_ahead == 1 )
				{
					*I2C_C1 |= I2C_C1_TXAK;
				}
				// ACK all but the final read
				else
				{
					*I2C_C1 &= ~( I2C_C1_TXAK );
				}

				// Dummy read comes first, note that this is not valid data!
				// This only triggers a read, actual data will come in the next interrupt call and overwrite this.
				// This is why we do not increment the received_data pointer.
				*channel->received_data = *I2C_D;
				channel->reads_ahead--;
			}
			// Not a restart, not a read, must be a write.
			else
			{
				*I2C_D = element;
			}
		}
	}

	channel->sequence++;
	return;

i2c_isr_stop:
	// Generate STOP ( set MST=0 ), switch to RX mode, and disable further interrupts.
	*I2C_C1 &= ~( I2C_C1_MST | I2C_C1_IICIE | I2C_C1_TXAK );
	channel->status = I2C_AVAILABLE;

	// Call the user-supplied callback function upon successful completion (if it exists).
	if ( channel->callback_fn )
	{
		// Delay 10 microseconds before starting linked function
		// TODO, is this chip dependent? -HaaTa
		delayMicroseconds(10);
		( *channel->callback_fn )( channel->user_data );
	}
	return;

i2c_isr_error:
	// Generate STOP and disable further interrupts.
	*I2C_C1 &= ~( I2C_C1_MST | I2C_C1_IICIE );
	channel->status = I2C_ERROR;
	return;
}

void i2c0_isr()
{
	i2c_isr( 0 );
}

void i2c1_isr()
{
	i2c_isr( 1 );
}

