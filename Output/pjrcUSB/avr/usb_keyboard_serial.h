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
void usb_device_reload();               // Enable firmware reflash mode
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
#define usb_device_software_reset() do { wdt_enable( WDTO_15MS ); for(;;); } while(0)

// See EPSIZE -> UECFG1X - 128 and 256 bytes are for endpoint 1 only
#define EP_SIZE(s)	((s) == 256 ? 0x50 : \
			((s) == 128 ? 0x40 : \
			((s) ==  64 ? 0x30 : \
			((s) ==  32 ? 0x20 : \
			((s) ==  16 ? 0x10 : \
			              0x00)))))

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

// CDC Configuration
// When you write data, it goes into a USB endpoint buffer, which
// is transmitted to the PC when it becomes full, or after a timeout
// with no more writes.  Even if you write in exactly packet-size
// increments, this timeout is used to send a "zero length packet"
// that tells the PC no more data is expected and it should pass
// any buffered data to the application that may be waiting.  If
// you want data sent immediately, call usb_serial_flush_output().
#define TRANSMIT_FLUSH_TIMEOUT	5   /* in milliseconds */

// If the PC is connected but not "listening", this is the length
// of time before usb_serial_getchar() returns with an error.  This
// is roughly equivilant to a real UART simply transmitting the
// bits on a wire where nobody is listening, except you get an error
// code which you can ignore for serial-like discard of data, or
// use to know your data wasn't sent.
#define TRANSMIT_TIMEOUT	25   /* in milliseconds */



// ----- Endpoint Configuration -----

#define ENDPOINT0_SIZE        	 32

#define KEYBOARD_NKRO_INTERFACE  0
#define KEYBOARD_NKRO_ENDPOINT   1
#define KEYBOARD_NKRO_SIZE       16
#define KEYBOARD_NKRO_HID_BUFFER EP_DOUBLE_BUFFER

#define KEYBOARD_INTERFACE       1
#define KEYBOARD_ENDPOINT        2
#define KEYBOARD_SIZE        	 8
#define KEYBOARD_HID_BUFFER      EP_DOUBLE_BUFFER

#define CDC_IAD_DESCRIPTOR       1
#define CDC_STATUS_INTERFACE     2
#define CDC_DATA_INTERFACE       3
#define CDC_ACM_ENDPOINT         3
#define CDC_RX_ENDPOINT        	 4
#define CDC_TX_ENDPOINT        	 5
#if defined(__AVR_AT90USB162__)
#define CDC_ACM_SIZE        	 16
#define CDC_ACM_BUFFER        	 EP_SINGLE_BUFFER
#define CDC_RX_SIZE        	 32
#define CDC_RX_BUFFER         	 EP_DOUBLE_BUFFER
#define CDC_TX_SIZE        	 32
#define CDC_TX_BUFFER        	 EP_DOUBLE_BUFFER
#else
#define CDC_ACM_SIZE        	 16
#define CDC_ACM_BUFFER        	 EP_SINGLE_BUFFER
#define CDC_RX_SIZE        	 64
#define CDC_RX_BUFFER         	 EP_DOUBLE_BUFFER
#define CDC_TX_SIZE        	 64
#define CDC_TX_BUFFER        	 EP_DOUBLE_BUFFER
#endif

// Endpoint 0 is reserved for the control endpoint
// Endpoint 1 has a 256 byte buffer
// Endpoints 2-6 have 64 byte buffers
static const uint8_t PROGMEM endpoint_config_table[] = {
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(KEYBOARD_NKRO_SIZE) | KEYBOARD_NKRO_HID_BUFFER, // 256 byte
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(KEYBOARD_SIZE)      | KEYBOARD_HID_BUFFER,      // 64 byte
	1, EP_TYPE_INTERRUPT_IN,  EP_SIZE(CDC_ACM_SIZE)       | CDC_ACM_BUFFER,           // 64 byte
	1, EP_TYPE_BULK_OUT,      EP_SIZE(CDC_RX_SIZE)        | CDC_RX_BUFFER,            // 64 byte
	1, EP_TYPE_BULK_IN,       EP_SIZE(CDC_TX_SIZE)        | CDC_TX_BUFFER,            // 64 byte
	0,                                                                                // 64 byte
};



// ----- Descriptor Configuration -----

// Descriptors are the data that your computer reads when it auto-detects
// this USB device (called "enumeration" in USB lingo).  The most commonly
// changed items are editable at the top of this file.  Changing things
// in here should only be done by those who've read chapter 9 of the USB
// spec and relevant portions of any USB class specifications!


static const uint8_t PROGMEM device_descriptor[] = {
	18,					// bLength
	1,					// bDescriptorType
	0x00, 0x02,				// bcdUSB
	0,					// bDeviceClass
	0,					// bDeviceSubClass
	0,					// bDeviceProtocol
	ENDPOINT0_SIZE,				// bMaxPacketSize0
	LSB(VENDOR_ID), MSB(VENDOR_ID),		// idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),	// idProduct
	0x00, 0x01,				// bcdDevice
	1,					// iManufacturer
	2,					// iProduct
	3,					// iSerialNumber
	1					// bNumConfigurations
};

// Keyboard Protocol 1, HID 1.11 spec, Appendix B, page 59-60
static const uint8_t PROGMEM keyboard_hid_report_desc[] = {
	// Keyboard Collection
        0x05, 0x01,          // Usage Page (Generic Desktop),
        0x09, 0x06,          // Usage (Keyboard),
        0xA1, 0x01,          // Collection (Application) - Keyboard,

	// Modifier Byte
        0x75, 0x01,          //   Report Size (1),
        0x95, 0x08,          //   Report Count (8),
        0x05, 0x07,          //   Usage Page (Key Codes),
        0x19, 0xE0,          //   Usage Minimum (224),
        0x29, 0xE7,          //   Usage Maximum (231),
        0x15, 0x00,          //   Logical Minimum (0),
        0x25, 0x01,          //   Logical Maximum (1),
        0x81, 0x02,          //   Input (Data, Variable, Absolute),

	// LED Report
        0x95, 0x05,          //   Report Count (5),
        0x75, 0x01,          //   Report Size (1),
        0x05, 0x08,          //   Usage Page (LEDs),
        0x19, 0x01,          //   Usage Minimum (1),
        0x29, 0x05,          //   Usage Maximum (5),
        0x91, 0x02,          //   Output (Data, Variable, Absolute),

	// LED Report Padding
        0x95, 0x01,          //   Report Count (1),
        0x75, 0x03,          //   Report Size (3),
        0x91, 0x03,          //   Output (Constant),

	// Normal Keys
        0x95, 0x06,          //   Report Count (6),
        0x75, 0x08,          //   Report Size (8),
        0x15, 0x00,          //   Logical Minimum (0),
        0x25, 0x7F,          //   Logical Maximum(104),
        0x05, 0x07,          //   Usage Page (Key Codes),
        0x19, 0x00,          //   Usage Minimum (0),
        0x29, 0x7F,          //   Usage Maximum (104),
        0x81, 0x00,          //   Input (Data, Array),
        0xc0,                // End Collection - Keyboard
};

// Keyboard Protocol 1, HID 1.11 spec, Appendix B, page 59-60
static const uint8_t PROGMEM keyboard_nkro_hid_report_desc[] = {
	/*
	// System Control Collection
        0x05, 0x01,          // Usage Page (Generic Desktop),
        0x09, 0x80,          // Usage (System Control),
        0xA1, 0x01,          // Collection (Application),
        0x85, 0x01,          //   Report ID (1),
        0x95, 0x06,          //   Report Count (6),
        0x75, 0x08,          //   Report Size (8),
        0x19, 0x81,          //   Usage Minimum (129),
        0x29, 0x83,          //   Usage Maximum (131),
        0x15, 0x00,          //   Logical Minimum (0),
        0x25, 0x01,          //   Logical Maximum (1),
        0x81, 0x02,          //   Input (Data, Variable, Absolute),
        0x95, 0x05,          //   Report Count (5),
        0x75, 0x01,          //   Report Size (1),
        0x81, 0x03,          //   Input (Constant, Data, Variable, Absolute),
        0xc0, 0x00,          // End Collection - System Control
	*/

	// Keyboard Collection
        0x05, 0x01,          // Usage Page (Generic Desktop),
        0x09, 0x06,          // Usage (Keyboard),
        0xA1, 0x01,          // Collection (Application) - Keyboard,

	// Modifier Byte
        0x75, 0x01,          //   Report Size (1),
        0x85, 0x02,          //   Report ID (2),
        0x95, 0x08,          //   Report Count (8),
        0x05, 0x07,          //   Usage Page (Key Codes),
        0x19, 0xE0,          //   Usage Minimum (224),
        0x29, 0xE7,          //   Usage Maximum (231),
        0x15, 0x00,          //   Logical Minimum (0),
        0x25, 0x01,          //   Logical Maximum (1),
        0x81, 0x02,          //   Input (Data, Variable, Absolute),

	// Media Keys
        0x95, 0x08,          //   Report Count (8),
        0x85, 0x03,          //   Report ID (3),
        0x75, 0x01,          //   Report Size (1),
        0x15, 0x00,          //   Logical Minimum (0),
        0x25, 0x01,          //   Logical Maximum (1),
        0x05, 0x0C,          //   Usage Page (Consumer),
        0x09, 0xE9,          //   Usage (Volume Increment),
        0x09, 0xEA,          //   Usage (Volume Decrement),
        0x09, 0xE2,          //   Usage (Mute),
        0x09, 0xCD,          //   Usage (Play/Pause),
        0x09, 0xB5,          //   Usage (Scan Next Track),
        0x09, 0xB6,          //   Usage (Scan Previous Track),
        0x09, 0xB7,          //   Usage (Stop),
        0x09, 0xB8,          //   Usage (Eject),
        0x81, 0x02,          //   Input (Data, Variable, Absolute),

	// LED Report
        0x95, 0x05,          //   Report Count (5),
        0x85, 0x01,          //   Report ID (1),
        0x75, 0x01,          //   Report Size (1),
        0x05, 0x08,          //   Usage Page (LEDs),
        0x19, 0x01,          //   Usage Minimum (1),
        0x29, 0x05,          //   Usage Maximum (5),
        0x91, 0x02,          //   Output (Data, Variable, Absolute),

	// LED Report Padding
        0x95, 0x01,          //   Report Count (1),
        0x75, 0x03,          //   Report Size (3),
        0x91, 0x03,          //   Output (Constant),

	/*
	// Misc Keys
        0x95, 0x06,          //   Report Count (6),
        0x75, 0x01,          //   Report Size (1),
        0x15, 0x00,          //   Logical Minimum (0),
        0x25, 0x7F,          //   Logical Maximum(104),
        0x05, 0x07,          //   Usage Page (Key Codes),
        0x19, 0x00,          //   Usage Minimum (0),
        0x29, 0x7F,          //   Usage Maximum (104),
        0x81, 0x00,          //   Input (Data, Array),
	*/

	// Normal Keys
        0x95, 0x06,          //   Report Count (6),
        0x85, 0x04,          //   Report ID (4),
        0x75, 0x08,          //   Report Size (8),
        0x15, 0x00,          //   Logical Minimum (0),
        0x25, 0x7F,          //   Logical Maximum(104),
        0x05, 0x07,          //   Usage Page (Key Codes),
        0x19, 0x00,          //   Usage Minimum (0),
        0x29, 0x7F,          //   Usage Maximum (104),
        0x81, 0x00,          //   Input (Data, Array),
        0xc0,                // End Collection - Keyboard
};

// <Configuration> + <Keyboard HID> + <NKRO Keyboard HID> + <Serial CDC>
#define CONFIG1_DESC_SIZE             (9 + 9+9+7 + 9+9+7 + 8+9+5+5+4+5+7+9+7+7)
#define KEYBOARD_HID_DESC_OFFSET      (9 + 9)
#define KEYBOARD_NKRO_HID_DESC_OFFSET (9 + 9+9+7 + 9)
#define SERIAL_CDC_DESC_OFFSET        (9 + 9+9+7 + 9+9+7)
static const uint8_t PROGMEM config1_descriptor[CONFIG1_DESC_SIZE] = {
// --- Configuration ---
// - 9 bytes -
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9, 					// bLength;
	2,					// bDescriptorType;
	LSB(CONFIG1_DESC_SIZE),			// wTotalLength
	MSB(CONFIG1_DESC_SIZE),
	4,					// bNumInterfaces
	1,					// bConfigurationValue
	0,					// iConfiguration
	0x80,					// bmAttributes
	250,					// bMaxPower

// --- Keyboard HID ---
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	KEYBOARD_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x01,					// bInterfaceSubClass (0x00 = Non-Boot, 0x01 = Boot)
	0x01,					// bInterfaceProtocol (0x01 = Keyboard)
	0,					// iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,					// bLength
	0x21,					// bDescriptorType
	0x11, 0x01,				// bcdHID
	33,					// bCountryCode - Defaulting to US for now. TODO
	1,					// bNumDescriptors
	0x22,					// bDescriptorType
	LSB(sizeof(keyboard_hid_report_desc)),	// wDescriptorLength
	MSB(sizeof(keyboard_hid_report_desc)),
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	KEYBOARD_ENDPOINT | 0x80,		// bEndpointAddress
	0x03,					// bmAttributes (0x03=intr)
	KEYBOARD_SIZE, 0,			// wMaxPacketSize
	1,                                      // bInterval

// --- NKRO Keyboard HID ---
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	KEYBOARD_NKRO_INTERFACE,		// bInterfaceNumber
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x03,					// bInterfaceClass (0x03 = HID)
	0x00,					// bInterfaceSubClass (0x00 = Non-Boot, 0x01 = Boot)
	0x01,					// bInterfaceProtocol (0x01 = Keyboard)
	0,					// iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,					// bLength
	0x21,					// bDescriptorType
	0x11, 0x01,				// bcdHID
	33,					// bCountryCode - Defaulting to US for now. TODO
	1,					// bNumDescriptors
	0x22,					// bDescriptorType
	                                        // wDescriptorLength
	LSB(sizeof(keyboard_nkro_hid_report_desc)),
	MSB(sizeof(keyboard_nkro_hid_report_desc)),
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	KEYBOARD_NKRO_ENDPOINT | 0x80,		// bEndpointAddress
	0x03,					// bmAttributes (0x03=intr)
	KEYBOARD_NKRO_SIZE, 0,			// wMaxPacketSize
	1,                                      // bInterval

// --- Serial CDC ---
// - 8 bytes -
        // interface association descriptor, USB ECN, Table 9-Z
        8,                                      // bLength
        11,                                     // bDescriptorType
        CDC_STATUS_INTERFACE,                   // bFirstInterface
        2,                                      // bInterfaceCount
        0x02,                                   // bFunctionClass
        0x02,                                   // bFunctionSubClass
        0x01,                                   // bFunctionProtocol
        4,                                      // iFunction
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	CDC_STATUS_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	1,					// bNumEndpoints
	0x02,					// bInterfaceClass
	0x02,					// bInterfaceSubClass
	0x01,					// bInterfaceProtocol
	0,					// iInterface
// - 5 bytes -
	// CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
	5,					// bFunctionLength
	0x24,					// bDescriptorType
	0x00,					// bDescriptorSubtype
	0x10, 0x01,				// bcdCDC
// - 5 bytes -
	// Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
	5,					// bFunctionLength
	0x24,					// bDescriptorType
	0x01,					// bDescriptorSubtype
	0x01,					// bmCapabilities
	1,					// bDataInterface
// - 4 bytes -
	// Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
	4,					// bFunctionLength
	0x24,					// bDescriptorType
	0x02,					// bDescriptorSubtype
	0x06,					// bmCapabilities
// - 5 bytes -
	// Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
	5,					// bFunctionLength
	0x24,					// bDescriptorType
	0x06,					// bDescriptorSubtype
	CDC_STATUS_INTERFACE,			// bMasterInterface
	CDC_DATA_INTERFACE,			// bSlaveInterface0
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	CDC_ACM_ENDPOINT | 0x80,		// bEndpointAddress
	0x03,					// bmAttributes (0x03=intr)
	CDC_ACM_SIZE, 0,			// wMaxPacketSize
	64,					// bInterval
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,					// bLength
	4,					// bDescriptorType
	CDC_DATA_INTERFACE,			// bInterfaceNumber
	0,					// bAlternateSetting
	2,					// bNumEndpoints
	0x0A,					// bInterfaceClass
	0x00,					// bInterfaceSubClass
	0x00,					// bInterfaceProtocol
	0,					// iInterface
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	CDC_RX_ENDPOINT,			// bEndpointAddress
	0x02,					// bmAttributes (0x02=bulk)
	CDC_RX_SIZE, 0,				// wMaxPacketSize
	0,					// bInterval
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,					// bLength
	5,					// bDescriptorType
	CDC_TX_ENDPOINT | 0x80,			// bEndpointAddress
	0x02,					// bmAttributes (0x02=bulk)
	CDC_TX_SIZE, 0,				// wMaxPacketSize
	0,					// bInterval
};


// Configuration Endpoint (0) Descriptor
struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	int16_t wString[];
};
static const struct usb_string_descriptor_struct PROGMEM string0 = {
	4,
	3,
	{0x0409}
};
static const struct usb_string_descriptor_struct PROGMEM string1 = {
	sizeof(STR_MANUFACTURER),
	3,
	STR_MANUFACTURER
};
static const struct usb_string_descriptor_struct PROGMEM string2 = {
	sizeof(STR_PRODUCT),
	3,
	STR_PRODUCT
};
static const struct usb_string_descriptor_struct PROGMEM string3 = {
	sizeof(STR_SERIAL),
	3,
	STR_SERIAL
};

// This table defines which descriptor data is sent for each specific
// request from the host (in wValue and wIndex).
static const struct descriptor_list_struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint8_t		length;
} PROGMEM descriptor_list[] = {
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config1_descriptor, sizeof(config1_descriptor)},
	{0x2200, KEYBOARD_INTERFACE, keyboard_hid_report_desc, sizeof(keyboard_hid_report_desc)},
	{0x2100, KEYBOARD_INTERFACE, config1_descriptor + KEYBOARD_HID_DESC_OFFSET, 9},
	{0x2200, KEYBOARD_NKRO_INTERFACE, keyboard_nkro_hid_report_desc, sizeof(keyboard_nkro_hid_report_desc)},
	{0x2100, KEYBOARD_NKRO_INTERFACE, config1_descriptor + KEYBOARD_NKRO_HID_DESC_OFFSET, 9},
	{0x0300, 0x0000, (const uint8_t *)&string0, 4},
	{0x0301, 0x0409, (const uint8_t *)&string1, sizeof(STR_MANUFACTURER)},
	{0x0302, 0x0409, (const uint8_t *)&string2, sizeof(STR_PRODUCT)},
	{0x0303, 0x0409, (const uint8_t *)&string3, sizeof(STR_SERIAL)}
};
#define NUM_DESC_LIST (sizeof(descriptor_list)/sizeof(struct descriptor_list_struct))


#endif // usb_keyboard_serial_h__

