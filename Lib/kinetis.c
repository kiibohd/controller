/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 * Modifications by Jacob Alexander 2014-2017
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

// Debug Includes
#if defined(_bootloader_)
#include <inttypes.h>
#include <debug.h>
#else
#include <print.h>
#endif

// Local Includes
#include "entropy.h"
#include "kinetis.h"



// ----- Variables -----

extern unsigned long _stext;
extern unsigned long _etext;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _estack;

const uint8_t sys_reset_to_loader_magic[22] = "\xff\x00\x7fRESET TO LOADER\x7f\x00\xff";



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
		if ( SIM_SCGC4 & SIM_SCGC4_USBOTG ) usb_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART0 )  uart0_status_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART1 )  uart1_status_isr();
		if ( SIM_SCGC4 & SIM_SCGC4_UART2 )  uart2_status_isr();
	}
}

void unused_isr()
{
	fault_isr();
}


// NVIC - SysTick ISR
extern volatile uint32_t systick_millis_count;
void systick_default_isr()
{
	systick_millis_count++;

	// Not necessary in bootloader
#if !defined(_bootloader_)
	// Reset cycle count register
	ARM_DEMCR |= ARM_DEMCR_TRCENA;
	ARM_DWT_CTRL &= ~ARM_DWT_CTRL_CYCCNTENA;
	ARM_DWT_CYCCNT = 0;
	ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
#endif
}


// NVIC - Non-Maskable Interrupt ISR
void nmi_default_isr()
{
	print("NMI!" NL );
}


// NVIC - Hard Fault ISR
void hard_fault_default_isr()
{
	print("Hard Fault! SCB_HFSR: ");
	printHex32( SCB_HFSR );
	print( NL );
	SOFTWARE_RESET();
}


// NVIC - Memory Manager Fault ISR
void memmanage_fault_default_isr()
{
	print("Memory Manager Fault! SCB_CFSR: ");
	printHex32( SCB_CFSR );
	print(" SCB_MMAR: ");
	printHex32( SCB_MMAR );
	print( NL );
}


// NVIC - Bus Fault ISR
void bus_fault_default_isr()
{
	print("Bus Fault! SCB_CFSR: ");
	printHex32( SCB_CFSR );
	print(" SCB_BFAR: ");
	printHex32( SCB_BFAR );
	print( NL );
}


// NVIC - Usage Fault ISR
void usage_fault_default_isr()
{
	print("Usage Fault! SCB_CFSR: ");
	printHex32( SCB_CFSR );
	print( NL );
}


// NVIC - Watchdog ISR
void watchdog_default_isr()
{
	print("Watchdog timeout! ");
	printHex( WDOG_TMROUTH );
	print(" ");
	printHex( WDOG_TMROUTL );
	print( NL );
	print("Watchdog Reset Count: ");
	printHex( WDOG_RSTCNT );
	print( NL );
}


// NVIC - Default ISR/Vector Linking
void nmi_isr()              __attribute__ ((weak, alias("nmi_default_isr")));
void hard_fault_isr()       __attribute__ ((weak, alias("hard_fault_default_isr")));
void memmanage_fault_isr()  __attribute__ ((weak, alias("memmanage_fault_default_isr")));
void bus_fault_isr()        __attribute__ ((weak, alias("bus_fault_default_isr")));
void usage_fault_isr()      __attribute__ ((weak, alias("usage_fault_default_isr")));
void svcall_isr()           __attribute__ ((weak, alias("unused_isr")));
void debugmonitor_isr()     __attribute__ ((weak, alias("unused_isr")));
void pendablesrvreq_isr()   __attribute__ ((weak, alias("unused_isr")));
void systick_isr()          __attribute__ ((weak, alias("systick_default_isr")));

void dma_ch0_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch1_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch2_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch3_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch4_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch5_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch6_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch7_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch8_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch9_isr()          __attribute__ ((weak, alias("unused_isr")));
void dma_ch10_isr()         __attribute__ ((weak, alias("unused_isr")));
void dma_ch11_isr()         __attribute__ ((weak, alias("unused_isr")));
void dma_ch12_isr()         __attribute__ ((weak, alias("unused_isr")));
void dma_ch13_isr()         __attribute__ ((weak, alias("unused_isr")));
void dma_ch14_isr()         __attribute__ ((weak, alias("unused_isr")));
void dma_ch15_isr()         __attribute__ ((weak, alias("unused_isr")));
void dma_error_isr()        __attribute__ ((weak, alias("unused_isr")));
void mcm_isr()              __attribute__ ((weak, alias("unused_isr")));
void randnum_isr()          __attribute__ ((weak, alias("unused_isr")));
void flash_cmd_isr()        __attribute__ ((weak, alias("unused_isr")));
void flash_error_isr()      __attribute__ ((weak, alias("unused_isr")));
void low_voltage_isr()      __attribute__ ((weak, alias("unused_isr")));
void wakeup_isr()           __attribute__ ((weak, alias("unused_isr")));
void watchdog_isr()         __attribute__ ((weak, alias("watchdog_default_isr")));
void i2c0_isr()             __attribute__ ((weak, alias("unused_isr")));
void i2c1_isr()             __attribute__ ((weak, alias("unused_isr")));
void i2c2_isr()             __attribute__ ((weak, alias("unused_isr")));
void i2c3_isr()             __attribute__ ((weak, alias("unused_isr")));
void spi0_isr()             __attribute__ ((weak, alias("unused_isr")));
void spi1_isr()             __attribute__ ((weak, alias("unused_isr")));
void spi2_isr()             __attribute__ ((weak, alias("unused_isr")));
void sdhc_isr()             __attribute__ ((weak, alias("unused_isr")));
void enet_timer_isr()       __attribute__ ((weak, alias("unused_isr")));
void enet_tx_isr()          __attribute__ ((weak, alias("unused_isr")));
void enet_rx_isr()          __attribute__ ((weak, alias("unused_isr")));
void enet_error_isr()       __attribute__ ((weak, alias("unused_isr")));
void can0_message_isr()     __attribute__ ((weak, alias("unused_isr")));
void can0_bus_off_isr()     __attribute__ ((weak, alias("unused_isr")));
void can0_error_isr()       __attribute__ ((weak, alias("unused_isr")));
void can0_tx_warn_isr()     __attribute__ ((weak, alias("unused_isr")));
void can0_rx_warn_isr()     __attribute__ ((weak, alias("unused_isr")));
void can0_wakeup_isr()      __attribute__ ((weak, alias("unused_isr")));
void can1_message_isr()     __attribute__ ((weak, alias("unused_isr")));
void can1_bus_off_isr()     __attribute__ ((weak, alias("unused_isr")));
void can1_error_isr()       __attribute__ ((weak, alias("unused_isr")));
void can1_tx_warn_isr()     __attribute__ ((weak, alias("unused_isr")));
void can1_rx_warn_isr()     __attribute__ ((weak, alias("unused_isr")));
void can1_wakeup_isr()      __attribute__ ((weak, alias("unused_isr")));
void i2s0_tx_isr()          __attribute__ ((weak, alias("unused_isr")));
void i2s0_rx_isr()          __attribute__ ((weak, alias("unused_isr")));
void i2s0_isr()             __attribute__ ((weak, alias("unused_isr")));
void uart0_lon_isr()        __attribute__ ((weak, alias("unused_isr")));
void uart0_status_isr()     __attribute__ ((weak, alias("unused_isr")));
void uart0_error_isr()      __attribute__ ((weak, alias("unused_isr")));
void uart1_status_isr()     __attribute__ ((weak, alias("unused_isr")));
void uart1_error_isr()      __attribute__ ((weak, alias("unused_isr")));
void uart2_status_isr()     __attribute__ ((weak, alias("unused_isr")));
void uart2_error_isr()      __attribute__ ((weak, alias("unused_isr")));
void uart3_status_isr()     __attribute__ ((weak, alias("unused_isr")));
void uart3_error_isr()      __attribute__ ((weak, alias("unused_isr")));
void uart4_status_isr()     __attribute__ ((weak, alias("unused_isr")));
void uart4_error_isr()      __attribute__ ((weak, alias("unused_isr")));
void uart5_status_isr()     __attribute__ ((weak, alias("unused_isr")));
void uart5_error_isr()      __attribute__ ((weak, alias("unused_isr")));
void lpuart0_status_isr()   __attribute__ ((weak, alias("unused_isr")));
void adc0_isr()             __attribute__ ((weak, alias("unused_isr")));
void adc1_isr()             __attribute__ ((weak, alias("unused_isr")));
void cmp0_isr()             __attribute__ ((weak, alias("unused_isr")));
void cmp1_isr()             __attribute__ ((weak, alias("unused_isr")));
void cmp2_isr()             __attribute__ ((weak, alias("unused_isr")));
void cmp3_isr()             __attribute__ ((weak, alias("unused_isr")));
void ftm0_isr()             __attribute__ ((weak, alias("unused_isr")));
void ftm1_isr()             __attribute__ ((weak, alias("unused_isr")));
void ftm2_isr()             __attribute__ ((weak, alias("unused_isr")));
void ftm3_isr()             __attribute__ ((weak, alias("unused_isr")));
void tpm0_isr()             __attribute__ ((weak, alias("unused_isr")));
void tpm1_isr()             __attribute__ ((weak, alias("unused_isr")));
void tpm2_isr()             __attribute__ ((weak, alias("unused_isr")));
void cmt_isr()              __attribute__ ((weak, alias("unused_isr")));
void rtc_alarm_isr()        __attribute__ ((weak, alias("unused_isr")));
void rtc_seconds_isr()      __attribute__ ((weak, alias("unused_isr")));
void pit_isr()              __attribute__ ((weak, alias("unused_isr")));
void pit0_isr()             __attribute__ ((weak, alias("unused_isr")));
void pit1_isr()             __attribute__ ((weak, alias("unused_isr")));
void pit2_isr()             __attribute__ ((weak, alias("unused_isr")));
void pit3_isr()             __attribute__ ((weak, alias("unused_isr")));
void pdb_isr()              __attribute__ ((weak, alias("unused_isr")));
void usb_isr()              __attribute__ ((weak, alias("unused_isr")));
void usb_charge_isr()       __attribute__ ((weak, alias("unused_isr")));
void usbhs_isr()            __attribute__ ((weak, alias("unused_isr")));
void usbhs_phy_isr()        __attribute__ ((weak, alias("unused_isr")));
void dac0_isr()             __attribute__ ((weak, alias("unused_isr")));
void dac1_isr()             __attribute__ ((weak, alias("unused_isr")));
void tsi0_isr()             __attribute__ ((weak, alias("unused_isr")));
void mcg_isr()              __attribute__ ((weak, alias("unused_isr")));
void lptmr_isr()            __attribute__ ((weak, alias("unused_isr")));
void porta_isr()            __attribute__ ((weak, alias("unused_isr")));
void portb_isr()            __attribute__ ((weak, alias("unused_isr")));
void portc_isr()            __attribute__ ((weak, alias("unused_isr")));
void portd_isr()            __attribute__ ((weak, alias("unused_isr")));
void porte_isr()            __attribute__ ((weak, alias("unused_isr")));
void portcd_isr()           __attribute__ ((weak, alias("unused_isr")));
void software_isr()         __attribute__ ((weak, alias("unused_isr")));


// NVIC - Interrupt Vector Table
__attribute__ ((section(".vectors"), used))
void (* const gVectors[])() =
{
	(void (*)(void))((unsigned long)&_estack),      //  0 ARM: Initial Stack Pointer
	ResetHandler,                                   //  1 ARM: Initial Program Counter
	nmi_isr,                                        //  2 ARM: Non-maskable Interrupt (NMI)
	hard_fault_isr,                                 //  3 ARM: Hard Fault
	memmanage_fault_isr,                            //  4 ARM: MemManage Fault
	bus_fault_isr,                                  //  5 ARM: Bus Fault
	usage_fault_isr,                                //  6 ARM: Usage Fault
	fault_isr,                                      //  7 --
	fault_isr,                                      //  8 --
	fault_isr,                                      //  9 --
	fault_isr,                                      // 10 --
	svcall_isr,                                     // 11 ARM: Supervisor call (SVCall)
	debugmonitor_isr,                               // 12 ARM: Debug Monitor
	fault_isr,                                      // 13 --
	pendablesrvreq_isr,                             // 14 ARM: Pendable req serv(PendableSrvReq)
	systick_isr,                                    // 15 ARM: System tick timer (SysTick)
#if defined(_mk20dx128_) || defined(_mk20dx128vlf5_)
	dma_ch0_isr,                                    // 16 DMA channel 0 transfer complete
	dma_ch1_isr,                                    // 17 DMA channel 1 transfer complete
	dma_ch2_isr,                                    // 18 DMA channel 2 transfer complete
	dma_ch3_isr,                                    // 19 DMA channel 3 transfer complete
	dma_error_isr,                                  // 20 DMA error interrupt channel
	unused_isr,                                     // 21 DMA --
	flash_cmd_isr,                                  // 22 Flash Memory Command complete
	flash_error_isr,                                // 23 Flash Read collision
	low_voltage_isr,                                // 24 Low-voltage detect/warning
	wakeup_isr,                                     // 25 Low Leakage Wakeup
	watchdog_isr,                                   // 26 Both EWM and WDOG interrupt
	i2c0_isr,                                       // 27 I2C0
	spi0_isr,                                       // 28 SPI0
	i2s0_tx_isr,                                    // 29 I2S0 Transmit
	i2s0_rx_isr,                                    // 30 I2S0 Receive
	uart0_lon_isr,                                  // 31 UART0 CEA709.1-B (LON) status
	uart0_status_isr,                               // 32 UART0 status
	uart0_error_isr,                                // 33 UART0 error
	uart1_status_isr,                               // 34 UART1 status
	uart1_error_isr,                                // 35 UART1 error
	uart2_status_isr,                               // 36 UART2 status
	uart2_error_isr,                                // 37 UART2 error
	adc0_isr,                                       // 38 ADC0
	cmp0_isr,                                       // 39 CMP0
	cmp1_isr,                                       // 40 CMP1
	ftm0_isr,                                       // 41 FTM0
	ftm1_isr,                                       // 42 FTM1
	cmt_isr,                                        // 43 CMT
	rtc_alarm_isr,                                  // 44 RTC Alarm interrupt
	rtc_seconds_isr,                                // 45 RTC Seconds interrupt
	pit0_isr,                                       // 46 PIT Channel 0
	pit1_isr,                                       // 47 PIT Channel 1
	pit2_isr,                                       // 48 PIT Channel 2
	pit3_isr,                                       // 49 PIT Channel 3
	pdb_isr,                                        // 50 PDB Programmable Delay Block
	usb_isr,                                        // 51 USB OTG
	usb_charge_isr,                                 // 52 USB Charger Detect
	tsi0_isr,                                       // 53 TSI0
	mcg_isr,                                        // 54 MCG
	lptmr_isr,                                      // 55 Low Power Timer
	porta_isr,                                      // 56 Pin detect (Port A)
	portb_isr,                                      // 57 Pin detect (Port B)
	portc_isr,                                      // 58 Pin detect (Port C)
	portd_isr,                                      // 59 Pin detect (Port D)
	porte_isr,                                      // 60 Pin detect (Port E)
	software_isr,                                   // 61 Software interrupt
#elif defined(_mk20dx128vlh7_) || defined(_mk20dx256_) || defined(_mk20dx256vlh7_)
	dma_ch0_isr,                                    // 16 DMA channel 0 transfer complete
	dma_ch1_isr,                                    // 17 DMA channel 1 transfer complete
	dma_ch2_isr,                                    // 18 DMA channel 2 transfer complete
	dma_ch3_isr,                                    // 19 DMA channel 3 transfer complete
	dma_ch4_isr,                                    // 20 DMA channel 4 transfer complete
	dma_ch5_isr,                                    // 21 DMA channel 5 transfer complete
	dma_ch6_isr,                                    // 22 DMA channel 6 transfer complete
	dma_ch7_isr,                                    // 23 DMA channel 7 transfer complete
	dma_ch8_isr,                                    // 24 DMA channel 8 transfer complete
	dma_ch9_isr,                                    // 25 DMA channel 9 transfer complete
	dma_ch10_isr,                                   // 26 DMA channel 10 transfer complete
	dma_ch11_isr,                                   // 27 DMA channel 10 transfer complete
	dma_ch12_isr,                                   // 28 DMA channel 10 transfer complete
	dma_ch13_isr,                                   // 29 DMA channel 10 transfer complete
	dma_ch14_isr,                                   // 30 DMA channel 10 transfer complete
	dma_ch15_isr,                                   // 31 DMA channel 10 transfer complete
	dma_error_isr,                                  // 32 DMA error interrupt channel
	unused_isr,                                     // 33 --
	flash_cmd_isr,                                  // 34 Flash Memory Command complete
	flash_error_isr,                                // 35 Flash Read collision
	low_voltage_isr,                                // 36 Low-voltage detect/warning
	wakeup_isr,                                     // 37 Low Leakage Wakeup
	watchdog_isr,                                   // 38 Both EWM and WDOG interrupt
	unused_isr,                                     // 39 --
	i2c0_isr,                                       // 40 I2C0
	i2c1_isr,                                       // 41 I2C1
	spi0_isr,                                       // 42 SPI0
	unused_isr,                                     // 43 -- (SPI1 doesn't actually exist for mk20dx256vlh7)
	unused_isr,                                     // 44 --
	can0_message_isr,                               // 45 CAN OR'ed Message buffer (0-15)
	can0_bus_off_isr,                               // 46 CAN Bus Off
	can0_error_isr,                                 // 47 CAN Error
	can0_tx_warn_isr,                               // 48 CAN Transmit Warning
	can0_rx_warn_isr,                               // 49 CAN Receive Warning
	can0_wakeup_isr,                                // 50 CAN Wake Up
	i2s0_tx_isr,                                    // 51 I2S0 Transmit
	i2s0_rx_isr,                                    // 52 I2S0 Receive
	unused_isr,                                     // 53 --
	unused_isr,                                     // 54 --
	unused_isr,                                     // 55 --
	unused_isr,                                     // 56 --
	unused_isr,                                     // 57 --
	unused_isr,                                     // 58 --
	unused_isr,                                     // 59 --
	uart0_lon_isr,                                  // 60 UART0 CEA709.1-B (LON) status
	uart0_status_isr,                               // 61 UART0 status
	uart0_error_isr,                                // 62 UART0 error
	uart1_status_isr,                               // 63 UART1 status
	uart1_error_isr,                                // 64 UART1 error
	uart2_status_isr,                               // 65 UART2 status
	uart2_error_isr,                                // 66 UART2 error
	unused_isr,                                     // 67 --
	unused_isr,                                     // 68 --
	unused_isr,                                     // 69 --
	unused_isr,                                     // 70 --
	unused_isr,                                     // 71 --
	unused_isr,                                     // 72 --
	adc0_isr,                                       // 73 ADC0
	adc1_isr,                                       // 74 ADC1
	cmp0_isr,                                       // 75 CMP0
	cmp1_isr,                                       // 76 CMP1
	cmp2_isr,                                       // 77 CMP2
	ftm0_isr,                                       // 78 FTM0
	ftm1_isr,                                       // 79 FTM1
	ftm2_isr,                                       // 80 FTM2
	cmt_isr,                                        // 81 CMT
	rtc_alarm_isr,                                  // 82 RTC Alarm interrupt
	rtc_seconds_isr,                                // 83 RTC Seconds interrupt
	pit0_isr,                                       // 84 PIT Channel 0
	pit1_isr,                                       // 85 PIT Channel 1
	pit2_isr,                                       // 86 PIT Channel 2
	pit3_isr,                                       // 87 PIT Channel 3
	pdb_isr,                                        // 88 PDB Programmable Delay Block
	usb_isr,                                        // 89 USB OTG
	usb_charge_isr,                                 // 90 USB Charger Detect
	unused_isr,                                     // 91 --
	unused_isr,                                     // 92 --
	unused_isr,                                     // 93 --
	unused_isr,                                     // 94 --
	unused_isr,                                     // 95 --
	unused_isr,                                     // 96 --
	dac0_isr,                                       // 97 DAC0
	unused_isr,                                     // 98 --
	tsi0_isr,                                       // 99 TSI0
	mcg_isr,                                        // 100 MCG
	lptmr_isr,                                      // 101 Low Power Timer
	unused_isr,                                     // 102 --
	porta_isr,                                      // 103 Pin detect (Port A)
	portb_isr,                                      // 104 Pin detect (Port B)
	portc_isr,                                      // 105 Pin detect (Port C)
	portd_isr,                                      // 106 Pin detect (Port D)
	porte_isr,                                      // 107 Pin detect (Port E)
	unused_isr,                                     // 108 --
	unused_isr,                                     // 109 --
	software_isr,                                   // 110 Software interrupt
#elif defined(_mk22fx512avlh12_)
	dma_ch0_isr,                                    // 16 DMA channel 0 transfer complete
	dma_ch1_isr,                                    // 17 DMA channel 1 transfer complete
	dma_ch2_isr,                                    // 18 DMA channel 2 transfer complete
	dma_ch3_isr,                                    // 19 DMA channel 3 transfer complete
	dma_ch4_isr,                                    // 20 DMA channel 4 transfer complete
	dma_ch5_isr,                                    // 21 DMA channel 5 transfer complete
	dma_ch6_isr,                                    // 22 DMA channel 6 transfer complete
	dma_ch7_isr,                                    // 23 DMA channel 7 transfer complete
	dma_ch8_isr,                                    // 24 DMA channel 8 transfer complete
	dma_ch9_isr,                                    // 25 DMA channel 9 transfer complete
	dma_ch10_isr,                                   // 26 DMA channel 10 transfer complete
	dma_ch11_isr,                                   // 27 DMA channel 10 transfer complete
	dma_ch12_isr,                                   // 28 DMA channel 10 transfer complete
	dma_ch13_isr,                                   // 29 DMA channel 10 transfer complete
	dma_ch14_isr,                                   // 30 DMA channel 10 transfer complete
	dma_ch15_isr,                                   // 31 DMA channel 10 transfer complete
	dma_error_isr,                                  // 32 DMA error interrupt channel
	mcm_isr,                                        // 33 MCM interrupt
	flash_cmd_isr,                                  // 34 Flash Memory Command complete
	flash_error_isr,                                // 35 Flash Read collision
	low_voltage_isr,                                // 36 Low-voltage detect/warning
	wakeup_isr,                                     // 37 Low Leakage Wakeup
	watchdog_isr,                                   // 38 Both EWM and WDOG interrupt
	unused_isr,                                     // 39 --
	i2c0_isr,                                       // 40 I2C0
	i2c1_isr,                                       // 41 I2C1
	spi0_isr,                                       // 42 SPI0
	unused_isr,                                     // 43 --
	i2s0_tx_isr,                                    // 44 I2S0 Transmit
	i2s0_rx_isr,                                    // 45 ISS0 Receive
	unused_isr,                                     // 46 --
	uart0_status_isr,                               // 47 UART0 status
	uart0_error_isr,                                // 48 UART0 error
	uart1_status_isr,                               // 49 UART1 status
	uart1_error_isr,                                // 50 UART1 error
	uart2_status_isr,                               // 51 UART2 status
	uart2_error_isr,                                // 52 UART2 error
	unused_isr,                                     // 53 --
	unused_isr,                                     // 54 --
	adc0_isr,                                       // 55 ADC0
	cmp0_isr,                                       // 56 CMP0
	cmp1_isr,                                       // 57 CMP1
	ftm0_isr,                                       // 58 FTM0
	ftm1_isr,                                       // 59 FTM1
	ftm2_isr,                                       // 60 FTM2
	cmt_isr,                                        // 61 CMT
	rtc_alarm_isr,                                  // 62 RTC Alarm interrupt
	rtc_seconds_isr,                                // 63 RTC Seconds interrupt
	pit0_isr,                                       // 64 PIT Channel 0
	pit1_isr,                                       // 65 PIT Channel 1
	pit2_isr,                                       // 66 PIT Channel 2
	pit3_isr,                                       // 67 PIT Channel 3
	pdb_isr,                                        // 68 PDB Programmable Delay Block
	usb_isr,                                        // 69 USB OTG
	usb_charge_isr,                                 // 70 USB Charger Detect
	unused_isr,                                     // 71 --
	dac0_isr,                                       // 72 DAC0
	mcg_isr,                                        // 73 MCG
	lptmr_isr,                                      // 74 Low Power Timer
	porta_isr,                                      // 75 Pin detect (Port A)
	portb_isr,                                      // 76 Pin detect (Port B)
	portc_isr,                                      // 77 Pin detect (Port C)
	portd_isr,                                      // 78 Pin detect (Port D)
	porte_isr,                                      // 79 Pin detect (Port E)
	software_isr,                                   // 80 Software interrupt
	unused_isr,                                     // 81 -- XXX (HaaTa) has description but no name in datasheet
	unused_isr,                                     // 82 --
	unused_isr,                                     // 83 --
	unused_isr,                                     // 84 --
	unused_isr,                                     // 85 --
	cmp2_isr,                                       // 86 CMP2
	ftm3_isr,                                       // 87 FTM3
	unused_isr,                                     // 88 --
	adc1_isr,                                       // 89 ADC1
	i2c2_isr,                                       // 90 I2C2
	can0_message_isr,                               // 91 CAN OR'ed Message buffer (0-15)
	can0_bus_off_isr,                               // 92 CAN Bus Off
	can0_error_isr,                                 // 93 CAN Error
	can0_tx_warn_isr,                               // 94 CAN Transmit Warning
	can0_rx_warn_isr,                               // 95 CAN Receive Warning
	can0_wakeup_isr,                                // 96 CAN Wake Up
	unused_isr,                                     // 97 --
#elif defined(_mk64fx512_)
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
	dma_ch11_isr,					// 27 DMA channel 11 transfer complete
	dma_ch12_isr,					// 28 DMA channel 12 transfer complete
	dma_ch13_isr,					// 29 DMA channel 13 transfer complete
	dma_ch14_isr,					// 30 DMA channel 14 transfer complete
	dma_ch15_isr,					// 31 DMA channel 15 transfer complete
	dma_error_isr,					// 32 DMA error interrupt channel
	mcm_isr,					// 33 MCM
	flash_cmd_isr,					// 34 Flash Memory Command complete
	flash_error_isr,				// 35 Flash Read collision
	low_voltage_isr,				// 36 Low-voltage detect/warning
	wakeup_isr,					// 37 Low Leakage Wakeup
	watchdog_isr,					// 38 Both EWM and WDOG interrupt
	randnum_isr,					// 39 Random Number Generator
	i2c0_isr,					// 40 I2C0
	i2c1_isr,					// 41 I2C1
	spi0_isr,					// 42 SPI0
	spi1_isr,					// 43 SPI1
	i2s0_tx_isr,					// 44 I2S0 Transmit
	i2s0_rx_isr,					// 45 I2S0 Receive
	unused_isr,					// 46 --
	uart0_status_isr,				// 47 UART0 status
	uart0_error_isr,				// 48 UART0 error
	uart1_status_isr,				// 49 UART1 status
	uart1_error_isr,				// 50 UART1 error
	uart2_status_isr,				// 51 UART2 status
	uart2_error_isr,				// 52 UART2 error
	uart3_status_isr,				// 53 UART3 status
	uart3_error_isr,				// 54 UART3 error
	adc0_isr,					// 55 ADC0
	cmp0_isr,					// 56 CMP0
	cmp1_isr,					// 57 CMP1
	ftm0_isr,					// 58 FTM0
	ftm1_isr,					// 59 FTM1
	ftm2_isr,					// 60 FTM2
	cmt_isr,					// 61 CMT
	rtc_alarm_isr,					// 62 RTC Alarm interrupt
	rtc_seconds_isr,				// 63 RTC Seconds interrupt
	pit0_isr,					// 64 PIT Channel 0
	pit1_isr,					// 65 PIT Channel 1
	pit2_isr,					// 66 PIT Channel 2
	pit3_isr,					// 67 PIT Channel 3
	pdb_isr,					// 68 PDB Programmable Delay Block
	usb_isr,					// 69 USB OTG
	usb_charge_isr,					// 70 USB Charger Detect
	unused_isr,					// 71 --
	dac0_isr,					// 72 DAC0
	mcg_isr,					// 73 MCG
	lptmr_isr,					// 74 Low Power Timer
	porta_isr,					// 75 Pin detect (Port A)
	portb_isr,					// 76 Pin detect (Port B)
	portc_isr,					// 77 Pin detect (Port C)
	portd_isr,					// 78 Pin detect (Port D)
	porte_isr,					// 79 Pin detect (Port E)
	software_isr,					// 80 Software interrupt
	spi2_isr,					// 81 SPI2
	uart4_status_isr,				// 82 UART4 status
	uart4_error_isr,				// 83 UART4 error
	uart5_status_isr,				// 84 UART4 status
	uart5_error_isr,				// 85 UART4 error
	cmp2_isr,					// 86 CMP2
	ftm3_isr,					// 87 FTM3
	dac1_isr,					// 88 DAC1
	adc1_isr,					// 89 ADC1
	i2c2_isr,					// 90 I2C2
	can0_message_isr,				// 91 CAN OR'ed Message buffer (0-15)
	can0_bus_off_isr,				// 92 CAN Bus Off
	can0_error_isr,					// 93 CAN Error
	can0_tx_warn_isr,				// 94 CAN Transmit Warning
	can0_rx_warn_isr,				// 95 CAN Receive Warning
	can0_wakeup_isr,				// 96 CAN Wake Up
	sdhc_isr,					// 97 SDHC
	enet_timer_isr,					// 98 Ethernet IEEE1588 Timers
	enet_tx_isr,					// 99 Ethernet Transmit
	enet_rx_isr,					// 100 Ethernet Receive
	enet_error_isr,					// 101 Ethernet Error
#elif defined(_mk66fx1m0_)
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
	dma_ch11_isr,					// 27 DMA channel 11 transfer complete
	dma_ch12_isr,					// 28 DMA channel 12 transfer complete
	dma_ch13_isr,					// 29 DMA channel 13 transfer complete
	dma_ch14_isr,					// 30 DMA channel 14 transfer complete
	dma_ch15_isr,					// 31 DMA channel 15 transfer complete
	dma_error_isr,					// 32 DMA error interrupt channel
	mcm_isr,					// 33 MCM
	flash_cmd_isr,					// 34 Flash Memory Command complete
	flash_error_isr,				// 35 Flash Read collision
	low_voltage_isr,				// 36 Low-voltage detect/warning
	wakeup_isr,					// 37 Low Leakage Wakeup
	watchdog_isr,					// 38 Both EWM and WDOG interrupt
	randnum_isr,					// 39 Random Number Generator
	i2c0_isr,					// 40 I2C0
	i2c1_isr,					// 41 I2C1
	spi0_isr,					// 42 SPI0
	spi1_isr,					// 43 SPI1
	i2s0_tx_isr,					// 44 I2S0 Transmit
	i2s0_rx_isr,					// 45 I2S0 Receive
	unused_isr,					// 46 --
	uart0_status_isr,				// 47 UART0 status
	uart0_error_isr,				// 48 UART0 error
	uart1_status_isr,				// 49 UART1 status
	uart1_error_isr,				// 50 UART1 error
	uart2_status_isr,				// 51 UART2 status
	uart2_error_isr,				// 52 UART2 error
	uart3_status_isr,				// 53 UART3 status
	uart3_error_isr,				// 54 UART3 error
	adc0_isr,					// 55 ADC0
	cmp0_isr,					// 56 CMP0
	cmp1_isr,					// 57 CMP1
	ftm0_isr,					// 58 FTM0
	ftm1_isr,					// 59 FTM1
	ftm2_isr,					// 60 FTM2
	cmt_isr,					// 61 CMT
	rtc_alarm_isr,					// 62 RTC Alarm interrupt
	rtc_seconds_isr,				// 63 RTC Seconds interrupt
	pit0_isr,					// 64 PIT Channel 0
	pit1_isr,					// 65 PIT Channel 1
	pit2_isr,					// 66 PIT Channel 2
	pit3_isr,					// 67 PIT Channel 3
	pdb_isr,					// 68 PDB Programmable Delay Block
	usb_isr,					// 69 USB OTG
	usb_charge_isr,					// 70 USB Charger Detect
	unused_isr,					// 71 --
	dac0_isr,					// 72 DAC0
	mcg_isr,					// 73 MCG
	lptmr_isr,					// 74 Low Power Timer
	porta_isr,					// 75 Pin detect (Port A)
	portb_isr,					// 76 Pin detect (Port B)
	portc_isr,					// 77 Pin detect (Port C)
	portd_isr,					// 78 Pin detect (Port D)
	porte_isr,					// 79 Pin detect (Port E)
	software_isr,					// 80 Software interrupt
	spi2_isr,					// 81 SPI2
	uart4_status_isr,				// 82 UART4 status
	uart4_error_isr,				// 83 UART4 error
	unused_isr,					// 84 --
	unused_isr,					// 85 --
	cmp2_isr,					// 86 CMP2
	ftm3_isr,					// 87 FTM3
	dac1_isr,					// 88 DAC1
	adc1_isr,					// 89 ADC1
	i2c2_isr,					// 90 I2C2
	can0_message_isr,				// 91 CAN OR'ed Message buffer (0-15)
	can0_bus_off_isr,				// 92 CAN Bus Off
	can0_error_isr,					// 93 CAN Error
	can0_tx_warn_isr,				// 94 CAN Transmit Warning
	can0_rx_warn_isr,				// 95 CAN Receive Warning
	can0_wakeup_isr,				// 96 CAN Wake Up
	sdhc_isr,					// 97 SDHC
	enet_timer_isr,					// 98 Ethernet IEEE1588 Timers
	enet_tx_isr,					// 99 Ethernet Transmit
	enet_rx_isr,					// 100 Ethernet Receive
	enet_error_isr,					// 101 Ethernet Error
	lpuart0_status_isr,				// 102 LPUART
	tsi0_isr,					// 103 TSI0
	tpm1_isr,					// 104 FTM1
	tpm2_isr,					// 105 FTM2
	usbhs_phy_isr,					// 106 USB-HS Phy
	i2c3_isr,					// 107 I2C3
	cmp3_isr,					// 108 CMP3
	usbhs_isr,					// 109 USB-HS
	can1_message_isr,				// 110 CAN OR'ed Message buffer (0-15)
	can1_bus_off_isr,				// 111 CAN Bus Off
	can1_error_isr,					// 112 CAN Error
	can1_tx_warn_isr,				// 113 CAN Transmit Warning
	can1_rx_warn_isr,				// 114 CAN Receive Warning
	can1_wakeup_isr,				// 115 CAN Wake Up
#endif
};


// ----- Flash Configuration -----

// Only necessary for Teensy 3s, MCHCK uses the Bootloader to handle this
#if defined(_teensy_3_)
// Flash Security Setting. On Teensy 3.2, you can lock the MK20 chip to prevent
// anyone from reading your code.  You CAN still reprogram your Teensy while
// security is set, but the bootloader will be unable to respond to auto-reboot
// requests from Arduino. Pressing the program button will cause a full chip
// erase to gain access, because the bootloader chip is locked out.  Normally,
// erase occurs when uploading begins, so if you press the Program button
// accidentally, simply power cycling will run your program again.  When
// security is locked, any Program button press causes immediate full erase.
// Special care must be used with the Program button, because it must be made
// accessible to initiate reprogramming, but it must not be accidentally
// pressed when Teensy Loader is not being used to reprogram.  To set lock the
// security change this to 0xDC.  Teensy 3.0 and 3.1 do not support security lock.
#define FSEC 0xDE

// Flash Options
#define FOPT 0xF9

__attribute__ ((section(".flashconfig"), used))
const uint8_t flashconfigbytes[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, FSEC, FOPT, 0xFF, 0xFF
};
#elif defined(_mk20dx128vlf5_) && defined(_bootloader_)
// XXX Byte labels may be in incorrect positions, double check before modifying
//     FSEC is in correct location -Jacob
__attribute__ ((section(".flashconfig"), used))
const uint8_t flashconfigbytes[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Backdoor Verif Key 28.3.1

	//
	// Protecting the first 4k of Flash memory from being over-written while running (bootloader protection)
	// Still possible to overwrite the bootloader using an external flashing device
	// For more details see:
	//  http://cache.freescale.com/files/training/doc/dwf/AMF_ENT_T1031_Boston.pdf (page 8)
	//  http://cache.freescale.com/files/microcontrollers/doc/app_note/AN4507.pdf
	//  http://cache.freescale.com/files/32bit/doc/ref_manual/K20P48M50SF0RM.pdf (28.34.6)
	//
	// XXX (HaaTa) 8 kB (this has changed from the original 4 kB)
	0xFF, 0xFF, 0xFF, 0xFC, // Program Flash Protection Bytes FPROT0-3

	0xBE, // Flash security byte FSEC
	0x03, // Flash nonvolatile option byte FOPT
	0xFF, // EEPROM Protection Byte FEPROT
	0xFF, // Data Flash Protection Byte FDPROT
};
#elif defined(_mk20dx128vlh7_) && defined(_bootloader_)
// XXX Byte labels may be in incorrect positions, double check before modifying
//     FSEC is in correct location -Jacob
__attribute__ ((section(".flashconfig"), used))
const uint8_t flashconfigbytes[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Backdoor Verif Key 28.3.1

	//
	// Protecting the first 4k of Flash memory from being over-written while running (bootloader protection)
	// Still possible to overwrite the bootloader using an external flashing device
	// For more details see:
	//  http://cache.freescale.com/files/training/doc/dwf/AMF_ENT_T1031_Boston.pdf (page 8)
	//  http://cache.freescale.com/files/microcontrollers/doc/app_note/AN4507.pdf
	//  http://cache.freescale.com/files/32bit/doc/ref_manual/K20P48M50SF0RM.pdf (28.34.6)
	//
	// XXX (HaaTa) 8 kB
	0xFF, 0xFF, 0xFF, 0xFC, // Program Flash Protection Bytes FPROT0-3

	0xBE, // Flash security byte FSEC
	0x03, // Flash nonvolatile option byte FOPT
	0xFF, // EEPROM Protection Byte FEPROT
	0xFF, // Data Flash Protection Byte FDPROT
};
#elif defined(_mk20dx256vlh7_) && defined(_bootloader_)
// XXX Byte labels may be in incorrect positions, double check before modifying
//     FSEC is in correct location -Jacob
__attribute__ ((section(".flashconfig"), used))
const uint8_t flashconfigbytes[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Backdoor Verif Key 28.3.1

	//
	// Protecting the first 8k of Flash memory from being over-written while running (bootloader protection)
	// Still possible to overwrite the bootloader using an external flashing device
	// For more details see:
	//  http://cache.freescale.com/files/training/doc/dwf/AMF_ENT_T1031_Boston.pdf (page 8)
	//  http://cache.freescale.com/files/microcontrollers/doc/app_note/AN4507.pdf
	//  http://cache.freescale.com/files/32bit/doc/ref_manual/K20P64M72SF1RM.pdf (28.34.6)
	//
	// XXX (HaaTa) 8 kB (minimum protected region)
	0xFF, 0xFF, 0xFF, 0xFE, // Program Flash Protection Bytes FPROT0-3

	0xBE, // Flash security byte FSEC
	0x03, // Flash nonvolatile option byte FOPT
	0xFF, // EEPROM Protection Byte FEPROT
	0xFF, // Data Flash Protection Byte FDPROT
};
#elif defined(_mk22fx512avlh12_) && defined(_bootloader_)
// XXX Byte labels may be in incorrect positions, double check before modifying
//     FSEC is in correct location -Jacob
__attribute__ ((section(".flashconfig"), used))
const uint8_t flashconfigbytes[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Backdoor Verif Key 28.3.1

	//
	// Protecting the first 8k of Flash memory from being over-written while running (bootloader protection)
	// Still possible to overwrite the bootloader using an external flashing device
	// For more details see:
	//  http://cache.freescale.com/files/training/doc/dwf/AMF_ENT_T1031_Boston.pdf (page 8)
	//  http://cache.freescale.com/files/microcontrollers/doc/app_note/AN4507.pdf
	//  http://cache.freescale.com/files/32bit/doc/ref_manual/K20P64M72SF1RM.pdf (28.34.6)
	//
	// XXX (HaaTa) 16 kB (minimum protected region)
	0xFF, 0xFF, 0xFF, 0xFE, // Program Flash Protection Bytes FPROT0-3

	0xBE, // Flash security byte FSEC
	0x03, // Flash nonvolatile option byte FOPT
	0xFF, // EEPROM Protection Byte FEPROT
	0xFF, // Data Flash Protection Byte FDPROT
};
#endif



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

__attribute__ ((section(".startup")))
void ResetHandler()
{
	// Disable Watchdog
	while ( WDOG_TMROUTL < 2 ); // Must wait for WDOG timer if already running, before jumping
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN;

	uint32_t *src = (uint32_t*)&_etext;
	uint32_t *dest = (uint32_t*)&_sdata;

	// Enable clocks to always-used peripherals
	SIM_SCGC5 = 0x00043F82; // Clocks active to all GPIO
	SIM_SCGC6 = SIM_SCGC6_FTM0 | SIM_SCGC6_FTM1 | SIM_SCGC6_ADC0 | SIM_SCGC6_FTFL;
#if defined(_teensy_3_0_)
	SIM_SCGC6 |= SIM_SCGC6_RTC;
#elif defined(_kii_v2_)
	SIM_SCGC3 = SIM_SCGC3_ADC1 | SIM_SCGC3_FTM2;
	SIM_SCGC6 |= SIM_SCGC6_RTC;
#elif defined(_teensy_3_5__3_6_)
	SIM_SCGC3 = SIM_SCGC3_ADC1 | SIM_SCGC3_FTM2 | SIM_SCGC3_FTM3;
	SIM_SCGC6 |= SIM_SCGC6_RTC;
#endif

	// Enable FPU
#if defined(_kinetis_fpu_)
	SCB_CPACR = 0x00F00000;
#endif

	// Invalidate cache XXX (HaaTa) I think that's what this is?
#if defined(_mk66fx1m0_)
	LMEM_PCCCR = 0x85000003;
#endif

#if defined(_teensy_3_) // Teensy 3s
	// if the RTC oscillator isn't enabled, get it started early
	if ( !(RTC_CR & RTC_CR_OSCE) )
	{
		RTC_SR = 0;
		RTC_CR = RTC_CR_SC16P | RTC_CR_SC4P | RTC_CR_OSCE;
	}
#endif

	// release I/O pins hold, if we woke up from VLLS mode
	if ( PMC_REGSC & PMC_REGSC_ACKISO )
	{
		PMC_REGSC |= PMC_REGSC_ACKISO;
	}

	// Since this is a write once register, make it visible to all F_CPU's
	// so we can enter into other sleep modes in the future at any speed
#if defined(_mk66fx1m0_)
	SMC_PMPROT = SMC_PMPROT_AHSRUN | SMC_PMPROT_AVLP | SMC_PMPROT_ALLS | SMC_PMPROT_AVLLS;
#else
	SMC_PMPROT = SMC_PMPROT_AVLP | SMC_PMPROT_ALLS | SMC_PMPROT_AVLLS;
#endif
	// Prepare RAM
	while ( dest < (uint32_t*)&_edata ) *dest++ = *src++;
	dest = (uint32_t*)&_sbss;
	while ( dest < (uint32_t*)&_ebss ) *dest++ = 0;

// MCHCK / Kiibohd-dfu
#if defined(_kii_v1_)
	// Default all interrupts to medium priority level
	for ( unsigned int i = 0; i < NVIC_NUM_INTERRUPTS; i++ )
	{
		NVIC_SET_PRIORITY( i, 128 );
	}

	// FLL at 48MHz
	MCG_C4 = MCG_C4_DMX32 | MCG_C4_DRST_DRS( 1 );

	// USB Clock and FLL select
	SIM_SOPT2 = SIM_SOPT2_USBSRC | SIM_SOPT2_TRACECLKSEL;

// Teensy 3.0 and 3.1 and Kiibohd-dfu (mk20dx256vlh7)
#else
#if defined(_teensy_3_)
	// use vector table in flash
	SCB_VTOR = 0;
#endif

	// default all interrupts to medium priority level
	for ( unsigned int i = 0; i < NVIC_NUM_INTERRUPTS; i++ )
	{
		NVIC_SET_PRIORITY( i, 128 );
	}

	// start in FEI mode
	// enable capacitors for crystal
	OSC0_CR = OSC_SC8P | OSC_SC2P;

	// enable osc, 8-32 MHz range, low power mode
	MCG_C2 = MCG_C2_RANGE0( 2 ) | MCG_C2_EREFS;

	// switch to crystal as clock source, FLL input = 16 MHz / 512
	MCG_C1 =  MCG_C1_CLKS( 2 ) | MCG_C1_FRDIV( 4 );

	// wait for crystal oscillator to begin
	while ( (MCG_S & MCG_S_OSCINIT0) == 0 );

	// wait for FLL to use oscillator
	while ( (MCG_S & MCG_S_IREFST) != 0 );

	// wait for MCGOUT to use oscillator
	while ( (MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST( 2 ) );

	// now we're in FBE mode
#if F_CPU == 72000000
	// config PLL input for 16 MHz Crystal / 8 = 2 MHz
	MCG_C5 = MCG_C5_PRDIV0( 7 );
#else
	// config PLL input for 16 MHz Crystal / 4 = 4 MHz
	MCG_C5 = MCG_C5_PRDIV0( 3 );
#endif

#if F_CPU == 72000000
	// config PLL for 72 MHz output (36 * 2 MHz Ext PLL)
	MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0( 12 );
#else
	// config PLL for 96 MHz output
	MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0( 0 );
#endif

	// wait for PLL to start using xtal as its input
	while ( !(MCG_S & MCG_S_PLLST) );

	// wait for PLL to lock
	while ( !(MCG_S & MCG_S_LOCK0) );

	// now we're in PBE mode
#if F_CPU == 96000000
	// config divisors: 96 MHz core, 48 MHz bus, 24 MHz flash
	SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1( 0 ) | SIM_CLKDIV1_OUTDIV2( 1 ) | SIM_CLKDIV1_OUTDIV4( 3 );
#elif F_CPU == 72000000
	// config divisors: 72 MHz core, 36 MHz bus, 24 MHz flash
	SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1( 0 ) | SIM_CLKDIV1_OUTDIV2( 1 ) | SIM_CLKDIV1_OUTDIV4( 2 );
#elif F_CPU == 48000000
	// config divisors: 48 MHz core, 48 MHz bus, 24 MHz flash
	SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1( 1 ) | SIM_CLKDIV1_OUTDIV2( 1 ) | SIM_CLKDIV1_OUTDIV4( 3 );
#elif F_CPU == 24000000
	// config divisors: 24 MHz core, 24 MHz bus, 24 MHz flash
	SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1( 3 ) | SIM_CLKDIV1_OUTDIV2( 3 ) | SIM_CLKDIV1_OUTDIV4( 3 );
#else
#error "Error, F_CPU must be 96000000, 72000000, 48000000, or 24000000"
#endif
	// switch to PLL as clock source, FLL input = 16 MHz / 512
	MCG_C1 = MCG_C1_CLKS( 0 ) | MCG_C1_FRDIV( 4 );

	// wait for PLL clock to be used
	while ( (MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST( 3 ) );

	// now we're in PEE mode
#if F_CPU == 72000000
	// configure USB for 48 MHz clock
	SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV( 2 ) | SIM_CLKDIV2_USBFRAC; // USB = 72 MHz PLL / 1.5
#else
	// configure USB for 48 MHz clock
	SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV( 1 ); // USB = 96 MHz PLL / 2
#endif

	// USB uses PLL clock, trace is CPU clock, CLKOUT=OSCERCLK0
	SIM_SOPT2 = SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_TRACECLKSEL | SIM_SOPT2_CLKOUTSEL( 6 );

#endif

	// Initialize the SysTick counter
	SYST_RVR = (F_CPU / 1000) - 1;
	SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;

	// Enable IRQs
	__enable_irq();

	// Intialize entropy for random numbers
	rand_initialize();

	// Start main
	main();
	while ( 1 ); // Shouldn't get here...
}



// ----- RAM Setup -----

char *__brkval = (char *)&_ebss;

void * _sbrk( int incr )
{
	char *prev = __brkval;
	__brkval += incr;
	return prev;
}



// ----- Interrupt Execution Priority -----

int nvic_execution_priority()
{
	int priority = 256;
	uint32_t primask, faultmask, basepri, ipsr;

	// full algorithm in ARM DDI0403D, page B1-639
	// this isn't quite complete, but hopefully good enough
	asm volatile( "mrs %0, faultmask\n" : "=r" (faultmask):: );
	if ( faultmask )
	{
		return -1;
	}

	asm volatile( "mrs %0, primask\n" : "=r" (primask):: );
	if ( primask )
	{
		return 0;
	}

	asm volatile( "mrs %0, ipsr\n" : "=r" (ipsr):: );
	if ( ipsr )
	{
		if ( ipsr < 16)
		{
			priority = 0; // could be non-zero
		}
		else
		{
			priority = NVIC_GET_PRIORITY( ipsr - 16 );
		}
	}

	asm volatile( "mrs %0, basepri\n" : "=r" (basepri):: );
	if ( basepri > 0 && basepri < priority )
	{
		priority = basepri;
	}

	return priority;
}

