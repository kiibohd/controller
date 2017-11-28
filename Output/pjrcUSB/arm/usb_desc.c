/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modified by Jacob Alexander (2013-2017)
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

// Generated Includes
#include <kll_defs.h>



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
	LSB(BCD_VERSION), MSB(BCD_VERSION),     // bcdDevice
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



// ----- USB HID Report Descriptors -----

// Each HID interface needs a special report descriptor that tells
// the meaning and format of the data.

// Keyboard Protocol 1, HID 1.11 spec, Appendix B, page 59-60
#if enableKeyboard_define == 1
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
};

// System Control and Consumer Control
static uint8_t sys_ctrl_report_desc[] = {
	// System Control Collection (8 bits)
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

	// Consumer Control Collection - Media Keys (16 bits)
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
	0x16, 0x01, 0x00,    //   Logical Minimum (1),
	0x26, 0x9D, 0x02,    //   Logical Maximum (669),
	0x05, 0x0C,          //   Usage Page (Consumer),
	0x19, 0x01,          //   Usage Minimum (1),
	0x2A, 0x9D, 0x02,    //   Usage Maximum (669),
	0x81, 0x00,          //   Input (Data, Array),
	0xc0,                // End Collection - Consumer Control
};
#endif


// Mouse Protocol 1, HID 1.11 spec, Appendix B, page 59-60, with wheel extension
#if enableMouse_define == 1
static uint8_t mouse_report_desc[] = {
	0x05, 0x01,        // Usage Page (Generic Desktop)
	0x09, 0x02,        // Usage (Mouse)
	0xA1, 0x01,        // Collection (Application)
	0x09, 0x02,        //   Usage (Mouse)
	0xA1, 0x02,        //   Collection (Logical)
	0x09, 0x01,        //     Usage (Pointer)

	// Buttons (16 bits)
	0xA1, 0x00,        //     Collection (Physical) - Buttons
	0x05, 0x09,        //       Usage Page (Button)
	0x19, 0x01,        //       Usage Minimum (Button 1)
	0x29, 0x10,        //       Usage Maximum (Button 16)
	0x15, 0x00,        //       Logical Minimum (0)
	0x25, 0x01,        //       Logical Maximum (1)
	0x75, 0x01,        //       Report Size (1)
	0x95, 0x10,        //       Report Count (16)
	0x81, 0x02,        //       Input (Data,Var,Abs)

	// Pointer (32 bits)
	0x05, 0x01,        //       Usage PAGE (Generic Desktop)
	0x09, 0x30,        //       Usage (X)
	0x09, 0x31,        //       Usage (Y)
	0x16, 0x01, 0x80,  //       Logical Minimum (-32 767)
	0x26, 0xFF, 0x7F,  //       Logical Maximum (32 767)
	0x75, 0x10,        //       Report Size (16)
	0x95, 0x02,        //       Report Count (2)
	0x81, 0x06,        //       Input (Data,Var,Rel)

	/*
	// Vertical Wheel
	// - Multiplier (2 bits)
	0xa1, 0x02,        //       Collection (Logical)
	0x09, 0x48,        //         Usage (Resolution Multiplier)
	0x15, 0x00,        //         Logical Minimum (0)
	0x25, 0x01,        //         Logical Maximum (1)
	0x35, 0x01,        //         Physical Minimum (1)
	0x45, 0x04,        //         Physical Maximum (4)
	0x75, 0x02,        //         Report Size (2)
	0x95, 0x01,        //         Report Count (1)
	0xa4,              //         Push
	0xb1, 0x02,        //         Feature (Data,Var,Abs)
	// - Device (8 bits)
	0x09, 0x38,        //         Usage (Wheel)
	0x15, 0x81,        //         Logical Minimum (-127)
	0x25, 0x7f,        //         Logical Maximum (127)
	0x35, 0x00,        //         Physical Minimum (0)        - reset physical
	0x45, 0x00,        //         Physical Maximum (0)
	0x75, 0x08,        //         Report Size (8)
	0x81, 0x06,        //         Input (Data,Var,Rel)
	0xc0,              //       End Collection - Vertical Wheel

	// Horizontal Wheel
	// - Multiplier (2 bits)
	0xa1, 0x02,        //       Collection (Logical)
	0x09, 0x48,        //         Usage (Resolution Multiplier)
	0xb4,              //         Pop
	0xb1, 0x02,        //         Feature (Data,Var,Abs)
	// - Padding (4 bits)
	0x35, 0x00,        //         Physical Minimum (0)        - reset physical
	0x45, 0x00,        //         Physical Maximum (0)
	0x75, 0x04,        //         Report Size (4)
	0xb1, 0x03,        //         Feature (Cnst,Var,Abs)
	// - Device (8 bits)
	0x05, 0x0c,        //         Usage Page (Consumer Devices)
	0x0a, 0x38, 0x02,  //         Usage (AC Pan)
	0x15, 0x81,        //         Logical Minimum (-127)
	0x25, 0x7f,        //         Logical Maximum (127)
	0x75, 0x08,        //         Report Size (8)
	0x81, 0x06,        //         Input (Data,Var,Rel)
	0xc0,              //       End Collection - Horizontal Wheel

	*/
	0xc0,              //     End Collection - Buttons
	0xc0,              //   End Collection - Mouse Logical
	0xc0               // End Collection - Mouse Application
};
#endif


// Joystick Protocol, HID 1.11 spec, Apendix D, page 64-65
#if enableJoystick_define == 1
static uint8_t joystick_report_desc[] = {
	0x05, 0x01,                     // Usage Page (Generic Desktop)
	0x09, 0x04,                     // Usage (Joystick)
	0xA1, 0x01,                     // Collection (Application)
	0x15, 0x00,                     // Logical Minimum (0)
	0x25, 0x01,                     // Logical Maximum (1)
	0x75, 0x01,                     // Report Size (1)
	0x95, 0x20,                     // Report Count (32)
	0x05, 0x09,                     // Usage Page (Button)
	0x19, 0x01,                     // Usage Minimum (Button #1)
	0x29, 0x20,                     // Usage Maximum (Button #32)
	0x81, 0x02,                     // Input (variable,absolute)
	0x15, 0x00,                     // Logical Minimum (0)
	0x25, 0x07,                     // Logical Maximum (7)
	0x35, 0x00,                     // Physical Minimum (0)
	0x46, 0x3B, 0x01,               // Physical Maximum (315)
	0x75, 0x04,                     // Report Size (4)
	0x95, 0x01,                     // Report Count (1)
	0x65, 0x14,                     // Unit (20)
	0x05, 0x01,                     // Usage Page (Generic Desktop)
	0x09, 0x39,                     // Usage (Hat switch)
	0x81, 0x42,                     // Input (variable,absolute,null_state)
	0x05, 0x01,                     // Usage Page (Generic Desktop)
	0x09, 0x01,                     // Usage (Pointer)
	0xA1, 0x00,                     // Collection ()
	0x15, 0x00,                     //   Logical Minimum (0)
	0x26, 0xFF, 0x03,               //   Logical Maximum (1023)
	0x75, 0x0A,                     //   Report Size (10)
	0x95, 0x04,                     //   Report Count (4)
	0x09, 0x30,                     //   Usage (X)
	0x09, 0x31,                     //   Usage (Y)
	0x09, 0x32,                     //   Usage (Z)
	0x09, 0x35,                     //   Usage (Rz)
	0x81, 0x02,                     //   Input (variable,absolute)
	0xC0,                           // End Collection
	0x15, 0x00,                     // Logical Minimum (0)
	0x26, 0xFF, 0x03,               // Logical Maximum (1023)
	0x75, 0x0A,                     // Report Size (10)
	0x95, 0x02,                     // Report Count (2)
	0x09, 0x36,                     // Usage (Slider)
	0x09, 0x36,                     // Usage (Slider)
	0x81, 0x02,                     // Input (variable,absolute)
	0xC0                            // End Collection
};
#endif



// ----- USB Configuration -----

// Check for non-selected USB descriptors to update the total interface count
// XXX This must be correct or some OSs/Init sequences will not initialize the keyboard/device
#if enableKeyboard_define != 1
#undef  KEYBOARD_INTERFACES
#define KEYBOARD_INTERFACES 0
#endif

#if enableMouse_define != 1
#undef  MOUSE_INTERFACES
#define MOUSE_INTERFACES 0
#endif

#if enableJoystick_define != 1
#undef  JOYSTICK_INTERFACES
#define JOYSTICK_INTERFACES 0
#endif

#if enableVirtualSerialPort_define != 1
#undef  CDC_INTERFACES
#define CDC_INTERFACES 0
#endif

#if enableRawIO_define != 1
#undef  RAWIO_INTERFACES
#define RAWIO_INTERFACES 0
#endif

// Determine number of interfaces
#define NUM_INTERFACE (KEYBOARD_INTERFACES + CDC_INTERFACES + MOUSE_INTERFACES + JOYSTICK_INTERFACES + RAWIO_INTERFACES)


// USB Configuration Descriptor.  This huge descriptor tells all
// of the devices capbilities.
static uint8_t config_descriptor[] = {
// --- Configuration ---
// - 9 bytes -
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9,                                      // bLength;
	2,                                      // bDescriptorType;
	0xFF,                                   // wTotalLength - XXX Set in usb_init (simplifies defines)
	0xFF,
	NUM_INTERFACE,                          // bNumInterfaces
	1,                                      // bConfigurationValue
	4,                                      // iConfiguration
	0xA0,                                   // bmAttributes
	250,                                    // bMaxPower - Entry Index 8


//
// --- Keyboard Endpoint Descriptors ---
//
#if enableKeyboard_define == 1
#define KEYBOARD_DESC_TOTAL_OFFSET     (KEYBOARD_DESC_SIZE + NKRO_KEYBOARD_DESC_SIZE + SYS_CTRL_DESC_SIZE)
#define NKRO_KEYBOARD_DESC_BASE_OFFSET (KEYBOARD_DESC_BASE_OFFSET + KEYBOARD_DESC_SIZE)
#define SYS_CTRL_DESC_BASE_OFFSET      (KEYBOARD_DESC_BASE_OFFSET + KEYBOARD_DESC_SIZE + NKRO_KEYBOARD_DESC_SIZE)

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
	KEYBOARD_INTERFACE + 5,                 // iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,                                      // bLength
	0x21,                                   // bDescriptorType
	0x11, 0x01,                             // bcdHID
	KeyboardLocale_define,                  // bCountryCode
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
	NKRO_KEYBOARD_INTERFACE + 5,            // iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,                                      // bLength
	0x21,                                   // bDescriptorType
	0x11, 0x01,                             // bcdHID
	KeyboardLocale_define,                  // bCountryCode
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

// --- System/Consumer Control ---
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	SYS_CTRL_INTERFACE,                     // bInterfaceNumber
	0,                                      // bAlternateSetting
	1,                                      // bNumEndpoints
	0x03,                                   // bInterfaceClass (0x03 = HID)
	0x01,                                   // bInterfaceSubClass (0x00 = Non-Boot, 0x01 = Boot)
	0x00,                                   // bInterfaceProtocol (0x00 = None)
	SYS_CTRL_INTERFACE + 5,                 // iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,                                      // bLength
	0x21,                                   // bDescriptorType
	0x11, 0x01,                             // bcdHID
	KeyboardLocale_define,                  // bCountryCode
	1,                                      // bNumDescriptors
	0x22,                                   // bDescriptorType
	LSB(sizeof(sys_ctrl_report_desc)),      // wDescriptorLength
	MSB(sizeof(sys_ctrl_report_desc)),
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	SYS_CTRL_ENDPOINT | 0x80,               // bEndpointAddress
	0x03,                                   // bmAttributes (0x03=intr)
	SYS_CTRL_SIZE, 0,                       // wMaxPacketSize
	SYS_CTRL_INTERVAL,                      // bInterval
#else
#define KEYBOARD_DESC_TOTAL_OFFSET (0)
#endif


//
// --- CDC / Serial Port Endpoint Descriptors ---
//
#if enableVirtualSerialPort_define == 1
#define SERIAL_CDC_DESC_TOTAL_OFFSET (SERIAL_CDC_DESC_SIZE)

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
	0,                                      // iFunction (XXX No interface index, don't give string -HaaTa)

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
	CDC_STATUS_INTERFACE + 5,               // iInterface
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
	CDC_DATA_INTERFACE + 5,                 // iInterface
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
#else
#define SERIAL_CDC_DESC_TOTAL_OFFSET (0)
#endif


//
// --- Raw IO Endpoint Descriptors ---
//
#if enableRawIO_define == 1
#define RAWIO_DESC_TOTAL_OFFSET (RAWIO_DESC_SIZE)

// --- Vendor Specific / RAW I/O ---
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	RAWIO_INTERFACE,                        // bInterfaceNumber
	0,                                      // bAlternateSetting
	2,                                      // bNumEndpoints
	0xFF,                                   // bInterfaceClass (0xFF)
	0xFF,                                   // bInterfaceSubClass
	0xFF,                                   // bInterfaceProtocol
	RAWIO_INTERFACE + 5,                    // iInterface

// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	RAWIO_TX_ENDPOINT | 0x80,               // bEndpointAddress
	0x02,                                   // bmAttributes (0x02=bulk)
	RAWIO_TX_SIZE, 0,                       // wMaxPacketSize
	RAWIO_TX_INTERVAL,                      // bInterval

// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	RAWIO_RX_ENDPOINT,                      // bEndpointAddress
	0x02,                                   // bmAttributes (0x02=bulk)
	RAWIO_RX_SIZE, 0,                       // wMaxPacketSize
	RAWIO_RX_INTERVAL,                      // bInterval
#else
#define RAWIO_DESC_TOTAL_OFFSET (0)
#endif


//
// --- Mouse Endpoint Descriptors ---
//
#if enableMouse_define == 1
#define MOUSE_DESC_TOTAL_OFFSET (MOUSE_DESC_SIZE)

// --- Mouse Interface ---
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	MOUSE_INTERFACE,                        // bInterfaceNumber
	0,                                      // bAlternateSetting
	1,                                      // bNumEndpoints
	0x03,                                   // bInterfaceClass (0x03 = HID)
	0x00,                                   // bInterfaceSubClass (0x01 = Boot)
	0x02,                                   // bInterfaceProtocol (0x02 = Mouse)
	MOUSE_INTERFACE + 5,                    // iInterface
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
#else
#define MOUSE_DESC_TOTAL_OFFSET (0)
#endif


//
// --- Joystick Endpoint Descriptors ---
//
#if enableJoystick_define == 1
#define JOYSTICK_DESC_TOTAL_OFFSET (JOYSTICK_DESC_SIZE)

// --- Joystick Interface ---
// - 9 bytes -
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,                                      // bLength
	4,                                      // bDescriptorType
	JOYSTICK_INTERFACE,                     // bInterfaceNumber
	0,                                      // bAlternateSetting
	1,                                      // bNumEndpoints
	0x03,                                   // bInterfaceClass (0x03 = HID)
	0x00,                                   // bInterfaceSubClass
	0x00,                                   // bInterfaceProtocol
	JOYSTICK_INTERFACE + 5,                 // iInterface
// - 9 bytes -
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,                                      // bLength
	0x21,                                   // bDescriptorType
	0x11, 0x01,                             // bcdHID
	0,                                      // bCountryCode
	1,                                      // bNumDescriptors
	0x22,                                   // bDescriptorType
	LSB(sizeof(joystick_report_desc)),      // wDescriptorLength
	MSB(sizeof(joystick_report_desc)),
// - 7 bytes -
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,                                      // bLength
	5,                                      // bDescriptorType
	JOYSTICK_ENDPOINT | 0x80,               // bEndpointAddress
	0x03,                                   // bmAttributes (0x03=intr)
	JOYSTICK_SIZE, 0,                       // wMaxPacketSize
	JOYSTICK_INTERVAL,                      // bInterval
#else
#define JOYSTICK_DESC_TOTAL_OFFSET (0)
#endif

};

uint8_t *usb_bMaxPower = &config_descriptor[8];



// ----- String Descriptors -----

// The descriptors above can provide human readable strings,
// referenced by index numbers.  These descriptors are the
// actual string data

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

#define usb_string_descriptor(name, str) \
	struct usb_string_descriptor_struct name = { \
		sizeof(str), \
		3, \
		{str} \
	}

usb_string_descriptor( usb_string_manufacturer_name_default, STR_MANUFACTURER );
usb_string_descriptor( usb_string_product_name_default, STR_PRODUCT );
usb_string_descriptor( usb_string_serial_number_default, STR_SERIAL );
usb_string_descriptor( usb_string_flashingstation_name, STR_CONFIG_NAME );

#if enableKeyboard_define == 1
usb_string_descriptor( usb_string_keyboard_name, KEYBOARD_NAME );
usb_string_descriptor( usb_string_nkro_keyboard_name, NKRO_KEYBOARD_NAME );
usb_string_descriptor( usb_string_sys_ctrl_name, SYS_CTRL_NAME );
#endif

#if enableVirtualSerialPort_define == 1
usb_string_descriptor( usb_string_cdc_status_name, CDC_STATUS_NAME );
usb_string_descriptor( usb_string_cdc_data_name, CDC_DATA_NAME );
#endif

#if enableRawIO_define == 1
usb_string_descriptor( usb_string_rawio_name, RAWIO_NAME );
#endif

#if enableMouse_define == 1
usb_string_descriptor( usb_string_mouse_name, MOUSE_NAME );
#endif

#if enableJoystick_define == 1
usb_string_descriptor( usb_string_joystick_name, JOYSTICK_NAME );
#endif



// ----- Descriptors List -----

#define iInterfaceString(num, var) \
	{0x0300 + 5 + num, 0x0409, (const uint8_t *)&var, 0 }

#define iInterfaceStringInitial(num, var) \
	{0x0300 + num, 0x0409, (const uint8_t *)&var, 0 }


// This table provides access to all the descriptor data above.

const usb_descriptor_list_t usb_descriptor_list[] = {
	//wValue, wIndex, address,          length
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
	{0x0600, 0x0000, device_qualifier_descriptor, sizeof(device_qualifier_descriptor)},
	{0x0A00, 0x0000, usb_debug_descriptor, sizeof(usb_debug_descriptor)},

	{0x0300, 0x0000, (const uint8_t *)&string0, 0},
	iInterfaceStringInitial( 1, usb_string_manufacturer_name ),
	iInterfaceStringInitial( 2, usb_string_product_name ),
	iInterfaceStringInitial( 3, usb_string_serial_number ),
	iInterfaceStringInitial( 4, usb_string_flashingstation_name ),

#if enableKeyboard_define == 1
	{0x2200, KEYBOARD_INTERFACE, keyboard_report_desc, sizeof(keyboard_report_desc)},
	{0x2100, KEYBOARD_INTERFACE, config_descriptor + KEYBOARD_DESC_BASE_OFFSET, 9},

	{0x2200, NKRO_KEYBOARD_INTERFACE, nkro_keyboard_report_desc, sizeof(nkro_keyboard_report_desc)},
	{0x2100, NKRO_KEYBOARD_INTERFACE, config_descriptor + NKRO_KEYBOARD_DESC_BASE_OFFSET, 9},

	{0x2200, SYS_CTRL_INTERFACE, sys_ctrl_report_desc, sizeof(sys_ctrl_report_desc)},
	{0x2100, SYS_CTRL_INTERFACE, config_descriptor + SYS_CTRL_DESC_BASE_OFFSET, 9},

	iInterfaceString( KEYBOARD_INTERFACE, usb_string_keyboard_name ),
	iInterfaceString( NKRO_KEYBOARD_INTERFACE, usb_string_nkro_keyboard_name ),
	iInterfaceString( SYS_CTRL_INTERFACE, usb_string_sys_ctrl_name ),
#endif

#if enableVirtualSerialPort_define == 1
	iInterfaceString( CDC_STATUS_INTERFACE, usb_string_cdc_status_name ),
	iInterfaceString( CDC_DATA_INTERFACE, usb_string_cdc_data_name ),
#endif

#if enableRawIO_define == 1
	iInterfaceString( RAWIO_INTERFACE, usb_string_rawio_name ),
#endif

#if enableMouse_define == 1
	{0x2200, MOUSE_INTERFACE, mouse_report_desc, sizeof(mouse_report_desc)},
	{0x2100, MOUSE_INTERFACE, config_descriptor + MOUSE_DESC_BASE_OFFSET, 9},
	iInterfaceString( MOUSE_INTERFACE, usb_string_mouse_name ),
#endif

#if enableJoystick_define == 1
	{0x2200, JOYSTICK_INTERFACE, joystick_report_desc, sizeof(joystick_report_desc)},
	{0x2100, JOYSTICK_INTERFACE, config_descriptor + JOYSTICK_DESC_BASE_OFFSET, 9},
	iInterfaceString( JOYSTICK_INTERFACE, usb_string_joystick_name ),
#endif

	{0, 0, NULL, 0}
};

// Simplifies defines for USB descriptors
void usb_set_config_descriptor_size()
{
	config_descriptor[2] = LSB( sizeof( config_descriptor ) );
	config_descriptor[3] = MSB( sizeof( config_descriptor ) );
}



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


