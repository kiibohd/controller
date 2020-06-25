/*
 * Copyright (C) 2014 Jan Rychter
 * Modifications (C) 2015-2020 Jacob Alexander
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

#if defined(_kinetis_)
uint32_t i2c_offset[] = {
	0x0,    // Bus 0
	0x1000, // Bus 1
};
#elif defined(_sam_)
Twi *twi_devs[] = {
	TWI0, TWI1
};
#endif

// ----- Functions -----

void i2c_isr( uint8_t ch );

// Initialize error counters
void i2c_initial()
{
	for ( uint8_t ch = ISSI_I2C_FirstBus_define; ch < ISSI_I2C_Buses_define + ISSI_I2C_FirstBus_define; ch++ )
	{
		volatile I2C_Channel *channel = &( i2c_channels[ch - ISSI_I2C_FirstBus_define] );
		channel->error_count = 0;
		channel->last_error = 0; // No error to begin with (resets on successful transaction)
	}
}

void i2c_setup(uint8_t fast)
{
	for ( uint8_t ch = ISSI_I2C_FirstBus_define; ch < ISSI_I2C_Buses_define + ISSI_I2C_FirstBus_define; ch++ )
	{
#if defined(_kinetis_)
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

	#if defined(_kii_v2_)
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
	#if ISSI_Chip_31FL3731_define == 1 && defined(_kii_v1_)
		// 0x53 -> 48 MHz / (2 * 72) = 333.333 kBaud
		// 0x40 => mul(2)
		// 0x13 => ICR(30)
		*I2C_F = 0x53;
		*I2C_FLT = 0x05;
	#elif ISSI_Chip_31FL3731_define == 1 && defined(_kii_v2_)
		// 0x4E -> 36 MHz / (2 * 56) = 321.428 kBaud
		// 0x40 => mul(2)
		// 0x0E => ICR(56)
		*I2C_F = 0x4E;
		*I2C_FLT = 0x04;
	#elif ISSI_Chip_31FL3732_define == 1 || ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
		/*
		// Works
		// 0x84 -> 36 MHz / (4 * 28) = 321.428 kBaud
		// 0x80 => mul(4)
		// 0x04 => ICR(28)
		*I2C_F = 0x84;
		*I2C_FLT = 0x02;

		// Also works, 80 fps, no errors (flicker?)
		// 0x0C -> 36 MHz / (1 * 44) = 818.181 kBaud
		// 0x00 => mul(1)
		// 0x0C => ICR(44)
		*I2C_F = 0x0C;
		*I2C_FLT = 0x02; // Glitch protection, reduce if you see bus errors
		*/

		// Also works, 86 fps, no errors, using frame delay of 50 us
		// 0x40 -> 36 MHz / (2 * 20) = 900 kBaud
		// 0x40 => mul(2)
		// 0x00 => ICR(20)
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

			// Set priority below USB, but not too low to maintain performance
			NVIC_SET_PRIORITY( IRQ_PIT_CH0, I2C_Priority_define );
			break;

	#if defined(_kii_v2_)
		case 1:

			// Enable I2C Interrupt
			NVIC_ENABLE_IRQ( IRQ_I2C1 );

			// Set priority below USB, but not too low to maintain performance
			NVIC_SET_PRIORITY( IRQ_PIT_CH1, I2C_Priority_define );
			break;
	#endif
		}

#elif defined(_sam_)

#if ISSI_Chip_31FL3731_define == 1
#define BAUD_FAST 400000
#define CK_FAST 1
#define BAUD 400000
#define CK 1
#elif ISSI_Chip_31FL3732_define == 1 || ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
// XXX (HaaTa): ATSAM4S seems to have a limitation of about 400 kHz
//              This is the maximum the datasheet supports.
//              ASF errors out when specifying over 400 kHz.
//              Setting the Clock Divider at 1, allows for a safe 460 kHz on ATSAM4S.
//              If you're reading this, please use an SPI ISSI chip, SPI works great on ATSAM4S.
//              Going much above 460 kHz is going to require serious tuning and pcb adjustments.
//              Just make sure you run a lot of USB traffic to test for flickering.
//
// XXX (HaaTa): The purpose of BAUD_FAST is for initialization of the ISSI controller
//              as some ISSI chips are buggy when intialized too slowly.
//              Speeding up this section (which will not have any flickering) seems to reduce errors.
#define BAUD_FAST 900000
#define CK_FAST 1
#define BAUD 460000
#define CK 1
#endif

		switch ( ch )
		{
		case 0:
			// Enable Peripheral / Disable PIO
			PIOA->PIO_PDR = (1 << 3) | (1 << 4);
			// Enable i2c clock
			PMC->PMC_PCER0 = (1 << ID_TWI0);
			break;
		case 1:
			MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4 | CCFG_SYSIO_SYSIO5; // Switch PB4 from TDI to GPIO
			PIOB->PIO_PDR = (1 << 4) | (1 << 5);
			PMC->PMC_PCER0 = (1 << ID_TWI1);
			break;
		}

		Twi *twi_dev = twi_devs[ch];
		if (fast)
		{
			uint16_t div = (F_CPU/BAUD_FAST - 4) / (2<<CK_FAST);

			// Set clock
			twi_dev->TWI_CWGR = TWI_CWGR_CLDIV(div) + TWI_CWGR_CHDIV(div) + TWI_CWGR_CKDIV(CK_FAST);
		}
		else
		{
			uint16_t div = (F_CPU/BAUD - 4) / (2<<CK);

			// Set clock
			twi_dev->TWI_CWGR = TWI_CWGR_CLDIV(div) + TWI_CWGR_CHDIV(div) + TWI_CWGR_CKDIV(CK);
		}

		// Enable master mode
		twi_dev->TWI_CR = TWI_CR_MSDIS | TWI_CR_SVDIS;
		twi_dev->TWI_CR = TWI_CR_MSEN;

		switch ( ch )
		{
		case 0:
			NVIC_SetPriority(TWI0_IRQn, I2C_Priority_define);
			NVIC_EnableIRQ(TWI0_IRQn);
			break;
		case 1:
			NVIC_SetPriority(TWI1_IRQn, I2C_Priority_define);
			NVIC_EnableIRQ(TWI1_IRQn);
			break;
		}

#endif
	}
}

// Checks if any bus has errored
uint8_t i2c_error()
{
	for ( uint8_t ch = ISSI_I2C_FirstBus_define; ch < ISSI_I2C_Buses_define + ISSI_I2C_FirstBus_define; ch++ )
	{
		volatile I2C_Channel *channel = &( i2c_channels[ch - ISSI_I2C_FirstBus_define] );
		if ( channel->status == I2C_ERROR )
		{
			return 1;
		}
	}

	return 0;
}

void i2c_reset()
{
	// Cleanup after an I2C error
	for ( uint8_t ch = ISSI_I2C_FirstBus_define; ch < ISSI_I2C_Buses_define + ISSI_I2C_FirstBus_define; ch++ )
	{
		volatile I2C_Channel *channel = &( i2c_channels[ch - ISSI_I2C_FirstBus_define] );
		channel->status = I2C_AVAILABLE;
	}

	i2c_setup(0);
}

uint8_t i2c_busy( uint8_t ch )
{
	volatile I2C_Channel *channel = &( i2c_channels[ch - ISSI_I2C_FirstBus_define] );
	if ( channel->status == I2C_BUSY )
	{
		return 1;
	}

	return 0;
}

uint8_t i2c_any_busy()
{
	for ( uint8_t ch = ISSI_I2C_FirstBus_define; ch < ISSI_I2C_Buses_define + ISSI_I2C_FirstBus_define; ch++ )
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
	int32_t result = 0;

	volatile I2C_Channel *channel = &( i2c_channels[ch - ISSI_I2C_FirstBus_define] );
	uint8_t address;

#if defined(_kinetis_)
	uint8_t status;
	volatile uint8_t *I2C_C1  = (uint8_t*)(&I2C0_C1) + i2c_offset[ch];
	volatile uint8_t *I2C_S   = (uint8_t*)(&I2C0_S) + i2c_offset[ch];
	volatile uint8_t *I2C_D   = (uint8_t*)(&I2C0_D) + i2c_offset[ch];
#elif defined(_sam_)
	Twi *twi_dev = twi_devs[ch];
#endif

	if ( channel->status == I2C_BUSY )
	{
		return -1;
	}

	// Check if there are back-to-back errors
	// in succession
	if ( channel->last_error > 5 )
	{
		warn_print("I2C Bus Error: ");
		printInt8( ch );
		print(" errors: ");
		printInt32( channel->error_count );
		print( NL );
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

#if defined(_kinetis_)
	// Acknowledge the interrupt request, just in case
	*I2C_S |= I2C_S_IICIF;
	*I2C_C1 = ( I2C_C1_IICEN | I2C_C1_IICIE );

	// Generate a start condition and prepare for transmitting.
	*I2C_C1 |= ( I2C_C1_MST | I2C_C1_TX );

	status = *I2C_S;
	if ( status & I2C_S_ARBL )
	{
		warn_printNL("Arbitration lost");
		result = -1;
		goto i2c_send_sequence_cleanup;
	}

	// Write the first (address) byte.
	address = *channel->sequence++;
	*I2C_D = address;

	// Everything is OK.
	return result;

i2c_send_sequence_cleanup:
	// Record error, and reset last error counter
	channel->error_count++;
	channel->last_error++;

	// Generate STOP and disable further interrupts.
	*I2C_C1 &= ~( I2C_C1_IICIE | I2C_C1_MST | I2C_C1_TX );
	channel->status = I2C_ERROR;

#elif defined(_sam_)
	// Convert 8 bit address to 7bit + RW
	address = *channel->sequence++;
	//print("Address: ");
	//printHex( address );
	//print( NL );

	uint8_t mread = address & 1;
	address >>= 1;

	// Set slave address
	twi_dev->TWI_MMR = TWI_MMR_DADR(address) | (mread ? TWI_MMR_MREAD : 0);

	// Enable interrupts
	twi_dev->TWI_IER = TWI_IER_RXRDY | TWI_IER_TXRDY | TWI_IER_TXCOMP | TWI_IER_ARBLST;
	//twi_dev->TWI_IDR = 0xFFFFFFFF;

	// Generate a start condition
	twi_dev->TWI_CR |= TWI_CR_START;

	// Fire off the first read or write.
	// The first (address) byte is automatically trasmitted before any data
	// Arbitration errors will be handled in the isr
	i2c_isr(ch);

	// Everything is OK.
	return result;
#endif

	return result;
}


void i2c_isr( uint8_t ch )
{
	volatile I2C_Channel* channel = &i2c_channels[ch - ISSI_I2C_FirstBus_define];
#if defined(_kinetis_)
	volatile uint8_t *I2C_C1  = (uint8_t*)(&I2C0_C1) + i2c_offset[ch];
	volatile uint8_t *I2C_S   = (uint8_t*)(&I2C0_S) + i2c_offset[ch];
	volatile uint8_t *I2C_D   = (uint8_t*)(&I2C0_D) + i2c_offset[ch];
#elif defined(_sam_)
	Twi *twi_dev = twi_devs[ch];
#endif

	uint16_t element;
	uint8_t status;

#if defined(_kinetis_)
	status = *I2C_S;

	// Acknowledge the interrupt request
	*I2C_S |= I2C_S_IICIF;

	// Arbitration problem
	if ( status & I2C_S_ARBL )
	{
		/* XXX (HaaTa) I2C Debugging
		warn_print("Arbitration error. Bus: ");
		printHex( ch );
		print(NL);
		*/

		*I2C_S |= I2C_S_ARBL;
		goto i2c_isr_error;
	}
#elif defined(_sam_)
	status = twi_dev->TWI_SR;

	// Arbitration problem
	if ( status & TWI_SR_ARBLST )
	{
		warn_print("Arbitration error. Bus: ");
		printHex( ch );
		print(NL);
		goto i2c_isr_error;
	}
#endif

	if ( channel->txrx == I2C_READING )
	{
		switch( channel->reads_ahead )
		{
		// All the reads in the sequence have been processed ( but note that the final data register read still needs to
		// be done below! Now, the next thing is either a restart or the end of a sequence.
		case 0:
#if defined(_kinetis_)
		// In any case, we need to switch to TX mode, either to generate a repeated start condition, or to avoid
		// triggering another I2C read when reading the contents of the data register.
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
#elif defined(_sam_)

			// Perform the final data register read now that it's safe to do so.
			*channel->received_data++ = twi_dev->TWI_RHR;

			// Do we have a repeated start?
			if ( ( channel->sequence < channel->sequence_end ) && ( *channel->sequence == I2C_RESTART ) )
			{
				// Generate a repeated start condition.
				twi_dev->TWI_MMR &= ~TWI_MMR_MREAD;
				twi_dev->TWI_CR |= TWI_CR_START;

				// A restart is processed immediately, so we need to get a new element from our sequence. This is safe, because
				// a sequence cannot end with a RESTART: there has to be something after it. Note that the only thing that can
				// come after a restart is an address write.
				channel->txrx = I2C_WRITING;
				channel->sequence++;
				element = *channel->sequence;
				twi_dev->TWI_THR = element;
			}
			else
			{
				goto i2c_isr_stop;
			}
#endif
			break;

		case 1:
#if defined(_kinetis_)
			// do not ACK the final read
			*I2C_C1 |= I2C_C1_TXAK;
			*channel->received_data++ = *I2C_D;
#elif defined(_sam_)
			twi_dev->TWI_CR |= TWI_CR_STOP; //No ACK
			*channel->received_data++ = twi_dev->TWI_RHR;
#endif
			break;

		default:
#if defined(_kinetis_)
			*channel->received_data++ = *I2C_D;
#elif defined(_sam_)
			*channel->received_data++ = twi_dev->TWI_RHR;
#endif
			break;
		}

		//print("Read: ");
		//printHex( *channel->received_data );
		//print( NL );

		channel->reads_ahead--;

	}
	// channel->txrx == I2C_WRITING
	else
	{
		// First, check if we are at the end of a sequence.
		if ( channel->sequence == channel->sequence_end )
			goto i2c_isr_stop;

		// We received a NACK. Generate a STOP condition and abort.
#if defined(_kinetis_)
		if ( status & I2C_S_RXAK )
		{
			warn_printNL("NACK Received");
			goto i2c_isr_error;
		}
#elif defined(_sam_)
		if ( status & TWI_SR_NACK )
		{
			warn_printNL("NACK Received");
			goto i2c_isr_error;
		}
#endif

		// check next thing in our sequence
		element = *channel->sequence;

		// Do we have a restart? If so, generate repeated start and make sure TX is on.
		if ( element == I2C_RESTART )
		{
#if defined(_kinetis_)
			*I2C_C1 |= I2C_C1_RSTA | I2C_C1_TX;
#elif defined(_sam_)
			twi_dev->TWI_MMR &= ~TWI_MMR_MREAD;
			twi_dev->TWI_CR |= TWI_CR_START;
#endif

			// A restart is processed immediately, so we need to get a new element from our sequence.
			// This is safe, because a sequence cannot end with a RESTART: there has to be something after it.
			channel->sequence++;
			element = *channel->sequence;

			// Note that the only thing that can come after a restart is a write.
#if defined(_kinetis_)
			*I2C_D = element;
#elif defined(_sam_)
			twi_dev->TWI_THR = element;
#endif
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
#if defined(_kinetis_)
				*I2C_C1 &= ~I2C_C1_TX;
#elif defined(_sam_)
				twi_dev->TWI_MMR |= TWI_MMR_MREAD;
#endif

				// do not ACK the final read
				if ( channel->reads_ahead == 1 )
				{
#if defined(_kinetis_)
					*I2C_C1 |= I2C_C1_TXAK;
#elif defined(_sam_)
					twi_dev->TWI_CR |= TWI_CR_STOP;
#endif
				}
				// ACK all but the final read
				else
				{
#if defined(_kinetis_)
					*I2C_C1 &= ~( I2C_C1_TXAK );
#endif
				}

				// Dummy read comes first, note that this is not valid data!
				// This only triggers a read, actual data will come in the next interrupt call and overwrite this.
				// This is why we do not increment the received_data pointer.
#if defined(_kinetis_)
				*channel->received_data = *I2C_D;
#elif defined(_sam_)
				*channel->received_data = twi_dev->TWI_RHR;
#endif
				channel->reads_ahead--;
			}
			// Not a restart, not a read, must be a write.
			else
			{
				//print("WRITE: ");
				//printHex(element);
#if defined(_kinetis_)
				*I2C_D = element;
#elif defined(_sam_)
				if (!(status & TWI_SR_TXRDY)) return;
				twi_dev->TWI_THR = element;
#endif
				//print( NL );
			}
		}
	}

	channel->sequence++;
	channel->last_error = 0; // No error
	return;

i2c_isr_stop:
	// Generate STOP ( set MST=0 ), switch to RX mode, and disable further interrupts.
#if defined(_kinetis_)
	*I2C_C1 &= ~( I2C_C1_MST | I2C_C1_IICIE | I2C_C1_TXAK );
#elif defined(_sam_)
	//print("\r\n ----- STOP ----- \r\n");
	twi_dev->TWI_CR |= TWI_CR_STOP;
	twi_dev->TWI_MMR &= ~TWI_MMR_MREAD;
	twi_dev->TWI_IDR = 0xFFFFFFFF;
#endif
	channel->status = I2C_AVAILABLE;

	// Call the user-supplied callback function upon successful completion (if it exists).
	if ( channel->callback_fn )
	{
		// Delay before starting linked function
#if ISSI_Chip_31FL3731_define == 1 || ISSI_Chip_31FL3732_define == 1
		delay_us(25);
#elif ISSI_Chip_31FL3733_define == 1 || ISSI_Chip_31FL3736_define == 1
		delay_us(10);
#endif
		( *channel->callback_fn )( channel->user_data );
	}
	return;

i2c_isr_error:
	// Record error, and reset last error counter
	channel->error_count++;
	channel->last_error++;

	// Generate STOP and disable further interrupts.
	warn_printNL("ISR error");
#if defined(_kinetis_)
	*I2C_C1 &= ~( I2C_C1_MST | I2C_C1_IICIE );
#elif defined(_sam_)
	twi_dev->TWI_CR |= TWI_CR_STOP;
	twi_dev->TWI_IDR = 0xFFFFFFFF;
#endif
	channel->status = I2C_ERROR;
	return;
}

#if defined(_kinetis_)
void i2c0_isr()
{
	i2c_isr( 0 );
}

void i2c1_isr()
{
	i2c_isr( 1 );
}

#elif defined(_sam_)
void TWI0_Handler()
{
	i2c_isr( 0 );
}

void TWI1_Handler()
{
	i2c_isr( 1 );
}
#endif
