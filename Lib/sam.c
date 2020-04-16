/* Copyright (C) 2017-2019 by Jacob Alexander
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
#if defined(_bootloader_)
#include <inttypes.h>
#include <debug.h>
#else
#include <print.h>
#include <led.h>
#endif

// Local Includes
#include "entropy.h"
#include "sam.h"
#include "sysview.h"

// ASF Includes
#include <sam/drivers/efc/efc.h>
#include <common/services/clock/osc.h>
#include <common/services/usb/udc/udc.h>

#define WDT_TICK_US (128 * 1000000 / BOARD_FREQ_SLCK_XTAL)
#define WDT_MAX_VALUE 4095

#define TRACE_BUFFER_SIZE 256

// ----- Variables -----

const uint8_t sys_reset_to_loader_magic[22] = "\xff\x00\x7fRESET TO LOADER\x7f\x00\xff";

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
extern uint32_t _ram_start_, _ram_end_;
extern uint32_t _sramfunc, _eramfunc;
extern uint32_t _sdata, _edata;

// Unique Id storage variable
// Must be read from flash using ReadUniqueID()
uint32_t sam_UniqueId[4];

uintptr_t __stack_chk_guard = 0xdeadbeef;

//__attribute__((__aligned__(TRACE_BUFFER_SIZE * sizeof(uint32_t)))) uint32_t mtb[TRACE_BUFFER_SIZE];

// ----- Function Declarations -----

extern int main();
void ResetHandler();



// ----- Interrupts -----

// NVIC - Default ISR
void fault_isr()
{
	print("Fault!" NL );

#if defined(FLASH_DEBUG)
	// Reset GPNVM bits to jump back to SAM-BA
	flash_clear_gpnvm(1);
	Reset_CleanupExternal();
	RSTC->RSTC_CR = RSTC_CR_KEY_PASSWD | RSTC_CR_EXTRST;
#else

#if defined(DEBUG) && defined(JLINK)
	asm volatile("BKPT #01");
#else
	while ( 1 )
	{
		// keep polling some communication while in fault
		// mode, so we don't completely die.
		/*if ( SIM_SCGC4 & SIM_SCGC4_USBOTG ) usb_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART0 )  uart0_status_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART1 )  uart1_status_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART2 )  uart2_status_isr();*/
	}
#endif
#endif
}


// Stack Overflow Interrupt
void __stack_chk_fail(void)
{
	uint32_t sp = __get_MSP();
	print("Stack overflow!" NL );
	print(" SP = ");
#if defined(_bootloader_)
	printHex(sp);
#else
	printHex32(sp);
#endif
	print(NL NL);
	fault_isr();
}

// Default ISR if not used
void unused_isr()
{
	Cortex_IRQ irq = get_current_isr();
	print("Unhandled ISR!" NL);
	print(" IRQn = ");
#if defined(_bootloader_)
	printHex(irq);
#else
	printHex32(irq);
#endif
	print(NL NL);
	fault_isr();
}

void __attribute__((naked)) debug_isr ( void ) {
	STACKED_ISR(hardfault_handler);
}

void __attribute__((naked)) mpu_isr ( void ) {
	STACKED_ISR(memfault_handler);
}

void __attribute__((naked)) bus_isr ( void ) {
	STACKED_ISR(busfault_handler);
}

void __attribute__((naked)) usage_isr ( void ) {
	STACKED_ISR(usagefault_handler);
}

// NVIC - SysTick ISR
extern volatile uint32_t systick_millis_count;
void systick_default_isr()
{
	SEGGER_SYSVIEW_RecordEnterISR();
	systick_millis_count++;

	// Not necessary in bootloader
#if !defined(_bootloader_)
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL &= DWT_CTRL_CYCCNTENA_Msk;
	// Check to see if SysTick is being starved by another IRQ
	// 12 cycle IRQ latency (plus some extra)
	// XXX (HaaTa) There seems to be a CPU bug where you need to wait some clock cycles before you can
	// clear the CYCCNT register on SAM4S (this wasn't the case on Kinetis)
	// TODO (HaaTa) Determine what is starving IRQs by about 2000 cycles
	if ( DWT->CYCCNT > F_CPU / 1000 + 30 )
	{
		/* XXX (HaaTa) The printing of this message is causing LED buffers to get clobbered (likely due to frame overflows)
		erro_printNL("SysTick is being starved by another IRQ...");
		printInt32( DWT->CYCCNT );
		print(" vs. ");
		printInt32( F_CPU / 1000 );
		print(NL);
		*/
	}
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif
	SEGGER_SYSVIEW_RecordExitISRToScheduler();
}

/* Cortex-M4 core handlers */
void NMI_Handler        ( void ) __attribute__ ((weak, alias("unused_isr")));
void HardFault_Handler  ( void ) __attribute__ ((weak, alias("fault_isr")));
void MemManage_Handler  ( void ) __attribute__ ((weak, alias("mpu_isr")));
void BusFault_Handler   ( void ) __attribute__ ((weak, alias("bus_isr")));
void UsageFault_Handler ( void ) __attribute__ ((weak, alias("usage_isr")));
void SVC_Handler        ( void ) __attribute__ ((weak, alias("unused_isr")));
void DebugMon_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void PendSV_Handler     ( void ) __attribute__ ((weak, alias("unused_isr")));
void SysTick_Handler    ( void ) __attribute__ ((weak, alias("systick_default_isr")));

/* Peripherals handlers */
void SUPC_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void RSTC_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
void RTC_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void RTT_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
//void WDT_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
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
void TC3_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void TC4_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void TC5_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void ADC_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
#ifdef _SAM4S_DACC_INSTANCE_
void DACC_Handler   ( void ) __attribute__ ((weak, alias("unused_isr")));
#endif /* _SAM4S_DACC_INSTANCE_ */
void PWM_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void CRCCU_Handler  ( void ) __attribute__ ((weak, alias("unused_isr")));
void ACC_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));
void UDP_Handler    ( void ) __attribute__ ((weak, alias("unused_isr")));


void WDT_Handler() {
	print("WDT!" NL );
#if defined(DEBUG) && defined(JLINK)
	__asm volatile("BKPT #01");
#else
	SOFTWARE_RESET();
	for( ;; );
#endif
}

/* Exception Table */
/* NOTE: Table alignment requirements mean that bits [6:0] of the table offset are always zero */
__attribute__ ((section(".vectors")))
const DeviceVectors exception_table = {

        /* Configure Initial Stack Pointer, using linker-generated symbols */
        .pvStack = (void*) (&_estack),

        .pfnReset_Handler      = (void*) ResetHandler,
        .pfnNMI_Handler        = (void*) NMI_Handler,

#if defined(DEBUG)
        .pfnHardFault_Handler  = (void*) debug_isr,
#else
        .pfnHardFault_Handler  = (void*) HardFault_Handler,
#endif
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
        .pfnSPI_Handler    = (void*) SPI_Handler,    /* 21 Serial Peripheral Interface */
        .pfnSSC_Handler    = (void*) SSC_Handler,    /* 22 Synchronous Serial Controller */
        .pfnTC0_Handler    = (void*) TC0_Handler,    /* 23 Timer/Counter 0 */
        .pfnTC1_Handler    = (void*) TC1_Handler,    /* 24 Timer/Counter 1 */
        .pfnTC2_Handler    = (void*) TC2_Handler,    /* 25 Timer/Counter 2 */
        .pfnTC3_Handler    = (void*) TC3_Handler,    /* 26 Timer/Counter 3 */
        .pfnTC4_Handler    = (void*) TC4_Handler,    /* 27 Timer/Counter 4 */
        .pfnTC5_Handler    = (void*) TC5_Handler,    /* 28 Timer/Counter 5 */
        .pfnADC_Handler    = (void*) ADC_Handler,    /* 29 Analog To Digital Converter */
#ifdef _SAM4S_DACC_INSTANCE_
        .pfnDACC_Handler   = (void*) DACC_Handler,   /* 30 Digital To Analog Converter */
#else
        .pvReserved30      = (void*) (0UL),          /* 30 Reserved */
#endif /* _SAM4S_DACC_INSTANCE_ */
        .pfnPWM_Handler    = (void*) PWM_Handler,    /* 31 Pulse Width Modulation */
        .pfnCRCCU_Handler  = (void*) CRCCU_Handler,  /* 32 CRC Calculation Unit */
        .pfnACC_Handler    = (void*) ACC_Handler,    /* 33 Analog Comparator */
        .pfnUDP_Handler    = (void*) UDP_Handler     /* 34 USB Device Port */
};

// ----- Flash Configuration -----

// ----- Functions -----

void *memset( void *addr, int val, unsigned int len )
{
#if defined(_cortex_m4_)
	// Optimized version of memset we split up the region into several segments
	// Adapted from: https://stackoverflow.com/a/57241273/9357160
	//
	// addr
	// * store single bytes
	// mid1
	// * store words, 4 at a time
	// mid2
	// * store words, 1 at a time
	// mid3
	// * store single bytes
	// end
	//
	// For large buffers, most of the time is spent between mid1 and mid2 which is
	// highly optimized.
	const uint32_t int_size = sizeof(uint32_t);

	// find first word-aligned address
	uint32_t ptr = (uint32_t) addr;

	// get end of memory to set
	uint32_t end = ptr + len;

	// get location of first word-aligned address at/after the start, but not
	// after the end
	uint32_t mid1 = (ptr + int_size - 1) / int_size * int_size;
	if (mid1 > end)
	{
		mid1 = end;
	}

	// get location of last word-aligned address at/before the end
	uint32_t mid3 = end / int_size * int_size;

	// get end location of optimized section
	uint32_t mid2 = mid1 + (mid3 - mid1) / (4 * int_size) * (4 * int_size);

	// create a word-sized integer
	uint32_t value = 0;
	for (uint16_t i = 0; i < int_size; ++i)
	{
		value <<= 8;
		value |= (uint8_t) val;
	}
	__ASM volatile (
		// store bytes
		"b Compare1%=\n"
		"Store1%=:\n"
		"strb %[value], [%[ptr]], #1\n"
		"Compare1%=:\n"
		"cmp %[ptr], %[mid1]\n"
		"bcc Store1%=\n"
		// store words optimized
		"b Compare2%=\n"
		"Store2%=:\n"
		"str %[value], [%[ptr]], #4\n"
		"str %[value], [%[ptr]], #4\n"
		"str %[value], [%[ptr]], #4\n"
		"str %[value], [%[ptr]], #4\n"
		"Compare2%=:\n"
		"cmp %[ptr], %[mid2]\n"
		"bcc Store2%=\n"
		// store words
		"b Compare3%=\n"
		"Store3%=:\n"
		"str %[value], [%[ptr]], #4\n"
		"Compare3%=:\n"
		"cmp %[ptr], %[mid3]\n"
		"bcc Store3%=\n"
		// store bytes
		"b Compare4%=\n"
		"Store4%=:\n"
		"strb %[value], [%[ptr]], #1\n"
		"Compare4%=:\n"
		"cmp %[ptr], %[end]\n"
		"bcc Store4%=\n"
		: // no outputs
		: [value] "r"(value),
		[ptr] "r"(ptr),
		[mid1] "r"(mid1),
		[mid2] "r"(mid2),
		[mid3] "r"(mid3),
		[end] "r"(end)
	);
	return addr;
#else
	char *buf = addr;

	for (; len > 0; --len, ++buf)
		*buf = val;
	return (addr);
#endif
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
// Fast memcpy (Cortex-M4 only), adapted from:
// https://cboard.cprogramming.com/c-programming/154333-fast-memcpy-alternative-32-bit-embedded-processor-posted-just-fyi-fwiw.html#post1149163
// Cortex-M4 can do unaligned accesses, even for 32-bit values
// Uses 32-bit values to do copies instead of 8-bit (should effectively speed up memcpy by 3x)
#if defined(_cortex_m4_)
	uint32_t i;
	uint32_t *pLongSrc;
	uint32_t *pLongDest;
	uint32_t numLongs = len / 4;
	uint32_t endLen = len & 0x03;

	// Convert byte addressing to long addressing
	pLongSrc = (uint32_t*) src;
	pLongDest = (uint32_t*) dst;

	// Copy long values, disregarding any 32-bit alignment issues
	for ( i = 0; i < numLongs; i++ )
	{
		*pLongDest++ = *pLongSrc++;
	}

	// Convert back to byte addressing
	uint8_t *srcbuf = (uint8_t*) pLongSrc;
	uint8_t *dstbuf = (uint8_t*) pLongDest;

	// Copy trailing bytes byte-by-byte
	for (; endLen > 0; --endLen, ++dstbuf, ++srcbuf)
		*dstbuf = *srcbuf;

	return (dst);
#else
	char *dstbuf = dst;
	const char *srcbuf = src;

	for (; len > 0; --len, ++dstbuf, ++srcbuf)
		*dstbuf = *srcbuf;
	return (dst);
#endif
}

// Reads Unique Id into sam_UniqueId buffer
uint32_t ReadUniqueID()
{
	// Send Start STUI (Read Unique Identifier)
	// Send Stop SPUI (Read Unique Identifier)
	// Read 32 x 4 bytes (128 bytes) into sam_UniqueId
	return efc_perform_read_sequence( EFC0, EEFC_FCR_FCMD_STUI, EEFC_FCR_FCMD_SPUI, sam_UniqueId, 4 );
}


// ----- Chip Entry Point -----

// ===== Target frequency (System clock)
// - PLLA source: XTAL             = 12MHz
// - PLLA output: XTAL * 20 / 1    = 240MHz
// - System clock source: PLLA
// - System clock prescaler: 2 (divided by 2)
// - System clock: 12 * 20 / 1 / 2 = 120MHz

// ===== Target frequency (USB Clock)
// - PLLB source: XTAL          = 12MHz
// - PLLB output: XTAL * 16 / 2 = 96Mhz
// - USB clock source: PLLB
// - USB clock divider: 2 (divided by 2)
// - USB clock: 12 * 16 / 2 / 2 = 48MHz

__attribute__ ((section(".startup")))
void ResetHandler()
{
	// Not locked up... Reset the watchdog timer
	WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;

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

	/* Initialize the stack */
	for (pDest = &_sstack; pDest < &_estack;) {
		*pDest++ = 0xDEADBEEF;
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

	PMC->PMC_MCKR = (PMC->PMC_MCKR & (~PMC_MCKR_CSS_Msk)) | PMC_MCKR_CSS_SLOW_CLK;
	for ( ; (PMC->PMC_SR & PMC_SR_MCKRDY) != PMC_SR_MCKRDY ; );

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
	PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | (CKGR_PLLAR_MULA(20-1) | CKGR_PLLAR_DIVA(1) | CKGR_PLLAR_PLLACOUNT(0x3ful));
	for ( ; (PMC->PMC_SR & PMC_SR_LOCKA) != PMC_SR_LOCKA ; );

	PMC->CKGR_PLLBR = (CKGR_PLLBR_MULB(16-1) | CKGR_PLLBR_DIVB(2) | CKGR_PLLBR_PLLBCOUNT(0x3ful));
	for ( ; (PMC->PMC_SR & PMC_SR_LOCKB) != PMC_SR_LOCKB ; );

	/* Step 6 - Select the master clock and processor clock
	* Source for MasterClock will be PLLA output (PMC_MCKR_CSS_PLLA_CLK), with 1/2 frequency division.
	* NOTE: Must change prescaler before changing source
	*/
	PMC->PMC_MCKR = (PMC->PMC_MCKR & (~PMC_MCKR_PRES_Msk)) | PMC_MCKR_PRES_CLK_2;
	for ( ; (PMC->PMC_SR & PMC_SR_MCKRDY) != PMC_SR_MCKRDY ; );

	PMC->PMC_MCKR = (PMC->PMC_MCKR & (~PMC_MCKR_CSS_Msk)) | PMC_MCKR_CSS_PLLA_CLK;
	//PMC->PMC_MCKR = PMC_MCKR_PRES_CLK_2 | PMC_MCKR_CSS_PLLA_CLK;
	for ( ; (PMC->PMC_SR & PMC_SR_MCKRDY) != PMC_SR_MCKRDY ; );

	/* Set the vector table base address */
	pSrc = (uint32_t *) & _sfixed;
	SCB->VTOR = ((uint32_t) pSrc);

	/* Disable PMC write protection */
	//PMC->PMC_WPMR = PMC_WPMR_WPKEY(0x504D43ul) & ~PMC_WPMR_WPEN;

	cm4_init();

	// Default to no access
	mpu_setup_region(0, &_sfixed,            &_efixed,          MPU_ATTR_FLASH         | MPU_ACCESS_RO_ALL     | 0);
	mpu_setup_region(1,
	         (uint32_t*)STORAGE_FLASH_START,
	                              (uint32_t*)STORAGE_FLASH_END, MPU_ATTR_FLASH         | MPU_ACCESS_RW_ALL     | MPU_ATTR_XN);
	mpu_setup_region(2, &_sramfunc,          &_eramfunc,        MPU_ATTR_INTERNAL_SRAM | MPU_ACCESS_RO_ALL     | 0);
	mpu_setup_region(3, (uint32_t*)_sdata,   &_ezero,           MPU_ATTR_INTERNAL_SRAM | MPU_ACCESS_RW_ALL     | MPU_ATTR_XN);
	mpu_setup_region(4, (uint32_t*)_sstack,  &_estack,          MPU_ATTR_INTERNAL_SRAM | MPU_ACCESS_RW_ALL     | MPU_ATTR_XN);
	mpu_setup_region(5, PERIPH_START,        PERIPH_END,        MPU_ATTR_PERIPHERALS   | MPU_ACCESS_PRIV_WRITE | MPU_ATTR_XN);
	mpu_setup_region(6, SYSTEM_END,          SYSTEM_END,        MPU_ACCESS_PRIV_WRITE); // Always XN
	mpu_enable();

	// Initialize the SysTick counter
	SysTick->LOAD = (F_CPU / 1000) - 1;
	SysTick->VAL = 0;
	SysTick->CALIB = F_CPU / 1000 / 8;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;

	// Enable IRQs
	__enable_irq();

	// Intialize entropy for random numbers
	rand_initialize();

#if !defined(_bootloader_)
	init_errorLED();
	errorLED(0);

	for ( int pos = 0; pos <= sizeof(sys_reset_to_loader_magic)/sizeof(GPBR->SYS_GPBR[0]); pos++ )
		GPBR->SYS_GPBR[ pos ] = 0x00000000;
#endif
	// Read Unique ID from flash
	ReadUniqueID();

	// Start main
	main();
	while ( 1 ); // Shouldn't get here...
}

bool main_kbd_enable(void)
{
	return true;
}

