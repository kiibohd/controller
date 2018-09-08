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

#include "crccu.h"

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

/**
 * \defgroup sam_drivers_crccu_group Cyclic Redundancy Check Calculation Unit (CRCCU)
 *
 * Driver for the Cyclic Redundancy Check Calculation Unit. It provides access to the main 
 * features of the CRCCU controller.
 *
 * @{
 */

/**
 * \brief Configure CRCCU descriptor.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 * \param ul_crc_dscr_addr CRCCU descriptor address.
 */
void crccu_configure_descriptor(Crccu *p_crccu, uint32_t ul_crc_dscr_addr)
{
	p_crccu->CRCCU_DSCR = ul_crc_dscr_addr;
}

/**
 * \brief Configure the CRCCU mode.
 *
 * \param p_crccu Pointer to a CRCCU instance. 
 * \param ul_mode CRC work mode.
 */
void crccu_configure_mode(Crccu *p_crccu, uint32_t ul_mode)
{
	p_crccu->CRCCU_MR = ul_mode;
}

/**
 * \brief Start CRCCU. 
 *
 * \note  To start the CRCCU, the user needs to set the CRC enable bit (ENABLE) 
 * in the CRCCU Mode Register (CRCCU_MR), and then configure it and finally set
 * the DMA enable bit (DMAEN) in the CRCCU DMA Enable Register (CRCCU_DMA_EN).
 *
 * \note The crccu_configure_descriptor() and crccu_configure_mode ()functions must 
 * be executed before calling this function.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 */
void crccu_enable_dma(Crccu *p_crccu)
{
	/* Start CRC calculation */
	p_crccu->CRCCU_DMA_EN = CRCCU_DMA_EN_DMAEN;
}

/**
 * \brief Disable CRCCU. 
 *
 * \param p_crccu Pointer to a CRCCU instance.
 */
void crccu_disable_dma(Crccu *p_crccu)
{
	p_crccu->CRCCU_DMA_DIS = CRCCU_DMA_DIS_DMADIS;
}

/**
 * \brief Reset CRCCU.
 */
void crccu_reset(Crccu *p_crccu)
{
	p_crccu->CRCCU_CR = CRCCU_CR_RESET;
}

/**
 * \brief Check if DMA channel is enabled.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 *
 * \return CRCCU DMA status.
 */
uint32_t crccu_get_dma_status(Crccu *p_crccu)
{
	return p_crccu->CRCCU_DMA_SR & CRCCU_DMA_SR_DMASR;
}

/**
 * \brief Enable CRCCU DMA transfer interrupt.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 */
void crccu_enable_dma_interrupt(Crccu *p_crccu)
{
	p_crccu->CRCCU_DMA_IER = CRCCU_DMA_IER_DMAIER;
}

/**
 * \brief Disable CRCCU DMA transfer interrupt.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 */
void crccu_disable_dma_interrupt(Crccu *p_crccu)
{
	p_crccu->CRCCU_DMA_IDR = CRCCU_DMA_IDR_DMAIDR;
}

/**
 * \brief Check if DMA buffer transfer has been terminated.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 *
 * \return DMA interrupt status.
 */
uint32_t crccu_get_dma_interrupt_status(Crccu *p_crccu)
{
	return p_crccu->CRCCU_DMA_ISR & CRCCU_DMA_ISR_DMAISR;
}

/**
 * \brief Get DMA interrupt mask.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 *
 * \return DMA interrupt mask.
 */
uint32_t crccu_get_dma_interrupt_mask(Crccu *p_crccu)
{
	return p_crccu->CRCCU_DMA_IMR & CRCCU_DMA_IMR_DMAIMR;
}

/**
 * \brief Read cyclic redundancy check value.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 *
 * \return CRC value.
 */
uint32_t crccu_read_crc_value(Crccu *p_crccu)
{
	return p_crccu->CRCCU_SR & CRCCU_SR_CRC_Msk;
}

/**
 * \brief Enable cyclic redundancy check error interrupt.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 */
void crccu_enable_error_interrupt(Crccu *p_crccu)
{
	p_crccu->CRCCU_IER = CRCCU_IER_ERRIER;
}

/**
 * \brief Disable cyclic redundancy check error interrupt.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 */
void crccu_disable_error_interrupt(Crccu *p_crccu)
{
	p_crccu->CRCCU_IDR = CRCCU_IDR_ERRIDR;
}

/**
 * \brief Check if there is a CRC error.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 *
 * \return Error interrupt status. 
 */
uint32_t crccu_get_error_interrupt_status(Crccu *p_crccu)
{
	return p_crccu->CRCCU_ISR & CRCCU_ISR_ERRISR;
}

/**
 * \brief Get check CRC error interrupt mask.
 *
 * \param p_crccu Pointer to a CRCCU instance.
 *
 * \return Error interrupt mask.
 */
uint32_t crccu_get_error_interrupt_mask(Crccu *p_crccu)
{
	return p_crccu->CRCCU_IMR & CRCCU_IMR_ERRIMR;
}

//@}

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
