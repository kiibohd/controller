/**
 * \file
 *
 * \brief Digital-to-Analog Converter Controller (DACC) driver for SAM.
 *
 * Copyright (c) 2011-2018 Microchip Technology Inc. and its subsidiaries.
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

#ifndef DACC_H_INCLUDED
#define DACC_H_INCLUDED

#include "compiler.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

//! DACC return codes
typedef enum dacc_rc {
	DACC_RC_OK = 0,        //!< Operation OK
	DACC_RC_INVALID_PARAM  //!< Invalid parameter
} dacc_rc_t;

#if (SAM3N) || (SAM4L) || (SAM4N)
//! DACC resolution in number of data bits
# define DACC_RESOLUTION     10
#else
//! DACC resolution in number of data bits
# define DACC_RESOLUTION     12
#endif
//! DACC max data value
#define DACC_MAX_DATA       ((1 << DACC_RESOLUTION) - 1)


void dacc_reset(Dacc *p_dacc);
#if (SAMV70 || SAMV71 || SAME70 || SAMS70)
uint32_t dacc_set_trigger(Dacc *p_dacc, uint32_t ul_trigger, uint32_t channel);
#else
uint32_t dacc_set_trigger(Dacc *p_dacc, uint32_t ul_trigger);
#endif
#if (SAMV70 || SAMV71 || SAME70 || SAMS70)
void dacc_disable_trigger(Dacc *p_dacc, uint32_t channel);
#else
void dacc_disable_trigger(Dacc *p_dacc);
#endif
uint32_t dacc_set_transfer_mode(Dacc *p_dacc, uint32_t ul_mode);
void dacc_enable_interrupt(Dacc *p_dacc, uint32_t ul_interrupt_mask);
void dacc_disable_interrupt(Dacc *p_dacc, uint32_t ul_interrupt_mask);
uint32_t dacc_get_interrupt_mask(Dacc *p_dacc);
uint32_t dacc_get_interrupt_status(Dacc *p_dacc);
#if (SAMV70 || SAMV71 || SAME70 || SAMS70)
void dacc_write_conversion_data(Dacc *p_dacc, uint32_t ul_data, uint32_t channel);
#else
void dacc_write_conversion_data(Dacc *p_dacc, uint32_t ul_data);
#endif
void dacc_set_writeprotect(Dacc *p_dacc, uint32_t ul_enable);
uint32_t dacc_get_writeprotect_status(Dacc *p_dacc);
#if !(SAM4L || SAMV70 || SAMV71 || SAME70 || SAMS70)
Pdc *dacc_get_pdc_base(Dacc *p_dacc);
#endif

#if (SAM3N) || (SAM4L) || (SAM4N) || defined(__DOXYGEN__)
void dacc_enable(Dacc *p_dacc);
void dacc_disable(Dacc *p_dacc);
uint32_t dacc_set_timing(Dacc *p_dacc, uint32_t ul_startup,
		uint32_t ul_clock_divider);
#endif

#if (SAM4E)
uint32_t dacc_set_timing(Dacc *p_dacc, 
		uint32_t ul_maxs,uint32_t ul_startup);
#endif

#if (SAM3S) || (SAM3XA) || (SAM4S) || (SAM4E) || (SAMV70) || (SAMV71) || (SAME70) || (SAMS70) || defined(__DOXYGEN__)
#if !(SAMV70 || SAMV71 || SAME70 || SAMS70)
uint32_t dacc_set_channel_selection(Dacc *p_dacc, uint32_t ul_channel);
void dacc_enable_flexible_selection(Dacc *p_dacc);
#endif

#if (SAM3S) || (SAM3XA)
uint32_t dacc_set_power_save(Dacc *p_dacc, uint32_t ul_sleep_mode,
		uint32_t ul_fast_wakeup_mode);
#endif

#if !(SAMV70 || SAMV71 || SAME70 || SAMS70 || SAM4E)
uint32_t dacc_set_timing(Dacc *p_dacc, uint32_t ul_refresh, uint32_t ul_maxs,
		uint32_t ul_startup);
#endif
#if (SAMV70 || SAMV71 || SAME70 || SAMS70)
uint32_t dacc_set_prescaler(Dacc *p_dacc, uint32_t ul_prescaler);
uint32_t dacc_set_osr(Dacc *p_dacc, uint32_t channel, uint32_t ul_osr);
#endif
uint32_t dacc_enable_channel(Dacc *p_dacc, uint32_t ul_channel);
uint32_t dacc_disable_channel(Dacc *p_dacc, uint32_t ul_channel);
uint32_t dacc_get_channel_status(Dacc *p_dacc);
uint32_t dacc_set_analog_control(Dacc *p_dacc, uint32_t ul_analog_control);
uint32_t dacc_get_analog_control(Dacc *p_dacc);
#endif

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond

#endif /* DACC_H_INCLUDED */
