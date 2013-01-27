#include "mk20dx128.h"


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
        while (1); // die
}

void unused_isr(void)
{
        while (1); // die
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
void dma_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void flash_cmd_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void flash_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void low_voltage_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void wakeup_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void watchdog_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void i2c0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void spi0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void i2s0_tx_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void i2s0_rx_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void uart0_lon_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart0_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart0_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart1_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart1_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart2_status_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void uart2_error_isr(void)	__attribute__ ((weak, alias("unused_isr")));
void adc0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void cmp0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void cmp1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void ftm0_isr(void)		__attribute__ ((weak, alias("unused_isr")));
void ftm1_isr(void)		__attribute__ ((weak, alias("unused_isr")));
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
#define TIME_T 1349049600 // default 1 Oct 2012
#endif
extern void rtc_set(unsigned long t);



void startup_unused_hook(void) {}
void startup_early_hook(void)		__attribute__ ((weak, alias("startup_unused_hook")));
void startup_late_hook(void)		__attribute__ ((weak, alias("startup_unused_hook")));


__attribute__ ((section(".startup")))
void ResetHandler(void)
{
        uint32_t *src = &_etext;
        uint32_t *dest = &_sdata;

	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE;
	startup_early_hook();

	// enable clocks to always-used peripherals
	SIM_SCGC5 = 0x00043F82;		// clocks active to all GPIO
	SIM_SCGC6 = SIM_SCGC6_RTC | SIM_SCGC6_FTM0 | SIM_SCGC6_FTM1 | SIM_SCGC6_ADC0 | SIM_SCGC6_FTFL;
	// if the RTC oscillator isn't enabled, get it started early
	if (!(RTC_CR & RTC_CR_OSCE)) {
		RTC_SR = 0;
		RTC_CR = RTC_CR_SC16P | RTC_CR_SC4P | RTC_CR_OSCE;
	}

	// TODO: do this while the PLL is waiting to lock....
        while (dest < &_edata) *dest++ = *src++;
        dest = &_sbss;
        while (dest < &_ebss) *dest++ = 0;
	SCB_VTOR = 0;	// use vector table in flash

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
        SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) |  SIM_CLKDIV1_OUTDIV4(3);
#elif F_CPU == 48000000
        // config divisors: 48 MHz core, 48 MHz bus, 24 MHz flash
        SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1) | SIM_CLKDIV1_OUTDIV2(1) |  SIM_CLKDIV1_OUTDIV4(3);
#elif F_CPU == 24000000
        // config divisors: 24 MHz core, 24 MHz bus, 24 MHz flash
        SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(3) | SIM_CLKDIV1_OUTDIV2(3) |  SIM_CLKDIV1_OUTDIV4(3);
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
        main();
        while (1) ;
}

// TODO: is this needed for c++ and where does it come from?
/*
void _init(void)
{
}
*/


void * _sbrk(int incr)
{
        static char *heap_end = (char *)&_ebss;
	char *prev = heap_end;

	heap_end += incr;
	return prev;
}

int _read(int file, char *ptr, int len)
{
	return 0;
}

int _write(int file, char *ptr, int len)
{
	return 0;
}

int _close(int fd)
{
	return -1;
}

int _lseek(int fd, long long offset, int whence)
{
	return -1;
}

void _exit(int status)
{
	while (1);
}

void __cxa_pure_virtual()
{
	while (1);
}



