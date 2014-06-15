/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander 2014
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

#include "mk20dx.h"


extern unsigned long _stext;
extern unsigned long _etext;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _estack;
//extern void __init_array_start(void);
//extern void __init_array_end(void);
extern int main (void);
void ResetHandler(void);
void _init_Teensyduino_internal_(void);
void __libc_init_array(void);


void fault_isr(void)
{
	while (1) {
		// keep polling some communication while in fault
		// mode, so we don't completely die.
		if (SIM_SCGC4 & SIM_SCGC4_USBOTG) usb_isr();
		if (SIM_SCGC4 & SIM_SCGC4_UART0) uart0_status_isr();
		if (SIM_SCGC4 & SIM_SCGC4_UART1) uart1_status_isr();
		if (SIM_SCGC4 & SIM_SCGC4_UART2) uart2_status_isr();
	}
}

void unused_isr(void)
{
	fault_isr();
}

extern volatile uint32_t systick_millis_count;
void systick_default_isr(void)
{
	systick_millis_count++;
}

void nmi_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void hard_fault_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void memmanage_fault_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void bus_fault_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void usage_fault_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void svcall_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void debugmonitor_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void pendablesrvreq_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void systick_isr(void)		__attribute__ ((weak, alias("systick_default_isr")));

void dma_ch0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch2_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch3_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch4_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch5_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch6_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch7_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch8_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch9_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch10_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch11_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch12_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch13_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch14_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_ch15_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dma_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void mcm_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void flash_cmd_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void flash_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void low_voltage_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void wakeup_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void watchdog_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void i2c0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void i2c1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void i2c2_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void spi0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void spi1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void spi2_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void sdhc_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void can0_message_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void can0_bus_off_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void can0_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void can0_tx_warn_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void can0_rx_warn_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void can0_wakeup_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void i2s0_tx_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void i2s0_rx_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void uart0_lon_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart0_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart0_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart1_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart1_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart2_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart2_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart3_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart3_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart4_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart4_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart5_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart5_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void adc0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void adc1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void cmp0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void cmp1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void cmp2_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void ftm0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void ftm1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void ftm2_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void ftm3_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void cmt_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void rtc_alarm_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void rtc_seconds_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void pit0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void pit1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void pit2_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void pit3_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void pdb_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void usb_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void usb_charge_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void dac0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void dac1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void tsi0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void mcg_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void lptmr_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void porta_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void portb_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void portc_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void portd_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void porte_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void software_isr(void)		__attribute__ ((weak, alias("unused_isr")));


// TODO: create AVR-stype ISR() macro, with default linkage to undefined handler
//
__attribute__ ((section(".vectors"), used))
void (* const gVectors[])(void) =
{
	(void (*)(void))((unsigned long)&_estack),	//  0 ARM: Initial Stack Pointer
	ResetHandler,					//  1 ARM: Initial Program Counter
	nmi_isr,					//  2 ARM: Non-maskable Interrupt (NMI)
	hard_fault_isr,					//  3 ARM: Hard Fault
	memmanage_fault_isr,				//  4 ARM: MemManage Fault
	bus_fault_isr,					//  5 ARM: Bus Fault
	usage_fault_isr,				//  6 ARM: Usage Fault
	fault_isr,					//  7 --
	fault_isr,					//  8 --
	fault_isr,					//  9 --
	fault_isr,					// 10 --
	svcall_isr,					// 11 ARM: Supervisor call (SVCall)
	debugmonitor_isr,				// 12 ARM: Debug Monitor
	fault_isr,					// 13 --
	pendablesrvreq_isr,				// 14 ARM: Pendable req serv(PendableSrvReq)
	systick_isr,					// 15 ARM: System tick timer (SysTick)
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_)
	dma_ch0_isr,					// 16 DMA channel 0 transfer complete
	dma_ch1_isr,					// 17 DMA channel 1 transfer complete
	dma_ch2_isr,					// 18 DMA channel 2 transfer complete
	dma_ch3_isr,					// 19 DMA channel 3 transfer complete
	dma_error_isr,					// 20 DMA error interrupt channel
	unused_isr,					// 21 DMA --
	flash_cmd_isr,					// 22 Flash Memory Command complete
	flash_error_isr,				// 23 Flash Read collision
	low_voltage_isr,				// 24 Low-voltage detect/warning
	wakeup_isr,					// 25 Low Leakage Wakeup
	watchdog_isr,					// 26 Both EWM and WDOG interrupt
	i2c0_isr,					// 27 I2C0
	spi0_isr,					// 28 SPI0
	i2s0_tx_isr,					// 29 I2S0 Transmit
	i2s0_rx_isr,					// 30 I2S0 Receive
	uart0_lon_isr,					// 31 UART0 CEA709.1-B (LON) status
	uart0_status_isr,				// 32 UART0 status
	uart0_error_isr,				// 33 UART0 error
	uart1_status_isr,				// 34 UART1 status
	uart1_error_isr,				// 35 UART1 error
	uart2_status_isr,				// 36 UART2 status
	uart2_error_isr,				// 37 UART2 error
	adc0_isr,					// 38 ADC0
	cmp0_isr,					// 39 CMP0
	cmp1_isr,					// 40 CMP1
	ftm0_isr,					// 41 FTM0
	ftm1_isr,					// 42 FTM1
	cmt_isr,					// 43 CMT
	rtc_alarm_isr,					// 44 RTC Alarm interrupt
	rtc_seconds_isr,				// 45 RTC Seconds interrupt
	pit0_isr,					// 46 PIT Channel 0
	pit1_isr,					// 47 PIT Channel 1
	pit2_isr,					// 48 PIT Channel 2
	pit3_isr,					// 49 PIT Channel 3
	pdb_isr,					// 50 PDB Programmable Delay Block
	usb_isr,					// 51 USB OTG
	usb_charge_isr,					// 52 USB Charger Detect
	tsi0_isr,					// 53 TSI0
	mcg_isr,					// 54 MCG
	lptmr_isr,					// 55 Low Power Timer
	porta_isr,					// 56 Pin detect (Port A)
	portb_isr,					// 57 Pin detect (Port B)
	portc_isr,					// 58 Pin detect (Port C)
	portd_isr,					// 59 Pin detect (Port D)
	porte_isr,					// 60 Pin detect (Port E)
	software_isr,					// 61 Software interrupt
#elif defined(_mk20dx256_)
	dma_ch0_isr,					// 16 DMA channel 0 transfer complete
	dma_ch1_isr,					// 17 DMA channel 1 transfer complete
	dma_ch2_isr,					// 18 DMA channel 2 transfer complete
	dma_ch3_isr,					// 19 DMA channel 3 transfer complete
	dma_ch4_isr,					// 20 DMA channel 4 transfer complete
	dma_ch5_isr,					// 21 DMA channel 5 transfer complete
	dma_ch6_isr,					// 22 DMA channel 6 transfer complete
	dma_ch7_isr,					// 23 DMA channel 7 transfer complete
	dma_ch8_isr,					// 24 DMA channel 8 transfer complete
	dma_ch9_isr,					// 25 DMA channel 9 transfer complete
	dma_ch10_isr,					// 26 DMA channel 10 transfer complete
	dma_ch11_isr,					// 27 DMA channel 10 transfer complete
	dma_ch12_isr,					// 28 DMA channel 10 transfer complete
	dma_ch13_isr,					// 29 DMA channel 10 transfer complete
	dma_ch14_isr,					// 30 DMA channel 10 transfer complete
	dma_ch15_isr,					// 31 DMA channel 10 transfer complete
	dma_error_isr,					// 32 DMA error interrupt channel
	unused_isr,					// 33 --
	flash_cmd_isr,					// 34 Flash Memory Command complete
	flash_error_isr,				// 35 Flash Read collision
	low_voltage_isr,				// 36 Low-voltage detect/warning
	wakeup_isr,					// 37 Low Leakage Wakeup
	watchdog_isr,					// 38 Both EWM and WDOG interrupt
	unused_isr,					// 39 --
	i2c0_isr,					// 40 I2C0
	i2c1_isr,					// 41 I2C1
	spi0_isr,					// 42 SPI0
	spi1_isr,					// 43 SPI1
	unused_isr,					// 44 --
	can0_message_isr,				// 45 CAN OR'ed Message buffer (0-15)
	can0_bus_off_isr,				// 46 CAN Bus Off
	can0_error_isr,					// 47 CAN Error
	can0_tx_warn_isr,				// 48 CAN Transmit Warning
	can0_rx_warn_isr,				// 49 CAN Receive Warning
	can0_wakeup_isr,				// 50 CAN Wake Up
	i2s0_tx_isr,					// 51 I2S0 Transmit
	i2s0_rx_isr,					// 52 I2S0 Receive
	unused_isr,					// 53 --
	unused_isr,					// 54 --
	unused_isr,					// 55 --
	unused_isr,					// 56 --
	unused_isr,					// 57 --
	unused_isr,					// 58 --
	unused_isr,					// 59 --
	uart0_lon_isr,					// 60 UART0 CEA709.1-B (LON) status
	uart0_status_isr,				// 61 UART0 status
	uart0_error_isr,				// 62 UART0 error
	uart1_status_isr,				// 63 UART1 status
	uart1_error_isr,				// 64 UART1 error
	uart2_status_isr,				// 65 UART2 status
	uart2_error_isr,				// 66 UART2 error
	unused_isr,					// 67 --
	unused_isr,					// 68 --
	unused_isr,					// 69 --
	unused_isr,					// 70 --
	unused_isr,					// 71 --
	unused_isr,					// 72 --
	adc0_isr,					// 73 ADC0
	adc1_isr,					// 74 ADC1
	cmp0_isr,					// 75 CMP0
	cmp1_isr,					// 76 CMP1
	cmp2_isr,					// 77 CMP2
	ftm0_isr,					// 78 FTM0
	ftm1_isr,					// 79 FTM1
	ftm2_isr,					// 80 FTM2
	cmt_isr,					// 81 CMT
	rtc_alarm_isr,					// 82 RTC Alarm interrupt
	rtc_seconds_isr,				// 83 RTC Seconds interrupt
	pit0_isr,					// 84 PIT Channel 0
	pit1_isr,					// 85 PIT Channel 1
	pit2_isr,					// 86 PIT Channel 2
	pit3_isr,					// 87 PIT Channel 3
	pdb_isr,					// 88 PDB Programmable Delay Block
	usb_isr,					// 89 USB OTG
	usb_charge_isr,					// 90 USB Charger Detect
	unused_isr,					// 91 --
	unused_isr,					// 92 --
	unused_isr,					// 93 --
	unused_isr,					// 94 --
	unused_isr,					// 95 --
	unused_isr,					// 96 --
	dac0_isr,					// 97 DAC0
	unused_isr,					// 98 --
	tsi0_isr,					// 99 TSI0
	mcg_isr,					// 100 MCG
	lptmr_isr,					// 101 Low Power Timer
	unused_isr,					// 102 --
	porta_isr,					// 103 Pin detect (Port A)
	portb_isr,					// 104 Pin detect (Port B)
	portc_isr,					// 105 Pin detect (Port C)
	portd_isr,					// 106 Pin detect (Port D)
	porte_isr,					// 107 Pin detect (Port E)
	unused_isr,					// 108 --
	unused_isr,					// 109 --
	software_isr,					// 110 Software interrupt
#endif
};

//void usb_isr(void)
//{
//}

__attribute__ ((section(".flashconfig"), used))
const uint8_t flashconfigbytes[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF
};


// Automatically initialize the RTC.  When the build defines the compile
// time, and the user has added a crystal, the RTC will automatically
// begin at the time of the first upload.
#ifndef TIME_T
#define TIME_T 1349049600 // default 1 Oct 2012 (never used, Arduino sets this)
#endif
extern void rtc_set(unsigned long t);



static void startup_unused_hook(void) {}
void startup_early_hook(void)		__attribute__ ((weak, alias("startup_unused_hook")));
void startup_late_hook(void)		__attribute__ ((weak, alias("startup_unused_hook")));


__attribute__ ((section(".startup")))
void ResetHandler(void)
{
#if defined(_mk20dx128vlf5_)
	/* Disable Watchdog */
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE;

        /* FLL at 48MHz */
	MCG_C4 = MCG_C4_DMX32 | MCG_C4_DRST_DRS(1);
	/*
        MCG.c4.raw = ((struct MCG_C4_t){
                        .drst_drs = MCG_DRST_DRS_MID,
                        .dmx32 = 1
                }).raw;
	*/
	SIM_SOPT2 = SIM_SOPT2_PLLFLLSEL;

	// release I/O pins hold, if we woke up from VLLS mode
	if (PMC_REGSC & PMC_REGSC_ACKISO) PMC_REGSC |= PMC_REGSC_ACKISO;

	uint32_t *src = &_etext;
	uint32_t *dest = &_sdata;
	unsigned int i;

	while (dest < &_edata) *dest++ = *src++;
	dest = &_sbss;
	while (dest < &_ebss) *dest++ = 0;
	SCB_VTOR = 0;	// use vector table in flash

	// default all interrupts to medium priority level
	for (i=0; i < NVIC_NUM_INTERRUPTS; i++) NVIC_SET_PRIORITY(i, 128);

	__enable_irq();
	__libc_init_array();

        //memcpy(&_sdata, &_sidata, (uintptr_t)&_edata - (uintptr_t)&_sdata);
        //memset(&_sbss, 0, (uintptr_t)&_ebss - (uintptr_t)&_sbss);
#else
	uint32_t *src = &_etext;
	uint32_t *dest = &_sdata;
	unsigned int i;

	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE;
	startup_early_hook();

	// enable clocks to always-used peripherals
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_)
	SIM_SCGC5 = 0x00043F82;		// clocks active to all GPIO
	SIM_SCGC6 = SIM_SCGC6_RTC | SIM_SCGC6_FTM0 | SIM_SCGC6_FTM1 | SIM_SCGC6_ADC0 | SIM_SCGC6_FTFL;
#elif defined(_mk20dx256_)
	SIM_SCGC3 = SIM_SCGC3_ADC1 | SIM_SCGC3_FTM2;
	SIM_SCGC5 = 0x00043F82;		// clocks active to all GPIO
	SIM_SCGC6 = SIM_SCGC6_RTC | SIM_SCGC6_FTM0 | SIM_SCGC6_FTM1 | SIM_SCGC6_ADC0 | SIM_SCGC6_FTFL;
#endif

	// if the RTC oscillator isn't enabled, get it started early
	if (!(RTC_CR & RTC_CR_OSCE)) {
		RTC_SR = 0;
		RTC_CR = RTC_CR_SC16P | RTC_CR_SC4P | RTC_CR_OSCE;
	}

	// release I/O pins hold, if we woke up from VLLS mode
	if (PMC_REGSC & PMC_REGSC_ACKISO) PMC_REGSC |= PMC_REGSC_ACKISO;

	// TODO: do this while the PLL is waiting to lock....
	while (dest < &_edata) *dest++ = *src++;
	dest = &_sbss;
	while (dest < &_ebss) *dest++ = 0;
	SCB_VTOR = 0;	// use vector table in flash

	// default all interrupts to medium priority level
	for (i=0; i < NVIC_NUM_INTERRUPTS; i++) NVIC_SET_PRIORITY(i, 128);

	// start in FEI mode
	// enable capacitors for crystal
	OSC0_CR = OSC_SC8P | OSC_SC2P;
	// enable osc, 8-32 MHz range, low power mode
	MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_EREFS;
	// switch to crystal as clock source, FLL input = 16 MHz / 512
	MCG_C1 =  MCG_C1_CLKS(2) | MCG_C1_FRDIV(4);
	// wait for crystal oscillator to begin
	while ((MCG_S & MCG_S_OSCINIT0) == 0) ;
	// wait for FLL to use oscillator
	while ((MCG_S & MCG_S_IREFST) != 0) ;
	// wait for MCGOUT to use oscillator
	while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(2)) ;
	// now we're in FBE mode
	// config PLL input for 16 MHz Crystal / 4 = 4 MHz
	MCG_C5 = MCG_C5_PRDIV0(3);
	// config PLL for 96 MHz output
	MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0(0);
	// wait for PLL to start using xtal as its input
	while (!(MCG_S & MCG_S_PLLST)) ;
	// wait for PLL to lock
	while (!(MCG_S & MCG_S_LOCK0)) ;
	// now we're in PBE mode
#if F_CPU == 96000000
	// config divisors: 96 MHz core, 48 MHz bus, 24 MHz flash
	SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) |	 SIM_CLKDIV1_OUTDIV4(3);
#elif F_CPU == 48000000
	// config divisors: 48 MHz core, 48 MHz bus, 24 MHz flash
	SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1) | SIM_CLKDIV1_OUTDIV2(1) |	 SIM_CLKDIV1_OUTDIV4(3);
#elif F_CPU == 24000000
	// config divisors: 24 MHz core, 24 MHz bus, 24 MHz flash
	SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(3) | SIM_CLKDIV1_OUTDIV2(3) |	 SIM_CLKDIV1_OUTDIV4(3);
#else
#error "Error, F_CPU must be 96000000, 48000000, or 24000000"
#endif
	// switch to PLL as clock source, FLL input = 16 MHz / 512
	MCG_C1 = MCG_C1_CLKS(0) | MCG_C1_FRDIV(4);
	// wait for PLL clock to be used
	while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(3)) ;
	// now we're in PEE mode
	// configure USB for 48 MHz clock
	SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(1); // USB = 96 MHz PLL / 2
	// USB uses PLL clock, trace is CPU clock, CLKOUT=OSCERCLK0
	SIM_SOPT2 = SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_TRACECLKSEL | SIM_SOPT2_CLKOUTSEL(6);

	// initialize the SysTick counter
	SYST_RVR = (F_CPU / 1000) - 1;
	SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;

	//init_pins();
	__enable_irq();

	//_init_Teensyduino_internal_(); XXX HaaTa - Why is this here? Perhaps fixed in a new version of the API?
	//if (RTC_SR & RTC_SR_TIF) rtc_set(TIME_T); XXX HaaTa - We don't care about the rtc

	__libc_init_array();

/*
	for (ptr = &__init_array_start; ptr < &__init_array_end; ptr++) {
		(*ptr)();
	}
*/
	startup_late_hook();
#endif
	main();
	while (1) ;
}

// TODO: is this needed for c++ and where does it come from?
/*
void _init(void)
{
}
*/

char *__brkval = (char *)&_ebss;

void * _sbrk(int incr)
{
	//static char *heap_end = (char *)&_ebss;
	//char *prev = heap_end;
	//heap_end += incr;

	char *prev = __brkval;
	__brkval += incr;
	return prev;
}

__attribute__((weak)) 
int _read(int file, char *ptr, int len)
{
	return 0;
}

/*  moved to Print.cpp, to support Print::printf()
__attribute__((weak)) 
int _write(int file, char *ptr, int len)
{
	return 0;
}
*/

__attribute__((weak)) 
int _close(int fd)
{
	return -1;
}

#include <sys/stat.h>

__attribute__((weak)) 
int _fstat(int fd, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

__attribute__((weak)) 
int _isatty(int fd)
{
	return 1;
}

__attribute__((weak)) 
int _lseek(int fd, long long offset, int whence)
{
	return -1;
}

__attribute__((weak)) 
void _exit(int status)
{
	while (1);
}

__attribute__((weak)) 
void __cxa_pure_virtual()
{
	while (1);
}

__attribute__((weak)) 
int __cxa_guard_acquire (int *g) 
{
	return 1;
}

__attribute__((weak)) 
void __cxa_guard_release(int *g)
{
}

int nvic_execution_priority(void)
{
	int priority=256;
	uint32_t primask, faultmask, basepri, ipsr;

	// full algorithm in ARM DDI0403D, page B1-639
	// this isn't quite complete, but hopefully good enough
	asm volatile("mrs %0, faultmask\n" : "=r" (faultmask)::);
	if (faultmask) return -1;
	asm volatile("mrs %0, primask\n" : "=r" (primask)::);
	if (primask) return 0;
	asm volatile("mrs %0, ipsr\n" : "=r" (ipsr)::);
	if (ipsr) {
		if (ipsr < 16) priority = 0; // could be non-zero
		else priority = NVIC_GET_PRIORITY(ipsr - 16);
	}
	asm volatile("mrs %0, basepri\n" : "=r" (basepri)::);
	if (basepri > 0 && basepri < priority) priority = basepri;
	return priority;
}

