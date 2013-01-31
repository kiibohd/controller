#ifndef _mk20dx128_h_
#define _mk20dx128_h_

//#define F_CPU 96000000
//#define F_CPU 48000000
//#define F_CPU 24000000
//#define F_BUS 48000000
//#define F_BUS 24000000
//#define F_MEM 24000000

#if (F_CPU == 96000000)
 #define F_BUS 48000000
 #define F_MEM 24000000
#elif (F_CPU == 48000000)
 #define F_BUS 48000000
 #define F_MEM 24000000
#elif (F_CPU == 24000000)
 #define F_BUS 24000000
 #define F_MEM 24000000
#endif


#ifndef NULL
#define NULL ((void *)0)
#endif

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

// chapter 11: Port control and interrupts (PORT)
#define PORTA_PCR0              *(volatile uint32_t *)0x40049000 // Pin Control Register n
#define PORT_PCR_ISF			(uint32_t)0x01000000		// Interrupt Status Flag
#define PORT_PCR_IRQC(n)		(uint32_t)(((n) & 15) << 16)	// Interrupt Configuration
#define PORT_PCR_IRQC_MASK		(uint32_t)0x000F0000
#define PORT_PCR_LK			(uint32_t)0x00008000		// Lock Register
#define PORT_PCR_MUX(n)			(uint32_t)(((n) & 7) << 8)	// Pin Mux Control
#define PORT_PCR_MUX_MASK		(uint32_t)0x00000700
#define PORT_PCR_DSE			(uint32_t)0x00000040		// Drive Strength Enable
#define PORT_PCR_ODE			(uint32_t)0x00000020		// Open Drain Enable
#define PORT_PCR_PFE			(uint32_t)0x00000010		// Passive Filter Enable
#define PORT_PCR_SRE			(uint32_t)0x00000004		// Slew Rate Enable
#define PORT_PCR_PE			(uint32_t)0x00000002		// Pull Enable
#define PORT_PCR_PS			(uint32_t)0x00000001		// Pull Select
#define PORTA_PCR1              *(volatile uint32_t *)0x40049004 // Pin Control Register n
#define PORTA_PCR2              *(volatile uint32_t *)0x40049008 // Pin Control Register n
#define PORTA_PCR3              *(volatile uint32_t *)0x4004900C // Pin Control Register n
#define PORTA_PCR4              *(volatile uint32_t *)0x40049010 // Pin Control Register n
#define PORTA_PCR5              *(volatile uint32_t *)0x40049014 // Pin Control Register n
#define PORTA_PCR6              *(volatile uint32_t *)0x40049018 // Pin Control Register n
#define PORTA_PCR7              *(volatile uint32_t *)0x4004901C // Pin Control Register n
#define PORTA_PCR8              *(volatile uint32_t *)0x40049020 // Pin Control Register n
#define PORTA_PCR9              *(volatile uint32_t *)0x40049024 // Pin Control Register n
#define PORTA_PCR10             *(volatile uint32_t *)0x40049028 // Pin Control Register n
#define PORTA_PCR11             *(volatile uint32_t *)0x4004902C // Pin Control Register n
#define PORTA_PCR12             *(volatile uint32_t *)0x40049030 // Pin Control Register n
#define PORTA_PCR13             *(volatile uint32_t *)0x40049034 // Pin Control Register n
#define PORTA_PCR14             *(volatile uint32_t *)0x40049038 // Pin Control Register n
#define PORTA_PCR15             *(volatile uint32_t *)0x4004903C // Pin Control Register n
#define PORTA_PCR16             *(volatile uint32_t *)0x40049040 // Pin Control Register n
#define PORTA_PCR17             *(volatile uint32_t *)0x40049044 // Pin Control Register n
#define PORTA_PCR18             *(volatile uint32_t *)0x40049048 // Pin Control Register n
#define PORTA_PCR19             *(volatile uint32_t *)0x4004904C // Pin Control Register n
#define PORTA_PCR20             *(volatile uint32_t *)0x40049050 // Pin Control Register n
#define PORTA_PCR21             *(volatile uint32_t *)0x40049054 // Pin Control Register n
#define PORTA_PCR22             *(volatile uint32_t *)0x40049058 // Pin Control Register n
#define PORTA_PCR23             *(volatile uint32_t *)0x4004905C // Pin Control Register n
#define PORTA_PCR24             *(volatile uint32_t *)0x40049060 // Pin Control Register n
#define PORTA_PCR25             *(volatile uint32_t *)0x40049064 // Pin Control Register n
#define PORTA_PCR26             *(volatile uint32_t *)0x40049068 // Pin Control Register n
#define PORTA_PCR27             *(volatile uint32_t *)0x4004906C // Pin Control Register n
#define PORTA_PCR28             *(volatile uint32_t *)0x40049070 // Pin Control Register n
#define PORTA_PCR29             *(volatile uint32_t *)0x40049074 // Pin Control Register n
#define PORTA_PCR30             *(volatile uint32_t *)0x40049078 // Pin Control Register n
#define PORTA_PCR31             *(volatile uint32_t *)0x4004907C // Pin Control Register n
#define PORTA_GPCLR             *(volatile uint32_t *)0x40049080 // Global Pin Control Low Register
#define PORTA_GPCHR             *(volatile uint32_t *)0x40049084 // Global Pin Control High Register
#define PORTA_ISFR              *(volatile uint32_t *)0x400490A0 // Interrupt Status Flag Register
#define PORTB_PCR0              *(volatile uint32_t *)0x4004A000 // Pin Control Register n
#define PORTB_PCR1              *(volatile uint32_t *)0x4004A004 // Pin Control Register n
#define PORTB_PCR2              *(volatile uint32_t *)0x4004A008 // Pin Control Register n
#define PORTB_PCR3              *(volatile uint32_t *)0x4004A00C // Pin Control Register n
#define PORTB_PCR4              *(volatile uint32_t *)0x4004A010 // Pin Control Register n
#define PORTB_PCR5              *(volatile uint32_t *)0x4004A014 // Pin Control Register n
#define PORTB_PCR6              *(volatile uint32_t *)0x4004A018 // Pin Control Register n
#define PORTB_PCR7              *(volatile uint32_t *)0x4004A01C // Pin Control Register n
#define PORTB_PCR8              *(volatile uint32_t *)0x4004A020 // Pin Control Register n
#define PORTB_PCR9              *(volatile uint32_t *)0x4004A024 // Pin Control Register n
#define PORTB_PCR10             *(volatile uint32_t *)0x4004A028 // Pin Control Register n
#define PORTB_PCR11             *(volatile uint32_t *)0x4004A02C // Pin Control Register n
#define PORTB_PCR12             *(volatile uint32_t *)0x4004A030 // Pin Control Register n
#define PORTB_PCR13             *(volatile uint32_t *)0x4004A034 // Pin Control Register n
#define PORTB_PCR14             *(volatile uint32_t *)0x4004A038 // Pin Control Register n
#define PORTB_PCR15             *(volatile uint32_t *)0x4004A03C // Pin Control Register n
#define PORTB_PCR16             *(volatile uint32_t *)0x4004A040 // Pin Control Register n
#define PORTB_PCR17             *(volatile uint32_t *)0x4004A044 // Pin Control Register n
#define PORTB_PCR18             *(volatile uint32_t *)0x4004A048 // Pin Control Register n
#define PORTB_PCR19             *(volatile uint32_t *)0x4004A04C // Pin Control Register n
#define PORTB_PCR20             *(volatile uint32_t *)0x4004A050 // Pin Control Register n
#define PORTB_PCR21             *(volatile uint32_t *)0x4004A054 // Pin Control Register n
#define PORTB_PCR22             *(volatile uint32_t *)0x4004A058 // Pin Control Register n
#define PORTB_PCR23             *(volatile uint32_t *)0x4004A05C // Pin Control Register n
#define PORTB_PCR24             *(volatile uint32_t *)0x4004A060 // Pin Control Register n
#define PORTB_PCR25             *(volatile uint32_t *)0x4004A064 // Pin Control Register n
#define PORTB_PCR26             *(volatile uint32_t *)0x4004A068 // Pin Control Register n
#define PORTB_PCR27             *(volatile uint32_t *)0x4004A06C // Pin Control Register n
#define PORTB_PCR28             *(volatile uint32_t *)0x4004A070 // Pin Control Register n
#define PORTB_PCR29             *(volatile uint32_t *)0x4004A074 // Pin Control Register n
#define PORTB_PCR30             *(volatile uint32_t *)0x4004A078 // Pin Control Register n
#define PORTB_PCR31             *(volatile uint32_t *)0x4004A07C // Pin Control Register n
#define PORTB_GPCLR             *(volatile uint32_t *)0x4004A080 // Global Pin Control Low Register
#define PORTB_GPCHR             *(volatile uint32_t *)0x4004A084 // Global Pin Control High Register
#define PORTB_ISFR              *(volatile uint32_t *)0x4004A0A0 // Interrupt Status Flag Register
#define PORTC_PCR0              *(volatile uint32_t *)0x4004B000 // Pin Control Register n
#define PORTC_PCR1              *(volatile uint32_t *)0x4004B004 // Pin Control Register n
#define PORTC_PCR2              *(volatile uint32_t *)0x4004B008 // Pin Control Register n
#define PORTC_PCR3              *(volatile uint32_t *)0x4004B00C // Pin Control Register n
#define PORTC_PCR4              *(volatile uint32_t *)0x4004B010 // Pin Control Register n
#define PORTC_PCR5              *(volatile uint32_t *)0x4004B014 // Pin Control Register n
#define PORTC_PCR6              *(volatile uint32_t *)0x4004B018 // Pin Control Register n
#define PORTC_PCR7              *(volatile uint32_t *)0x4004B01C // Pin Control Register n
#define PORTC_PCR8              *(volatile uint32_t *)0x4004B020 // Pin Control Register n
#define PORTC_PCR9              *(volatile uint32_t *)0x4004B024 // Pin Control Register n
#define PORTC_PCR10             *(volatile uint32_t *)0x4004B028 // Pin Control Register n
#define PORTC_PCR11             *(volatile uint32_t *)0x4004B02C // Pin Control Register n
#define PORTC_PCR12             *(volatile uint32_t *)0x4004B030 // Pin Control Register n
#define PORTC_PCR13             *(volatile uint32_t *)0x4004B034 // Pin Control Register n
#define PORTC_PCR14             *(volatile uint32_t *)0x4004B038 // Pin Control Register n
#define PORTC_PCR15             *(volatile uint32_t *)0x4004B03C // Pin Control Register n
#define PORTC_PCR16             *(volatile uint32_t *)0x4004B040 // Pin Control Register n
#define PORTC_PCR17             *(volatile uint32_t *)0x4004B044 // Pin Control Register n
#define PORTC_PCR18             *(volatile uint32_t *)0x4004B048 // Pin Control Register n
#define PORTC_PCR19             *(volatile uint32_t *)0x4004B04C // Pin Control Register n
#define PORTC_PCR20             *(volatile uint32_t *)0x4004B050 // Pin Control Register n
#define PORTC_PCR21             *(volatile uint32_t *)0x4004B054 // Pin Control Register n
#define PORTC_PCR22             *(volatile uint32_t *)0x4004B058 // Pin Control Register n
#define PORTC_PCR23             *(volatile uint32_t *)0x4004B05C // Pin Control Register n
#define PORTC_PCR24             *(volatile uint32_t *)0x4004B060 // Pin Control Register n
#define PORTC_PCR25             *(volatile uint32_t *)0x4004B064 // Pin Control Register n
#define PORTC_PCR26             *(volatile uint32_t *)0x4004B068 // Pin Control Register n
#define PORTC_PCR27             *(volatile uint32_t *)0x4004B06C // Pin Control Register n
#define PORTC_PCR28             *(volatile uint32_t *)0x4004B070 // Pin Control Register n
#define PORTC_PCR29             *(volatile uint32_t *)0x4004B074 // Pin Control Register n
#define PORTC_PCR30             *(volatile uint32_t *)0x4004B078 // Pin Control Register n
#define PORTC_PCR31             *(volatile uint32_t *)0x4004B07C // Pin Control Register n
#define PORTC_GPCLR             *(volatile uint32_t *)0x4004B080 // Global Pin Control Low Register
#define PORTC_GPCHR             *(volatile uint32_t *)0x4004B084 // Global Pin Control High Register
#define PORTC_ISFR              *(volatile uint32_t *)0x4004B0A0 // Interrupt Status Flag Register
#define PORTD_PCR0              *(volatile uint32_t *)0x4004C000 // Pin Control Register n
#define PORTD_PCR1              *(volatile uint32_t *)0x4004C004 // Pin Control Register n
#define PORTD_PCR2              *(volatile uint32_t *)0x4004C008 // Pin Control Register n
#define PORTD_PCR3              *(volatile uint32_t *)0x4004C00C // Pin Control Register n
#define PORTD_PCR4              *(volatile uint32_t *)0x4004C010 // Pin Control Register n
#define PORTD_PCR5              *(volatile uint32_t *)0x4004C014 // Pin Control Register n
#define PORTD_PCR6              *(volatile uint32_t *)0x4004C018 // Pin Control Register n
#define PORTD_PCR7              *(volatile uint32_t *)0x4004C01C // Pin Control Register n
#define PORTD_PCR8              *(volatile uint32_t *)0x4004C020 // Pin Control Register n
#define PORTD_PCR9              *(volatile uint32_t *)0x4004C024 // Pin Control Register n
#define PORTD_PCR10             *(volatile uint32_t *)0x4004C028 // Pin Control Register n
#define PORTD_PCR11             *(volatile uint32_t *)0x4004C02C // Pin Control Register n
#define PORTD_PCR12             *(volatile uint32_t *)0x4004C030 // Pin Control Register n
#define PORTD_PCR13             *(volatile uint32_t *)0x4004C034 // Pin Control Register n
#define PORTD_PCR14             *(volatile uint32_t *)0x4004C038 // Pin Control Register n
#define PORTD_PCR15             *(volatile uint32_t *)0x4004C03C // Pin Control Register n
#define PORTD_PCR16             *(volatile uint32_t *)0x4004C040 // Pin Control Register n
#define PORTD_PCR17             *(volatile uint32_t *)0x4004C044 // Pin Control Register n
#define PORTD_PCR18             *(volatile uint32_t *)0x4004C048 // Pin Control Register n
#define PORTD_PCR19             *(volatile uint32_t *)0x4004C04C // Pin Control Register n
#define PORTD_PCR20             *(volatile uint32_t *)0x4004C050 // Pin Control Register n
#define PORTD_PCR21             *(volatile uint32_t *)0x4004C054 // Pin Control Register n
#define PORTD_PCR22             *(volatile uint32_t *)0x4004C058 // Pin Control Register n
#define PORTD_PCR23             *(volatile uint32_t *)0x4004C05C // Pin Control Register n
#define PORTD_PCR24             *(volatile uint32_t *)0x4004C060 // Pin Control Register n
#define PORTD_PCR25             *(volatile uint32_t *)0x4004C064 // Pin Control Register n
#define PORTD_PCR26             *(volatile uint32_t *)0x4004C068 // Pin Control Register n
#define PORTD_PCR27             *(volatile uint32_t *)0x4004C06C // Pin Control Register n
#define PORTD_PCR28             *(volatile uint32_t *)0x4004C070 // Pin Control Register n
#define PORTD_PCR29             *(volatile uint32_t *)0x4004C074 // Pin Control Register n
#define PORTD_PCR30             *(volatile uint32_t *)0x4004C078 // Pin Control Register n
#define PORTD_PCR31             *(volatile uint32_t *)0x4004C07C // Pin Control Register n
#define PORTD_GPCLR             *(volatile uint32_t *)0x4004C080 // Global Pin Control Low Register
#define PORTD_GPCHR             *(volatile uint32_t *)0x4004C084 // Global Pin Control High Register
#define PORTD_ISFR              *(volatile uint32_t *)0x4004C0A0 // Interrupt Status Flag Register
#define PORTE_PCR0              *(volatile uint32_t *)0x4004D000 // Pin Control Register n
#define PORTE_PCR1              *(volatile uint32_t *)0x4004D004 // Pin Control Register n
#define PORTE_PCR2              *(volatile uint32_t *)0x4004D008 // Pin Control Register n
#define PORTE_PCR3              *(volatile uint32_t *)0x4004D00C // Pin Control Register n
#define PORTE_PCR4              *(volatile uint32_t *)0x4004D010 // Pin Control Register n
#define PORTE_PCR5              *(volatile uint32_t *)0x4004D014 // Pin Control Register n
#define PORTE_PCR6              *(volatile uint32_t *)0x4004D018 // Pin Control Register n
#define PORTE_PCR7              *(volatile uint32_t *)0x4004D01C // Pin Control Register n
#define PORTE_PCR8              *(volatile uint32_t *)0x4004D020 // Pin Control Register n
#define PORTE_PCR9              *(volatile uint32_t *)0x4004D024 // Pin Control Register n
#define PORTE_PCR10             *(volatile uint32_t *)0x4004D028 // Pin Control Register n
#define PORTE_PCR11             *(volatile uint32_t *)0x4004D02C // Pin Control Register n
#define PORTE_PCR12             *(volatile uint32_t *)0x4004D030 // Pin Control Register n
#define PORTE_PCR13             *(volatile uint32_t *)0x4004D034 // Pin Control Register n
#define PORTE_PCR14             *(volatile uint32_t *)0x4004D038 // Pin Control Register n
#define PORTE_PCR15             *(volatile uint32_t *)0x4004D03C // Pin Control Register n
#define PORTE_PCR16             *(volatile uint32_t *)0x4004D040 // Pin Control Register n
#define PORTE_PCR17             *(volatile uint32_t *)0x4004D044 // Pin Control Register n
#define PORTE_PCR18             *(volatile uint32_t *)0x4004D048 // Pin Control Register n
#define PORTE_PCR19             *(volatile uint32_t *)0x4004D04C // Pin Control Register n
#define PORTE_PCR20             *(volatile uint32_t *)0x4004D050 // Pin Control Register n
#define PORTE_PCR21             *(volatile uint32_t *)0x4004D054 // Pin Control Register n
#define PORTE_PCR22             *(volatile uint32_t *)0x4004D058 // Pin Control Register n
#define PORTE_PCR23             *(volatile uint32_t *)0x4004D05C // Pin Control Register n
#define PORTE_PCR24             *(volatile uint32_t *)0x4004D060 // Pin Control Register n
#define PORTE_PCR25             *(volatile uint32_t *)0x4004D064 // Pin Control Register n
#define PORTE_PCR26             *(volatile uint32_t *)0x4004D068 // Pin Control Register n
#define PORTE_PCR27             *(volatile uint32_t *)0x4004D06C // Pin Control Register n
#define PORTE_PCR28             *(volatile uint32_t *)0x4004D070 // Pin Control Register n
#define PORTE_PCR29             *(volatile uint32_t *)0x4004D074 // Pin Control Register n
#define PORTE_PCR30             *(volatile uint32_t *)0x4004D078 // Pin Control Register n
#define PORTE_PCR31             *(volatile uint32_t *)0x4004D07C // Pin Control Register n
#define PORTE_GPCLR             *(volatile uint32_t *)0x4004D080 // Global Pin Control Low Register
#define PORTE_GPCHR             *(volatile uint32_t *)0x4004D084 // Global Pin Control High Register
#define PORTE_ISFR              *(volatile uint32_t *)0x4004D0A0 // Interrupt Status Flag Register

// Chapter 12: System Integration Module (SIM)
#define SIM_SOPT1               *(volatile uint32_t *)0x40047000 // System Options Register 1
#define SIM_SOPT1CFG            *(volatile uint32_t *)0x40047004 // SOPT1 Configuration Register
#define SIM_SOPT2               *(volatile uint32_t *)0x40048004 // System Options Register 2
#define SIM_SOPT2_USBSRC		(uint32_t)0x00040000		// 0=USB_CLKIN, 1=FFL/PLL 
#define SIM_SOPT2_PLLFLLSEL		(uint32_t)0x00010000		// 0=FLL, 1=PLL
#define SIM_SOPT2_TRACECLKSEL		(uint32_t)0x00001000		// 0=MCGOUTCLK, 1=CPU
#define SIM_SOPT2_PTD7PAD		(uint32_t)0x00000800		// 0=normal, 1=double drive PTD7
#define SIM_SOPT2_CLKOUTSEL(n)		(uint32_t)(((n) & 7) << 5)	// Selects the clock to output on the CLKOUT pin.
#define SIM_SOPT2_RTCCLKOUTSEL		(uint32_t)0x00000010		// RTC clock out select
#define SIM_SOPT4               *(volatile uint32_t *)0x4004800C // System Options Register 4
#define SIM_SOPT5               *(volatile uint32_t *)0x40048010 // System Options Register 5
#define SIM_SOPT7               *(volatile uint32_t *)0x40048018 // System Options Register 7
#define SIM_SDID                *(const    uint32_t *)0x40048024 // System Device Identification Register
#define SIM_SCGC4               *(volatile uint32_t *)0x40048034 // System Clock Gating Control Register 4
#define SIM_SCGC4_VREF			(uint32_t)0x00100000		// VREF Clock Gate Control
#define SIM_SCGC4_CMP			(uint32_t)0x00080000		// Comparator Clock Gate Control
#define SIM_SCGC4_USBOTG		(uint32_t)0x00040000		// USB Clock Gate Control
#define SIM_SCGC4_UART2			(uint32_t)0x00001000		// UART2 Clock Gate Control
#define SIM_SCGC4_UART1			(uint32_t)0x00000800		// UART1 Clock Gate Control
#define SIM_SCGC4_UART0			(uint32_t)0x00000400		// UART0 Clock Gate Control
#define SIM_SCGC4_I2C0			(uint32_t)0x00000040		// I2C0 Clock Gate Control
#define SIM_SCGC4_CMT			(uint32_t)0x00000004		// CMT Clock Gate Control
#define SIM_SCGC4_EWM			(uint32_t)0x00000002		// EWM Clock Gate Control
#define SIM_SCGC5               *(volatile uint32_t *)0x40048038 // System Clock Gating Control Register 5
#define SIM_SCGC5_PORTE			(uint32_t)0x00002000		// Port E Clock Gate Control
#define SIM_SCGC5_PORTD			(uint32_t)0x00001000		// Port D Clock Gate Control
#define SIM_SCGC5_PORTC			(uint32_t)0x00000800		// Port C Clock Gate Control
#define SIM_SCGC5_PORTB			(uint32_t)0x00000400		// Port B Clock Gate Control
#define SIM_SCGC5_PORTA			(uint32_t)0x00000200		// Port A Clock Gate Control
#define SIM_SCGC5_TSI			(uint32_t)0x00000020		// Touch Sense Input TSI Clock Gate Control
#define SIM_SCGC5_LPTIMER		(uint32_t)0x00000001		// Low Power Timer Access Control
#define SIM_SCGC6               *(volatile uint32_t *)0x4004803C // System Clock Gating Control Register 6
#define SIM_SCGC6_RTC			(uint32_t)0x20000000		// RTC Access
#define SIM_SCGC6_ADC0			(uint32_t)0x08000000		// ADC0 Clock Gate Control
#define SIM_SCGC6_FTM1			(uint32_t)0x02000000		// FTM1 Clock Gate Control
#define SIM_SCGC6_FTM0			(uint32_t)0x01000000		// FTM0 Clock Gate Control
#define SIM_SCGC6_PIT			(uint32_t)0x00800000		// PIT Clock Gate Control
#define SIM_SCGC6_PDB			(uint32_t)0x00400000		// PDB Clock Gate Control
#define SIM_SCGC6_USBDCD		(uint32_t)0x00200000		// USB DCD Clock Gate Control
#define SIM_SCGC6_CRC			(uint32_t)0x00040000		// CRC Clock Gate Control
#define SIM_SCGC6_I2S			(uint32_t)0x00008000		// I2S Clock Gate Control
#define SIM_SCGC6_SPI0			(uint32_t)0x00001000		// SPI0 Clock Gate Control
#define SIM_SCGC6_DMAMUX		(uint32_t)0x00000002		// DMA Mux Clock Gate Control
#define SIM_SCGC6_FTFL			(uint32_t)0x00000001		// Flash Memory Clock Gate Control
#define SIM_SCGC7               *(volatile uint32_t *)0x40048040 // System Clock Gating Control Register 7
#define SIM_CLKDIV1             *(volatile uint32_t *)0x40048044 // System Clock Divider Register 1
#define SIM_CLKDIV1_OUTDIV1(n)		(uint32_t)(((n) & 0x0F) << 28)	// divide value for the core/system clock
#define SIM_CLKDIV1_OUTDIV2(n)		(uint32_t)(((n) & 0x0F) << 24)	// divide value for the peripheral clock
#define SIM_CLKDIV1_OUTDIV4(n)		(uint32_t)(((n) & 0x0F) << 16)	// divide value for the flash clock
#define SIM_CLKDIV2             *(volatile uint32_t *)0x40048048 // System Clock Divider Register 2
#define SIM_CLKDIV2_USBDIV(n)		(uint32_t)(((n) & 0x07) << 1)
#define SIM_CLKDIV2_USBFRAC		(uint32_t)0x01
#define SIM_FCFG1               *(const    uint32_t *)0x4004804C // Flash Configuration Register 1
#define SIM_FCFG2               *(const    uint32_t *)0x40048050 // Flash Configuration Register 2
#define SIM_UIDH                *(const    uint32_t *)0x40048054 // Unique Identification Register High
#define SIM_UIDMH               *(const    uint32_t *)0x40048058 // Unique Identification Register Mid-High
#define SIM_UIDML               *(const    uint32_t *)0x4004805C // Unique Identification Register Mid Low
#define SIM_UIDL                *(const    uint32_t *)0x40048060 // Unique Identification Register Low

// Chapter 13: Reset Control Module (RCM)
#define RCM_SRS0                *(volatile uint8_t  *)0x4007F000 // System Reset Status Register 0
#define RCM_SRS1                *(volatile uint8_t  *)0x4007F001 // System Reset Status Register 1
#define RCM_RPFC                *(volatile uint8_t  *)0x4007F004 // Reset Pin Filter Control Register
#define RCM_RPFW                *(volatile uint8_t  *)0x4007F005 // Reset Pin Filter Width Register
#define RCM_MR                  *(volatile uint8_t  *)0x4007F007 // Mode Register

// Chapter 14: System Mode Controller
#define SMC_PMPROT              *(volatile uint8_t  *)0x4007E000 // Power Mode Protection Register
#define SMC_PMCTRL              *(volatile uint8_t  *)0x4007E001 // Power Mode Control Register
#define SMC_VLLSCTRL            *(volatile uint8_t  *)0x4007E002 // VLLS Control Register
#define SMC_PMSTAT              *(volatile uint8_t  *)0x4007E003 // Power Mode Status Register

// Chapter 15: Power Management Controller
#define PMC_LVDSC1              *(volatile uint8_t  *)0x4007D000 // Low Voltage Detect Status And Control 1 register
#define PMC_LVDSC2              *(volatile uint8_t  *)0x4007D001 // Low Voltage Detect Status And Control 2 register
#define PMC_REGSC               *(volatile uint8_t  *)0x4007D002 // Regulator Status And Control register

// Chapter 16: Low-Leakage Wakeup Unit (LLWU)
#define LLWU_PE1                *(volatile uint8_t  *)0x4007C000 // LLWU Pin Enable 1 register
#define LLWU_PE2                *(volatile uint8_t  *)0x4007C001 // LLWU Pin Enable 2 register
#define LLWU_PE3                *(volatile uint8_t  *)0x4007C002 // LLWU Pin Enable 3 register
#define LLWU_PE4                *(volatile uint8_t  *)0x4007C003 // LLWU Pin Enable 4 register
#define LLWU_ME                 *(volatile uint8_t  *)0x4007C004 // LLWU Module Enable register
#define LLWU_F1                 *(volatile uint8_t  *)0x4007C005 // LLWU Flag 1 register
#define LLWU_F2                 *(volatile uint8_t  *)0x4007C006 // LLWU Flag 2 register
#define LLWU_F3                 *(volatile uint8_t  *)0x4007C007 // LLWU Flag 3 register
#define LLWU_FILT1              *(volatile uint8_t  *)0x4007C008 // LLWU Pin Filter 1 register
#define LLWU_FILT2              *(volatile uint8_t  *)0x4007C009 // LLWU Pin Filter 2 register
#define LLWU_RST                *(volatile uint8_t  *)0x4007C00A // LLWU Reset Enable register

// Chapter 17: Miscellaneous Control Module (MCM)
#define MCM_PLASC               *(volatile uint16_t *)0xE0080008 // Crossbar Switch (AXBS) Slave Configuration
#define MCM_PLAMC               *(volatile uint16_t *)0xE008000A // Crossbar Switch (AXBS) Master Configuration
#define MCM_PLACR               *(volatile uint32_t *)0xE008000C // Crossbar Switch (AXBS) Control Register

// Chapter 20: Direct Memory Access Multiplexer (DMAMUX)
#define DMAMUX0_CHCFG0          *(volatile uint8_t  *)0x40021000 // Channel Configuration register
#define DMAMUX0_CHCFG1          *(volatile uint8_t  *)0x40021001 // Channel Configuration register
#define DMAMUX0_CHCFG2          *(volatile uint8_t  *)0x40021002 // Channel Configuration register
#define DMAMUX0_CHCFG3          *(volatile uint8_t  *)0x40021003 // Channel Configuration register
#define DMAMUX0_CHCFG4          *(volatile uint8_t  *)0x40021004 // Channel Configuration register
#define DMAMUX0_CHCFG5          *(volatile uint8_t  *)0x40021005 // Channel Configuration register
#define DMAMUX0_CHCFG6          *(volatile uint8_t  *)0x40021006 // Channel Configuration register
#define DMAMUX0_CHCFG7          *(volatile uint8_t  *)0x40021007 // Channel Configuration register
#define DMAMUX0_CHCFG8          *(volatile uint8_t  *)0x40021008 // Channel Configuration register
#define DMAMUX0_CHCFG9          *(volatile uint8_t  *)0x40021009 // Channel Configuration register
#define DMAMUX0_CHCFG10         *(volatile uint8_t  *)0x4002100A // Channel Configuration register
#define DMAMUX0_CHCFG11         *(volatile uint8_t  *)0x4002100B // Channel Configuration register
#define DMAMUX0_CHCFG12         *(volatile uint8_t  *)0x4002100C // Channel Configuration register
#define DMAMUX0_CHCFG13         *(volatile uint8_t  *)0x4002100D // Channel Configuration register
#define DMAMUX0_CHCFG14         *(volatile uint8_t  *)0x4002100E // Channel Configuration register
#define DMAMUX0_CHCFG15         *(volatile uint8_t  *)0x4002100F // Channel Configuration register

// Chapter 21: Direct Memory Access Controller (eDMA)
#define DMA_CR                  *(volatile uint32_t *)0x40008000 // Control Register
#define DMA_ES                  *(volatile uint32_t *)0x40008004 // Error Status Register
#define DMA_ERQ                 *(volatile uint32_t *)0x4000800C // Enable Request Register
#define DMA_EEI                 *(volatile uint32_t *)0x40008014 // Enable Error Interrupt Register
#define DMA_CEEI                *(volatile uint8_t  *)0x40008018 // Clear Enable Error Interrupt Register
#define DMA_SEEI                *(volatile uint8_t  *)0x40008019 // Set Enable Error Interrupt Register
#define DMA_CERQ                *(volatile uint8_t  *)0x4000801A // Clear Enable Request Register
#define DMA_SERQ                *(volatile uint8_t  *)0x4000801B // Set Enable Request Register
#define DMA_CDNE                *(volatile uint8_t  *)0x4000801C // Clear DONE Status Bit Register
#define DMA_SSRT                *(volatile uint8_t  *)0x4000801D // Set START Bit Register
#define DMA_CERR                *(volatile uint8_t  *)0x4000801E // Clear Error Register
#define DMA_CINT                *(volatile uint8_t  *)0x4000801F // Clear Interrupt Request Register
#define DMA_INT                 *(volatile uint32_t *)0x40008024 // Interrupt Request Register
#define DMA_ERR                 *(volatile uint32_t *)0x4000802C // Error Register
#define DMA_HRS                 *(volatile uint32_t *)0x40008034 // Hardware Request Status Register
#define DMA_DCHPRI3             *(volatile uint8_t  *)0x40008100 // Channel n Priority Register
#define DMA_DCHPRI2             *(volatile uint8_t  *)0x40008101 // Channel n Priority Register
#define DMA_DCHPRI1             *(volatile uint8_t  *)0x40008102 // Channel n Priority Register
#define DMA_DCHPRI0             *(volatile uint8_t  *)0x40008103 // Channel n Priority Register

#define DMA_TCD0_SADDR          *(volatile uint32_t *)0x40009000 // TCD Source Address
#define DMA_TCD0_SOFF           *(volatile uint16_t *)0x40009004 // TCD Signed Source Address Offset
#define DMA_TCD0_ATTR           *(volatile uint16_t *)0x40009006 // TCD Transfer Attributes
#define DMA_TCD0_NBYTES_MLNO    *(volatile uint32_t *)0x40009008 // TCD Minor Byte Count (Minor Loop Disabled)
#define DMA_TCD0_NBYTES_MLOFFNO *(volatile uint32_t *)0x40009008 // TCD Signed Minor Loop Offset (Minor Loop Enabled and Offset Disabled)
#define DMA_TCD0_NBYTES_MLOFFYES *(volatile uint32_t *)0x40009008 // TCD Signed Minor Loop Offset (Minor Loop and Offset Enabled)
#define DMA_TCD0_SLAST          *(volatile uint32_t *)0x4000900C // TCD Last Source Address Adjustment
#define DMA_TCD0_DADDR          *(volatile uint32_t *)0x40009010 // TCD Destination Address

#define DMA_TCD0_DOFF           0x40009014 // TCD Signed Destination Address Offset
#define DMA_TCD0_DLASTSGA       0x40009018 // TCD Last Destination Address Adjustment/Scatter Gather Address
#define DMA_TCD0_CSR            0x4000901C // TCD Control and Status
#define DMA_TCD0_BITER_ELINKYES 0x4000901E // TCD Beginning Minor Loop Link, Major Loop Count, Channel Linking Enabled
#define DMA_TCD0_BITER_ELINKNO  0x4000901E // TCD Beginning Minor Loop Link, Major Loop Count, Channel Linking Disabled
#define DMA_TCD1_SADDR          0x40009020 // TCD Source Address
#define DMA_TCD1_SOFF           0x40009024 // TCD Signed Source Address Offset
#define DMA_TCD1_ATTR           0x40009026 // TCD Transfer Attributes
#define DMA_TCD1_NBYTES_MLNO    0x40009028 // TCD Minor Byte Count, Minor Loop Disabled
#define DMA_TCD1_NBYTES_MLOFFNO 0x40009028 // TCD Signed Minor Loop Offset, Minor Loop Enabled and Offset Disabled
#define DMA_TCD1_NBYTES_MLOFFYES 0x40009028 // TCD Signed Minor Loop Offset, Minor Loop and Offset Enabled
#define DMA_TCD1_SLAST          0x4000902C // TCD Last Source Address Adjustment
#define DMA_TCD1_DADDR          0x40009030 // TCD Destination Address
#define DMA_TCD1_DOFF           0x40009034 // TCD Signed Destination Address Offset
#define DMA_TCD1_CITER_ELINKYES 0x40009036 // TCD Current Minor Loop Link, Major Loop Count, Channel Linking Enabled
#define DMA_TCD1_CITER_ELINKNO  0x40009036 // ??
#define DMA_TCD1_DLASTSGA       0x40009038 // TCD Last Destination Address Adjustment/Scatter Gather Address
#define DMA_TCD1_CSR            0x4000903C // TCD Control and Status
#define DMA_TCD1_BITER_ELINKYES 0x4000903E // TCD Beginning Minor Loop Link, Major Loop Count Channel Linking Enabled
#define DMA_TCD1_BITER_ELINKNO  0x4000903E // TCD Beginning Minor Loop Link, Major Loop Count, Channel Linking Disabled
#define DMA_TCD2_SADDR          0x40009040 // TCD Source Address
#define DMA_TCD2_SOFF           0x40009044 // TCD Signed Source Address Offset
#define DMA_TCD2_ATTR           0x40009046 // TCD Transfer Attributes
#define DMA_TCD2_NBYTES_MLNO    0x40009048 // TCD Minor Byte Count, Minor Loop Disabled
#define DMA_TCD2_NBYTES_MLOFFNO 0x40009048 // TCD Signed Minor Loop Offset, Minor Loop Enabled and Offset Disabled
#define DMA_TCD2_NBYTES_MLOFFYES 0x40009048 // TCD Signed Minor Loop Offset, Minor Loop and Offset Enabled
#define DMA_TCD2_SLAST          0x4000904C // TCD Last Source Address Adjustment
#define DMA_TCD2_DADDR          0x40009050 // TCD Destination Address
#define DMA_TCD2_DOFF           0x40009054 // TCD Signed Destination Address Offset
#define DMA_TCD2_CITER_ELINKYES 0x40009056 // TCD Current Minor Loop Link, Major Loop Count, Channel Linking Enabled
#define DMA_TCD2_CITER_ELINKNO  0x40009056 // ??
#define DMA_TCD2_DLASTSGA       0x40009058 // TCD Last Destination Address Adjustment/Scatter Gather Address
#define DMA_TCD2_CSR            0x4000905C // TCD Control and Status
#define DMA_TCD2_BITER_ELINKYES 0x4000905E // TCD Beginning Minor Loop Link, Major Loop Count, Channel Linking Enabled
#define DMA_TCD2_BITER_ELINKNO  0x4000905E // TCD Beginning Minor Loop Link, Major Loop Count, Channel Linking Disabled
#define DMA_TCD3_SADDR          0x40009060 // TCD Source Address
#define DMA_TCD3_SOFF           0x40009064 // TCD Signed Source Address Offset
#define DMA_TCD3_ATTR           0x40009066 // TCD Transfer Attributes
#define DMA_TCD3_NBYTES_MLOFFNO 0x40009068 // TCD Signed Minor Loop Offset, Minor Loop Enabled and Offset Disabled
#define DMA_TCD3_NBYTES_MLOFFYES 0x40009068 // TCD Signed Minor Loop Offset, Minor Loop and Offset Enabled
#define DMA_TCD3_SLAST          0x4000906C // TCD Last Source Address Adjustment
#define DMA_TCD3_DADDR          0x40009070 // TCD Destination Address
#define DMA_TCD3_DOFF           0x40009074 // TCD Signed Destination Address Offset
#define DMA_TCD3_CITER_ELINKYES 0x40009076 // TCD Current Minor Loop Link, Major Loop Count, Channel Linking Enabled
#define DMA_TCD3_CITER_ELINKNO  0x40009076 // ??
#define DMA_TCD3_DLASTSGA       0x40009078 // TCD Last Destination Address Adjustment/Scatter Gather Address
#define DMA_TCD3_CSR            0x4000907C // TCD Control and Status
#define DMA_TCD3_BITER_ELINKYES 0x4000907E // TCD Beginning Minor Loop Link, Major Loop Count ,Channel Linking Enabled
#define DMA_TCD3_BITER_ELINKNO  0x4000907E // TCD Beginning Minor Loop Link, Major Loop Count ,Channel Linking Disabled
#define DMA_TCD4_BITER_ELINKYES 0x4000909E // TCD Beginning Minor Loop Link, Major Loop Count ,Channel Linking Enabled

// Chapter 22: External Watchdog Monitor (EWM)
#define EWM_CTRL                *(volatile uint8_t  *)0x40061000 // Control Register
#define EWM_SERV                *(volatile uint8_t  *)0x40061001 // Service Register
#define EWM_CMPL                *(volatile uint8_t  *)0x40061002 // Compare Low Register
#define EWM_CMPH                *(volatile uint8_t  *)0x40061003 // Compare High Register

// Chapter 23: Watchdog Timer (WDOG)
#define WDOG_STCTRLH            *(volatile uint16_t *)0x40052000 // Watchdog Status and Control Register High
#define WDOG_STCTRLH_DISTESTWDOG	(uint16_t)0x4000		// Allows the WDOG's functional test mode to be disabled permanently.
#define WDOG_STCTRLH_BYTESEL(n)		(uint16_t)(((n) & 3) << 12)	// selects the byte to be tested when the watchdog is in the byte test mode.
#define WDOG_STCTRLH_TESTSEL		(uint16_t)0x0800
#define WDOG_STCTRLH_TESTWDOG		(uint16_t)0x0400
#define WDOG_STCTRLH_WAITEN		(uint16_t)0x0080
#define WDOG_STCTRLH_STOPEN		(uint16_t)0x0040
#define WDOG_STCTRLH_DBGEN		(uint16_t)0x0020
#define WDOG_STCTRLH_ALLOWUPDATE	(uint16_t)0x0010
#define WDOG_STCTRLH_WINEN		(uint16_t)0x0008
#define WDOG_STCTRLH_IRQRSTEN		(uint16_t)0x0004
#define WDOG_STCTRLH_CLKSRC		(uint16_t)0x0002
#define WDOG_STCTRLH_WDOGEN		(uint16_t)0x0001
#define WDOG_STCTRLL            *(volatile uint16_t *)0x40052002 // Watchdog Status and Control Register Low
#define WDOG_TOVALH             *(volatile uint16_t *)0x40052004 // Watchdog Time-out Value Register High
#define WDOG_TOVALL             *(volatile uint16_t *)0x40052006 // Watchdog Time-out Value Register Low
#define WDOG_WINH               *(volatile uint16_t *)0x40052008 // Watchdog Window Register High
#define WDOG_WINL               *(volatile uint16_t *)0x4005200A // Watchdog Window Register Low
#define WDOG_REFRESH            *(volatile uint16_t *)0x4005200C // Watchdog Refresh register
#define WDOG_UNLOCK             *(volatile uint16_t *)0x4005200E // Watchdog Unlock register
#define WDOG_UNLOCK_SEQ1		(uint16_t)0xC520
#define WDOG_UNLOCK_SEQ2		(uint16_t)0xD928
#define WDOG_TMROUTH            *(volatile uint16_t *)0x40052010 // Watchdog Timer Output Register High
#define WDOG_TMROUTL            *(volatile uint16_t *)0x40052012 // Watchdog Timer Output Register Low
#define WDOG_RSTCNT             *(volatile uint16_t *)0x40052014 // Watchdog Reset Count register
#define WDOG_PRESC              *(volatile uint16_t *)0x40052016 // Watchdog Prescaler register

// Chapter 24: Multipurpose Clock Generator (MCG)
#define MCG_C1                  *(volatile uint8_t  *)0x40064000 // MCG Control 1 Register
#define MCG_C1_IREFSTEN			(uint8_t)0x01			// Internal Reference Stop Enable, Controls whether or not the internal reference clock remains enabled when the MCG enters Stop mode.
#define MCG_C1_IRCLKEN			(uint8_t)0x02			// Internal Reference Clock Enable, Enables the internal reference clock for use as MCGIRCLK.
#define MCG_C1_IREFS			(uint8_t)0x04			// Internal Reference Select, Selects the reference clock source for the FLL.
#define MCG_C1_FRDIV(n)			(uint8_t)(((n) & 0x07) << 3)	// FLL External Reference Divider, Selects the amount to divide down the external reference clock for the FLL
#define MCG_C1_CLKS(n)			(uint8_t)(((n) & 0x03) << 6)	// Clock Source Select, Selects the clock source for MCGOUTCLK
#define MCG_C2                  *(volatile uint8_t  *)0x40064001 // MCG Control 2 Register
#define MCG_C2_IRCS			(uint8_t)0x01			// Internal Reference Clock Select, Selects between the fast or slow internal reference clock source.
#define MCG_C2_LP			(uint8_t)0x02			// Low Power Select, Controls whether the FLL or PLL is disabled in BLPI and BLPE modes.
#define MCG_C2_EREFS			(uint8_t)0x04			// External Reference Select, Selects the source for the external reference clock. 
#define MCG_C2_HGO0			(uint8_t)0x08			// High Gain Oscillator Select, Controls the crystal oscillator mode of operation
#define MCG_C2_RANGE0(n)		(uint8_t)(((n) & 0x03) << 4)	// Frequency Range Select, Selects the frequency range for the crystal oscillator
#define MCG_C2_LOCRE0			(uint8_t)0x80			// Loss of Clock Reset Enable, Determines whether an interrupt or a reset request is made following a loss of OSC0 
#define MCG_C3                  *(volatile uint8_t  *)0x40064002 // MCG Control 3 Register
#define MCG_C3_SCTRIM(n)		(uint8_t)(n)			// Slow Internal Reference Clock Trim Setting
#define MCG_C4                  *(volatile uint8_t  *)0x40064003 // MCG Control 4 Register
#define MCG_C4_SCFTRIM			(uint8_t)0x01			// Slow Internal Reference Clock Fine Trim
#define MCG_C4_FCTRIM(n)		(uint8_t)(((n) & 0x0F) << 1)	// Fast Internal Reference Clock Trim Setting
#define MCG_C4_DRST_DRS(n)		(uint8_t)(((n) & 0x03) << 5)	// DCO Range Select
#define MCG_C4_DMX32			(uint8_t)0x80			// DCO Maximum Frequency with 32.768 kHz Reference, controls whether the DCO frequency range is narrowed
#define MCG_C5                  *(volatile uint8_t  *)0x40064004 // MCG Control 5 Register
#define MCG_C5_PRDIV0(n)		(uint8_t)((n) & 0x1F)		// PLL External Reference Divider
#define MCG_C5_PLLSTEN0			(uint8_t)0x20			// PLL Stop Enable
#define MCG_C5_PLLCLKEN0		(uint8_t)0x40			// PLL Clock Enable
#define MCG_C6                  *(volatile uint8_t  *)0x40064005 // MCG Control 6 Register
#define MCG_C6_VDIV0(n)			(uint8_t)((n) & 0x1F)		// VCO 0 Divider
#define MCG_C6_CME0			(uint8_t)0x20			// Clock Monitor Enable
#define MCG_C6_PLLS			(uint8_t)0x40			// PLL Select, Controls whether the PLL or FLL output is selected as the MCG source when CLKS[1:0]=00. 
#define MCG_C6_LOLIE0			(uint8_t)0x80			// Loss of Lock Interrrupt Enable
#define MCG_S                   *(volatile uint8_t  *)0x40064006 // MCG Status Register
#define MCG_S_IRCST			(uint8_t)0x01			// Internal Reference Clock Status
#define MCG_S_OSCINIT0			(uint8_t)0x02			// OSC Initialization,  resets to 0, is set to 1 after the initialization cycles of the crystal oscillator
#define MCG_S_CLKST(n)			(uint8_t)(((n) & 0x03) << 2)	// Clock Mode Status, 0=FLL is selected, 1= Internal ref, 2=External ref, 3=PLL
#define MCG_S_CLKST_MASK		(uint8_t)0x0C
#define MCG_S_IREFST			(uint8_t)0x10			// Internal Reference Status
#define MCG_S_PLLST			(uint8_t)0x20			// PLL Select Status
#define MCG_S_LOCK0			(uint8_t)0x40			// Lock Status, 0=PLL Unlocked, 1=PLL Locked
#define MCG_S_LOLS0			(uint8_t)0x80			// Loss of Lock Status
#define MCG_SC                  *(volatile uint8_t  *)0x40064008 // MCG Status and Control Register
#define MCG_SC_LOCS0			(uint8_t)0x01			// OSC0 Loss of Clock Status
#define MCG_SC_FCRDIV(n)		(uint8_t)(((n) & 0x07) << 1)	// Fast Clock Internal Reference Divider
#define MCG_SC_FLTPRSRV			(uint8_t)0x10			// FLL Filter Preserve Enable
#define MCG_SC_ATMF			(uint8_t)0x20			// Automatic Trim Machine Fail Flag
#define MCG_SC_ATMS			(uint8_t)0x40			// Automatic Trim Machine Select
#define MCG_SC_ATME			(uint8_t)0x80			// Automatic Trim Machine Enable
#define MCG_ATCVH               *(volatile uint8_t  *)0x4006400A // MCG Auto Trim Compare Value High Register
#define MCG_ATCVL               *(volatile uint8_t  *)0x4006400B // MCG Auto Trim Compare Value Low Register
#define MCG_C7                  *(volatile uint8_t  *)0x4006400C // MCG Control 7 Register
#define MCG_C8                  *(volatile uint8_t  *)0x4006400D // MCG Control 8 Register

// Chapter 25: Oscillator (OSC)
#define OSC0_CR                 *(volatile uint8_t  *)0x40065000 // OSC Control Register
#define OSC_SC16P			(uint8_t)0x01			// Oscillator 16 pF Capacitor Load Configure
#define OSC_SC8P			(uint8_t)0x02			// Oscillator 8 pF Capacitor Load Configure
#define OSC_SC4P			(uint8_t)0x04			// Oscillator 4 pF Capacitor Load Configure
#define OSC_SC2P			(uint8_t)0x08			// Oscillator 2 pF Capacitor Load Configure
#define OSC_EREFSTEN			(uint8_t)0x20			// External Reference Stop Enable, Controls whether or not the external reference clock (OSCERCLK) remains enabled when MCU enters Stop mode.
#define OSC_ERCLKEN			(uint8_t)0x80			// External Reference Enable, Enables external reference clock (OSCERCLK).

// Chapter 27: Flash Memory Controller (FMC)
#define FMC_PFAPR		*(volatile uint32_t *)0x4001F000	// Flash Access Protection
#define FMC_PFB0CR		*(volatile uint32_t *)0x4001F004	// Flash Control
#define FMC_TAGVDW0S0		*(volatile uint32_t *)0x4001F100	// Cache Tag Storage
#define FMC_TAGVDW0S1		*(volatile uint32_t *)0x4001F104	// Cache Tag Storage
#define FMC_TAGVDW1S0		*(volatile uint32_t *)0x4001F108	// Cache Tag Storage
#define FMC_TAGVDW1S1		*(volatile uint32_t *)0x4001F10C	// Cache Tag Storage
#define FMC_TAGVDW2S0		*(volatile uint32_t *)0x4001F110	// Cache Tag Storage
#define FMC_TAGVDW2S1		*(volatile uint32_t *)0x4001F114	// Cache Tag Storage
#define FMC_TAGVDW3S0		*(volatile uint32_t *)0x4001F118	// Cache Tag Storage
#define FMC_TAGVDW3S1		*(volatile uint32_t *)0x4001F11C	// Cache Tag Storage
#define FMC_DATAW0S0		*(volatile uint32_t *)0x4001F200	// Cache Data Storage
#define FMC_DATAW0S1		*(volatile uint32_t *)0x4001F204	// Cache Data Storage
#define FMC_DATAW1S0		*(volatile uint32_t *)0x4001F208	// Cache Data Storage
#define FMC_DATAW1S1		*(volatile uint32_t *)0x4001F20C	// Cache Data Storage
#define FMC_DATAW2S0		*(volatile uint32_t *)0x4001F210	// Cache Data Storage
#define FMC_DATAW2S1		*(volatile uint32_t *)0x4001F214	// Cache Data Storage
#define FMC_DATAW3S0		*(volatile uint32_t *)0x4001F218	// Cache Data Storage
#define FMC_DATAW3S1		*(volatile uint32_t *)0x4001F21C	// Cache Data Storage

// Chapter 28: Flash Memory Module (FTFL)
#define FTFL_FSTAT		*(volatile uint8_t  *)0x40020000      // Flash Status Register
#define FTFL_FSTAT_CCIF			(uint8_t)0x80			// Command Complete Interrupt Flag
#define FTFL_FSTAT_RDCOLERR		(uint8_t)0x40			// Flash Read Collision Error Flag
#define FTFL_FSTAT_ACCERR		(uint8_t)0x20			// Flash Access Error Flag
#define FTFL_FSTAT_FPVIOL		(uint8_t)0x10			// Flash Protection Violation Flag
#define FTFL_FSTAT_MGSTAT0		(uint8_t)0x01			// Memory Controller Command Completion Status Flag
#define FTFL_FCNFG		*(volatile uint8_t  *)0x40020001      // Flash Configuration Register
#define FTFL_FCNFG_CCIE			(uint8_t)0x80			// Command Complete Interrupt Enable
#define FTFL_FCNFG_RDCOLLIE		(uint8_t)0x40			// Read Collision Error Interrupt Enable
#define FTFL_FCNFG_ERSAREQ		(uint8_t)0x20			// Erase All Request
#define FTFL_FCNFG_ERSSUSP		(uint8_t)0x10			// Erase Suspend
#define FTFL_FCNFG_PFLSH		(uint8_t)0x04			// Flash memory configuration
#define FTFL_FCNFG_RAMRDY		(uint8_t)0x02			// RAM Ready
#define FTFL_FCNFG_EEERDY		(uint8_t)0x01			// EEPROM Ready
#define FTFL_FSEC               *(const    uint8_t  *)0x40020002      // Flash Security Register
#define FTFL_FOPT               *(const    uint8_t  *)0x40020003      // Flash Option Register
#define FTFL_FCCOB3             *(volatile uint8_t  *)0x40020004      // Flash Common Command Object Registers
#define FTFL_FCCOB2             *(volatile uint8_t  *)0x40020005
#define FTFL_FCCOB1             *(volatile uint8_t  *)0x40020006
#define FTFL_FCCOB0             *(volatile uint8_t  *)0x40020007
#define FTFL_FCCOB7             *(volatile uint8_t  *)0x40020008
#define FTFL_FCCOB6             *(volatile uint8_t  *)0x40020009
#define FTFL_FCCOB5             *(volatile uint8_t  *)0x4002000A
#define FTFL_FCCOB4             *(volatile uint8_t  *)0x4002000B
#define FTFL_FCCOBB             *(volatile uint8_t  *)0x4002000C
#define FTFL_FCCOBA             *(volatile uint8_t  *)0x4002000D
#define FTFL_FCCOB9             *(volatile uint8_t  *)0x4002000E
#define FTFL_FCCOB8             *(volatile uint8_t  *)0x4002000F
#define FTFL_FPROT3             *(volatile uint8_t  *)0x40020010      // Program Flash Protection Registers
#define FTFL_FPROT2             *(volatile uint8_t  *)0x40020011      // Program Flash Protection Registers
#define FTFL_FPROT1             *(volatile uint8_t  *)0x40020012      // Program Flash Protection Registers
#define FTFL_FPROT0             *(volatile uint8_t  *)0x40020013      // Program Flash Protection Registers
#define FTFL_FEPROT             *(volatile uint8_t  *)0x40020016      // EEPROM Protection Register
#define FTFL_FDPROT             *(volatile uint8_t  *)0x40020017      // Data Flash Protection Register

// Chapter 30: Cyclic Redundancy Check (CRC)
#define CRC_CRC                 *(volatile uint32_t *)0x40032000 // CRC Data register
#define CRC_GPOLY               *(volatile uint32_t *)0x40032004 // CRC Polynomial register
#define CRC_CTRL                *(volatile uint32_t *)0x40032008 // CRC Control register

// Chapter 31: Analog-to-Digital Converter (ADC)
#define ADC0_SC1A               *(volatile uint32_t *)0x4003B000 // ADC status and control registers 1
#define ADC0_SC1B               *(volatile uint32_t *)0x4003B004 // ADC status and control registers 1
#define ADC_SC1_COCO			(uint32_t)0x80			// Conversion complete flag
#define ADC_SC1_AIEN			(uint32_t)0x40			// Interrupt enable
#define ADC_SC1_DIFF			(uint32_t)0x20			// Differential mode enable
#define ADC_SC1_ADCH(n)			(uint32_t)((n) & 0x1F)		// Input channel select
#define ADC0_CFG1               *(volatile uint32_t *)0x4003B008 // ADC configuration register 1
#define ADC_CFG1_ADLPC			(uint32_t)0x80			// Low-power configuration
#define ADC_CFG1_ADIV(n)		(uint32_t)(((n) & 3) << 5)	// Clock divide select, 0=direct, 1=div2, 2=div4, 3=div8
#define ADC_CFG1_ADLSMP			(uint32_t)0x10			// Sample time configuration, 0=Short, 1=Long
#define ADC_CFG1_MODE(n)		(uint32_t)(((n) & 3) << 2)	// Conversion mode, 0=8 bit, 1=12 bit, 2=10 bit, 3=16 bit
#define ADC_CFG1_ADICLK(n)		(uint32_t)(((n) & 3) << 0)	// Input clock, 0=bus, 1=bus/2, 2=OSCERCLK, 3=async
#define ADC0_CFG2               *(volatile uint32_t *)0x4003B00C // Configuration register 2
#define ADC_CFG2_MUXSEL			(uint32_t)0x10			// 0=a channels, 1=b channels
#define ADC_CFG2_ADACKEN		(uint32_t)0x08			// async clock enable
#define ADC_CFG2_ADHSC			(uint32_t)0x04			// High speed configuration
#define ADC_CFG2_ADLSTS(n)		(uint32_t)(((n) & 3) << 0)	// Sample time, 0=24 cycles, 1=12 cycles, 2=6 cycles, 3=2 cycles
#define ADC0_RA                 *(volatile uint32_t *)0x4003B010 // ADC data result register
#define ADC0_RB                 *(volatile uint32_t *)0x4003B014 // ADC data result register
#define ADC0_CV1                *(volatile uint32_t *)0x4003B018 // Compare value registers
#define ADC0_CV2                *(volatile uint32_t *)0x4003B01C // Compare value registers
#define ADC0_SC2                *(volatile uint32_t *)0x4003B020 // Status and control register 2
#define ADC_SC2_ADACT			(uint32_t)0x80			// Conversion active
#define ADC_SC2_ADTRG			(uint32_t)0x40			// Conversion trigger select, 0=software, 1=hardware
#define ADC_SC2_ACFE			(uint32_t)0x20			// Compare function enable
#define ADC_SC2_ACFGT			(uint32_t)0x10			// Compare function greater than enable
#define ADC_SC2_ACREN			(uint32_t)0x08			// Compare function range enable
#define ADC_SC2_DMAEN			(uint32_t)0x04			// DMA enable
#define ADC_SC2_REFSEL(n)		(uint32_t)(((n) & 3) << 0)	// Voltage reference, 0=vcc/external, 1=1.2 volts
#define ADC0_SC3                *(volatile uint32_t *)0x4003B024 // Status and control register 3
#define ADC_SC3_CAL			(uint32_t)0x80			// Calibration, 1=begin, stays set while cal in progress
#define ADC_SC3_CALF			(uint32_t)0x40			// Calibration failed flag
#define ADC_SC3_ADCO			(uint32_t)0x08			// Continuous conversion enable
#define ADC_SC3_AVGE			(uint32_t)0x04			// Hardware average enable
#define ADC_SC3_AVGS(n)			(uint32_t)(((n) & 3) << 0)	// avg select, 0=4 samples, 1=8 samples, 2=16 samples, 3=32 samples
#define ADC0_OFS                *(volatile uint32_t *)0x4003B028 // ADC offset correction register
#define ADC0_PG                 *(volatile uint32_t *)0x4003B02C // ADC plus-side gain register
#define ADC0_MG                 *(volatile uint32_t *)0x4003B030 // ADC minus-side gain register
#define ADC0_CLPD               *(volatile uint32_t *)0x4003B034 // ADC plus-side general calibration value register
#define ADC0_CLPS               *(volatile uint32_t *)0x4003B038 // ADC plus-side general calibration value register
#define ADC0_CLP4               *(volatile uint32_t *)0x4003B03C // ADC plus-side general calibration value register
#define ADC0_CLP3               *(volatile uint32_t *)0x4003B040 // ADC plus-side general calibration value register
#define ADC0_CLP2               *(volatile uint32_t *)0x4003B044 // ADC plus-side general calibration value register
#define ADC0_CLP1               *(volatile uint32_t *)0x4003B048 // ADC plus-side general calibration value register
#define ADC0_CLP0               *(volatile uint32_t *)0x4003B04C // ADC plus-side general calibration value register
#define ADC0_CLMD               *(volatile uint32_t *)0x4003B054 // ADC minus-side general calibration value register
#define ADC0_CLMS               *(volatile uint32_t *)0x4003B058 // ADC minus-side general calibration value register
#define ADC0_CLM4               *(volatile uint32_t *)0x4003B05C // ADC minus-side general calibration value register
#define ADC0_CLM3               *(volatile uint32_t *)0x4003B060 // ADC minus-side general calibration value register
#define ADC0_CLM2               *(volatile uint32_t *)0x4003B064 // ADC minus-side general calibration value register
#define ADC0_CLM1               *(volatile uint32_t *)0x4003B068 // ADC minus-side general calibration value register
#define ADC0_CLM0               *(volatile uint32_t *)0x4003B06C // ADC minus-side general calibration value register
//#define MCG_C2_RANGE0(n)		(uint8_t)(((n) & 0x03) << 4)	// Frequency Range Select, Selects the frequency range for the crystal oscillator
//#define MCG_C2_LOCRE0			(uint8_t)0x80			// Loss of Clock Reset Enable, Determines whether an interrupt or a reset request is made following a loss of OSC0 

// Chapter 32: Comparator (CMP)
#define CMP0_CR0                *(volatile uint8_t  *)0x40073000 // CMP Control Register 0
#define CMP0_CR1                *(volatile uint8_t  *)0x40073001 // CMP Control Register 1
#define CMP0_FPR                *(volatile uint8_t  *)0x40073002 // CMP Filter Period Register
#define CMP0_SCR                *(volatile uint8_t  *)0x40073003 // CMP Status and Control Register
#define CMP0_DACCR              *(volatile uint8_t  *)0x40073004 // DAC Control Register
#define CMP0_MUXCR              *(volatile uint8_t  *)0x40073005 // MUX Control Register
#define CMP1_CR0                *(volatile uint8_t  *)0x40073008 // CMP Control Register 0
#define CMP1_CR1                *(volatile uint8_t  *)0x40073009 // CMP Control Register 1
#define CMP1_FPR                *(volatile uint8_t  *)0x4007300A // CMP Filter Period Register
#define CMP1_SCR                *(volatile uint8_t  *)0x4007300B // CMP Status and Control Register
#define CMP1_DACCR              *(volatile uint8_t  *)0x4007300C // DAC Control Register
#define CMP1_MUXCR              *(volatile uint8_t  *)0x4007300D // MUX Control Register

// Chapter 33: Voltage Reference (VREFV1)
#define VREF_TRM                *(volatile uint8_t  *)0x40074000 // VREF Trim Register
#define VREF_SC                 *(volatile uint8_t  *)0x40074001 // VREF Status and Control Register

// Chapter 34: Programmable Delay Block (PDB)
#define PDB0_SC                 *(volatile uint32_t *)0x40036000 // Status and Control Register
#define PDB_SC_LDMOD(n)			(((n) & 3) << 18)	// Load Mode Select
#define PDB_SC_PDBEIE			0x00020000		// Sequence Error Interrupt Enable
#define PDB_SC_SWTRIG			0x00010000		// Software Trigger
#define PDB_SC_DMAEN			0x00008000		// DMA Enable
#define PDB_SC_PRESCALER(n)		(((n) & 7) << 12)	// Prescaler Divider Select
#define PDB_SC_TRGSEL(n)		(((n) & 15) << 8)	// Trigger Input Source Select
#define PDB_SC_PDBEN			0x00000080		// PDB Enable
#define PDB_SC_PDBIF			0x00000040		// PDB Interrupt Flag
#define PDB_SC_PDBIE			0x00000020		// PDB Interrupt Enable.
#define PDB_SC_MULT(n)			(((n) & 3) << 2)	// Multiplication Factor
#define PDB_SC_CONT			0x00000002		// Continuous Mode Enable
#define PDB_SC_LDOK			0x00000001		// Load OK
#define PDB0_MOD                *(volatile uint32_t *)0x40036004 // Modulus Register
#define PDB0_CNT                *(volatile uint32_t *)0x40036008 // Counter Register
#define PDB0_IDLY               *(volatile uint32_t *)0x4003600C // Interrupt Delay Register
#define PDB0_CH0C1              *(volatile uint32_t *)0x40036010 // Channel n Control Register 1
#define PDB0_CH0S               *(volatile uint32_t *)0x40036014 // Channel n Status Register
#define PDB0_CH0DLY0            *(volatile uint32_t *)0x40036018 // Channel n Delay 0 Register
#define PDB0_CH0DLY1            *(volatile uint32_t *)0x4003601C // Channel n Delay 1 Register
#define PDB0_POEN               *(volatile uint32_t *)0x40036190 // Pulse-Out n Enable Register
#define PDB0_PO0DLY             *(volatile uint32_t *)0x40036194 // Pulse-Out n Delay Register
#define PDB0_PO1DLY             *(volatile uint32_t *)0x40036198 // Pulse-Out n Delay Register

// Chapter 35: FlexTimer Module (FTM)
#define FTM0_SC                 *(volatile uint32_t *)0x40038000 // Status And Control
#define FTM_SC_TOF			0x80				// Timer Overflow Flag
#define FTM_SC_TOIE			0x40				// Timer Overflow Interrupt Enable
#define FTM_SC_CPWMS			0x20				// Center-Aligned PWM Select
#define FTM_SC_CLKS(n)			(((n) & 3) << 3)		// Clock Source Selection
#define FTM_SC_PS(n)			(((n) & 7) << 0)		// Prescale Factor Selection
#define FTM0_CNT                *(volatile uint32_t *)0x40038004 // Counter
#define FTM0_MOD                *(volatile uint32_t *)0x40038008 // Modulo
#define FTM0_C0SC		*(volatile uint32_t *)0x4003800C // Channel 0 Status And Control
#define FTM0_C0V		*(volatile uint32_t *)0x40038010 // Channel 0 Value
#define FTM0_C1SC		*(volatile uint32_t *)0x40038014 // Channel 1 Status And Control
#define FTM0_C1V		*(volatile uint32_t *)0x40038018 // Channel 1 Value
#define FTM0_C2SC		*(volatile uint32_t *)0x4003801C // Channel 2 Status And Control
#define FTM0_C2V		*(volatile uint32_t *)0x40038020 // Channel 2 Value
#define FTM0_C3SC		*(volatile uint32_t *)0x40038024 // Channel 3 Status And Control
#define FTM0_C3V		*(volatile uint32_t *)0x40038028 // Channel 3 Value
#define FTM0_C4SC		*(volatile uint32_t *)0x4003802C // Channel 4 Status And Control
#define FTM0_C4V		*(volatile uint32_t *)0x40038030 // Channel 4 Value
#define FTM0_C5SC		*(volatile uint32_t *)0x40038034 // Channel 5 Status And Control
#define FTM0_C5V		*(volatile uint32_t *)0x40038038 // Channel 5 Value
#define FTM0_C6SC		*(volatile uint32_t *)0x4003803C // Channel 6 Status And Control
#define FTM0_C6V		*(volatile uint32_t *)0x40038040 // Channel 6 Value
#define FTM0_C7SC		*(volatile uint32_t *)0x40038044 // Channel 7 Status And Control
#define FTM0_C7V		*(volatile uint32_t *)0x40038048 // Channel 7 Value
#define FTM0_CNTIN              *(volatile uint32_t *)0x4003804C // Counter Initial Value
#define FTM0_STATUS             *(volatile uint32_t *)0x40038050 // Capture And Compare Status
#define FTM0_MODE               *(volatile uint32_t *)0x40038054 // Features Mode Selection
#define FTM_MODE_FAULTIE		0x80				// Fault Interrupt Enable
#define FTM_MODE_FAULTM(n)		(((n) & 3) << 5)		// Fault Control Mode
#define FTM_MODE_CAPTEST		0x10				// Capture Test Mode Enable
#define FTM_MODE_PWMSYNC		0x08				// PWM Synchronization Mode
#define FTM_MODE_WPDIS			0x04				// Write Protection Disable
#define FTM_MODE_INIT			0x02				// Initialize The Channels Output
#define FTM_MODE_FTMEN			0x01				// FTM Enable
#define FTM0_SYNC               *(volatile uint32_t *)0x40038058 // Synchronization
#define FTM_SYNC_SWSYNC			0x80				// 
#define FTM_SYNC_TRIG2			0x40				// 
#define FTM_SYNC_TRIG1			0x20				// 
#define FTM_SYNC_TRIG0			0x10				// 
#define FTM_SYNC_SYNCHOM		0x08				// 
#define FTM_SYNC_REINIT			0x04				// 
#define FTM_SYNC_CNTMAX			0x02				// 
#define FTM_SYNC_CNTMIN			0x01				// 
#define FTM0_OUTINIT            *(volatile uint32_t *)0x4003805C // Initial State For Channels Output
#define FTM0_OUTMASK            *(volatile uint32_t *)0x40038060 // Output Mask
#define FTM0_COMBINE            *(volatile uint32_t *)0x40038064 // Function For Linked Channels
#define FTM0_DEADTIME           *(volatile uint32_t *)0x40038068 // Deadtime Insertion Control
#define FTM0_EXTTRIG            *(volatile uint32_t *)0x4003806C // FTM External Trigger
#define FTM0_POL                *(volatile uint32_t *)0x40038070 // Channels Polarity
#define FTM0_FMS                *(volatile uint32_t *)0x40038074 // Fault Mode Status
#define FTM0_FILTER             *(volatile uint32_t *)0x40038078 // Input Capture Filter Control
#define FTM0_FLTCTRL            *(volatile uint32_t *)0x4003807C // Fault Control
#define FTM0_QDCTRL             *(volatile uint32_t *)0x40038080 // Quadrature Decoder Control And Status
#define FTM0_CONF               *(volatile uint32_t *)0x40038084 // Configuration
#define FTM0_FLTPOL             *(volatile uint32_t *)0x40038088 // FTM Fault Input Polarity
#define FTM0_SYNCONF            *(volatile uint32_t *)0x4003808C // Synchronization Configuration
#define FTM0_INVCTRL            *(volatile uint32_t *)0x40038090 // FTM Inverting Control
#define FTM0_SWOCTRL            *(volatile uint32_t *)0x40038094 // FTM Software Output Control
#define FTM0_PWMLOAD            *(volatile uint32_t *)0x40038098 // FTM PWM Load
#define FTM1_SC                 *(volatile uint32_t *)0x40039000 // Status And Control
#define FTM1_CNT                *(volatile uint32_t *)0x40039004 // Counter
#define FTM1_MOD                *(volatile uint32_t *)0x40039008 // Modulo
#define FTM1_C0SC		*(volatile uint32_t *)0x4003900C // Channel 0 Status And Control
#define FTM1_C0V		*(volatile uint32_t *)0x40039010 // Channel 0 Value
#define FTM1_C1SC		*(volatile uint32_t *)0x40039014 // Channel 1 Status And Control
#define FTM1_C1V		*(volatile uint32_t *)0x40039018 // Channel 1 Value
#define FTM1_CNTIN              *(volatile uint32_t *)0x4003904C // Counter Initial Value
#define FTM1_STATUS             *(volatile uint32_t *)0x40039050 // Capture And Compare Status
#define FTM1_MODE               *(volatile uint32_t *)0x40039054 // Features Mode Selection
#define FTM1_SYNC               *(volatile uint32_t *)0x40039058 // Synchronization
#define FTM1_OUTINIT            *(volatile uint32_t *)0x4003905C // Initial State For Channels Output
#define FTM1_OUTMASK            *(volatile uint32_t *)0x40039060 // Output Mask
#define FTM1_COMBINE            *(volatile uint32_t *)0x40039064 // Function For Linked Channels
#define FTM1_DEADTIME           *(volatile uint32_t *)0x40039068 // Deadtime Insertion Control
#define FTM1_EXTTRIG            *(volatile uint32_t *)0x4003906C // FTM External Trigger
#define FTM1_POL                *(volatile uint32_t *)0x40039070 // Channels Polarity
#define FTM1_FMS                *(volatile uint32_t *)0x40039074 // Fault Mode Status
#define FTM1_FILTER             *(volatile uint32_t *)0x40039078 // Input Capture Filter Control
#define FTM1_FLTCTRL            *(volatile uint32_t *)0x4003907C // Fault Control
#define FTM1_QDCTRL             *(volatile uint32_t *)0x40039080 // Quadrature Decoder Control And Status
#define FTM1_CONF               *(volatile uint32_t *)0x40039084 // Configuration
#define FTM1_FLTPOL             *(volatile uint32_t *)0x40039088 // FTM Fault Input Polarity
#define FTM1_SYNCONF            *(volatile uint32_t *)0x4003908C // Synchronization Configuration
#define FTM1_INVCTRL            *(volatile uint32_t *)0x40039090 // FTM Inverting Control
#define FTM1_SWOCTRL            *(volatile uint32_t *)0x40039094 // FTM Software Output Control
#define FTM1_PWMLOAD            *(volatile uint32_t *)0x40039098 // FTM PWM Load

// Chapter 36: Periodic Interrupt Timer (PIT)
#define PIT_MCR                 *(volatile uint32_t *)0x40037000 // PIT Module Control Register
#define PIT_LDVAL0              *(volatile uint32_t *)0x40037100 // Timer Load Value Register
#define PIT_CVAL0               *(volatile uint32_t *)0x40037104 // Current Timer Value Register
#define PIT_TCTRL0              *(volatile uint32_t *)0x40037108 // Timer Control Register
#define PIT_TFLG0               *(volatile uint32_t *)0x4003710C // Timer Flag Register
#define PIT_LDVAL1              *(volatile uint32_t *)0x40037110 // Timer Load Value Register
#define PIT_CVAL1               *(volatile uint32_t *)0x40037114 // Current Timer Value Register
#define PIT_TCTRL1              *(volatile uint32_t *)0x40037118 // Timer Control Register
#define PIT_TFLG1               *(volatile uint32_t *)0x4003711C // Timer Flag Register
#define PIT_LDVAL2              *(volatile uint32_t *)0x40037120 // Timer Load Value Register
#define PIT_CVAL2               *(volatile uint32_t *)0x40037124 // Current Timer Value Register
#define PIT_TCTRL2              *(volatile uint32_t *)0x40037128 // Timer Control Register
#define PIT_TFLG2               *(volatile uint32_t *)0x4003712C // Timer Flag Register
#define PIT_LDVAL3              *(volatile uint32_t *)0x40037130 // Timer Load Value Register
#define PIT_CVAL3               *(volatile uint32_t *)0x40037134 // Current Timer Value Register
#define PIT_TCTRL3              *(volatile uint32_t *)0x40037138 // Timer Control Register
#define PIT_TFLG3               *(volatile uint32_t *)0x4003713C // Timer Flag Register

// Chapter 37: Low-Power Timer (LPTMR)
#define LPTMR0_CSR              *(volatile uint32_t *)0x40040000 // Low Power Timer Control Status Register
#define LPTMR0_PSR              *(volatile uint32_t *)0x40040004 // Low Power Timer Prescale Register
#define LPTMR0_CMR              *(volatile uint32_t *)0x40040008 // Low Power Timer Compare Register
#define LPTMR0_CNR              *(volatile uint32_t *)0x4004000C // Low Power Timer Counter Register

// Chapter 38: Carrier Modulator Transmitter (CMT)
#define CMT_CGH1                *(volatile uint8_t  *)0x40062000 // CMT Carrier Generator High Data Register 1
#define CMT_CGL1                *(volatile uint8_t  *)0x40062001 // CMT Carrier Generator Low Data Register 1
#define CMT_CGH2                *(volatile uint8_t  *)0x40062002 // CMT Carrier Generator High Data Register 2
#define CMT_CGL2                *(volatile uint8_t  *)0x40062003 // CMT Carrier Generator Low Data Register 2
#define CMT_OC                  *(volatile uint8_t  *)0x40062004 // CMT Output Control Register
#define CMT_MSC                 *(volatile uint8_t  *)0x40062005 // CMT Modulator Status and Control Register
#define CMT_CMD1                *(volatile uint8_t  *)0x40062006 // CMT Modulator Data Register Mark High
#define CMT_CMD2                *(volatile uint8_t  *)0x40062007 // CMT Modulator Data Register Mark Low
#define CMT_CMD3                *(volatile uint8_t  *)0x40062008 // CMT Modulator Data Register Space High
#define CMT_CMD4                *(volatile uint8_t  *)0x40062009 // CMT Modulator Data Register Space Low
#define CMT_PPS                 *(volatile uint8_t  *)0x4006200A // CMT Primary Prescaler Register
#define CMT_DMA                 *(volatile uint8_t  *)0x4006200B // CMT Direct Memory Access Register

// Chapter 39: Real Time Clock (RTC)
#define RTC_TSR                 *(volatile uint32_t *)0x4003D000 // RTC Time Seconds Register
#define RTC_TPR                 *(volatile uint32_t *)0x4003D004 // RTC Time Prescaler Register
#define RTC_TAR                 *(volatile uint32_t *)0x4003D008 // RTC Time Alarm Register
#define RTC_TCR                 *(volatile uint32_t *)0x4003D00C // RTC Time Compensation Register
#define RTC_TCR_CIC(n)			(((n) & 255) << 24)		// Compensation Interval Counter
#define RTC_TCR_TCV(n)			(((n) & 255) << 16)		// Time Compensation Value
#define RTC_TCR_CIR(n)			(((n) & 255) << 8)		// Compensation Interval Register
#define RTC_TCR_TCR(n)			(((n) & 255) << 0)		// Time Compensation Register
#define RTC_CR                  *(volatile uint32_t *)0x4003D010 // RTC Control Register
#define RTC_CR_SC2P			(uint32_t)0x00002000		// 
#define RTC_CR_SC4P			(uint32_t)0x00001000		// 
#define RTC_CR_SC8P			(uint32_t)0x00000800		// 
#define RTC_CR_SC16P			(uint32_t)0x00000400		// 
#define RTC_CR_CLKO			(uint32_t)0x00000200		// 
#define RTC_CR_OSCE			(uint32_t)0x00000100		// 
#define RTC_CR_UM			(uint32_t)0x00000008		// 
#define RTC_CR_SUP			(uint32_t)0x00000004		// 
#define RTC_CR_WPE			(uint32_t)0x00000002		// 
#define RTC_CR_SWR			(uint32_t)0x00000001		// 
#define RTC_SR                  *(volatile uint32_t *)0x4003D014 // RTC Status Register
#define RTC_SR_TCE			(uint32_t)0x00000010		// 
#define RTC_SR_TAF			(uint32_t)0x00000004		// 
#define RTC_SR_TOF			(uint32_t)0x00000002		// 
#define RTC_SR_TIF			(uint32_t)0x00000001		// 
#define RTC_LR                  *(volatile uint32_t *)0x4003D018 // RTC Lock Register
#define RTC_IER                 *(volatile uint32_t *)0x4003D01C // RTC Interrupt Enable Register
#define RTC_WAR                 *(volatile uint32_t *)0x4003D800 // RTC Write Access Register
#define RTC_RAR                 *(volatile uint32_t *)0x4003D804 // RTC Read Access Register

// Chapter 40: Universal Serial Bus OTG Controller (USBOTG)
#define USB0_PERID              *(const    uint8_t  *)0x40072000 // Peripheral ID register
#define USB0_IDCOMP             *(const    uint8_t  *)0x40072004 // Peripheral ID Complement register
#define USB0_REV                *(const    uint8_t  *)0x40072008 // Peripheral Revision register
#define USB0_ADDINFO            *(volatile uint8_t  *)0x4007200C // Peripheral Additional Info register
#define USB0_OTGISTAT           *(volatile uint8_t  *)0x40072010 // OTG Interrupt Status register
#define USB_OTGISTAT_IDCHG		(uint8_t)0x80			//
#define USB_OTGISTAT_ONEMSEC		(uint8_t)0x40			//
#define USB_OTGISTAT_LINE_STATE_CHG	(uint8_t)0x20			//
#define USB_OTGISTAT_SESSVLDCHG		(uint8_t)0x08			//
#define USB_OTGISTAT_B_SESS_CHG		(uint8_t)0x04			//
#define USB_OTGISTAT_AVBUSCHG		(uint8_t)0x01			//
#define USB0_OTGICR             *(volatile uint8_t  *)0x40072014 // OTG Interrupt Control Register
#define USB_OTGICR_IDEN			(uint8_t)0x80			// 
#define USB_OTGICR_ONEMSECEN		(uint8_t)0x40			// 
#define USB_OTGICR_LINESTATEEN		(uint8_t)0x20			// 
#define USB_OTGICR_SESSVLDEN		(uint8_t)0x08			// 
#define USB_OTGICR_BSESSEN		(uint8_t)0x04			// 
#define USB_OTGICR_AVBUSEN		(uint8_t)0x01			// 
#define USB0_OTGSTAT            *(volatile uint8_t  *)0x40072018 // OTG Status register
#define USB_OTGSTAT_ID			(uint8_t)0x80			// 
#define USB_OTGSTAT_ONEMSECEN		(uint8_t)0x40			// 
#define USB_OTGSTAT_LINESTATESTABLE	(uint8_t)0x20			// 
#define USB_OTGSTAT_SESS_VLD		(uint8_t)0x08			// 
#define USB_OTGSTAT_BSESSEND		(uint8_t)0x04			// 
#define USB_OTGSTAT_AVBUSVLD		(uint8_t)0x01			// 
#define USB0_OTGCTL             *(volatile uint8_t  *)0x4007201C // OTG Control Register
#define USB_OTGCTL_DPHIGH		(uint8_t)0x80			// 
#define USB_OTGCTL_DPLOW		(uint8_t)0x20			// 
#define USB_OTGCTL_DMLOW		(uint8_t)0x10			// 
#define USB_OTGCTL_OTGEN		(uint8_t)0x04			// 
#define USB0_ISTAT              *(volatile uint8_t  *)0x40072080 // Interrupt Status Register
#define USB_ISTAT_STALL			(uint8_t)0x80			// 
#define USB_ISTAT_ATTACH		(uint8_t)0x40			// 
#define USB_ISTAT_RESUME		(uint8_t)0x20			// 
#define USB_ISTAT_SLEEP			(uint8_t)0x10			// 
#define USB_ISTAT_TOKDNE		(uint8_t)0x08			// 
#define USB_ISTAT_SOFTOK		(uint8_t)0x04			// 
#define USB_ISTAT_ERROR			(uint8_t)0x02			// 
#define USB_ISTAT_USBRST		(uint8_t)0x01			// 
#define USB0_INTEN              *(volatile uint8_t  *)0x40072084 // Interrupt Enable Register
#define USB_INTEN_STALLEN		(uint8_t)0x80			// 
#define USB_INTEN_ATTACHEN		(uint8_t)0x40			// 
#define USB_INTEN_RESUMEEN		(uint8_t)0x20			// 
#define USB_INTEN_SLEEPEN		(uint8_t)0x10			// 
#define USB_INTEN_TOKDNEEN		(uint8_t)0x08			// 
#define USB_INTEN_SOFTOKEN		(uint8_t)0x04			// 
#define USB_INTEN_ERROREN		(uint8_t)0x02			// 
#define USB_INTEN_USBRSTEN		(uint8_t)0x01			// 
#define USB0_ERRSTAT            *(volatile uint8_t  *)0x40072088 // Error Interrupt Status Register
#define USB_ERRSTAT_BTSERR		(uint8_t)0x80			// 
#define USB_ERRSTAT_DMAERR		(uint8_t)0x20			// 
#define USB_ERRSTAT_BTOERR		(uint8_t)0x10			// 
#define USB_ERRSTAT_DFN8		(uint8_t)0x08			// 
#define USB_ERRSTAT_CRC16		(uint8_t)0x04			// 
#define USB_ERRSTAT_CRC5EOF		(uint8_t)0x02			// 
#define USB_ERRSTAT_PIDERR		(uint8_t)0x01			// 
#define USB0_ERREN              *(volatile uint8_t  *)0x4007208C // Error Interrupt Enable Register
#define USB_ERREN_BTSERREN		(uint8_t)0x80			// 
#define USB_ERREN_DMAERREN		(uint8_t)0x20			// 
#define USB_ERREN_BTOERREN		(uint8_t)0x10			// 
#define USB_ERREN_DFN8EN		(uint8_t)0x08			// 
#define USB_ERREN_CRC16EN		(uint8_t)0x04			// 
#define USB_ERREN_CRC5EOFEN		(uint8_t)0x02			// 
#define USB_ERREN_PIDERREN		(uint8_t)0x01			// 
#define USB0_STAT               *(volatile uint8_t  *)0x40072090 // Status Register
#define USB_STAT_TX			(uint8_t)0x08			// 
#define USB_STAT_ODD			(uint8_t)0x04			// 
#define USB_STAT_ENDP(n)		(uint8_t)((n) >> 4)		// 
#define USB0_CTL                *(volatile uint8_t  *)0x40072094 // Control Register
#define USB_CTL_JSTATE			(uint8_t)0x80			// 
#define USB_CTL_SE0			(uint8_t)0x40			// 
#define USB_CTL_TXSUSPENDTOKENBUSY	(uint8_t)0x20			// 
#define USB_CTL_RESET			(uint8_t)0x10			// 
#define USB_CTL_HOSTMODEEN		(uint8_t)0x08			// 
#define USB_CTL_RESUME			(uint8_t)0x04			// 
#define USB_CTL_ODDRST			(uint8_t)0x02			// 
#define USB_CTL_USBENSOFEN		(uint8_t)0x01			// 
#define USB0_ADDR               *(volatile uint8_t  *)0x40072098 // Address Register
#define USB0_BDTPAGE1           *(volatile uint8_t  *)0x4007209C // BDT Page Register 1
#define USB0_FRMNUML            *(volatile uint8_t  *)0x400720A0 // Frame Number Register Low
#define USB0_FRMNUMH            *(volatile uint8_t  *)0x400720A4 // Frame Number Register High
#define USB0_TOKEN              *(volatile uint8_t  *)0x400720A8 // Token Register
#define USB0_SOFTHLD            *(volatile uint8_t  *)0x400720AC // SOF Threshold Register
#define USB0_BDTPAGE2           *(volatile uint8_t  *)0x400720B0 // BDT Page Register 2
#define USB0_BDTPAGE3           *(volatile uint8_t  *)0x400720B4 // BDT Page Register 3
#define USB0_ENDPT0             *(volatile uint8_t  *)0x400720C0 // Endpoint Control Register
#define USB_ENDPT_HOSTWOHUB		(uint8_t)0x80			// host only, enable low speed
#define USB_ENDPT_RETRYDIS		(uint8_t)0x40			// host only, set to disable NAK retry
#define USB_ENDPT_EPCTLDIS		(uint8_t)0x10			// 0=control, 1=bulk, interrupt, isync
#define USB_ENDPT_EPRXEN		(uint8_t)0x08			// enables the endpoint for RX transfers.
#define USB_ENDPT_EPTXEN		(uint8_t)0x04			// enables the endpoint for TX transfers.
#define USB_ENDPT_EPSTALL		(uint8_t)0x02			// set to stall endpoint
#define USB_ENDPT_EPHSHK		(uint8_t)0x01			// enable handshaking during a transaction, generally set unless Isochronous
#define USB0_ENDPT1             *(volatile uint8_t  *)0x400720C4 // Endpoint Control Register
#define USB0_ENDPT2             *(volatile uint8_t  *)0x400720C8 // Endpoint Control Register
#define USB0_ENDPT3             *(volatile uint8_t  *)0x400720CC // Endpoint Control Register
#define USB0_ENDPT4             *(volatile uint8_t  *)0x400720D0 // Endpoint Control Register
#define USB0_ENDPT5             *(volatile uint8_t  *)0x400720D4 // Endpoint Control Register
#define USB0_ENDPT6             *(volatile uint8_t  *)0x400720D8 // Endpoint Control Register
#define USB0_ENDPT7             *(volatile uint8_t  *)0x400720DC // Endpoint Control Register
#define USB0_ENDPT8             *(volatile uint8_t  *)0x400720E0 // Endpoint Control Register
#define USB0_ENDPT9             *(volatile uint8_t  *)0x400720E4 // Endpoint Control Register
#define USB0_ENDPT10            *(volatile uint8_t  *)0x400720E8 // Endpoint Control Register
#define USB0_ENDPT11            *(volatile uint8_t  *)0x400720EC // Endpoint Control Register
#define USB0_ENDPT12            *(volatile uint8_t  *)0x400720F0 // Endpoint Control Register
#define USB0_ENDPT13            *(volatile uint8_t  *)0x400720F4 // Endpoint Control Register
#define USB0_ENDPT14            *(volatile uint8_t  *)0x400720F8 // Endpoint Control Register
#define USB0_ENDPT15            *(volatile uint8_t  *)0x400720FC // Endpoint Control Register
#define USB0_USBCTRL            *(volatile uint8_t  *)0x40072100 // USB Control Register
#define USB_USBCTRL_SUSP		(uint8_t)0x80			// Places the USB transceiver into the suspend state.
#define USB_USBCTRL_PDE			(uint8_t)0x40			// Enables the weak pulldowns on the USB transceiver.
#define USB0_OBSERVE            *(volatile uint8_t  *)0x40072104 // USB OTG Observe Register
#define USB_OBSERVE_DPPU		(uint8_t)0x80			// 
#define USB_OBSERVE_DPPD		(uint8_t)0x40			// 
#define USB_OBSERVE_DMPD		(uint8_t)0x10			// 
#define USB0_CONTROL            *(volatile uint8_t  *)0x40072108 // USB OTG Control Register
#define USB_CONTROL_DPPULLUPNONOTG	(uint8_t)0x10			//  Provides control of the DP PULLUP in the USB OTG module, if USB is configured in non-OTG device mode.
#define USB0_USBTRC0            *(volatile uint8_t  *)0x4007210C // USB Transceiver Control Register 0
#define USB_USBTRC_USBRESET		(uint8_t)0x80			//
#define USB_USBTRC_USBRESMEN		(uint8_t)0x20			//
#define USB_USBTRC_SYNC_DET		(uint8_t)0x02			//
#define USB_USBTRC_USB_RESUME_INT	(uint8_t)0x01			//
#define USB0_USBFRMADJUST       *(volatile uint8_t  *)0x40072114 // Frame Adjust Register

// Chapter 41: USB Device Charger Detection Module (USBDCD)
#define USBDCD_CONTROL          *(volatile uint32_t *)0x40035000 // Control register
#define USBDCD_CLOCK            *(volatile uint32_t *)0x40035004 // Clock register
#define USBDCD_STATUS           *(volatile uint32_t *)0x40035008 // Status register
#define USBDCD_TIMER0           *(volatile uint32_t *)0x40035010 // TIMER0 register
#define USBDCD_TIMER1           *(volatile uint32_t *)0x40035014 // TIMER1 register
#define USBDCD_TIMER2           *(volatile uint32_t *)0x40035018 // TIMER2 register

// Chapter 43: SPI (DSPI)
#define SPI0_MCR                *(volatile uint32_t *)0x4002C000 // DSPI Module Configuration Register
#define SPI_MCR_MSTR			(uint32_t)0x80000000		// Master/Slave Mode Select
#define SPI_MCR_CONT_SCKE		(uint32_t)0x40000000		// 
#define SPI_MCR_DCONF(n)		(((n) & 3) << 28)		// 
#define SPI_MCR_FRZ			(uint32_t)0x08000000		// 
#define SPI_MCR_MTFE			(uint32_t)0x04000000		// 
#define SPI_MCR_ROOE			(uint32_t)0x01000000		// 
#define SPI_MCR_PCSIS(n)		(((n) & 0x1F) << 16)		//
#define SPI_MCR_DOZE			(uint32_t)0x00008000		// 
#define SPI_MCR_MDIS			(uint32_t)0x00004000		// 
#define SPI_MCR_DIS_TXF			(uint32_t)0x00002000		// 
#define SPI_MCR_DIS_RXF			(uint32_t)0x00001000		// 
#define SPI_MCR_CLR_TXF			(uint32_t)0x00000800		// 
#define SPI_MCR_CLR_RXF			(uint32_t)0x00000400		// 
#define SPI_MCR_SMPL_PT(n)		(((n) & 3) << 8)		//
#define SPI_MCR_HALT			(uint32_t)0x00000001		// 
#define SPI0_TCR                *(volatile uint32_t *)0x4002C008 // DSPI Transfer Count Register
#define SPI0_CTAR0              *(volatile uint32_t *)0x4002C00C // DSPI Clock and Transfer Attributes Register, In Master Mode
#define SPI_CTAR_DBR			(uint32_t)0x80000000		// Double Baud Rate
#define SPI_CTAR_FMSZ(n)		(((n) & 15) << 27)		// Frame Size (+1)
#define SPI_CTAR_CPOL			(uint32_t)0x04000000		// Clock Polarity
#define SPI_CTAR_CPHA			(uint32_t)0x02000000		// Clock Phase
#define SPI_CTAR_LSBFE			(uint32_t)0x01000000		// LSB First
#define SPI_CTAR_PCSSCK(n)		(((n) & 3) << 22)		// PCS to SCK Delay Prescaler
#define SPI_CTAR_PASC(n)		(((n) & 3) << 20)		// After SCK Delay Prescaler
#define SPI_CTAR_PDT(n)			(((n) & 3) << 18)		// Delay after Transfer Prescaler
#define SPI_CTAR_PBR(n)			(((n) & 3) << 16)		// Baud Rate Prescaler
#define SPI_CTAR_CSSCK(n)		(((n) & 15) << 12)		// PCS to SCK Delay Scaler
#define SPI_CTAR_ASC(n)			(((n) & 15) << 8)		// After SCK Delay Scaler
#define SPI_CTAR_DT(n)			(((n) & 15) << 4)		// Delay After Transfer Scaler
#define SPI_CTAR_BR(n)			(((n) & 15) << 0)		// Baud Rate Scaler
#define SPI0_CTAR0_SLAVE        *(volatile uint32_t *)0x4002C00C // DSPI Clock and Transfer Attributes Register, In Slave Mode
#define SPI0_CTAR1              *(volatile uint32_t *)0x4002C010 // DSPI Clock and Transfer Attributes Register, In Master Mode
#define SPI0_SR                 *(volatile uint32_t *)0x4002C02C // DSPI Status Register
#define SPI_SR_TCF			(uint32_t)0x80000000		// Transfer Complete Flag
#define SPI_SR_TXRXS			(uint32_t)0x40000000		// TX and RX Status
#define SPI_SR_EOQF			(uint32_t)0x10000000		// End of Queue Flag
#define SPI_SR_TFUF			(uint32_t)0x08000000		// Transmit FIFO Underflow Flag
#define SPI_SR_TFFF			(uint32_t)0x02000000		// Transmit FIFO Fill Flag
#define SPI_SR_RFOF			(uint32_t)0x00080000		// Receive FIFO Overflow Flag
#define SPI_SR_RFDF			(uint32_t)0x00020000		// Receive FIFO Drain Flag
#define SPI0_RSER               *(volatile uint32_t *)0x4002C030 // DSPI DMA/Interrupt Request Select and Enable Register
#define SPI0_PUSHR              *(volatile uint32_t *)0x4002C034 // DSPI PUSH TX FIFO Register In Master Mode
#define SPI0_PUSHR_CONT			(uint32_t)0x80000000		// 
#define SPI0_PUSHR_CTAS(n)		(((n) & 7) << 28)		// 
#define SPI0_PUSHR_EOQ			(uint32_t)0x08000000		// 
#define SPI0_PUSHR_CTCNT		(uint32_t)0x04000000		// 
#define SPI0_PUSHR_PCS(n)		(((n) & 31) << 16)		//
#define SPI0_PUSHR_SLAVE        *(volatile uint32_t *)0x4002C034 // DSPI PUSH TX FIFO Register In Slave Mode
#define SPI0_POPR               *(volatile uint32_t *)0x4002C038 // DSPI POP RX FIFO Register
#define SPI0_TXFR0              *(volatile uint32_t *)0x4002C03C // DSPI Transmit FIFO Registers
#define SPI0_TXFR1              *(volatile uint32_t *)0x4002C040 // DSPI Transmit FIFO Registers
#define SPI0_TXFR2              *(volatile uint32_t *)0x4002C044 // DSPI Transmit FIFO Registers
#define SPI0_TXFR3              *(volatile uint32_t *)0x4002C048 // DSPI Transmit FIFO Registers
#define SPI0_RXFR0              *(volatile uint32_t *)0x4002C07C // DSPI Receive FIFO Registers
#define SPI0_RXFR1              *(volatile uint32_t *)0x4002C080 // DSPI Receive FIFO Registers
#define SPI0_RXFR2              *(volatile uint32_t *)0x4002C084 // DSPI Receive FIFO Registers
#define SPI0_RXFR3              *(volatile uint32_t *)0x4002C088 // DSPI Receive FIFO Registers

// Chapter 44: Inter-Integrated Circuit (I2C)
#define I2C0_A1                 *(volatile uint8_t  *)0x40066000 // I2C Address Register 1
#define I2C0_F                  *(volatile uint8_t  *)0x40066001 // I2C Frequency Divider register
#define I2C0_C1                 *(volatile uint8_t  *)0x40066002 // I2C Control Register 1
#define I2C_C1_IICEN			(uint8_t)0x80			// I2C Enable
#define I2C_C1_IICIE			(uint8_t)0x40			// I2C Interrupt Enable
#define I2C_C1_MST			(uint8_t)0x20			// Master Mode Select
#define I2C_C1_TX			(uint8_t)0x10			// Transmit Mode Select
#define I2C_C1_TXAK			(uint8_t)0x08			// Transmit Acknowledge Enable
#define I2C_C1_RSTA			(uint8_t)0x04			// Repeat START
#define I2C_C1_WUEN			(uint8_t)0x02			// Wakeup Enable
#define I2C_C1_DMAEN			(uint8_t)0x01			// DMA Enable
#define I2C0_S                  *(volatile uint8_t  *)0x40066003 // I2C Status register
#define I2C_S_TCF			(uint8_t)0x80			// Transfer Complete Flag
#define I2C_S_IAAS			(uint8_t)0x40			// Addressed As A Slave
#define I2C_S_BUSY			(uint8_t)0x20			// Bus Busy
#define I2C_S_ARBL			(uint8_t)0x10			// Arbitration Lost
#define I2C_S_RAM			(uint8_t)0x08			// Range Address Match
#define I2C_S_SRW			(uint8_t)0x04			// Slave Read/Write
#define I2C_S_IICIF			(uint8_t)0x02			// Interrupt Flag
#define I2C_S_RXAK			(uint8_t)0x01			// Receive Acknowledge
#define I2C0_D                  *(volatile uint8_t  *)0x40066004 // I2C Data I/O register
#define I2C0_C2                 *(volatile uint8_t  *)0x40066005 // I2C Control Register 2
#define I2C_C2_GCAEN			(uint8_t)0x80			// General Call Address Enable
#define I2C_C2_ADEXT			(uint8_t)0x40			// Address Extension
#define I2C_C2_HDRS			(uint8_t)0x20			// High Drive Select
#define I2C_C2_SBRC			(uint8_t)0x10			// Slave Baud Rate Control
#define I2C_C2_RMEN			(uint8_t)0x08			// Range Address Matching Enable
#define I2C_C2_AD(n)			((n) & 7)			// Slave Address, upper 3 bits
#define I2C0_FLT                *(volatile uint8_t  *)0x40066006 // I2C Programmable Input Glitch Filter register
#define I2C0_RA                 *(volatile uint8_t  *)0x40066007 // I2C Range Address register
#define I2C0_SMB                *(volatile uint8_t  *)0x40066008 // I2C SMBus Control and Status register
#define I2C0_A2                 *(volatile uint8_t  *)0x40066009 // I2C Address Register 2
#define I2C0_SLTH               *(volatile uint8_t  *)0x4006600A // I2C SCL Low Timeout Register High
#define I2C0_SLTL               *(volatile uint8_t  *)0x4006600B // I2C SCL Low Timeout Register Low

// Chapter 45: Universal Asynchronous Receiver/Transmitter (UART)
#define UART0_BDH               *(volatile uint8_t  *)0x4006A000 // UART Baud Rate Registers: High
#define UART0_BDL               *(volatile uint8_t  *)0x4006A001 // UART Baud Rate Registers: Low
#define UART0_C1                *(volatile uint8_t  *)0x4006A002 // UART Control Register 1
#define UART_C1_LOOPS			(uint8_t)0x80			// When LOOPS is set, the RxD pin is disconnected from the UART and the transmitter output is internally connected to the receiver input
#define UART_C1_UARTSWAI		(uint8_t)0x40			// UART Stops in Wait Mode
#define UART_C1_RSRC			(uint8_t)0x20			// When LOOPS is set, the RSRC field determines the source for the receiver shift register input
#define UART_C1_M			(uint8_t)0x10			// 9-bit or 8-bit Mode Select
#define UART_C1_WAKE			(uint8_t)0x08			// Determines which condition wakes the UART
#define UART_C1_ILT			(uint8_t)0x04			// Idle Line Type Select
#define UART_C1_PE			(uint8_t)0x02			// Parity Enable
#define UART_C1_PT			(uint8_t)0x01			// Parity Type, 0=even, 1=odd
#define UART0_C2                *(volatile uint8_t  *)0x4006A003 // UART Control Register 2
#define UART_C2_TIE			(uint8_t)0x80			// Transmitter Interrupt or DMA Transfer Enable.
#define UART_C2_TCIE			(uint8_t)0x40			// Transmission Complete Interrupt Enable
#define UART_C2_RIE			(uint8_t)0x20			// Receiver Full Interrupt or DMA Transfer Enable
#define UART_C2_ILIE			(uint8_t)0x10			// Idle Line Interrupt Enable
#define UART_C2_TE			(uint8_t)0x08			// Transmitter Enable
#define UART_C2_RE			(uint8_t)0x04			// Receiver Enable
#define UART_C2_RWU			(uint8_t)0x02			// Receiver Wakeup Control
#define UART_C2_SBK			(uint8_t)0x01			// Send Break
#define UART0_S1                *(volatile uint8_t  *)0x4006A004 // UART Status Register 1
#define UART_S1_TDRE			(uint8_t)0x80			// Transmit Data Register Empty Flag
#define UART_S1_TC			(uint8_t)0x40			// Transmit Complete Flag
#define UART_S1_RDRF			(uint8_t)0x20			// Receive Data Register Full Flag
#define UART_S1_IDLE			(uint8_t)0x10			// Idle Line Flag
#define UART_S1_OR			(uint8_t)0x08			// Receiver Overrun Flag
#define UART_S1_NF			(uint8_t)0x04			// Noise Flag
#define UART_S1_FE			(uint8_t)0x02			// Framing Error Flag
#define UART_S1_PF			(uint8_t)0x01			// Parity Error Flag
#define UART0_S2                *(volatile uint8_t  *)0x4006A005 // UART Status Register 2
#define UART_S2_RXINV			(uint8_t)0x10			// RX Line Inversion Enable
#define UART0_C3                *(volatile uint8_t  *)0x4006A006 // UART Control Register 3
#define UART_S2_TXINV			(uint8_t)0x10			// TX Line Inversion Enable
#define UART0_D                 *(volatile uint8_t  *)0x4006A007 // UART Data Register
#define UART0_MA1               *(volatile uint8_t  *)0x4006A008 // UART Match Address Registers 1
#define UART0_MA2               *(volatile uint8_t  *)0x4006A009 // UART Match Address Registers 2
#define UART0_C4                *(volatile uint8_t  *)0x4006A00A // UART Control Register 4
#define UART0_C5                *(volatile uint8_t  *)0x4006A00B // UART Control Register 5
#define UART0_ED                *(volatile uint8_t  *)0x4006A00C // UART Extended Data Register
#define UART0_MODEM             *(volatile uint8_t  *)0x4006A00D // UART Modem Register
#define UART0_IR                *(volatile uint8_t  *)0x4006A00E // UART Infrared Register
#define UART0_PFIFO             *(volatile uint8_t  *)0x4006A010 // UART FIFO Parameters
#define UART_PFIFO_TXFE			(uint8_t)0x80
#define UART_PFIFO_RXFE			(uint8_t)0x08
#define UART0_CFIFO             *(volatile uint8_t  *)0x4006A011 // UART FIFO Control Register
#define UART_CFIFO_TXFLUSH		(uint8_t)0x80			// 
#define UART_CFIFO_RXFLUSH		(uint8_t)0x40			// 
#define UART_CFIFO_RXOFE		(uint8_t)0x04			// 
#define UART_CFIFO_TXOFE		(uint8_t)0x02			// 
#define UART_CFIFO_RXUFE		(uint8_t)0x01			// 
#define UART0_SFIFO             *(volatile uint8_t  *)0x4006A012 // UART FIFO Status Register
#define UART_SFIFO_TXEMPT		(uint8_t)0x80
#define UART_SFIFO_RXEMPT		(uint8_t)0x40
#define UART_SFIFO_RXOF			(uint8_t)0x04
#define UART_SFIFO_TXOF			(uint8_t)0x02
#define UART_SFIFO_RXUF			(uint8_t)0x01
#define UART0_TWFIFO            *(volatile uint8_t  *)0x4006A013 // UART FIFO Transmit Watermark
#define UART0_TCFIFO            *(volatile uint8_t  *)0x4006A014 // UART FIFO Transmit Count
#define UART0_RWFIFO            *(volatile uint8_t  *)0x4006A015 // UART FIFO Receive Watermark
#define UART0_RCFIFO            *(volatile uint8_t  *)0x4006A016 // UART FIFO Receive Count
#define UART0_C7816             *(volatile uint8_t  *)0x4006A018 // UART 7816 Control Register
#define UART0_IE7816            *(volatile uint8_t  *)0x4006A019 // UART 7816 Interrupt Enable Register
#define UART0_IS7816            *(volatile uint8_t  *)0x4006A01A // UART 7816 Interrupt Status Register
#define UART0_WP7816T0          *(volatile uint8_t  *)0x4006A01B // UART 7816 Wait Parameter Register
#define UART0_WP7816T1          *(volatile uint8_t  *)0x4006A01B // UART 7816 Wait Parameter Register
#define UART0_WN7816            *(volatile uint8_t  *)0x4006A01C // UART 7816 Wait N Register
#define UART0_WF7816            *(volatile uint8_t  *)0x4006A01D // UART 7816 Wait FD Register
#define UART0_ET7816            *(volatile uint8_t  *)0x4006A01E // UART 7816 Error Threshold Register
#define UART0_TL7816            *(volatile uint8_t  *)0x4006A01F // UART 7816 Transmit Length Register
#define UART0_C6                *(volatile uint8_t  *)0x4006A021 // UART CEA709.1-B Control Register 6
#define UART0_PCTH              *(volatile uint8_t  *)0x4006A022 // UART CEA709.1-B Packet Cycle Time Counter High
#define UART0_PCTL              *(volatile uint8_t  *)0x4006A023 // UART CEA709.1-B Packet Cycle Time Counter Low
#define UART0_B1T               *(volatile uint8_t  *)0x4006A024 // UART CEA709.1-B Beta1 Timer
#define UART0_SDTH              *(volatile uint8_t  *)0x4006A025 // UART CEA709.1-B Secondary Delay Timer High
#define UART0_SDTL              *(volatile uint8_t  *)0x4006A026 // UART CEA709.1-B Secondary Delay Timer Low
#define UART0_PRE               *(volatile uint8_t  *)0x4006A027 // UART CEA709.1-B Preamble
#define UART0_TPL               *(volatile uint8_t  *)0x4006A028 // UART CEA709.1-B Transmit Packet Length
#define UART0_IE                *(volatile uint8_t  *)0x4006A029 // UART CEA709.1-B Interrupt Enable Register
#define UART0_WB                *(volatile uint8_t  *)0x4006A02A // UART CEA709.1-B WBASE
#define UART0_S3                *(volatile uint8_t  *)0x4006A02B // UART CEA709.1-B Status Register
#define UART0_S4                *(volatile uint8_t  *)0x4006A02C // UART CEA709.1-B Status Register
#define UART0_RPL               *(volatile uint8_t  *)0x4006A02D // UART CEA709.1-B Received Packet Length
#define UART0_RPREL             *(volatile uint8_t  *)0x4006A02E // UART CEA709.1-B Received Preamble Length
#define UART0_CPW               *(volatile uint8_t  *)0x4006A02F // UART CEA709.1-B Collision Pulse Width
#define UART0_RIDT              *(volatile uint8_t  *)0x4006A030 // UART CEA709.1-B Receive Indeterminate Time
#define UART0_TIDT              *(volatile uint8_t  *)0x4006A031 // UART CEA709.1-B Transmit Indeterminate Time
#define UART1_BDH               *(volatile uint8_t  *)0x4006B000 // UART Baud Rate Registers: High
#define UART1_BDL               *(volatile uint8_t  *)0x4006B001 // UART Baud Rate Registers: Low
#define UART1_C1                *(volatile uint8_t  *)0x4006B002 // UART Control Register 1
#define UART1_C2                *(volatile uint8_t  *)0x4006B003 // UART Control Register 2
#define UART1_S1                *(volatile uint8_t  *)0x4006B004 // UART Status Register 1
#define UART1_S2                *(volatile uint8_t  *)0x4006B005 // UART Status Register 2
#define UART1_C3                *(volatile uint8_t  *)0x4006B006 // UART Control Register 3
#define UART1_D                 *(volatile uint8_t  *)0x4006B007 // UART Data Register
#define UART1_MA1               *(volatile uint8_t  *)0x4006B008 // UART Match Address Registers 1
#define UART1_MA2               *(volatile uint8_t  *)0x4006B009 // UART Match Address Registers 2
#define UART1_C4                *(volatile uint8_t  *)0x4006B00A // UART Control Register 4
#define UART1_C5                *(volatile uint8_t  *)0x4006B00B // UART Control Register 5
#define UART1_ED                *(volatile uint8_t  *)0x4006B00C // UART Extended Data Register
#define UART1_MODEM             *(volatile uint8_t  *)0x4006B00D // UART Modem Register
#define UART1_IR                *(volatile uint8_t  *)0x4006B00E // UART Infrared Register
#define UART1_PFIFO             *(volatile uint8_t  *)0x4006B010 // UART FIFO Parameters
#define UART1_CFIFO             *(volatile uint8_t  *)0x4006B011 // UART FIFO Control Register
#define UART1_SFIFO             *(volatile uint8_t  *)0x4006B012 // UART FIFO Status Register
#define UART1_TWFIFO            *(volatile uint8_t  *)0x4006B013 // UART FIFO Transmit Watermark
#define UART1_TCFIFO            *(volatile uint8_t  *)0x4006B014 // UART FIFO Transmit Count
#define UART1_RWFIFO            *(volatile uint8_t  *)0x4006B015 // UART FIFO Receive Watermark
#define UART1_RCFIFO            *(volatile uint8_t  *)0x4006B016 // UART FIFO Receive Count
#define UART1_C7816             *(volatile uint8_t  *)0x4006B018 // UART 7816 Control Register
#define UART1_IE7816            *(volatile uint8_t  *)0x4006B019 // UART 7816 Interrupt Enable Register
#define UART1_IS7816            *(volatile uint8_t  *)0x4006B01A // UART 7816 Interrupt Status Register
#define UART1_WP7816T0          *(volatile uint8_t  *)0x4006B01B // UART 7816 Wait Parameter Register
#define UART1_WP7816T1          *(volatile uint8_t  *)0x4006B01B // UART 7816 Wait Parameter Register
#define UART1_WN7816            *(volatile uint8_t  *)0x4006B01C // UART 7816 Wait N Register
#define UART1_WF7816            *(volatile uint8_t  *)0x4006B01D // UART 7816 Wait FD Register
#define UART1_ET7816            *(volatile uint8_t  *)0x4006B01E // UART 7816 Error Threshold Register
#define UART1_TL7816            *(volatile uint8_t  *)0x4006B01F // UART 7816 Transmit Length Register
#define UART1_C6                *(volatile uint8_t  *)0x4006B021 // UART CEA709.1-B Control Register 6
#define UART1_PCTH              *(volatile uint8_t  *)0x4006B022 // UART CEA709.1-B Packet Cycle Time Counter High
#define UART1_PCTL              *(volatile uint8_t  *)0x4006B023 // UART CEA709.1-B Packet Cycle Time Counter Low
#define UART1_B1T               *(volatile uint8_t  *)0x4006B024 // UART CEA709.1-B Beta1 Timer
#define UART1_SDTH              *(volatile uint8_t  *)0x4006B025 // UART CEA709.1-B Secondary Delay Timer High
#define UART1_SDTL              *(volatile uint8_t  *)0x4006B026 // UART CEA709.1-B Secondary Delay Timer Low
#define UART1_PRE               *(volatile uint8_t  *)0x4006B027 // UART CEA709.1-B Preamble
#define UART1_TPL               *(volatile uint8_t  *)0x4006B028 // UART CEA709.1-B Transmit Packet Length
#define UART1_IE                *(volatile uint8_t  *)0x4006B029 // UART CEA709.1-B Interrupt Enable Register
#define UART1_WB                *(volatile uint8_t  *)0x4006B02A // UART CEA709.1-B WBASE
#define UART1_S3                *(volatile uint8_t  *)0x4006B02B // UART CEA709.1-B Status Register
#define UART1_S4                *(volatile uint8_t  *)0x4006B02C // UART CEA709.1-B Status Register
#define UART1_RPL               *(volatile uint8_t  *)0x4006B02D // UART CEA709.1-B Received Packet Length
#define UART1_RPREL             *(volatile uint8_t  *)0x4006B02E // UART CEA709.1-B Received Preamble Length
#define UART1_CPW               *(volatile uint8_t  *)0x4006B02F // UART CEA709.1-B Collision Pulse Width
#define UART1_RIDT              *(volatile uint8_t  *)0x4006B030 // UART CEA709.1-B Receive Indeterminate Time
#define UART1_TIDT              *(volatile uint8_t  *)0x4006B031 // UART CEA709.1-B Transmit Indeterminate Time
#define UART2_BDH               *(volatile uint8_t  *)0x4006C000 // UART Baud Rate Registers: High
#define UART2_BDL               *(volatile uint8_t  *)0x4006C001 // UART Baud Rate Registers: Low
#define UART2_C1                *(volatile uint8_t  *)0x4006C002 // UART Control Register 1
#define UART2_C2                *(volatile uint8_t  *)0x4006C003 // UART Control Register 2
#define UART2_S1                *(volatile uint8_t  *)0x4006C004 // UART Status Register 1
#define UART2_S2                *(volatile uint8_t  *)0x4006C005 // UART Status Register 2
#define UART2_C3                *(volatile uint8_t  *)0x4006C006 // UART Control Register 3
#define UART2_D                 *(volatile uint8_t  *)0x4006C007 // UART Data Register
#define UART2_MA1               *(volatile uint8_t  *)0x4006C008 // UART Match Address Registers 1
#define UART2_MA2               *(volatile uint8_t  *)0x4006C009 // UART Match Address Registers 2
#define UART2_C4                *(volatile uint8_t  *)0x4006C00A // UART Control Register 4
#define UART2_C5                *(volatile uint8_t  *)0x4006C00B // UART Control Register 5
#define UART2_ED                *(volatile uint8_t  *)0x4006C00C // UART Extended Data Register
#define UART2_MODEM             *(volatile uint8_t  *)0x4006C00D // UART Modem Register
#define UART2_IR                *(volatile uint8_t  *)0x4006C00E // UART Infrared Register
#define UART2_PFIFO             *(volatile uint8_t  *)0x4006C010 // UART FIFO Parameters
#define UART2_CFIFO             *(volatile uint8_t  *)0x4006C011 // UART FIFO Control Register
#define UART2_SFIFO             *(volatile uint8_t  *)0x4006C012 // UART FIFO Status Register
#define UART2_TWFIFO            *(volatile uint8_t  *)0x4006C013 // UART FIFO Transmit Watermark
#define UART2_TCFIFO            *(volatile uint8_t  *)0x4006C014 // UART FIFO Transmit Count
#define UART2_RWFIFO            *(volatile uint8_t  *)0x4006C015 // UART FIFO Receive Watermark
#define UART2_RCFIFO            *(volatile uint8_t  *)0x4006C016 // UART FIFO Receive Count
#define UART2_C7816             *(volatile uint8_t  *)0x4006C018 // UART 7816 Control Register
#define UART2_IE7816            *(volatile uint8_t  *)0x4006C019 // UART 7816 Interrupt Enable Register
#define UART2_IS7816            *(volatile uint8_t  *)0x4006C01A // UART 7816 Interrupt Status Register
#define UART2_WP7816T0          *(volatile uint8_t  *)0x4006C01B // UART 7816 Wait Parameter Register
#define UART2_WP7816T1          *(volatile uint8_t  *)0x4006C01B // UART 7816 Wait Parameter Register
#define UART2_WN7816            *(volatile uint8_t  *)0x4006C01C // UART 7816 Wait N Register
#define UART2_WF7816            *(volatile uint8_t  *)0x4006C01D // UART 7816 Wait FD Register
#define UART2_ET7816            *(volatile uint8_t  *)0x4006C01E // UART 7816 Error Threshold Register
#define UART2_TL7816            *(volatile uint8_t  *)0x4006C01F // UART 7816 Transmit Length Register
#define UART2_C6                *(volatile uint8_t  *)0x4006C021 // UART CEA709.1-B Control Register 6
#define UART2_PCTH              *(volatile uint8_t  *)0x4006C022 // UART CEA709.1-B Packet Cycle Time Counter High
#define UART2_PCTL              *(volatile uint8_t  *)0x4006C023 // UART CEA709.1-B Packet Cycle Time Counter Low
#define UART2_B1T               *(volatile uint8_t  *)0x4006C024 // UART CEA709.1-B Beta1 Timer
#define UART2_SDTH              *(volatile uint8_t  *)0x4006C025 // UART CEA709.1-B Secondary Delay Timer High
#define UART2_SDTL              *(volatile uint8_t  *)0x4006C026 // UART CEA709.1-B Secondary Delay Timer Low
#define UART2_PRE               *(volatile uint8_t  *)0x4006C027 // UART CEA709.1-B Preamble
#define UART2_TPL               *(volatile uint8_t  *)0x4006C028 // UART CEA709.1-B Transmit Packet Length
#define UART2_IE                *(volatile uint8_t  *)0x4006C029 // UART CEA709.1-B Interrupt Enable Register
#define UART2_WB                *(volatile uint8_t  *)0x4006C02A // UART CEA709.1-B WBASE
#define UART2_S3                *(volatile uint8_t  *)0x4006C02B // UART CEA709.1-B Status Register
#define UART2_S4                *(volatile uint8_t  *)0x4006C02C // UART CEA709.1-B Status Register
#define UART2_RPL               *(volatile uint8_t  *)0x4006C02D // UART CEA709.1-B Received Packet Length
#define UART2_RPREL             *(volatile uint8_t  *)0x4006C02E // UART CEA709.1-B Received Preamble Length
#define UART2_CPW               *(volatile uint8_t  *)0x4006C02F // UART CEA709.1-B Collision Pulse Width
#define UART2_RIDT              *(volatile uint8_t  *)0x4006C030 // UART CEA709.1-B Receive Indeterminate Time
#define UART2_TIDT              *(volatile uint8_t  *)0x4006C031 // UART CEA709.1-B Transmit Indeterminate Time

// Chapter 46: Synchronous Audio Interface (SAI)
#define I2S0_TCSR               *(volatile uint32_t *)0x4002F000 // SAI Transmit Control Register
#define I2S0_TCR1               *(volatile uint32_t *)0x4002F004 // SAI Transmit Configuration 1 Register
#define I2S0_TCR2               *(volatile uint32_t *)0x4002F008 // SAI Transmit Configuration 2 Register
#define I2S0_TCR3               *(volatile uint32_t *)0x4002F00C // SAI Transmit Configuration 3 Register
#define I2S0_TCR4               *(volatile uint32_t *)0x4002F010 // SAI Transmit Configuration 4 Register
#define I2S0_TCR5               *(volatile uint32_t *)0x4002F014 // SAI Transmit Configuration 5 Register
#define I2S0_TDR0               *(volatile uint32_t *)0x4002F020 // SAI Transmit Data Register
#define I2S0_TFR0               *(volatile uint32_t *)0x4002F040 // SAI Transmit FIFO Register
#define I2S0_TMR                *(volatile uint32_t *)0x4002F060 // SAI Transmit Mask Register
#define I2S0_RCSR               *(volatile uint32_t *)0x4002F080 // SAI Receive Control Register
#define I2S0_RCR1               *(volatile uint32_t *)0x4002F084 // SAI Receive Configuration 1 Register
#define I2S0_RCR2               *(volatile uint32_t *)0x4002F088 // SAI Receive Configuration 2 Register
#define I2S0_RCR3               *(volatile uint32_t *)0x4002F08C // SAI Receive Configuration 3 Register
#define I2S0_RCR4               *(volatile uint32_t *)0x4002F090 // SAI Receive Configuration 4 Register
#define I2S0_RCR5               *(volatile uint32_t *)0x4002F094 // SAI Receive Configuration 5 Register
#define I2S0_RDR0               *(volatile uint32_t *)0x4002F0A0 // SAI Receive Data Register
#define I2S0_RFR0               *(volatile uint32_t *)0x4002F0C0 // SAI Receive FIFO Register
#define I2S0_RMR                *(volatile uint32_t *)0x4002F0E0 // SAI Receive Mask Register
#define I2S0_MCR                *(volatile uint32_t *)0x4002F100 // SAI MCLK Control Register
#define I2S0_MDR                *(volatile uint32_t *)0x4002F104 // SAI MCLK Divide Register

// Chapter 47: General-Purpose Input/Output (GPIO)
#define GPIOA_PDOR              *(volatile uint32_t *)0x400FF000 // Port Data Output Register
#define GPIOA_PSOR              *(volatile uint32_t *)0x400FF004 // Port Set Output Register
#define GPIOA_PCOR              *(volatile uint32_t *)0x400FF008 // Port Clear Output Register
#define GPIOA_PTOR              *(volatile uint32_t *)0x400FF00C // Port Toggle Output Register
#define GPIOA_PDIR              *(volatile uint32_t *)0x400FF010 // Port Data Input Register
#define GPIOA_PDDR              *(volatile uint32_t *)0x400FF014 // Port Data Direction Register
#define GPIOB_PDOR              *(volatile uint32_t *)0x400FF040 // Port Data Output Register
#define GPIOB_PSOR              *(volatile uint32_t *)0x400FF044 // Port Set Output Register
#define GPIOB_PCOR              *(volatile uint32_t *)0x400FF048 // Port Clear Output Register
#define GPIOB_PTOR              *(volatile uint32_t *)0x400FF04C // Port Toggle Output Register
#define GPIOB_PDIR              *(volatile uint32_t *)0x400FF050 // Port Data Input Register
#define GPIOB_PDDR              *(volatile uint32_t *)0x400FF054 // Port Data Direction Register
#define GPIOC_PDOR              *(volatile uint32_t *)0x400FF080 // Port Data Output Register
#define GPIOC_PSOR              *(volatile uint32_t *)0x400FF084 // Port Set Output Register
#define GPIOC_PCOR              *(volatile uint32_t *)0x400FF088 // Port Clear Output Register
#define GPIOC_PTOR              *(volatile uint32_t *)0x400FF08C // Port Toggle Output Register
#define GPIOC_PDIR              *(volatile uint32_t *)0x400FF090 // Port Data Input Register
#define GPIOC_PDDR              *(volatile uint32_t *)0x400FF094 // Port Data Direction Register
#define GPIOD_PDOR              *(volatile uint32_t *)0x400FF0C0 // Port Data Output Register
#define GPIOD_PSOR              *(volatile uint32_t *)0x400FF0C4 // Port Set Output Register
#define GPIOD_PCOR              *(volatile uint32_t *)0x400FF0C8 // Port Clear Output Register
#define GPIOD_PTOR              *(volatile uint32_t *)0x400FF0CC // Port Toggle Output Register
#define GPIOD_PDIR              *(volatile uint32_t *)0x400FF0D0 // Port Data Input Register
#define GPIOD_PDDR              *(volatile uint32_t *)0x400FF0D4 // Port Data Direction Register
#define GPIOE_PDOR              *(volatile uint32_t *)0x400FF100 // Port Data Output Register
#define GPIOE_PSOR              *(volatile uint32_t *)0x400FF104 // Port Set Output Register
#define GPIOE_PCOR              *(volatile uint32_t *)0x400FF108 // Port Clear Output Register
#define GPIOE_PTOR              *(volatile uint32_t *)0x400FF10C // Port Toggle Output Register
#define GPIOE_PDIR              *(volatile uint32_t *)0x400FF110 // Port Data Input Register
#define GPIOE_PDDR              *(volatile uint32_t *)0x400FF114 // Port Data Direction Register

// Chapter 48: Touch sense input (TSI)
#define TSI0_GENCS              *(volatile uint32_t *)0x40045000 // General Control and Status Register
#define TSI_GENCS_LPCLKS		(uint32_t)0x10000000		// 
#define TSI_GENCS_LPSCNITV(n)		(((n) & 15) << 24)		// 
#define TSI_GENCS_NSCN(n)		(((n) & 31) << 19)		// 
#define TSI_GENCS_PS(n)			(((n) & 7) << 16)		// 
#define TSI_GENCS_EOSF			(uint32_t)0x00008000		// 
#define TSI_GENCS_OUTRGF		(uint32_t)0x00004000		// 
#define TSI_GENCS_EXTERF		(uint32_t)0x00002000		// 
#define TSI_GENCS_OVRF			(uint32_t)0x00001000		// 
#define TSI_GENCS_SCNIP			(uint32_t)0x00000200		// 
#define TSI_GENCS_SWTS			(uint32_t)0x00000100		// 
#define TSI_GENCS_TSIEN			(uint32_t)0x00000080		// 
#define TSI_GENCS_TSIIE			(uint32_t)0x00000040		// 
#define TSI_GENCS_ERIE			(uint32_t)0x00000020		// 
#define TSI_GENCS_ESOR			(uint32_t)0x00000010		// 
#define TSI_GENCS_STM			(uint32_t)0x00000002		// 
#define TSI_GENCS_STPE			(uint32_t)0x00000001		// 
#define TSI0_SCANC              *(volatile uint32_t *)0x40045004 // SCAN Control Register
#define TSI_SCANC_REFCHRG(n)		(((n) & 15) << 24)		// 
#define TSI_SCANC_EXTCHRG(n)		(((n) & 7) << 16)		// 
#define TSI_SCANC_SMOD(n)		(((n) & 255) << 8)		// 
#define TSI_SCANC_AMCLKS(n)		(((n) & 3) << 3)		// 
#define TSI_SCANC_AMPSC(n)		(((n) & 7) << 0)		// 
#define TSI0_PEN                *(volatile uint32_t *)0x40045008 // Pin Enable Register
#define TSI0_WUCNTR             *(volatile uint32_t *)0x4004500C // Wake-Up Channel Counter Register
#define TSI0_CNTR1              *(volatile uint32_t *)0x40045100 // Counter Register
#define TSI0_CNTR3              *(volatile uint32_t *)0x40045104 // Counter Register
#define TSI0_CNTR5              *(volatile uint32_t *)0x40045108 // Counter Register
#define TSI0_CNTR7              *(volatile uint32_t *)0x4004510C // Counter Register
#define TSI0_CNTR9              *(volatile uint32_t *)0x40045110 // Counter Register
#define TSI0_CNTR11             *(volatile uint32_t *)0x40045114 // Counter Register
#define TSI0_CNTR13             *(volatile uint32_t *)0x40045118 // Counter Register
#define TSI0_CNTR15             *(volatile uint32_t *)0x4004511C // Counter Register
#define TSI0_THRESHOLD          *(volatile uint32_t *)0x40045120 // Low Power Channel Threshold Register

// Nested Vectored Interrupt Controller, Table 3-4 & ARMv7 ref, appendix B3.4 (page 750)
#define NVIC_ENABLE_IRQ(n)	(*((volatile uint32_t *)0xE000E100 + (n >> 5)) = (1 << (n & 31)))
#define NVIC_DISABLE_IRQ(n)	(*((volatile uint32_t *)0xE000E180 + (n >> 5)) = (1 << (n & 31)))
#define NVIC_SET_PENDING(n)	(*((volatile uint32_t *)0xE000E200 + (n >> 5)) = (1 << (n & 31)))
#define NVIC_CLEAR_PENDING(n)	(*((volatile uint32_t *)0xE000E280 + (n >> 5)) = (1 << (n & 31)))

#define NVIC_ISER0		*(volatile uint32_t *)0xE000E100
#define NVIC_ISER1		*(volatile uint32_t *)0xE000E104
#define NVIC_ICER0		*(volatile uint32_t *)0xE000E180
#define NVIC_ICER1		*(volatile uint32_t *)0xE000E184

//#define NVIC_SET_PRIORITY(n, p)	
#define IRQ_DMA_CH0		0
#define IRQ_DMA_CH1		1
#define IRQ_DMA_CH2		2
#define IRQ_DMA_CH3		3
#define IRQ_DMA_ERROR		4
#define IRQ_FTFL_COMPLETE	6
#define IRQ_FTFL_COLLISION	7
#define IRQ_LOW_VOLTAGE		8
#define IRQ_LLWU		9
#define IRQ_WDOG		10
#define IRQ_I2C0		11
#define IRQ_SPI0		12
#define IRQ_I2S0_TX		13
#define IRQ_I2S0_RX		14
#define IRQ_UART0_LON		15
#define IRQ_UART0_STATUS	16
#define IRQ_UART0_ERROR		17
#define IRQ_UART1_STATUS	18
#define IRQ_UART1_ERROR		19
#define IRQ_UART2_STATUS	20
#define IRQ_UART2_ERROR		21
#define IRQ_ADC0		22
#define IRQ_CMP0		23
#define IRQ_CMP1		24
#define IRQ_FTM0		25
#define IRQ_FTM1		26
#define IRQ_CMT			27
#define IRQ_RTC_ALARM		28
#define IRQ_RTC_SECOND		29
#define IRQ_PIT_CH0		30
#define IRQ_PIT_CH1		31
#define IRQ_PIT_CH2		32
#define IRQ_PIT_CH3		33
#define IRQ_PDB			34
#define IRQ_USBOTG		35
#define IRQ_USBDCD		36
#define IRQ_TSI			37
#define IRQ_MCG			38
#define IRQ_LPTMR		39
#define IRQ_PORTA		40
#define IRQ_PORTB		41
#define IRQ_PORTC		42
#define IRQ_PORTD		43
#define IRQ_PORTE		44
#define IRQ_SOFTWARE		45


#define __disable_irq() asm volatile("CPSID i");
#define __enable_irq()  asm volatile("CPSIE i");


// System Control Space (SCS), ARMv7 ref manual, B3.2, page 708
#define SCB_CPUID		*(const    uint32_t *)0xE000ED00 // CPUID Base Register
#define SCB_ICSR		*(volatile uint32_t *)0xE000ED04 // Interrupt Control and State
#define SCB_ICSR_PENDSTSET		(uint32_t)0x04000000
#define SCB_VTOR		*(volatile uint32_t *)0xE000ED08 // Vector Table Offset
#define SCB_AIRCR		*(volatile uint32_t *)0xE000ED0C // Application Interrupt and Reset Control
#define SCB_SCR			*(volatile uint32_t *)0xE000ED10 // System Control Register
#define SCB_CCR			*(volatile uint32_t *)0xE000ED14 // Configuration and Control
#define SCB_SHPR1		*(volatile uint32_t *)0xE000ED18 // System Handler Priority Register 1
#define SCB_SHPR2		*(volatile uint32_t *)0xE000ED1C // System Handler Priority Register 2
#define SCB_SHPR3		*(volatile uint32_t *)0xE000ED20 // System Handler Priority Register 3
#define SCB_SHCSR		*(volatile uint32_t *)0xE000ED24 // System Handler Control and State
#define SCB_CFSR		*(volatile uint32_t *)0xE000ED28 // Configurable Fault Status Register
#define SCB_HFSR		*(volatile uint32_t *)0xE000ED2C // HardFault Status
#define SCB_DFSR		*(volatile uint32_t *)0xE000ED30 // Debug Fault Status
#define SCB_MMFAR		*(volatile uint32_t *)0xE000ED34 // MemManage Fault Address

#define SYST_CSR		*(volatile uint32_t *)0xE000E010 // SysTick Control and Status
#define SYST_CSR_COUNTFLAG		(uint32_t)0x00010000
#define SYST_CSR_CLKSOURCE		(uint32_t)0x00000004
#define SYST_CSR_TICKINT		(uint32_t)0x00000002
#define SYST_CSR_ENABLE			(uint32_t)0x00000001
#define SYST_RVR		*(volatile uint32_t *)0xE000E014 // SysTick Reload Value Register
#define SYST_CVR		*(volatile uint32_t *)0xE000E018 // SysTick Current Value Register
#define SYST_CALIB		*(const    uint32_t *)0xE000E01C // SysTick Calibration Value


#define ARM_DEMCR               *(volatile uint32_t *)0xE000EDFC // Debug Exception and Monitor Control
#define ARM_DEMCR_TRCENA		(1 << 24)        // Enable debugging & monitoring blocks
#define ARM_DWT_CTRL            *(volatile uint32_t *)0xE0001000 // DWT control register
#define ARM_DWT_CTRL_CYCCNTENA          (1 << 0)                // Enable cycle count
#define ARM_DWT_CYCCNT          *(volatile uint32_t *)0xE0001004 // Cycle count register


extern void nmi_isr(void);
extern void hard_fault_isr(void);
extern void memmanage_fault_isr(void);
extern void bus_fault_isr(void);
extern void usage_fault_isr(void);
extern void svcall_isr(void);
extern void debugmonitor_isr(void);
extern void pendablesrvreq_isr(void);
extern void systick_isr(void);
extern void dma_ch0_isr(void);
extern void dma_ch1_isr(void);
extern void dma_ch2_isr(void);
extern void dma_ch3_isr(void);
extern void dma_error_isr(void);
extern void flash_cmd_isr(void);
extern void flash_error_isr(void);
extern void low_voltage_isr(void);
extern void wakeup_isr(void);
extern void watchdog_isr(void);
extern void i2c0_isr(void);
extern void spi0_isr(void);
extern void i2s0_tx_isr(void);
extern void i2s0_rx_isr(void);
extern void uart0_lon_isr(void);
extern void uart0_status_isr(void);
extern void uart0_error_isr(void);
extern void uart1_status_isr(void);
extern void uart1_error_isr(void);
extern void uart2_status_isr(void);
extern void uart2_error_isr(void);
extern void adc0_isr(void);
extern void cmp0_isr(void);
extern void cmp1_isr(void);
extern void ftm0_isr(void);
extern void ftm1_isr(void);
extern void cmt_isr(void);
extern void rtc_alarm_isr(void);
extern void rtc_seconds_isr(void);
extern void pit0_isr(void);
extern void pit1_isr(void);
extern void pit2_isr(void);
extern void pit3_isr(void);
extern void pdb_isr(void);
extern void usb_isr(void);
extern void usb_charge_isr(void);
extern void tsi0_isr(void);
extern void mcg_isr(void);
extern void lptmr_isr(void);
extern void porta_isr(void);
extern void portb_isr(void);
extern void portc_isr(void);
extern void portd_isr(void);
extern void porte_isr(void);
extern void software_isr(void);




#ifdef __cplusplus
}
#endif
#endif
