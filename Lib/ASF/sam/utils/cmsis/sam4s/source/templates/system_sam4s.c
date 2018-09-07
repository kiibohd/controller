/**
 * \file
 *
 * \brief Provides the low-level initialization functions that called 
 * on chip startup.
 *
 * Copyright (c) 2011-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#include "sam4s.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/* External oscillator definition, to be overriden by application */
#define CHIP_FREQ_XTAL_12M (12000000UL)

#if (!defined CHIP_FREQ_XTAL)
#  define CHIP_FREQ_XTAL CHIP_FREQ_XTAL_12M
#endif

/* Clock Settings (4MHz) using Internal Fast RC */
uint32_t SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ;

/**
 * \brief Setup the microcontroller system.
 *
 * Initialize the System and update the SystemFrequency variable.
 */
void SystemInit( void )
{
	/*
	 * TODO:
	 * Add code to initialize the system according to your application.
	 *
	 * For SAM4S, the internal 4MHz fast RC oscillator is the default clock
	 * selected at system reset state.
	 */

	/* Set FWS according to default clock configuration */
	EFC0->EEFC_FMR = EEFC_FMR_FWS(1)|EEFC_FMR_CLOE;
#if defined(ID_EFC1)
	EFC1->EEFC_FMR = EEFC_FMR_FWS(1)|EEFC_FMR_CLOE;
#endif
}

/**
 * \brief Get Core Clock Frequency.
 */
void SystemCoreClockUpdate( void )
{
	/* Determine clock frequency according to clock register values */
	switch ( PMC->PMC_MCKR & (uint32_t) PMC_MCKR_CSS_Msk ) {
	case PMC_MCKR_CSS_SLOW_CLK: /* Slow clock */
			if ( SUPC->SUPC_SR & SUPC_SR_OSCSEL ) {
				SystemCoreClock = CHIP_FREQ_XTAL_32K;
			} else {
				SystemCoreClock = CHIP_FREQ_SLCK_RC;
			}
		break;
		
	case PMC_MCKR_CSS_MAIN_CLK: /* Main clock */
		if ( PMC->CKGR_MOR & CKGR_MOR_MOSCSEL ) {
			SystemCoreClock = CHIP_FREQ_XTAL;
		} else {
			SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ;
			
			switch ( PMC->CKGR_MOR & CKGR_MOR_MOSCRCF_Msk ) {
			case CKGR_MOR_MOSCRCF_4_MHz:
				SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ;
			break;
			
			case CKGR_MOR_MOSCRCF_8_MHz:
				SystemCoreClock = CHIP_FREQ_MAINCK_RC_8MHZ;
			break;
			
			case CKGR_MOR_MOSCRCF_12_MHz:
				SystemCoreClock = CHIP_FREQ_MAINCK_RC_12MHZ;
			break;
			
			default:
			break;
			}
		}
		break;
		
	case PMC_MCKR_CSS_PLLA_CLK:	/* PLLA clock */
	case PMC_MCKR_CSS_PLLB_CLK:	/* PLLB clock */
			if ( PMC->CKGR_MOR & CKGR_MOR_MOSCSEL ) {
				SystemCoreClock = CHIP_FREQ_XTAL;
			} else {
				SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ;
				
				switch ( PMC->CKGR_MOR & CKGR_MOR_MOSCRCF_Msk ) {
				case CKGR_MOR_MOSCRCF_4_MHz:
					SystemCoreClock = CHIP_FREQ_MAINCK_RC_4MHZ;
					break;
				
				case CKGR_MOR_MOSCRCF_8_MHz:
					SystemCoreClock = CHIP_FREQ_MAINCK_RC_8MHZ;
					break;
				
				case CKGR_MOR_MOSCRCF_12_MHz:
					SystemCoreClock = CHIP_FREQ_MAINCK_RC_12MHZ;
					break;
				
				default:
					break;
				}
			}
			
			if ( (uint32_t)(PMC->PMC_MCKR & (uint32_t) PMC_MCKR_CSS_Msk) ==
					PMC_MCKR_CSS_PLLA_CLK ) {
				SystemCoreClock *= ((((PMC->CKGR_PLLAR) & CKGR_PLLAR_MULA_Msk)
						>> CKGR_PLLAR_MULA_Pos) + 1U);
				SystemCoreClock /= ((((PMC->CKGR_PLLAR) & CKGR_PLLAR_DIVA_Msk)
						>> CKGR_PLLAR_DIVA_Pos));
			} else {
				SystemCoreClock *= ((((PMC->CKGR_PLLBR) & CKGR_PLLBR_MULB_Msk)
						>> CKGR_PLLBR_MULB_Pos) + 1U);
				SystemCoreClock /= ((((PMC->CKGR_PLLBR) & CKGR_PLLBR_DIVB_Msk)
						>> CKGR_PLLBR_DIVB_Pos));
			}
		break;
		
	default:
		break;
	}

	if ( (PMC->PMC_MCKR & PMC_MCKR_PRES_Msk) == PMC_MCKR_PRES_CLK_3 ) {
		SystemCoreClock /= 3U;
	} else {
		SystemCoreClock >>=
			((PMC->PMC_MCKR & PMC_MCKR_PRES_Msk) >> PMC_MCKR_PRES_Pos);
	}
}

/**
 * \brief Initialize flash wait state according to operating frequency.
 *
 * \param ul_clk System clock frequency.
 */
void system_init_flash( uint32_t ul_clk )
{
  /* Set FWS for embedded Flash access according to operating frequency */
#if !defined(ID_EFC1)
	if ( ul_clk < CHIP_FREQ_FWS_0 ) {
		EFC0->EEFC_FMR = EEFC_FMR_FWS(0)|EEFC_FMR_CLOE;
	} else {
		if ( ul_clk < CHIP_FREQ_FWS_1 ) {
			EFC0->EEFC_FMR = EEFC_FMR_FWS(1)|EEFC_FMR_CLOE;
		} else {
			if ( ul_clk < CHIP_FREQ_FWS_2 ) {
				EFC0->EEFC_FMR = EEFC_FMR_FWS(2)|EEFC_FMR_CLOE;
			} else {
				if ( ul_clk < CHIP_FREQ_FWS_3 ) {
					EFC0->EEFC_FMR = EEFC_FMR_FWS(3)|EEFC_FMR_CLOE;
				} else {
					if ( ul_clk < CHIP_FREQ_FWS_4 ) {
					EFC0->EEFC_FMR = EEFC_FMR_FWS(4)|EEFC_FMR_CLOE;
					} else {
					EFC0->EEFC_FMR = EEFC_FMR_FWS(5)|EEFC_FMR_CLOE;
					}
				}
			}
		}
	}
#else
	if ( ul_clk < CHIP_FREQ_FWS_0 ) {
		EFC0->EEFC_FMR = EEFC_FMR_FWS(0)|EEFC_FMR_CLOE;
		EFC1->EEFC_FMR = EEFC_FMR_FWS(0)|EEFC_FMR_CLOE;
	} else {
		if ( ul_clk < CHIP_FREQ_FWS_1 ) {
			EFC0->EEFC_FMR = EEFC_FMR_FWS(1)|EEFC_FMR_CLOE;
			EFC1->EEFC_FMR = EEFC_FMR_FWS(1)|EEFC_FMR_CLOE;
		} else {
			if ( ul_clk < CHIP_FREQ_FWS_2 ) {
				EFC0->EEFC_FMR = EEFC_FMR_FWS(2)|EEFC_FMR_CLOE;
				EFC1->EEFC_FMR = EEFC_FMR_FWS(2)|EEFC_FMR_CLOE;
			} else {
				if ( ul_clk < CHIP_FREQ_FWS_3 ) {
					EFC0->EEFC_FMR = EEFC_FMR_FWS(3)|EEFC_FMR_CLOE;
					EFC1->EEFC_FMR = EEFC_FMR_FWS(3)|EEFC_FMR_CLOE;
				} else {
					if ( ul_clk < CHIP_FREQ_FWS_4 ) {
						EFC0->EEFC_FMR = EEFC_FMR_FWS(4)|EEFC_FMR_CLOE;
						EFC1->EEFC_FMR = EEFC_FMR_FWS(4)|EEFC_FMR_CLOE;
					} else {
						EFC0->EEFC_FMR = EEFC_FMR_FWS(5)|EEFC_FMR_CLOE;
						EFC1->EEFC_FMR = EEFC_FMR_FWS(5)|EEFC_FMR_CLOE;
					}
				}
			}
		}
	}
#endif
}

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
