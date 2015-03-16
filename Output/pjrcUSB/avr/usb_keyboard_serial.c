/* USB Keyboard and CDC Serial Device for Teensy USB Development Board
 * Copyright (c) 2009 PJRC.COM, LLC
 * Modifications by Jacob Alexander (2011-2014)
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


// Local Includes
#include "usb_keyboard_serial.h"
#include <print.h>


// ----- Variables -----

// zero when we are not configured, non-zero when enumerated
static volatile uint8_t usb_configuration = 0;

// the time remaining before we transmit any partially full
// packet, or send a zero length packet.
static volatile uint8_t transmit_flush_timer = 0;
static uint8_t transmit_previous_timeout = 0;

// serial port settings (baud rate, control signals, etc) set
// by the PC.  These are ignored, but kept in RAM.
static uint8_t cdc_line_coding[7] = {0x00, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x08};
static uint8_t cdc_line_rtsdtr = 0;



// ----- USB Keyboard Functions -----

// Sends normal keyboard out to host
// NOTE: Make sure to match the descriptor
void usb_keyboard_toHost()
{
	uint8_t i;

	// Modifiers
	UEDATX = USBKeys_Modifiers;

	// Reserved Byte
	UEDATX = 0x00;

	// Normal Keys, only supports 6 in Boot mode
	for ( i = 0; i < 6; i++)
	{
		UEDATX = USBKeys_Keys[i];
	}
	UEINTX = 0x00;
}

// send the contents of USBKeys_Keys and USBKeys_Modifiers
inline void usb_keyboard_send()
{
	uint8_t intr_state, timeout;

	intr_state = SREG;
	timeout = UDFNUML + 50;

	// Ready to transmit keypresses?
	do
	{
		SREG = intr_state;

		// has the USB gone offline? or exceeded timeout?
		if ( !usb_configuration || UDFNUML == timeout )
		{
			erro_print("USB Offline? Timeout?");
			return;
		}

		// get ready to try checking again
		intr_state = SREG;
		cli();

		// If not using Boot protocol, send NKRO
		UENUM = USBKeys_Protocol ? KEYBOARD_NKRO_ENDPOINT : KEYBOARD_ENDPOINT;
	} while ( !( UEINTX & (1 << RWAL) ) );

	switch ( USBKeys_Protocol )
	{
	// Send boot keyboard interrupt packet(s)
	case 0:
		usb_keyboard_toHost();
		USBKeys_Changed = USBKeyChangeState_None;
		break;

	// Send NKRO keyboard interrupts packet(s)
	case 1:
		// Check system control keys
		if ( USBKeys_Changed & USBKeyChangeState_System )
		{
			UEDATX = 0x02; // ID
			UEDATX = USBKeys_SysCtrl;
			UEINTX = 0; // Finished with ID

			USBKeys_Changed &= ~USBKeyChangeState_System; // Mark sent
		}

		// Check consumer control keys
		if ( USBKeys_Changed & USBKeyChangeState_Consumer )
		{
			UEDATX = 0x03; // ID
			UEDATX = (uint8_t)(USBKeys_ConsCtrl & 0x00FF);
			UEDATX = (uint8_t)(USBKeys_ConsCtrl >> 8);
			UEINTX = 0; // Finished with ID

			USBKeys_Changed &= ~USBKeyChangeState_Consumer; // Mark sent
		}

		// Standard HID Keyboard
		if ( USBKeys_Changed )
		{
			UEDATX = 0x01; // ID

			// Modifiers
			UEDATX = USBKeys_Modifiers;

			// 4-49 (first 6 bytes)
			for ( uint8_t byte = 0; byte < 6; byte++ )
				UEDATX = USBKeys_Keys[ byte ];

			// 51-155 (Middle 14 bytes)
			for ( uint8_t byte = 6; byte < 20; byte++ )
				UEDATX = USBKeys_Keys[ byte ];

			// 157-164 (Next byte)
			for ( uint8_t byte = 20; byte < 21; byte++ )
				UEDATX = USBKeys_Keys[ byte ];

			// 176-221 (last 6 bytes)
			for ( uint8_t byte = 21; byte < 27; byte++ )
				UEDATX = USBKeys_Keys[ byte ];

			UEINTX = 0; // Finished with ID

			USBKeys_Changed = USBKeyChangeState_None; // Mark sent
		}

		break;
	}

	USBKeys_Idle_Count = 0;
	SREG = intr_state;
}



// ----- USB Virtual Serial Port (CDC) Functions -----

// get the next character, or -1 if nothing received
int16_t usb_serial_getchar()
{
	uint8_t c, intr_state;

	// interrupts are disabled so these functions can be
	// used from the main program or interrupt context,
	// even both in the same program!
	intr_state = SREG;
	cli();
	if (!usb_configuration) {
		SREG = intr_state;
		return -1;
	}
	UENUM = CDC_RX_ENDPOINT;
	retry:
	c = UEINTX;
	if (!(c & (1<<RWAL))) {
		// no data in buffer
		if (c & (1<<RXOUTI)) {
			UEINTX = 0x6B;
			goto retry;
		}
		SREG = intr_state;
		return -2;
	}
	// take one byte out of the buffer
	c = UEDATX;
	// if buffer completely used, release it
	if (!(UEINTX & (1<<RWAL))) UEINTX = 0x6B;
	SREG = intr_state;
	return c;
}

// number of bytes available in the receive buffer
uint8_t usb_serial_available()
{
	uint8_t n=0, i, intr_state;

	intr_state = SREG;
	cli();
	if (usb_configuration) {
		UENUM = CDC_RX_ENDPOINT;
		n = UEBCLX;
		if (!n) {
			i = UEINTX;
			if (i & (1<<RXOUTI) && !(i & (1<<RWAL))) UEINTX = 0x6B;
		}
	}
	SREG = intr_state;
	return n;
}

// discard any buffered input
void usb_serial_flush_input()
{
	uint8_t intr_state;

	if (usb_configuration) {
		intr_state = SREG;
		cli();
		UENUM = CDC_RX_ENDPOINT;
		while ((UEINTX & (1<<RWAL))) {
			UEINTX = 0x6B;
		}
		SREG = intr_state;
	}
}

// transmit a character.  0 returned on success, -1 on error
int8_t usb_serial_putchar( uint8_t c )
{
	uint8_t timeout, intr_state;

	// if we're not online (enumerated and configured), error
	if (!usb_configuration) return -1;
	// interrupts are disabled so these functions can be
	// used from the main program or interrupt context,
	// even both in the same program!
	intr_state = SREG;
	cli();
	UENUM = CDC_TX_ENDPOINT;
	// if we gave up due to timeout before, don't wait again
	if (transmit_previous_timeout) {
		if (!(UEINTX & (1<<RWAL))) {
			SREG = intr_state;
			return -1;
		}
		transmit_previous_timeout = 0;
	}
	// wait for the FIFO to be ready to accept data
	timeout = UDFNUML + TRANSMIT_TIMEOUT;
	while (1) {
		// are we ready to transmit?
		if (UEINTX & (1<<RWAL)) break;
		SREG = intr_state;
		// have we waited too long?  This happens if the user
		// is not running an application that is listening
		if (UDFNUML == timeout) {
			transmit_previous_timeout = 1;
			return -1;
		}
		// has the USB gone offline?
		if (!usb_configuration) return -1;
		// get ready to try checking again
		intr_state = SREG;
		cli();
		UENUM = CDC_TX_ENDPOINT;
	}
	// actually write the byte into the FIFO
	UEDATX = c;
	// if this completed a packet, transmit it now!
	if (!(UEINTX & (1<<RWAL))) UEINTX = 0x3A;
	transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
	SREG = intr_state;
	return 0;
}


// transmit a character, but do not wait if the buffer is full,
//   0 returned on success, -1 on buffer full or error
int8_t usb_serial_putchar_nowait( uint8_t c )
{
	uint8_t intr_state;

	if (!usb_configuration) return -1;
	intr_state = SREG;
	cli();
	UENUM = CDC_TX_ENDPOINT;
	if (!(UEINTX & (1<<RWAL))) {
		// buffer is full
		SREG = intr_state;
		return -2;
	}
	// actually write the byte into the FIFO
	UEDATX = c;
		// if this completed a packet, transmit it now!
	if (!(UEINTX & (1<<RWAL))) UEINTX = 0x3A;
	transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
	SREG = intr_state;
	return 0;
}

// transmit a buffer.
//  0 returned on success, -1 on error
// This function is optimized for speed!  Each call takes approx 6.1 us overhead
// plus 0.25 us per byte.  12 Mbit/sec USB has 8.67 us per-packet overhead and
// takes 0.67 us per byte.  If called with 64 byte packet-size blocks, this function
// can transmit at full USB speed using 43% CPU time.  The maximum theoretical speed
// is 19 packets per USB frame, or 1216 kbytes/sec.  However, bulk endpoints have the
// lowest priority, so any other USB devices will likely reduce the speed.  Speed
// can also be limited by how quickly the PC-based software reads data, as the host
// controller in the PC will not allocate bandwitdh without a pending read request.
// (thanks to Victor Suarez for testing and feedback and initial code)

int8_t usb_serial_write( const char *buffer, uint16_t size )
{
	uint8_t timeout, intr_state, write_size;

	// if we're not online (enumerated and configured), error
	if (!usb_configuration) return -1;
	// interrupts are disabled so these functions can be
	// used from the main program or interrupt context,
	// even both in the same program!
	intr_state = SREG;
	cli();
	UENUM = CDC_TX_ENDPOINT;
	// if we gave up due to timeout before, don't wait again

	if (transmit_previous_timeout) {
		if (!(UEINTX & (1<<RWAL))) {
			SREG = intr_state;
			return -2;
		}
		transmit_previous_timeout = 0;
	}

	// each iteration of this loop transmits a packet
	while (size) {
		// wait for the FIFO to be ready to accept data
		timeout = UDFNUML + TRANSMIT_TIMEOUT;
		while (1) {
			// are we ready to transmit?
			if (UEINTX & (1<<RWAL)) break;
			SREG = intr_state;
			// have we waited too long?  This happens if the user
			// is not running an application that is listening
			if (UDFNUML == timeout) {
				transmit_previous_timeout = 1;
				return -3;
			}
			// has the USB gone offline?
			if (!usb_configuration) return -4;
			// get ready to try checking again
			intr_state = SREG;
			cli();
			UENUM = CDC_TX_ENDPOINT;
		}

		// compute how many bytes will fit into the next packet
		write_size = CDC_TX_SIZE - UEBCLX;
		if (write_size > size) write_size = size;
		size -= write_size;

		// write the packet
		switch (write_size) {
			#if (CDC_TX_SIZE == 64)
			case 64: UEDATX = *buffer++;
			case 63: UEDATX = *buffer++;
			case 62: UEDATX = *buffer++;
			case 61: UEDATX = *buffer++;
			case 60: UEDATX = *buffer++;
			case 59: UEDATX = *buffer++;
			case 58: UEDATX = *buffer++;
			case 57: UEDATX = *buffer++;
			case 56: UEDATX = *buffer++;
			case 55: UEDATX = *buffer++;
			case 54: UEDATX = *buffer++;
			case 53: UEDATX = *buffer++;
			case 52: UEDATX = *buffer++;
			case 51: UEDATX = *buffer++;
			case 50: UEDATX = *buffer++;
			case 49: UEDATX = *buffer++;
			case 48: UEDATX = *buffer++;
			case 47: UEDATX = *buffer++;
			case 46: UEDATX = *buffer++;
			case 45: UEDATX = *buffer++;
			case 44: UEDATX = *buffer++;
			case 43: UEDATX = *buffer++;
			case 42: UEDATX = *buffer++;
			case 41: UEDATX = *buffer++;
			case 40: UEDATX = *buffer++;
			case 39: UEDATX = *buffer++;
			case 38: UEDATX = *buffer++;
			case 37: UEDATX = *buffer++;
			case 36: UEDATX = *buffer++;
			case 35: UEDATX = *buffer++;
			case 34: UEDATX = *buffer++;
			case 33: UEDATX = *buffer++;
			#endif
			#if (CDC_TX_SIZE >= 32)
			case 32: UEDATX = *buffer++;
			case 31: UEDATX = *buffer++;
			case 30: UEDATX = *buffer++;
			case 29: UEDATX = *buffer++;
			case 28: UEDATX = *buffer++;
			case 27: UEDATX = *buffer++;
			case 26: UEDATX = *buffer++;
			case 25: UEDATX = *buffer++;
			case 24: UEDATX = *buffer++;
			case 23: UEDATX = *buffer++;
			case 22: UEDATX = *buffer++;
			case 21: UEDATX = *buffer++;
			case 20: UEDATX = *buffer++;
			case 19: UEDATX = *buffer++;
			case 18: UEDATX = *buffer++;
			case 17: UEDATX = *buffer++;
			#endif
			#if (CDC_TX_SIZE >= 16)
			case 16: UEDATX = *buffer++;
			case 15: UEDATX = *buffer++;
			case 14: UEDATX = *buffer++;
			case 13: UEDATX = *buffer++;
			case 12: UEDATX = *buffer++;
			case 11: UEDATX = *buffer++;
			case 10: UEDATX = *buffer++;
			case  9: UEDATX = *buffer++;
			#endif
			case  8: UEDATX = *buffer++;
			case  7: UEDATX = *buffer++;
			case  6: UEDATX = *buffer++;
			case  5: UEDATX = *buffer++;
			case  4: UEDATX = *buffer++;
			case  3: UEDATX = *buffer++;
			case  2: UEDATX = *buffer++;
			default:
			case  1: UEDATX = *buffer++;
			case  0: break;
		}
		// if this completed a packet, transmit it now!
		if (!(UEINTX & (1<<RWAL))) UEINTX = 0x3A;
		transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
		SREG = intr_state;
	}
	return 0;
}

// immediately transmit any buffered output.
// This doesn't actually transmit the data - that is impossible!
// USB devices only transmit when the host allows, so the best
// we can do is release the FIFO buffer for when the host wants it
void usb_serial_flush_output()
{
	uint8_t intr_state;

	intr_state = SREG;
	cli();
	if (transmit_flush_timer) {
		UENUM = CDC_TX_ENDPOINT;
		UEINTX = 0x3A;
		transmit_flush_timer = 0;
	}
	SREG = intr_state;
}

// functions to read the various async serial settings.  These
// aren't actually used by USB at all (communication is always
// at full USB speed), but they are set by the host so we can
// set them properly if we're converting the USB to a real serial
// communication
uint32_t usb_serial_get_baud()
{
	uint32_t *baud = (uint32_t*)cdc_line_coding;
	return *baud;
}
uint8_t usb_serial_get_stopbits()
{
	return cdc_line_coding[4];
}
uint8_t usb_serial_get_paritytype()
{
	return cdc_line_coding[5];
}
uint8_t usb_serial_get_numbits()
{
	return cdc_line_coding[6];
}
uint8_t usb_serial_get_control()
{
	return cdc_line_rtsdtr;
}

// write the control signals, DCD, DSR, RI, etc
// There is no CTS signal.  If software on the host has transmitted
// data to you but you haven't been calling the getchar function,
// it remains buffered (either here or on the host) and can not be
// lost because you weren't listening at the right time, like it
// would in real serial communication.
int8_t usb_serial_set_control( uint8_t signals )
{
	uint8_t intr_state;

	intr_state = SREG;
	cli();
	if (!usb_configuration) {
		// we're not enumerated/configured
		SREG = intr_state;
		return -1;
	}

	UENUM = CDC_ACM_ENDPOINT;
	if (!(UEINTX & (1<<RWAL))) {
		// unable to write
		// TODO; should this try to abort the previously
		// buffered message??
		SREG = intr_state;
		return -1;
	}
	UEDATX = 0xA1;
	UEDATX = 0x20;
	UEDATX = 0;
	UEDATX = 0;
	UEDATX = 0; // 0 seems to work nicely.  what if this is 1??
	UEDATX = 0;
	UEDATX = 1;
	UEDATX = 0;
	UEDATX = signals;
	UEINTX = 0x3A;
	SREG = intr_state;
	return 0;
}



// ----- General USB Functions -----

// Set the avr into firmware reload mode
void usb_device_reload()
{
	cli();
	// Disable watchdog, if enabled
	// Disable all peripherals

	UDCON = 1;
	USBCON = (1 << FRZCLK);  // Disable USB
	UCSR1B = 0;
	_delay_ms( 5 );

#if defined(__AVR_AT90USB162__)                // Teensy 1.0
	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0;
	TIMSK0 = 0; TIMSK1 = 0; UCSR1B = 0;
	DDRB = 0; DDRC = 0; DDRD = 0;
	PORTB = 0; PORTC = 0; PORTD = 0;
	asm volatile("jmp 0x3E00");
#elif defined(__AVR_ATmega32U4__)              // Teensy 2.0
	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
	TIMSK0 = 0; TIMSK1 = 0; TIMSK3 = 0; TIMSK4 = 0; UCSR1B = 0; TWCR = 0;
	DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0; TWCR = 0;
	PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
	asm volatile("jmp 0x7E00");
#elif defined(__AVR_AT90USB646__)              // Teensy++ 1.0
	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
	TIMSK0 = 0; TIMSK1 = 0; TIMSK2 = 0; TIMSK3 = 0; UCSR1B = 0; TWCR = 0;
	DDRA = 0; DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0;
	PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
	asm volatile("jmp 0xFC00");
#elif defined(__AVR_AT90USB1286__)             // Teensy++ 2.0
	EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
	TIMSK0 = 0; TIMSK1 = 0; TIMSK2 = 0; TIMSK3 = 0; UCSR1B = 0; TWCR = 0;
	DDRA = 0; DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0;
	PORTA = 0; PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
	asm volatile("jmp 0x1FC00");
#endif
}


// WDT Setup for software reset the chip
void wdt_init()
{
	MCUSR = 0;
	wdt_disable();
}


// initialize USB
uint8_t usb_init()
{
	// Check to see if a usb cable has been plugged in
	// XXX Not tested (also, not currently needed) -HaaTa
	//if ( USB0_STAT & (1 << 1)
	//      return 0;

	HW_CONFIG();
	USB_FREEZE();                           // enable USB
	PLL_CONFIG();                           // config PLL
	while (!(PLLCSR & (1<<PLOCK))) ;        // wait for PLL lock
	USB_CONFIG();                           // start USB clock
	UDCON = 0;                              // enable attach resistor
	usb_configuration = 0;
	UDIEN = (1<<EORSTE) | (1<<SOFE);
	sei();

	// Disable watchdog timer after possible software reset
	//wdt_init(); // XXX Not working...seems to be ok without this, not sure though

	return 1;
}

// return 0 if the USB is not configured, or the configuration
// number selected by the HOST
uint8_t usb_configured()
{
	return usb_configuration;
}

// USB Device Interrupt - handle all device-level events
// the transmit buffer flushing is triggered by the start of frame
//
ISR( USB_GEN_vect )
{
	uint8_t intbits, t_cdc;

	intbits = UDINT;
	UDINT = 0;
	if ( intbits & (1 << EORSTI) )
	{
		UENUM = 0;
		UECONX = 1;
		UECFG0X = EP_TYPE_CONTROL;
		UECFG1X = EP_SIZE(ENDPOINT0_SIZE) | EP_SINGLE_BUFFER;
		UEIENX = (1 << RXSTPE);
		usb_configuration = 0;
		cdc_line_rtsdtr = 0;
	}
	if ( (intbits & (1 << SOFI)) && usb_configuration )
	{
		t_cdc = transmit_flush_timer;
		if ( t_cdc )
		{
			transmit_flush_timer = --t_cdc;
			if ( !t_cdc )
			{
				UENUM = CDC_TX_ENDPOINT;
				UEINTX = 0x3A;
			}
		}
		static uint8_t div4 = 0;
		if ( USBKeys_Idle_Config && (++div4 & 3) == 0 )
		{
			USBKeys_Idle_Count++;
			if ( USBKeys_Idle_Count == USBKeys_Idle_Config )
			{
				// XXX TODO Is this even used? If so, when? -Jacob
				// From hasu's code, this section looks like it could fix the Mac SET_IDLE problem
				// Send normal keyboard interrupt packet(s)
				switch ( USBKeys_Protocol )
				{
				// Send boot keyboard interrupt packet(s)
				case 0: usb_keyboard_toHost();     break;
				// Send NKRO keyboard interrupts packet(s)
				//case 1: usb_nkrokeyboard_toHost(); break; // XXX Not valid anymore
				}
				print("IDLE");
			}
		}
	}
}



// Misc functions to wait for ready and send/receive packets
static inline void usb_wait_in_ready()
{
	while (!(UEINTX & (1<<TXINI))) ;
}
static inline void usb_send_in()
{
	UEINTX = ~(1<<TXINI);
}
static inline void usb_wait_receive_out()
{
	while (!(UEINTX & (1<<RXOUTI))) ;
}
static inline void usb_ack_out()
{
	UEINTX = ~(1<<RXOUTI);
}



// USB Endpoint Interrupt - endpoint 0 is handled here.  The
// other endpoints are manipulated by the user-callable
// functions, and the start-of-frame interrupt.
//
ISR( USB_COM_vect )
{
	uint8_t intbits;
	const uint8_t *list;
	const uint8_t *cfg;
	uint8_t i, n, len, en;
	uint8_t *p;
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
	uint16_t desc_val;
	const uint8_t *desc_addr;
	uint8_t desc_length;

	UENUM = 0;
	intbits = UEINTX;
	if (intbits & (1<<RXSTPI))
	{
		bmRequestType = UEDATX;
		bRequest = UEDATX;
		wValue = UEDATX;
		wValue |= (UEDATX << 8);
		wIndex = UEDATX;
		wIndex |= (UEDATX << 8);
		wLength = UEDATX;
		wLength |= (UEDATX << 8);
		UEINTX = ~((1<<RXSTPI) | (1<<RXOUTI) | (1<<TXINI));

		if ( bRequest == GET_DESCRIPTOR )
		{
			list = (const uint8_t *)descriptor_list;
			for ( i = 0; ; i++ )
			{
				if ( i >= NUM_DESC_LIST )
				{
					UECONX = (1 << STALLRQ) | (1 << EPEN);  //stall
					return;
				}
				desc_val = pgm_read_word(list);
				if ( desc_val != wValue )
				{
					list += sizeof( struct descriptor_list_struct );
					continue;
				}
				list += 2;
				desc_val = pgm_read_word(list);
				if ( desc_val != wIndex )
				{
					list += sizeof(struct descriptor_list_struct) - 2;
					continue;
				}
				list += 2;
				desc_addr = (const uint8_t *)pgm_read_word(list);
				list += 2;
				desc_length = pgm_read_byte(list);
				break;
			}
			len = (wLength < 256) ? wLength : 255;
			if (len > desc_length) len = desc_length;
			do {
				// wait for host ready for IN packet
				do {
					i = UEINTX;
				} while (!(i & ((1<<TXINI)|(1<<RXOUTI))));
				if (i & (1<<RXOUTI)) return;    // abort
				// send IN packet
				n = len < ENDPOINT0_SIZE ? len : ENDPOINT0_SIZE;
				for (i = n; i; i--) {
					UEDATX = pgm_read_byte(desc_addr++);
				}
				len -= n;
				usb_send_in();
			} while (len || n == ENDPOINT0_SIZE);
			return;
		}

		if (bRequest == SET_ADDRESS) {
			usb_send_in();
			usb_wait_in_ready();
			UDADDR = wValue | (1<<ADDEN);
			return;
		}

		if ( bRequest == SET_CONFIGURATION && bmRequestType == 0 )
		{
			usb_configuration = wValue;
			cdc_line_rtsdtr = 0;
			transmit_flush_timer = 0;
			usb_send_in();
			cfg = endpoint_config_table;
			// Setup each of the 6 additional endpoints (0th already configured)
			for ( i = 1; i < 6; i++ )
			{
				UENUM = i;
				en = pgm_read_byte(cfg++);
				UECONX = en;
				if (en)
				{
					UECFG0X = pgm_read_byte(cfg++);
					UECFG1X = pgm_read_byte(cfg++);
				}
			}
			UERST = 0x7E;
			UERST = 0;
			return;
		}

		if (bRequest == GET_CONFIGURATION && bmRequestType == 0x80) {
			usb_wait_in_ready();
			UEDATX = usb_configuration;
			usb_send_in();
			return;
		}

		if ( ( wIndex == KEYBOARD_INTERFACE      && USBKeys_Protocol == 0 )
		  || ( wIndex == KEYBOARD_NKRO_INTERFACE && USBKeys_Protocol == 1 ) )
		{
			if ( bmRequestType == 0xA1)
			{
				if ( bRequest == HID_GET_REPORT )
				{
					usb_wait_in_ready();

					// Send normal keyboard interrupt packet(s)
					switch ( USBKeys_Protocol )
					{
					// Send boot keyboard interrupt packet(s)
					case 0: usb_keyboard_toHost();     break;
					// Send NKRO keyboard interrupts packet(s)
					//case 1: usb_nkrokeyboard_toHost(); break; // XXX Not valid anymore
					}

					usb_send_in();
					return;
				}
				if ( bRequest == HID_GET_IDLE )
				{
					usb_wait_in_ready();
					UEDATX = USBKeys_Idle_Config;
					usb_send_in();
					return;
				}
				if ( bRequest == HID_GET_PROTOCOL )
				{
					usb_wait_in_ready();
					UEDATX = USBKeys_Protocol;
					usb_send_in();
					return;
				}
			}
			if ( bmRequestType == 0x21 )
			{
				if ( bRequest == HID_SET_REPORT )
				{
					usb_wait_receive_out();
					USBKeys_LEDs = UEDATX;
					usb_ack_out();
					usb_send_in();
					return;
				}
				if ( bRequest == HID_SET_IDLE )
				{
					usb_wait_in_ready();
					USBKeys_Idle_Config = (wValue >> 8);
					USBKeys_Idle_Count = 0;
					usb_send_in();
					//print("HID IDLE");
					return;
				}
				if ( bRequest == HID_SET_PROTOCOL )
				{
					usb_wait_in_ready();
					USBKeys_Protocol = wValue; // 0 - Boot Mode, 1 - NKRO Mode
					usb_send_in();
					//print("HID SET");
					return;
				}
			}
		}

		if (bRequest == CDC_GET_LINE_CODING && bmRequestType == 0xA1) {
			usb_wait_in_ready();
			p = cdc_line_coding;
			for (i=0; i<7; i++) {
				UEDATX = *p++;
			}
			usb_send_in();
			return;
		}

		if (bRequest == CDC_SET_LINE_CODING && bmRequestType == 0x21) {
			usb_wait_receive_out();
			p = cdc_line_coding;
			for (i=0; i<7; i++) {
				*p++ = UEDATX;
			}
			usb_ack_out();
			usb_send_in();
			return;
		}

		if (bRequest == CDC_SET_CONTROL_LINE_STATE && bmRequestType == 0x21) {
			cdc_line_rtsdtr = wValue;
			usb_wait_in_ready();
			usb_send_in();
			return;
		}

		if (bRequest == GET_STATUS) {
			usb_wait_in_ready();
			i = 0;
			if (bmRequestType == 0x82) {
				UENUM = wIndex;
				if (UECONX & (1<<STALLRQ)) i = 1;
				UENUM = 0;
			}
			UEDATX = i;
			UEDATX = 0;
			usb_send_in();
			return;
		}

		if ((bRequest == CLEAR_FEATURE || bRequest == SET_FEATURE)
		  && bmRequestType == 0x02 && wValue == 0) {
			i = wIndex & 0x7F;
			if (i >= 1 && i <= MAX_ENDPOINT) {
				usb_send_in();
				UENUM = i;
				if (bRequest == SET_FEATURE) {
					UECONX = (1<<STALLRQ)|(1<<EPEN);
				} else {
					UECONX = (1<<STALLRQC)|(1<<RSTDT)|(1<<EPEN);
					UERST = (1 << i);
					UERST = 0;
				}
				return;
			}
		}
	}
	UECONX = (1 << STALLRQ) | (1 << EPEN);  // stall
}

