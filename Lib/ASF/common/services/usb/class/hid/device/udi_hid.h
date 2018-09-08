/**
 * \file
 *
 * \brief USB Device Human Interface Device (HID) interface definitions.
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

#ifndef _UDI_HID_H_
#define _UDI_HID_H_

#include "conf_usb.h"
#include "usb_protocol.h"
#include "usb_protocol_hid.h"
#include "udd.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \ingroup udi_group
 * \defgroup udi_hid_group USB Device Interface (UDI) for Human Interface Device (HID)
 *
 * Common library for all Human Interface Device (HID) implementation.
 *
 * @{
 */

/**
 * \brief Decode HID setup request
 *
 * \param rate         Pointer on rate of current HID interface
 * \param protocol     Pointer on protocol of current HID interface
 * \param report_desc  Pointer on report descriptor of current HID interface
 * \param set_report   Pointer on set_report callback of current HID interface
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
bool udi_hid_setup( uint8_t *rate, uint8_t *protocol, uint8_t *report_desc, bool (*setup_report)(void) );

//@}

#ifdef __cplusplus
}
#endif
#endif // _UDI_HID_H_
