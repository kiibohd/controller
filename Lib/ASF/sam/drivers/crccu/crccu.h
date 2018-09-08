/**
 * \file
 *
 * \brief Cyclic Redundancy Check Calculation Unit (CRCCU) driver for SAM.
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

#ifndef CRCCU_H_INCLUDED
#define CRCCU_H_INCLUDED

#include "compiler.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

/* CRCCU transfer control width register offset */
#define CRCCU_TR_CTRL_TRWIDTH_POS  (24)
/* CRCCU transfer control width register byte */
#define CRCCU_TR_CTRL_TRWIDTH_BYTE (0 << CRCCU_TR_CTRL_TRWIDTH_POS)
/* CRCCU transfer control width register halfword */
#define CRCCU_TR_CTRL_TRWIDTH_HALFWORD (1 << CRCCU_TR_CTRL_TRWIDTH_POS)
/* CRCCU transfer control width register word */
#define CRCCU_TR_CTRL_TRWIDTH_WORD (2 << CRCCU_TR_CTRL_TRWIDTH_POS)

/* CRCCU context done interrupt enable offset */
#define CRCCU_TR_CTRL_IEN_OFFSET   (27)
/*The transfer done status bit is set at the end of the transfer.*/
#define CRCCU_TR_CTRL_IEN_ENABLE   (0 << CRCCU_TR_CTRL_IEN_OFFSET)
/*The transfer done status bit is not set at the end of the transfer.*/
#define CRCCU_TR_CTRL_IEN_DISABLE  (1 << CRCCU_TR_CTRL_IEN_OFFSET)

/** CRCCU descriptor type */
typedef struct crccu_dscr_type {
	uint32_t ul_tr_addr;	/* TR_ADDR */
	uint32_t ul_tr_ctrl;	/* TR_CTRL */
#if (SAM3S8 || SAM3SD8 || SAM4S || SAM4L || SAMG55)
	uint32_t ul_reserved[2];	/* Reserved register */
#elif SAM3S
	uint32_t ul_reserved[52];	/* TR_CRC begins at offset 0xE0 */
#endif
	uint32_t ul_tr_crc;	/* TR_CRC */
} crccu_dscr_type_t;

void crccu_configure_descriptor(Crccu *p_crccu, uint32_t ul_crc_dscr_addr);
void crccu_configure_mode(Crccu *p_crccu, uint32_t ul_mode);
void crccu_enable_dma(Crccu *p_crccu);
void crccu_disable_dma(Crccu *p_crccu);
void crccu_reset(Crccu *p_crccu);
uint32_t crccu_get_dma_status(Crccu *p_crccu);
void crccu_enable_dma_interrupt(Crccu *p_crccu);
void crccu_disable_dma_interrupt(Crccu *p_crccu);
uint32_t crccu_get_dma_interrupt_status(Crccu *p_crccu);
uint32_t crccu_get_dma_interrupt_mask(Crccu *p_crccu);
uint32_t crccu_read_crc_value(Crccu *p_crccu);
void crccu_enable_error_interrupt(Crccu *p_crccu);
void crccu_disable_error_interrupt(Crccu *p_crccu);
uint32_t crccu_get_error_interrupt_status(Crccu *p_crccu);
uint32_t crccu_get_error_interrupt_mask(Crccu *p_crccu);

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond

#endif /* CRCCU_H_INCLUDED */
