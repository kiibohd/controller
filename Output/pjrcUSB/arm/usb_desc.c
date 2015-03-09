/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modified by Jacob Alexander (2013-2014)
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

// ----- Includes -----

// Local Includes
#include "usb_desc.h"



// ----- Macros -----

#define LSB(n) ((n) & 255)
#define MSB(n) (((n) >> 8) & 255)



// ----- USB Device Descriptor -----

// USB Device Descriptor.  The USB host reads this first, to learn
// what type of device is connected.
static uint8_t device_descriptor[] = {
	18,                                     // bLength
	1,                                      // bDescriptorType
	0x00, 0x02,                             // bcdUSB
	DEVICE_CLASS,                           // bDeviceClass
	DEVICE_SUBCLASS,                        // bDeviceSubClass
	DEVICE_PROTOCOL,                        // bDeviceProtocol
	EP0_SIZE,                               // bMaxPacketSize0
	LSB(VENDOR_ID), MSB(VENDOR_ID),         // idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),       // idProduct
	0x00, 0x01,                             // bcdDevice
	1,                                      // iManufacturer
	2,                                      // iProduct
	3,                                      // iSerialNumber
	1                                       // bNumConfigurations
};

// USB Device Qualifier Descriptor
static uint8_t device_qualifier_descriptor[] = {
	0                                       // Indicate only single speed
	/* Device qualifier example (used for specifying multiple USB speeds)
	10,                                     // bLength
	6,                                      // bDescriptorType
	0x00, 0x02,                             // bcdUSB
	DEVICE_CLASS,                           // bDeviceClass
	DEVICE_SUBCLASS,                        // bDeviceSubClass
	DEVICE_PROTOCOL,                        // bDeviceProtocol
	EP0_SIZE,                               // bMaxPacketSize0
	0,                                      // bNumOtherSpeedConfigurations
	0                                       // bReserved
	*/
};

// USB Debug Descriptor
// XXX Not sure of exact use, lsusb requests it
static uint8_t usb_debug_descriptor[] = {
	0
};

// XXX
// These descriptors must NOT be "const", because the USB DMA
// has trouble accessing flash memory with enough bandwidth
// while the processor is executing from flash.
// XXX



// ----- USB HID Report Descriptsors -----

// Each HID interface needs a special report descriptor that tells
// the meaning and format of the data.

// Keyboard Protocol 1, HID 1.11 spec, Appendix B, page 59-60
static uint8_t keyboard_report_desc[] = {
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

	// Reserved Byte
	0x75, 0x08,          //   Report Size (8),
	0x95, 0x01,          //   Report Count (1),
	0x81, 0x03,          //   Output (Constant),

	// LED Report
	0x75, 0x01,          //   Report Size (1),
	0x95, 0x05,          //   Report Count (5),
	0x05, 0x08,          //   Usage Page (LEDs),
	0x19, 0x01,          //   Usage Minimum (1),
	0x29, 0x05,          //   Usage Maximum (5),
	0x91, 0x02,          //   Output (Data, Variable, Absolute),

	// LED Report Padding
	0x75, 0x03,          //   Report Size (3),
	0x95, 0x01,          //   Report Count (1),
	0x91, 0x03,          //   Output (Constant),

	// Normal Keys
	0x75, 0x08,          //   Report Size (8),
	0x95, 0x06,          //   Report Count (6),
	0x15, 0x00,          //   Logical Minimum (0),
	0x25, 0x7F,          //   Logical Maximum(104),
	0x05, 0x07,          //   Usage Page (Key Codes),
	0x19, 0x00,          //   Usage Minimum (0),
	0x29, 0x7F,          //   Usage Maximum (104),
	0x81, 0x00,          //   Input (Data, Array),
	0xc0,                // End Collection - Keyboard
};

// Keyboard Protocol 1, HID 1.11 spec, Appendix B, page 59-60
static uint8_t nkro_keyboard_report_desc[] = {
	// Keyboard Collection
	0x05, 0x01,          // Usage Page (Generic Desktop),
	0x09, 0x06,          // Usage (Keyboard),
	0xA1, 0x01,          // Collection (Application) - Keyboard,

	// LED Report
	0x85, 0x01,          //   Report ID (1),
	0x75, 0x01,          //   Report Size (1),
	0x95, 0x05,          //   Report Count (5),
	0x05, 0x08,          //   Usage Page (LEDs),
	0x19, 0x01,          //   Usage Minimum (1),
	0x29, 0x05,          //   Usage Maximum (5),
	0x91, 0x02,          //   Output (Data, Variable, Absolute),

	// LED Report Padding
	0x75, 0x03,          //   Report Size (3),
	0x95, 0x01,          //   Report Count (1),
	0x91, 0x03,          //   Output (Constant),

	// Normal Keys - Using an NKRO Bitmap
	//
	// NOTES:
	// Supports all keys defined by the spec, except 1-3 which define error events
	//  and 0 which is "no keys pressed"
	// See http://www.usb.org/developers/hidpage/Hut1_12v2.pdf Chapter 10
	// Or Macros/PartialMap/usb_hid.h
	//
	// 50 (ISO \ due to \ bug) and 156 (Clear due to Delete bug) must be excluded
	//  due to a Linux bug with bitmaps (not useful anyways)
	// 165-175 are reserved/unused as well as 222-223 and 232-65535
	//
	// Compatibility Notes:
	//  - Using a second endpoint for a boot mode device helps with compatibility
	//  - DO NOT use Padding in the descriptor for bitfields
	//    (Mac OSX silently fails... Windows/Linux work correctly)
	//  - DO NOT use Report IDs, Windows 8.1 will not update keyboard correctly (modifiers disappear)
	//    (all other OSs, including OSX work fine...)
	//    (you can use them *iff* you only have 1 per collection)
	//  - Mac OSX and Windows 8.1 are extremely picky about padding
	//
	// Packing of bitmaps are as follows:
	//   4-49  :  6 bytes (0x04-0x31) ( 46 bits + 2 padding bits for 6 bytes total)
	//  51-155 : 14 bytes (0x33-0x9B) (105 bits + 6 padding bits for 15 bytes total)
	// 157-164 :  1 byte  (0x9D-0xA4) (  8 bits)
	// 176-221 :  6 bytes (0xB0-0xDD) ( 46 bits + 2 padding bits for 6 bytes total)
	// 224-231 :  1 byte  (0xE0-0xE7) (  8 bits)

	// Modifier Byte
	0x75, 0x01,          //   Report Size (1),
	0x95, 0x08,          //   Report Count (8),
	0x15, 0x00,          //   Logical Minimum (0),
	0x25, 0x01,          //   Logical Maximum (1),
	0x05, 0x07,          //   Usage Page (Key Codes),
	0x19, 0xE0,          //   Usage Minimum (224),
	0x29, 0xE7,          //   Usage Maximum (231),
	0x81, 0x02,          //   Input (Data, Variable, Absolute),

	// 4-49 (6 bytes/46 bits) - MainKeys
	0x75, 0x01,          //   Report Size (1),
	0x95, 0x2E,          //   Report Count (46),
	0x15, 0x00,          //   Logical Minimum (0),
	0x25, 0x01,          //   Logical Maximum (1),
	0x05, 0x07,          //   Usage Page (Key Codes),
	0x19, 0x04,          //   Usage Minimum (4),
	0x29, 0x31,          //   Usage Maximum (49),
	0x81, 0x02,          //   Input (Data, Variable, Absolute, Bitfield),

	// Padding (2 bits)
	0x75, 0x02,          //   Report Size (2),
	0x95, 0x01,          //   Report Count (1),
	0x81, 0x03,          //   Input (Constant),

	// 51-155 (14 bytes/105 bits) - SecondaryKeys
	0x75, 0x01,          //   Report Size (1),
	0x95, 0x69,          //   Report Count (105),
	0x15, 0x00,          //   Logical Minimum (0),
	0x25, 0x01,          //   Logical Maximum (1),
	0x05, 0x07,          //   Usage Page (Key Codes),
	0x19, 0x33,          //   Usage Minimum (51),
	0x29, 0x9B,          //   Usage Maximum (155),
	0x81, 0x02,          //   Input (Data, Variable, Absolute, Bitfield),

	// Padding (7 bits)
	0x75, 0x07,          //   Report Size (7),
	0x95, 0x01,          //   Report Count (1),
	0x81, 0x03,          //   Input (Constant),

	// 157-164 (1 byte/8 bits) - TertiaryKeys
	0x75, 0x01,          //   Report Size (1),
	0x95, 0x08,          //   Report Count (8),
	0x15, 0x00,          //   Logical Minimum (0),
	0x25, 0x01,          //   Logical Maximum (1),
	0x05, 0x07,          //   Usage Page (Key Codes),
	0x19, 0x9D,          //   Usage Minimum (157),
	0x29, 0xA4,          //   Usage Maximum (164),
	0x81, 0x02,          //   Input (Data, Variable, Absolute, Bitfield),

	// 176-221 (6 bytes/46 bits) - QuartiaryKeys
	0x75, 0x01,          //   Report Size (1),
	0x95, 0x2E,          //   Report Count (46),
	0x15, 0x00,          //   Logical Minimum (0),
	0x25, 0x01,          //   Logical Maximum (1),
	0x05, 0x07,          //   Usage Page (Key Codes),
	0x19, 0xB0,          //   Usage Minimum (176),
	0x29, 0xDD,          //   Usage Maximum (221),
	0x81, 0x02,          //   Input (Data, Variable, Absolute, Bitfield),

	// Padding (2 bits)
	0x75, 0x02,          //   Report Size (2),
	0x95, 0x01,          //   Report Count (1),
	0x81, 0x03,          //   Input (Constant),
	0xc0,                // End Collection - Keyboard

	// System Control Collection
	//
	// NOTES:
	// Not bothering with NKRO for this table. If there's need, I can implement it. -HaaTa
	// Using a 1KRO scheme
	0x05, 0x01,          // Usage Page (Generic Desktop),
	0x09, 0x80,          // Usage (System Control),
	0xA1, 0x01,          // Collection (Application),
	0x85, 0x02,          //   Report ID (2),
	0x75, 0x08,          //   Report Size (8),
	0x95, 0x01,          //   Report Count (1),
	0x16, 0x81, 0x00,    //   Logical Minimum (129),
	0x26, 0xB7, 0x00,    //   Logical Maximum (183),
	0x19, 0x81,          //   Usage Minimum (129),
	0x29, 0xB7,          //   Usage Maximum (183),
	0x81, 0x00,          //   Input (Data, Array),
	0xc0,                // End Collection - System Control

	// Consumer Control Collection - Media Keys
	//
	// NOTES:
	// Not bothering with NKRO for this table. If there's a need, I can implement it. -HaaTa
	// Using a 1KRO scheme
	0x05, 0x0c,          // Usage Page (Consumer),
	0x09, 0x01,          // Usage (Consumer Control),
	0xA1, 0x01,          // Collection (Application),
	0x85, 0x03,          //   Report ID (3),
	0x75, 0x10,          //   Report Size (16),
	0x95, 0x01,          //   Report Count (1),
	0x16, 0x20, 0x00,    //   Logical Minimum (32),
	0x26, 0x9C, 0x02,    //   Logical Maximum (668),
	0x05, 0x0C,          //   Usage Page (Consumer),
	0x19, 0x20,          //   Usage Minimum (32),
	0x2A, 0x9C, 0x02,    //   Usage Maximum (668),
	0x81, 0x00,          //   Input (Data, Array),
	0xc0,                // End Collection - Consumer Control
};

/* MOUSE
// Mouse Protocol 1, HID 1.11 spec, Appendix B, page 59-60, with wheel extension
static uint8_t mouse_report_desc[] = {
	0x05, 0x01,                     // Usage Page (Generic Desktop)
	0x09, 0x02,                     // Usage (Mouse)
	0xA1, 0x01,                     // Collection (Application)
	0x05, 0x09,                     //   Usage Page (Button)
	0x19, 0x01,                     //   Usage Minimum (Button #1)
	0x29, 0x03,                     //   Usage Maximum (Button #3)
	0x15, 0x00,                     //   Logical Minimum (0)
	0x25, 0x01,                     //   Logical Maximum (1)
	0x95, 0x03,                     //   Report Count (3)
	0x75, 0x01,                     //   Report Size (1)
	0x81, 0x02,                     //   Input (Data, Variable, Absolute)
	0x95, 0x01,                     //   Report Count (1)
	0x75, 0x05,                     //   Report Size (5)
	0x81, 0x03,                     //   Input (Constant)
	0x05, 0x01,                     //   Usage Page (Generic Desktop)
	0x09, 0x30,                     //   Usage (X)
	0x09, 0x31,                     //   Usage (Y)
	0x15, 0x00,                     //   Logical Minimum (0)
	0x26, 0xFF, 0x7F,               //   Logical Maximum (32767)
	0x75, 0x10,                     //   Report Size (16),
	0x95, 0x02,                     //   Report Count (2),
	0x81, 0x02,                     //   Input (Data, Variable, Absolute)
	0x09, 0x38,                     //   Usage (Wheel)
	0x15, 0x81,                     //   Logical Minimum (-127)
	0x25, 0x7F,                     //   Logical Maximum (127)
	0x75, 0x08,                     //   Report Size (8),
	0x95, 0x01,                     //   Report Count (1),
	0x81, 0x06,                     //   Input (Data, Variable, Relative)
	0xC0                            // End Collection
};
*/



// ----- USB Configuration -----

// USB Configuration Descriptor.  This huge descriptor tells all
// of the devices capbilities.
static uint8_t config_descriptor[CONFIG_DESC_SIZE] = {
// --- Configuration ---
// - 9 bytes -
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9,                                      // bLength;
	2,                                      // bDescriptorType;
	LSB(CONFIG_DESC_SIZE),                  // wTotalLength
	MSB(CONFIG_DESC_SIZE),
	NUM_INTERFACE,                          // bNumInterfaces
	1,                                      // bConfigurationValue
	0,                                      // iConfiguration
	0xA0,                                   // bmAttributes
	250,                                    // bMaxPower

// --- Keyboard HID --- Boot Mode Keyboard Interface
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	KEYBOARD_INTERFACE,                     // bInterfaceNumber
	0,                                      // bAlternateSetting
	1,                                      // bNumEndpoints
	0x03,                                   // bInterfaceClass (0x03 = HID)
	0x01,                                   // bInterfaceSubClass (0x00 = Non-Boot, 0x01 = Boot)
	0x01,                                   // bInterfaceProtocol (0x01 = Keyboard)
	0,                                      // iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,                                      // bLength
	0x21,                                   // bDescriptorType
	0x11, 0x01,                             // bcdHID
	0,                                      // bCountryCode
	1,                                      // bNumDescriptors
	0x22,                                   // bDescriptorType
	LSB(sizeof(keyboard_report_desc)),      // wDescriptorLength
	MSB(sizeof(keyboard_report_desc)),
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	KEYBOARD_ENDPOINT | 0x80,               // bEndpointAddress
	0x03,                                   // bmAttributes (0x03=intr)
	KEYBOARD_SIZE, 0,                       // wMaxPacketSize
	KEYBOARD_INTERVAL,                      // bInterval

// --- NKRO Keyboard HID --- OS Mode Keyboard Interface
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	NKRO_KEYBOARD_INTERFACE,                // bInterfaceNumber
	0,                                      // bAlternateSetting
	1,                                      // bNumEndpoints
	0x03,                                   // bInterfaceClass (0x03 = HID)
	0x00,                                   // bInterfaceSubClass (0x00 = Non-Boot, 0x01 = Boot)
	0x01,                                   // bInterfaceProtocol (0x01 = Keyboard)
	0,                                      // iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,                                      // bLength
	0x21,                                   // bDescriptorType
	0x11, 0x01,                             // bcdHID
	0,                                      // bCountryCode
	1,                                      // bNumDescriptors
	0x22,                                   // bDescriptorType
	LSB(sizeof(nkro_keyboard_report_desc)), // wDescriptorLength
	MSB(sizeof(nkro_keyboard_report_desc)),
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	NKRO_KEYBOARD_ENDPOINT | 0x80,          // bEndpointAddress
	0x03,                                   // bmAttributes (0x03=intr)
	NKRO_KEYBOARD_SIZE, 0,                  // wMaxPacketSize
	NKRO_KEYBOARD_INTERVAL,                 // bInterval

// --- Serial CDC --- CDC IAD Descriptor
// - 8 bytes -
	// interface association descriptor, USB ECN, Table 9-Z
	8,                                      // bLength
	11,                                     // bDescriptorType
	CDC_STATUS_INTERFACE,                   // bFirstInterface
	2,                                      // bInterfaceCount
	0x02,                                   // bFunctionClass
	0x02,                                   // bFunctionSubClass
	0x01,                                   // bFunctionProtocol
	0,                                      // iFunction

// --- Serial CDC --- CDC Data Interface
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	CDC_STATUS_INTERFACE,                   // bInterfaceNumber
	0,                                      // bAlternateSetting
	1,                                      // bNumEndpoints
	0x02,                                   // bInterfaceClass
	0x02,                                   // bInterfaceSubClass
	0x01,                                   // bInterfaceProtocol
	0,                                      // iInterface
// - 5 bytes -
	// CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
	5,                                      // bFunctionLength
	0x24,                                   // bDescriptorType
	0x00,                                   // bDescriptorSubtype
	0x10, 0x01,                             // bcdCDC
// - 5 bytes -
	// Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
	5,                                      // bFunctionLength
	0x24,                                   // bDescriptorType
	0x01,                                   // bDescriptorSubtype
	0x01,                                   // bmCapabilities
	CDC_DATA_INTERFACE,                     // bDataInterface
// - 4 bytes -
	// Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
	4,                                      // bFunctionLength
	0x24,                                   // bDescriptorType
	0x02,                                   // bDescriptorSubtype
	0x06,                                   // bmCapabilities
// - 5 bytes -
	// Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
	5,                                      // bFunctionLength
	0x24,                                   // bDescriptorType
	0x06,                                   // bDescriptorSubtype
	CDC_STATUS_INTERFACE,                   // bMasterInterface
	CDC_DATA_INTERFACE,                     // bSlaveInterface0
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	CDC_ACM_ENDPOINT | 0x80,                // bEndpointAddress
	0x03,                                   // bmAttributes (0x03=intr)
	CDC_ACM_SIZE, 0,                        // wMaxPacketSize
	64,                                     // bInterval
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	CDC_DATA_INTERFACE,                     // bInterfaceNumber
	0,                                      // bAlternateSetting
	2,                                      // bNumEndpoints
	0x0A,                                   // bInterfaceClass
	0x00,                                   // bInterfaceSubClass
	0x00,                                   // bInterfaceProtocol
	0,                                      // iInterface
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	CDC_RX_ENDPOINT,                        // bEndpointAddress
	0x02,                                   // bmAttributes (0x02=bulk)
	CDC_RX_SIZE, 0,                         // wMaxPacketSize
	0,                                      // bInterval
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	CDC_TX_ENDPOINT | 0x80,                 // bEndpointAddress
	0x02,                                   // bmAttributes (0x02=bulk)
	CDC_TX_SIZE, 0,                         // wMaxPacketSize
	0,                                      // bInterval

/*
// Mouse Interface
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	MOUSE_INTERFACE,                        // bInterfaceNumber
	0,                                      // bAlternateSetting
	1,                                      // bNumEndpoints
	0x03,                                   // bInterfaceClass (0x03 = HID)
	0x00,                                   // bInterfaceSubClass (0x01 = Boot)
	0x00,                                   // bInterfaceProtocol (0x02 = Mouse)
	0,                                      // iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,                                      // bLength
	0x21,                                   // bDescriptorType
	0x11, 0x01,                             // bcdHID
	0,                                      // bCountryCode
	1,                                      // bNumDescriptors
	0x22,                                   // bDescriptorType
	LSB(sizeof(mouse_report_desc)),         // wDescriptorLength
	MSB(sizeof(mouse_report_desc)),
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	MOUSE_ENDPOINT | 0x80,                  // bEndpointAddress
	0x03,                                   // bmAttributes (0x03=intr)
	MOUSE_SIZE, 0,                          // wMaxPacketSize
	MOUSE_INTERVAL,                         // bInterval
#endif // MOUSE_INTERFACE
*/
};



// ----- String Descriptors -----

// The descriptors above can provide human readable strings,
// referenced by index numbers.  These descriptors are the
// actual string data

struct usb_string_descriptor_struct {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wString[];
};

extern struct usb_string_descriptor_struct usb_string_manufacturer_name
	__attribute__ ((weak, alias("usb_string_manufacturer_name_default")));
extern struct usb_string_descriptor_struct usb_string_product_name
	__attribute__ ((weak, alias("usb_string_product_name_default")));
extern struct usb_string_descriptor_struct usb_string_serial_number
	__attribute__ ((weak, alias("usb_string_serial_number_default")));

struct usb_string_descriptor_struct string0 = {
	4,
	3,
	{0x0409}
};

struct usb_string_descriptor_struct usb_string_manufacturer_name_default = {
	sizeof(STR_MANUFACTURER),
	3,
	{STR_MANUFACTURER}
};
struct usb_string_descriptor_struct usb_string_product_name_default = {
	sizeof(STR_PRODUCT),
	3,
	{STR_PRODUCT}
};
struct usb_string_descriptor_struct usb_string_serial_number_default = {
	sizeof(STR_SERIAL),
	3,
	{STR_SERIAL}
};



// ----- Descriptors List -----

// This table provides access to all the descriptor data above.

const usb_descriptor_list_t usb_descriptor_list[] = {
	//wValue, wIndex, address,          length
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
	{0x0600, 0x0000, device_qualifier_descriptor, sizeof(device_qualifier_descriptor)},
	{0x0A00, 0x0000, usb_debug_descriptor, sizeof(usb_debug_descriptor)},
	{0x2200, KEYBOARD_INTERFACE, keyboard_report_desc, sizeof(keyboard_report_desc)},
	{0x2100, KEYBOARD_INTERFACE, config_descriptor + KEYBOARD_DESC_OFFSET, 9},
	{0x2200, NKRO_KEYBOARD_INTERFACE, nkro_keyboard_report_desc, sizeof(nkro_keyboard_report_desc)},
	{0x2100, NKRO_KEYBOARD_INTERFACE, config_descriptor + NKRO_KEYBOARD_DESC_OFFSET, 9},
/* MOUSE
	{0x2200, MOUSE_INTERFACE, mouse_report_desc, sizeof(mouse_report_desc)},
	{0x2100, MOUSE_INTERFACE, config_descriptor+MOUSE_DESC_OFFSET, 9},
*/
	{0x0300, 0x0000, (const uint8_t *)&string0, 0},
	{0x0301, 0x0409, (const uint8_t *)&usb_string_manufacturer_name, 0},
	{0x0302, 0x0409, (const uint8_t *)&usb_string_product_name, 0},
	{0x0303, 0x0409, (const uint8_t *)&usb_string_serial_number, 0},
	{0, 0, NULL, 0}
};



// ----- Endpoint Configuration -----

// See usb_desc.h for Endpoint configuration
// 0x00 = not used
// 0x19 = Recieve only
// 0x15 = Transmit only
// 0x1D = Transmit & Recieve
//
const uint8_t usb_endpoint_config_table[NUM_ENDPOINTS] =
{
#if (defined(ENDPOINT1_CONFIG) && NUM_ENDPOINTS >= 1)
	ENDPOINT1_CONFIG,
#elif (NUM_ENDPOINTS >= 1)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT2_CONFIG) && NUM_ENDPOINTS >= 2)
	ENDPOINT2_CONFIG,
#elif (NUM_ENDPOINTS >= 2)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT3_CONFIG) && NUM_ENDPOINTS >= 3)
	ENDPOINT3_CONFIG,
#elif (NUM_ENDPOINTS >= 3)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT4_CONFIG) && NUM_ENDPOINTS >= 4)
	ENDPOINT4_CONFIG,
#elif (NUM_ENDPOINTS >= 4)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT5_CONFIG) && NUM_ENDPOINTS >= 5)
	ENDPOINT5_CONFIG,
#elif (NUM_ENDPOINTS >= 5)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT6_CONFIG) && NUM_ENDPOINTS >= 6)
	ENDPOINT6_CONFIG,
#elif (NUM_ENDPOINTS >= 6)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT7_CONFIG) && NUM_ENDPOINTS >= 7)
	ENDPOINT7_CONFIG,
#elif (NUM_ENDPOINTS >= 7)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT8_CONFIG) && NUM_ENDPOINTS >= 8)
	ENDPOINT8_CONFIG,
#elif (NUM_ENDPOINTS >= 8)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT9_CONFIG) && NUM_ENDPOINTS >= 9)
	ENDPOINT9_CONFIG,
#elif (NUM_ENDPOINTS >= 9)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT10_CONFIG) && NUM_ENDPOINTS >= 10)
	ENDPOINT10_CONFIG,
#elif (NUM_ENDPOINTS >= 10)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT11_CONFIG) && NUM_ENDPOINTS >= 11)
	ENDPOINT11_CONFIG,
#elif (NUM_ENDPOINTS >= 11)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT12_CONFIG) && NUM_ENDPOINTS >= 12)
	ENDPOINT12_CONFIG,
#elif (NUM_ENDPOINTS >= 12)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT13_CONFIG) && NUM_ENDPOINTS >= 13)
	ENDPOINT13_CONFIG,
#elif (NUM_ENDPOINTS >= 13)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT14_CONFIG) && NUM_ENDPOINTS >= 14)
	ENDPOINT14_CONFIG,
#elif (NUM_ENDPOINTS >= 14)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT15_CONFIG) && NUM_ENDPOINTS >= 15)
	ENDPOINT15_CONFIG,
#elif (NUM_ENDPOINTS >= 15)
	ENDPOINT_UNUSED,
#endif
};


