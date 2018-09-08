/**
 * \file
 *
 * \brief Default HID keyboard configuration for a USB Device
 * with a single interface HID keyboard
 *
 * Copyright (c) 2009-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef _UDI_HID_KBD_CONF_H_
#define _UDI_HID_KBD_CONF_H_

/**
 * \addtogroup udi_hid_keyboard_group_single_desc
 * @{
 */

//! Control endpoint size
#define  USB_DEVICE_EP_CTRL_SIZE    8

//! Endpoint number used by HID keyboard interface
#define  UDI_HID_KBD_EP_IN           (1 | USB_EP_DIR_IN)

//! Interface number
#define  UDI_HID_KBD_IFACE_NUMBER    0

/**
 * \name UDD Configuration
 */
//@{
//! 1 endpoint used by HID keyboard standard interface
#undef USB_DEVICE_MAX_EP   // undefine this definition in header file
#define  USB_DEVICE_MAX_EP             1
//@}

//@}

#include "udi_hid_kbd.h"

#endif // _UDI_HID_KBD_CONF_H_
