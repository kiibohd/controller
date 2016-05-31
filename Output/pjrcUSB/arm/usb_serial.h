/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander (2013-2016)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

// ----- Includes -----

// Compiler Includes
#include <inttypes.h>



// ----- Defines -----

// Compatibility defines from AVR
#define PROGMEM
#define PGM_P  const char *
#define PSTR(str) (str)

#define USB_SERIAL_DTR  0x01
#define USB_SERIAL_RTS  0x02



// ----- Structs -----

// See: Universal Serial Bus Class Definitions for Communication Devices 1.11 Table 50
// dwDTERate   - Baud Rate : 4 bytes
// bCharFormat - Stop Bits : 1 byte
//   0: 1   stop bit
//   1: 1.5 stop bits
//   2: 2   stop bits
// bParityType - Parity    : 1 byte
//   0: None
//   1: Odd
//   2: Even
//   3: Mark
//   4: Space
// bDataBits   - Data Bits : 1 byte
//  (5,6,7,8 or 16)
//
// Struct is 7 bytes wide
typedef struct USBCDCLineCoding {
	uint32_t dwDTERate;   // Baud Rate
	uint8_t  bCharFormat; // Stop Bits
	uint8_t  bParityType; // Parity
	uint8_t  bDataBits;   // Data Bits
} USBCDCLineCoding;



// ----- Variables -----

extern volatile USBCDCLineCoding usb_cdc_line_coding;

extern volatile uint8_t usb_cdc_line_rtsdtr;
extern volatile uint8_t usb_cdc_transmit_flush_timer;
extern volatile uint8_t usb_configuration;



// ----- Functions -----

int usb_serial_available();
int usb_serial_getchar();
int usb_serial_peekchar();
int usb_serial_putchar( uint8_t c );
int usb_serial_read( void *buffer, uint32_t size );
int usb_serial_write( const void *buffer, uint32_t size );

void usb_serial_flush_input();
void usb_serial_flush_output();

