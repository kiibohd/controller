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

#ifndef usb_keyboard_serial_h__
#define usb_keyboard_serial_h__

// Compiler Includes
#include <stdint.h>

// AVR Includes
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// AVR Util Includes
#include <util/delay.h>

// Local Includes
#include "output_com.h"


// ----- Function Declarations -----

// Basic USB Configuration
void usb_init(void);			// initialize everything
uint8_t usb_configured(void);		// is the USB port configured

// Keyboard HID Functions
int8_t usb_keyboard_send(void);

// Chip Level Functions
void usb_debug_reload();                // Enable firmware reflash mode
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3"))); // Needed for software reset

// USB Serial CDC Functions
int16_t usb_serial_getchar(void);	// receive a character (-1 if timeout/error)
uint8_t usb_serial_available(void);	// number of bytes in receive buffer
void usb_serial_flush_input(void);	// discard any buffered input

// transmitting data
int8_t usb_serial_putchar(uint8_t c);	// transmit a character
int8_t usb_serial_putchar_nowait(uint8_t c);  // transmit a character, do not wait
int8_t usb_serial_write(const char *buffer, uint16_t size); // transmit a buffer
void usb_serial_flush_output(void);	// immediately transmit any buffered output

// serial parameters
uint32_t usb_serial_get_baud(void);	// get the baud rate
uint8_t usb_serial_get_stopbits(void);	// get the number of stop bits
uint8_t usb_serial_get_paritytype(void);// get the parity type
uint8_t usb_serial_get_numbits(void);	// get the number of data bits
uint8_t usb_serial_get_control(void);	// get the RTS and DTR signal state
int8_t usb_serial_set_control(uint8_t signals); // set DSR, DCD, RI, etc



// ----- Macros -----

// Software reset the chip
#define usb_debug_software_reset() do { wdt_enable( WDTO_15MS ); for(;;); } while(0)

#define EP_SIZE(s)	((s) == 64 ? 0x30 :	\
			((s) == 32 ? 0x20 :	\
			((s) == 16 ? 0x10 :	\
			             0x00)))

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)



// ----- Defines -----

// constants corresponding to the various serial parameters
#define USB_SERIAL_DTR			0x01
#define USB_SERIAL_RTS			0x02
#define USB_SERIAL_1_STOP		0
#define USB_SERIAL_1_5_STOP		1
#define USB_SERIAL_2_STOP		2
#define USB_SERIAL_PARITY_NONE		0
#define USB_SERIAL_PARITY_ODD		1
#define USB_SERIAL_PARITY_EVEN		2
#define USB_SERIAL_PARITY_MARK		3
#define USB_SERIAL_PARITY_SPACE		4
#define USB_SERIAL_DCD			0x01
#define USB_SERIAL_DSR			0x02
#define USB_SERIAL_BREAK		0x04
#define USB_SERIAL_RI			0x08
#define USB_SERIAL_FRAME_ERR		0x10
#define USB_SERIAL_PARITY_ERR		0x20
#define USB_SERIAL_OVERRUN_ERR		0x40

#define EP_TYPE_CONTROL			0x00
#define EP_TYPE_BULK_IN			0x81
#define EP_TYPE_BULK_OUT		0x80
#define EP_TYPE_INTERRUPT_IN		0xC1
#define EP_TYPE_INTERRUPT_OUT		0xC0
#define EP_TYPE_ISOCHRONOUS_IN		0x41
#define EP_TYPE_ISOCHRONOUS_OUT		0x40

#define EP_SINGLE_BUFFER		0x02
#define EP_DOUBLE_BUFFER		0x06

#define MAX_ENDPOINT		4

#if defined(__AVR_AT90USB162__)
#define HW_CONFIG()
#define PLL_CONFIG() (PLLCSR = ((1<<PLLE)|(1<<PLLP0)))
#define USB_CONFIG() (USBCON = (1<<USBE))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))

#elif defined(__AVR_ATmega32U4__)
#define HW_CONFIG() (UHWCON = 0x01)
#define PLL_CONFIG() (PLLCSR = 0x12)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))

#elif defined(__AVR_AT90USB646__)
#define HW_CONFIG() (UHWCON = 0x81)
#define PLL_CONFIG() (PLLCSR = 0x1A)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))

#elif defined(__AVR_AT90USB1286__)
#define HW_CONFIG() (UHWCON = 0x81)
#define PLL_CONFIG() (PLLCSR = 0x16)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#endif

// standard control endpoint request types
#define GET_STATUS			0
#define CLEAR_FEATURE			1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR			6
#define GET_CONFIGURATION		8
#define SET_CONFIGURATION		9
#define GET_INTERFACE			10
#define SET_INTERFACE			11

// HID (human interface device)
#define HID_GET_REPORT			1
#define HID_GET_IDLE			2
#define HID_GET_PROTOCOL		3
#define HID_SET_REPORT			9
#define HID_SET_IDLE			10
#define HID_SET_PROTOCOL		11

// CDC (communication class device)
#define CDC_SET_LINE_CODING		0x20
#define CDC_GET_LINE_CODING		0x21
#define CDC_SET_CONTROL_LINE_STATE	0x22



// ----- Endpoint Configuration -----

#define ENDPOINT0_SIZE		32

#define KEYBOARD_INTERFACE	0
#define KEYBOARD_ENDPOINT	2
#define KEYBOARD_SIZE		8
#define KEYBOARD_INTERVAL	1
#define KEYBOARD_HID_BUFFER	EP_DOUBLE_BUFFER

#define CDC_IAD_DESCRIPTOR	1
#define CDC_STATUS_INTERFACE	1
#define CDC_DATA_INTERFACE	2
#define CDC_ACM_ENDPOINT	3
#define CDC_RX_ENDPOINT		4
#define CDC_TX_ENDPOINT		5
#if defined(__AVR_AT90USB162__)
#define CDC_ACM_SIZE		16
#define CDC_ACM_BUFFER		EP_SINGLE_BUFFER
#define CDC_RX_SIZE		32
#define CDC_RX_BUFFER 		EP_DOUBLE_BUFFER
#define CDC_TX_SIZE		32
#define CDC_TX_BUFFER		EP_DOUBLE_BUFFER
#else
#define CDC_ACM_SIZE		16
#define CDC_ACM_BUFFER		EP_SINGLE_BUFFER
#define CDC_RX_SIZE		64
#define CDC_RX_BUFFER 		EP_DOUBLE_BUFFER
#define CDC_TX_SIZE		64
#define CDC_TX_BUFFER		EP_DOUBLE_BUFFER
#endif

// Endpoint 0 is reserved for the control endpoint
// Endpoint 1 has a 256 byte buffer
// Endpoints 2-6 have 64 byte buffers
static const uint8_t PROGMEM endpoint_config_table[] = {
	0, // 256 byte
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(KEYBOARD_SIZE) | KEYBOARD_HID_BUFFER, // 64 byte
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(CDC_ACM_SIZE)  | CDC_ACM_BUFFER,      // 64 byte
	1, EP_TYPE_BULK_OUT,      EP_SIZE(CDC_RX_SIZE)   | CDC_RX_BUFFER,       // 64 byte
	1, EP_TYPE_BULK_IN,       EP_SIZE(CDC_TX_SIZE)   | CDC_TX_BUFFER,       // 64 byte
};

#endif // usb_keyboard_serial_h__

