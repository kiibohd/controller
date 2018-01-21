/* Copyright (C) 2017 by Jacob Alexander
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

// ----- Includes -----

// Debug Includes
#include "led.h"
#if defined(_bootloader_)
#include <inttypes.h>
#include <debug.h>
#else
#include <print.h>
#endif

// Local Includes
#include "entropy.h"
#include "sam.h"



// ----- Variables -----

/* Initialize segments */
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sstack;
extern uint32_t _estack;

// ----- Function Declarations -----

extern int main();
void ResetHandler();



// ----- Interrupts -----

// NVIC - Default ISR
void fault_isr()
{
	print("Fault!" NL );
	while ( 1 )
	{
		// keep polling some communication while in fault
		// mode, so we don't completely die.
		/*if ( SIM_SCGC4 & SIM_SCGC4_USBOTG ) usb_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART0 )  uart0_status_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART1 )  uart1_status_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART2 )  uart2_status_isr();*/
	}
}

void unused_isr()
{
	fault_isr();
}

// NVIC - Default ISR/Vector Linking

/* Cortex-M4 core handlers */
void NMI_Handler        ( void ) __attribute__ ((weak, alias("unused_isr")));
void HardFault_Handler  ( void ) __attribute__ ((weak, alias("unused_isr")));
void MemManage_Handler  ( void ) __attribute__ ((weak, alias("unused_isr")));
void BusFault_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void UsageFault_Handler ( void ) __attribute__ ((weak, alias("unused_isr")));
void SVC_Handler        ( void ) __attribute__ ((weak, alias("unused_isr")));
void DebugMon_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void PendSV_Handler     ( void ) __attribute__ ((weak, alias("unused_isr")));
void SysTick_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));

/* Peripherals handlers */
void SUPC_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void RSTC_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void RTC_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void RTT_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void WDT_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void PMC_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void EFC0_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
#ifdef _SAM4S_EFC1_INSTANCE_
void EFC1_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_EFC1_INSTANCE_ */
void UART0_Handler  ( void ) __attribute__ ((weak, alias("unused_isr")));
void UART1_Handler  ( void ) __attribute__ ((weak, alias("unused_isr")));
void PIOA_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void PIOB_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
#ifdef _SAM4S_PIOC_INSTANCE_
void PIOC_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_PIOC_INSTANCE_ */
void USART0_Handler ( void ) __attribute__ ((weak, alias("unused_isr")));
#ifdef _SAM4S_USART1_INSTANCE_
void USART1_Handler ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_USART1_INSTANCE_ */
#ifdef _SAM4S_HSMCI_INSTANCE_
void HSMCI_Handler  ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_HSMCI_INSTANCE_ */
void TWI0_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void TWI1_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void SPI_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void SSC_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void TC0_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void TC1_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void TC2_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
#ifdef _SAM4S_TC1_INSTANCE_
void TC3_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_TC1_INSTANCE_ */
#ifdef _SAM4S_TC1_INSTANCE_
void TC4_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_TC1_INSTANCE_ */
#ifdef _SAM4S_TC1_INSTANCE_
void TC5_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_TC1_INSTANCE_ */
void ADC_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
#ifdef _SAM4S_DACC_INSTANCE_
void DACC_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_DACC_INSTANCE_ */
void PWM_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void CRCCU_Handler  ( void ) __attribute__ ((weak, alias("unused_isr")));
void ACC_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void UDP_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));

/* Exception Table */
__attribute__ ((section(".vectors")))
const DeviceVectors exception_table = {

        /* Configure Initial Stack Pointer, using linker-generated symbols */
        .pvStack = (void*) (&_estack),

        .pfnReset_Handler      = (void*) ResetHandler,
        .pfnNMI_Handler        = (void*) NMI_Handler,
        .pfnHardFault_Handler  = (void*) HardFault_Handler,
        .pfnMemManage_Handler  = (void*) MemManage_Handler,
        .pfnBusFault_Handler   = (void*) BusFault_Handler,
        .pfnUsageFault_Handler = (void*) UsageFault_Handler,
        .pfnReserved1_Handler  = (void*) (0UL),          /* Reserved */
        .pfnReserved2_Handler  = (void*) (0UL),          /* Reserved */
        .pfnReserved3_Handler  = (void*) (0UL),          /* Reserved */
        .pfnReserved4_Handler  = (void*) (0UL),          /* Reserved */
        .pfnSVC_Handler        = (void*) SVC_Handler,
        .pfnDebugMon_Handler   = (void*) DebugMon_Handler,
        .pfnReserved5_Handler  = (void*) (0UL),          /* Reserved */
        .pfnPendSV_Handler     = (void*) PendSV_Handler,
        .pfnSysTick_Handler    = (void*) SysTick_Handler,

        /* Configurable interrupts */
        .pfnSUPC_Handler   = (void*) SUPC_Handler,   /* 0  Supply Controller */
        .pfnRSTC_Handler   = (void*) RSTC_Handler,   /* 1  Reset Controller */
        .pfnRTC_Handler    = (void*) RTC_Handler,    /* 2  Real Time Clock */
        .pfnRTT_Handler    = (void*) RTT_Handler,    /* 3  Real Time Timer */
        .pfnWDT_Handler    = (void*) WDT_Handler,    /* 4  Watchdog Timer */
        .pfnPMC_Handler    = (void*) PMC_Handler,    /* 5  Power Management Controller */
        .pfnEFC0_Handler   = (void*) EFC0_Handler,   /* 6  Enhanced Embedded Flash Controller 0 */
#ifdef _SAM4S_EFC1_INSTANCE_
        .pfnEFC1_Handler   = (void*) EFC1_Handler,   /* 7  Enhanced Embedded Flash Controller 1 */
#else
        .pvReserved7       = (void*) (0UL),          /* 7  Reserved */
#endif /* _SAM4S_EFC1_INSTANCE_ */
        .pfnUART0_Handler  = (void*) UART0_Handler,  /* 8  UART 0 */
        .pfnUART1_Handler  = (void*) UART1_Handler,  /* 9  UART 1 */
        .pvReserved10      = (void*) (0UL),          /* 10 Reserved */
        .pfnPIOA_Handler   = (void*) PIOA_Handler,   /* 11 Parallel I/O Controller A */
        .pfnPIOB_Handler   = (void*) PIOB_Handler,   /* 12 Parallel I/O Controller B */
#ifdef _SAM4S_PIOC_INSTANCE_
        .pfnPIOC_Handler   = (void*) PIOC_Handler,   /* 13 Parallel I/O Controller C */
#else
        .pvReserved13      = (void*) (0UL),          /* 13 Reserved */
#endif /* _SAM4S_PIOC_INSTANCE_ */
        .pfnUSART0_Handler = (void*) USART0_Handler, /* 14 USART 0 */
#ifdef _SAM4S_USART1_INSTANCE_
        .pfnUSART1_Handler = (void*) USART1_Handler, /* 15 USART 1 */
#else
        .pvReserved15      = (void*) (0UL),          /* 15 Reserved */
#endif /* _SAM4S_USART1_INSTANCE_ */
        .pvReserved16      = (void*) (0UL),          /* 16 Reserved */
        .pvReserved17      = (void*) (0UL),          /* 17 Reserved */
#ifdef _SAM4S_HSMCI_INSTANCE_
        .pfnHSMCI_Handler  = (void*) HSMCI_Handler,  /* 18 Multimedia Card Interface */
#else
        .pvReserved18      = (void*) (0UL),          /* 18 Reserved */
#endif /* _SAM4S_HSMCI_INSTANCE_ */
        .pfnTWI0_Handler   = (void*) TWI0_Handler,   /* 19 Two Wire Interface 0 */
        .pfnTWI1_Handler   = (void*) TWI1_Handler,   /* 20 Two Wire Interface 1 */
        //.pfnSPI_Handler    = (void*) SPI_Handler,    /* 21 Serial Peripheral Interface */ //FIXME
        .pfnSSC_Handler    = (void*) SSC_Handler,    /* 22 Synchronous Serial Controller */
        .pfnTC0_Handler    = (void*) TC0_Handler,    /* 23 Timer/Counter 0 */
        .pfnTC1_Handler    = (void*) TC1_Handler,    /* 24 Timer/Counter 1 */
        .pfnTC2_Handler    = (void*) TC2_Handler,    /* 25 Timer/Counter 2 */
#ifdef _SAM4S_TC1_INSTANCE_
        .pfnTC3_Handler    = (void*) TC3_Handler,    /* 26 Timer/Counter 3 */
#else
        .pvReserved26      = (void*) (0UL),          /* 26 Reserved */
#endif /* _SAM4S_TC1_INSTANCE_ */
#ifdef _SAM4S_TC1_INSTANCE_
        .pfnTC4_Handler    = (void*) TC4_Handler,    /* 27 Timer/Counter 4 */
#else
        .pvReserved27      = (void*) (0UL),          /* 27 Reserved */
#endif /* _SAM4S_TC1_INSTANCE_ */
#ifdef _SAM4S_TC1_INSTANCE_
        .pfnTC5_Handler    = (void*) TC5_Handler,    /* 28 Timer/Counter 5 */
#else
        .pvReserved28      = (void*) (0UL),          /* 28 Reserved */
#endif /* _SAM4S_TC1_INSTANCE_ */
        .pfnADC_Handler    = (void*) ADC_Handler,    /* 29 Analog To Digital Converter */
#ifdef _SAM4S_DACC_INSTANCE_
        .pfnDACC_Handler   = (void*) DACC_Handler,   /* 30 Digital To Analog Converter */
#else
        .pvReserved30      = (void*) (0UL),          /* 30 Reserved */
#endif /* _SAM4S_DACC_INSTANCE_ */
        .pfnPWM_Handler    = (void*) PWM_Handler,    /* 31 Pulse Width Modulation */
        .pfnCRCCU_Handler  = (void*) CRCCU_Handler,  /* 32 CRC Calculation Unit */
        .pfnACC_Handler    = (void*) ACC_Handler,    /* 33 Analog Comparator */
        //.pfnUDP_Handler    = (void*) UDP_Handler     /* 34 USB Device Port */ //FIXME
};


// ----- Flash Configuration -----

// ----- Functions -----

void *memset( void *addr, int val, unsigned int len )
{
	char *buf = addr;

	for (; len > 0; --len, ++buf)
		*buf = val;
	return (addr);
}

int memcmp( const void *a, const void *b, unsigned int len )
{
	const uint8_t *ap = a, *bp = b;
	int val = 0;

	for (; len > 0 && (val = *ap - *bp) == 0; --len, ++ap, ++bp)
		/* NOTHING */;
	return (val);
}

void *memcpy( void *dst, const void *src, unsigned int len )
{
	char *dstbuf = dst;
	const char *srcbuf = src;

	for (; len > 0; --len, ++dstbuf, ++srcbuf)
		*dstbuf = *srcbuf;
	return (dst);
}


// ----- Chip Entry Point -----

void ResetHandler()
{
	// TODO (HaaTa) - Add initialization code, this should be the firmware entry point
	
	uint32_t *pSrc, *pDest;
	/* Initialize the relocate segment */
	pSrc = &_etext;
	pDest = &_srelocate;

	if (pSrc != pDest) {
			for (; pDest < &_erelocate;) {
					*pDest++ = *pSrc++;
			}
	}

	/* Clear the zero segment */
	for (pDest = &_szero; pDest < &_ezero;) {
			*pDest++ = 0;
	}


	/* Set 6 FWS for Embedded Flash Access according to 120MHz configuration */
	EFC0->EEFC_FMR = EEFC_FMR_FWS(5)|EEFC_FMR_CLOE;
#if defined(EFC1) // Only valid for products with two flash banks
	EFC1->EEFC_FMR = EEFC_FMR_FWS(5)|EEFC_FMR_CLOE;
#endif // EFC1

	/*
	* We are coming from a Hard Reset or Backup mode.
	* The core is clocked by Internal Fast RC @ 4MHz.
	* We intend to use the device @120MHz from external Oscillator.
	* Steps are (cf datasheet chapter '29.14 Programming Sequence'):
	* 1- Activation of external oscillator
	* 2- Switch the MAINCK to the main crystal oscillator
	* 3- Wait for the MOSCSELS to be set
	* 4- Check the main clock frequency
	* 5- Set PLLx and Divider
	* 6- Select the master clock and processor clock
	* 7- Select the programmable clocks (optional)
	*/

	/* Step 1 - Activation of external oscillator
	* As we are clocking the core from internal Fast RC, we keep the bit CKGR_MOR_MOSCRCEN.
	* Main Crystal Oscillator Start-up Time (CKGR_MOR_MOSCXTST) is set to maximum value.
	* Then, we wait the startup time to be finished by checking PMC_SR_MOSCXTS in PMC_SR.
	*/
	PMC->CKGR_MOR = CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTST(0xfful) | CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN;
	for ( ; (PMC->PMC_SR & PMC_SR_MOSCXTS) != PMC_SR_MOSCXTS ; );

	/* Step 2 - Switch the MAINCK to the main crystal oscillator
	* We add the CKGR_MOR_MOSCSEL bit.
	* Then we wait for the selection to be done by checking PMC_SR_MOSCSELS in PMC_SR.
	*/
	PMC->CKGR_MOR = CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCXTST(0xfful) | CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCSEL;
	/* Step 3 - Wait for the MOSCSELS to be set */
	for ( ; (PMC->PMC_SR & PMC_SR_MOSCSELS) != PMC_SR_MOSCSELS ; );

	/* Step 4 - Check the main clock frequency */
	/* As written in the DS, we could check the MAINF value here (0x18a2) */

	/* Step 5 - Set PLLx and Divider
	* The external oscillator is 12MHz. As we intend to clock the system @120MHz,
	* we need to multiply the oscillator frequency by 10.
	* This can be done by setting MULx to value 9 and DIV to 1.
	* We set the maximum PLL Lock time to maximum in CKGR_PLLAR_PLLACOUNT.
	*/
	//PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | (CKGR_PLLAR_MULA(0x1dul) | CKGR_PLLAR_DIVA(3ul) | CKGR_PLLAR_PLLACOUNT(0x1ul));
	PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | (CKGR_PLLAR_MULA(0x9ul) | CKGR_PLLAR_DIVA(1ul) | CKGR_PLLAR_PLLACOUNT(0x3ful));
	for ( ; (PMC->PMC_SR & PMC_SR_LOCKA) != PMC_SR_LOCKA ; );

	/* Step 6 - Select the master clock and processor clock
	* Source for MasterClock will be PLLA output (PMC_MCKR_CSS_PLLA_CLK), without frequency division.
	*/
	PMC->PMC_MCKR = PMC_MCKR_PRES_CLK_1 | PMC_MCKR_CSS_PLLA_CLK;
	for ( ; (PMC->PMC_SR & PMC_SR_MCKRDY) != PMC_SR_MCKRDY ; );

	/*
	* Step 7 - Select the programmable clocks
	*
	* Output MCK on PCK1/pin PA17
	* Used to validate Master Clock settings
	*/
	//  PMC->PMC_SCER = PMC_SCER_PCK1 ;

	//SystemCoreClock=__SYSTEM_CLOCK_120MHZ;
	
	/* exception_table being initialized, setup vectors in RAM */
	//vectorSetOrigin( &exception_table );

	/* Set the vector table base address */
	pSrc = (uint32_t *) & _sfixed;
	SCB->VTOR = ((uint32_t) pSrc & SCB_VTOR_TBLOFF_Msk);


	// Enable IRQs
	__enable_irq();

	// Intialize entropy for random numbers
	rand_initialize();

	init_errorLED();
	errorLED(1);

	// Start main
	main();
	while ( 1 ); // Shouldn't get here...
}



// ----- RAM Setup -----

// ----- Interrupt Execution Priority -----

