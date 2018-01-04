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

#pragma once

// ----- Defines -----

#ifndef NULL
#define NULL ((void *)0)
#endif

#define NOP() do {} while ( 0 )

//TODO: Do theese seperately in mcu_compat
#define _SAM4S_EFC1_INSTANCE_
#define _SAM4S_PIOC_INSTANCE_
#define _SAM4S_USART1_INSTANCE_
#define _SAM4S_HSMCI_INSTANCE_
#define _SAM4S_TC1_INSTANCE_
#define _SAM4S_DACC_INSTANCE_


// ----- Includes -----

#include <stdint.h>



// ----- Registers -----

// ----- Macros -----

#define SOFTWARE_RESET NOP //TODO: Lookup macro
#define __disable_irq() asm volatile("CPSID i":::"memory");
#define __enable_irq()  asm volatile("CPSIE i":::"memory");


// ----- Variables -----

/* IO definitions (access restrictions to peripheral registers) */
/**
    \defgroup CMSIS_glob_defs CMSIS Global Defines
    <strong>IO Type Qualifiers</strong> are used
    \li to specify the access to peripheral variables.
    \li for automatic generation of peripheral register debug information.
*/
#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */


/* ************************************************************************** */
/*   PERIPHERAL ID DEFINITIONS FOR SAM4SD32C */
/* ************************************************************************** */
/** \addtogroup SAM4SD32C_id Peripheral Ids Definitions */
/*@{*/

typedef struct {
  __O  uint32_t PIO_PER;        /**< \brief (Pio Offset: 0x0000) PIO Enable Register */
  __O  uint32_t PIO_PDR;        /**< \brief (Pio Offset: 0x0004) PIO Disable Register */
  __I  uint32_t PIO_PSR;        /**< \brief (Pio Offset: 0x0008) PIO Status Register */
  __I  uint32_t Reserved1[1];
  __O  uint32_t PIO_OER;        /**< \brief (Pio Offset: 0x0010) Output Enable Register */
  __O  uint32_t PIO_ODR;        /**< \brief (Pio Offset: 0x0014) Output Disable Register */
  __I  uint32_t PIO_OSR;        /**< \brief (Pio Offset: 0x0018) Output Status Register */
  __I  uint32_t Reserved2[1];
  __O  uint32_t PIO_IFER;       /**< \brief (Pio Offset: 0x0020) Glitch Input Filter Enable Register */
  __O  uint32_t PIO_IFDR;       /**< \brief (Pio Offset: 0x0024) Glitch Input Filter Disable Register */
  __I  uint32_t PIO_IFSR;       /**< \brief (Pio Offset: 0x0028) Glitch Input Filter Status Register */
  __I  uint32_t Reserved3[1];
  __O  uint32_t PIO_SODR;       /**< \brief (Pio Offset: 0x0030) Set Output Data Register */
  __O  uint32_t PIO_CODR;       /**< \brief (Pio Offset: 0x0034) Clear Output Data Register */
  __IO uint32_t PIO_ODSR;       /**< \brief (Pio Offset: 0x0038) Output Data Status Register */
  __I  uint32_t PIO_PDSR;       /**< \brief (Pio Offset: 0x003C) Pin Data Status Register */
  __O  uint32_t PIO_IER;        /**< \brief (Pio Offset: 0x0040) Interrupt Enable Register */
  __O  uint32_t PIO_IDR;        /**< \brief (Pio Offset: 0x0044) Interrupt Disable Register */
  __I  uint32_t PIO_IMR;        /**< \brief (Pio Offset: 0x0048) Interrupt Mask Register */
  __I  uint32_t PIO_ISR;        /**< \brief (Pio Offset: 0x004C) Interrupt Status Register */
  __O  uint32_t PIO_MDER;       /**< \brief (Pio Offset: 0x0050) Multi-driver Enable Register */
  __O  uint32_t PIO_MDDR;       /**< \brief (Pio Offset: 0x0054) Multi-driver Disable Register */
  __I  uint32_t PIO_MDSR;       /**< \brief (Pio Offset: 0x0058) Multi-driver Status Register */
  __I  uint32_t Reserved4[1];
  __O  uint32_t PIO_PUDR;       /**< \brief (Pio Offset: 0x0060) Pull-up Disable Register */
  __O  uint32_t PIO_PUER;       /**< \brief (Pio Offset: 0x0064) Pull-up Enable Register */
  __I  uint32_t PIO_PUSR;       /**< \brief (Pio Offset: 0x0068) Pad Pull-up Status Register */
  __I  uint32_t Reserved5[1];
  __IO uint32_t PIO_ABCDSR[2];  /**< \brief (Pio Offset: 0x0070) Peripheral Select Register */
  __I  uint32_t Reserved6[2];
  __O  uint32_t PIO_IFSCDR;     /**< \brief (Pio Offset: 0x0080) Input Filter Slow Clock Disable Register */
  __O  uint32_t PIO_IFSCER;     /**< \brief (Pio Offset: 0x0084) Input Filter Slow Clock Enable Register */
  __I  uint32_t PIO_IFSCSR;     /**< \brief (Pio Offset: 0x0088) Input Filter Slow Clock Status Register */
  __IO uint32_t PIO_SCDR;       /**< \brief (Pio Offset: 0x008C) Slow Clock Divider Debouncing Register */
  __O  uint32_t PIO_PPDDR;      /**< \brief (Pio Offset: 0x0090) Pad Pull-down Disable Register */
  __O  uint32_t PIO_PPDER;      /**< \brief (Pio Offset: 0x0094) Pad Pull-down Enable Register */
  __I  uint32_t PIO_PPDSR;      /**< \brief (Pio Offset: 0x0098) Pad Pull-down Status Register */
  __I  uint32_t Reserved7[1];
  __O  uint32_t PIO_OWER;       /**< \brief (Pio Offset: 0x00A0) Output Write Enable */
  __O  uint32_t PIO_OWDR;       /**< \brief (Pio Offset: 0x00A4) Output Write Disable */
  __I  uint32_t PIO_OWSR;       /**< \brief (Pio Offset: 0x00A8) Output Write Status Register */
  __I  uint32_t Reserved8[1];
  __O  uint32_t PIO_AIMER;      /**< \brief (Pio Offset: 0x00B0) Additional Interrupt Modes Enable Register */
  __O  uint32_t PIO_AIMDR;      /**< \brief (Pio Offset: 0x00B4) Additional Interrupt Modes Disable Register */
  __I  uint32_t PIO_AIMMR;      /**< \brief (Pio Offset: 0x00B8) Additional Interrupt Modes Mask Register */
  __I  uint32_t Reserved9[1];
  __O  uint32_t PIO_ESR;        /**< \brief (Pio Offset: 0x00C0) Edge Select Register */
  __O  uint32_t PIO_LSR;        /**< \brief (Pio Offset: 0x00C4) Level Select Register */
  __I  uint32_t PIO_ELSR;       /**< \brief (Pio Offset: 0x00C8) Edge/Level Status Register */
  __I  uint32_t Reserved10[1];
  __O  uint32_t PIO_FELLSR;     /**< \brief (Pio Offset: 0x00D0) Falling Edge/Low-Level Select Register */
  __O  uint32_t PIO_REHLSR;     /**< \brief (Pio Offset: 0x00D4) Rising Edge/ High-Level Select Register */
  __I  uint32_t PIO_FRLHSR;     /**< \brief (Pio Offset: 0x00D8) Fall/Rise - Low/High Status Register */
  __I  uint32_t Reserved11[1];
  __I  uint32_t PIO_LOCKSR;     /**< \brief (Pio Offset: 0x00E0) Lock Status */
  __IO uint32_t PIO_WPMR;       /**< \brief (Pio Offset: 0x00E4) Write Protection Mode Register */
  __I  uint32_t PIO_WPSR;       /**< \brief (Pio Offset: 0x00E8) Write Protection Status Register */
  __I  uint32_t Reserved12[5];
  __IO uint32_t PIO_SCHMITT;    /**< \brief (Pio Offset: 0x0100) Schmitt Trigger Register */
  __I  uint32_t Reserved13[19];
  __IO uint32_t PIO_PCMR;       /**< \brief (Pio Offset: 0x150) Parallel Capture Mode Register */
  __O  uint32_t PIO_PCIER;      /**< \brief (Pio Offset: 0x154) Parallel Capture Interrupt Enable Register */
  __O  uint32_t PIO_PCIDR;      /**< \brief (Pio Offset: 0x158) Parallel Capture Interrupt Disable Register */
  __I  uint32_t PIO_PCIMR;      /**< \brief (Pio Offset: 0x15C) Parallel Capture Interrupt Mask Register */
  __I  uint32_t PIO_PCISR;      /**< \brief (Pio Offset: 0x160) Parallel Capture Interrupt Status Register */
  __I  uint32_t PIO_PCRHR;      /**< \brief (Pio Offset: 0x164) Parallel Capture Reception Holding Register */
  __IO uint32_t PIO_RPR;        /**< \brief (Pio Offset: 0x168) Receive Pointer Register */
  __IO uint32_t PIO_RCR;        /**< \brief (Pio Offset: 0x16C) Receive Counter Register */
  __I  uint32_t Reserved14[2];
  __IO uint32_t PIO_RNPR;       /**< \brief (Pio Offset: 0x178) Receive Next Pointer Register */
  __IO uint32_t PIO_RNCR;       /**< \brief (Pio Offset: 0x17C) Receive Next Counter Register */
  __I  uint32_t Reserved15[2];
  __O  uint32_t PIO_PTCR;       /**< \brief (Pio Offset: 0x188) Transfer Control Register */
  __I  uint32_t PIO_PTSR;       /**< \brief (Pio Offset: 0x18C) Transfer Status Register */
} Pio;

#define ID_SUPC   ( 0) /**< \brief Supply Controller (SUPC) */
#define ID_RSTC   ( 1) /**< \brief Reset Controller (RSTC) */
#define ID_RTC    ( 2) /**< \brief Real Time Clock (RTC) */
#define ID_RTT    ( 3) /**< \brief Real Time Timer (RTT) */
#define ID_WDT    ( 4) /**< \brief Watchdog Timer (WDT) */
#define ID_PMC    ( 5) /**< \brief Power Management Controller (PMC) */
#define ID_EFC0   ( 6) /**< \brief Enhanced Embedded Flash Controller 0 (EFC0) */
#define ID_EFC1   ( 7) /**< \brief Enhanced Embedded Flash Controller 1 (EFC1) */
#define ID_UART0  ( 8) /**< \brief UART 0 (UART0) */
#define ID_UART1  ( 9) /**< \brief UART 1 (UART1) */
#define ID_SMC    (10) /**< \brief Static Memory Controller (SMC) */
#define ID_PIOA   (11) /**< \brief Parallel I/O Controller A (PIOA) */
#define ID_PIOB   (12) /**< \brief Parallel I/O Controller B (PIOB) */
#define ID_PIOC   (13) /**< \brief Parallel I/O Controller C (PIOC) */
#define ID_USART0 (14) /**< \brief USART 0 (USART0) */
#define ID_USART1 (15) /**< \brief USART 1 (USART1) */
#define ID_HSMCI  (18) /**< \brief Multimedia Card Interface (HSMCI) */
#define ID_TWI0   (19) /**< \brief Two Wire Interface 0 (TWI0) */
#define ID_TWI1   (20) /**< \brief Two Wire Interface 1 (TWI1) */
#define ID_SPI0   (21) /**< \brief Serial Peripheral Interface (SPI0) */
#define ID_SSC    (22) /**< \brief Synchronous Serial Controller (SSC) */
#define ID_TC0    (23) /**< \brief Timer/Counter 0 (TC0) */
#define ID_TC1    (24) /**< \brief Timer/Counter 1 (TC1) */
#define ID_TC2    (25) /**< \brief Timer/Counter 2 (TC2) */
#define ID_TC3    (26) /**< \brief Timer/Counter 3 (TC3) */
#define ID_TC4    (27) /**< \brief Timer/Counter 4 (TC4) */
#define ID_TC5    (28) /**< \brief Timer/Counter 5 (TC5) */
#define ID_ADC    (29) /**< \brief Analog To Digital Converter (ADC) */
#define ID_DACC   (30) /**< \brief Digital To Analog Converter (DACC) */
#define ID_PWM    (31) /**< \brief Pulse Width Modulation (PWM) */
#define ID_CRCCU  (32) /**< \brief CRC Calculation Unit (CRCCU) */
#define ID_ACC    (33) /**< \brief Analog Comparator (ACC) */
#define ID_USBDEV (34) /**< \brief USB Device Port (USBDEV) */

#define ID_PERIPH_COUNT (35) /**< \brief Number of peripheral IDs */
/*@}*/

/* ************************************************************************** */
/*   BASE ADDRESS DEFINITIONS FOR SAM4SD32C */
/* ************************************************************************** */
/** \addtogroup SAM4SD32C_base Peripheral Base Address Definitions */
/*@{*/

#define HSMCI      ((Hsmci  *)0x40000000U) /**< \brief (HSMCI     ) Base Address */
#define PDC_HSMCI  ((Pdc    *)0x40000100U) /**< \brief (PDC_HSMCI ) Base Address */
#define SSC        ((Ssc    *)0x40004000U) /**< \brief (SSC       ) Base Address */
#define PDC_SSC    ((Pdc    *)0x40004100U) /**< \brief (PDC_SSC   ) Base Address */
#define SPI0       ((Spi    *)0x40008000U) /**< \brief (SPI0      ) Base Address */
#define PDC_SPI0   ((Pdc    *)0x40008100U) /**< \brief (PDC_SPI0  ) Base Address */
#define TC0        ((Tc     *)0x40010000U) /**< \brief (TC0       ) Base Address */
#define TC1        ((Tc     *)0x40014000U) /**< \brief (TC1       ) Base Address */
#define TWI0       ((Twi    *)0x40018000U) /**< \brief (TWI0      ) Base Address */
#define PDC_TWI0   ((Pdc    *)0x40018100U) /**< \brief (PDC_TWI0  ) Base Address */
#define TWI1       ((Twi    *)0x4001C000U) /**< \brief (TWI1      ) Base Address */
#define PDC_TWI1   ((Pdc    *)0x4001C100U) /**< \brief (PDC_TWI1  ) Base Address */
#define PWM        ((Pwm    *)0x40020000U) /**< \brief (PWM       ) Base Address */
#define PDC_PWM    ((Pdc    *)0x40020100U) /**< \brief (PDC_PWM   ) Base Address */
#define USART0     ((Usart  *)0x40024000U) /**< \brief (USART0    ) Base Address */
#define PDC_USART0 ((Pdc    *)0x40024100U) /**< \brief (PDC_USART0) Base Address */
#define USART1     ((Usart  *)0x40028000U) /**< \brief (USART1    ) Base Address */
#define PDC_USART1 ((Pdc    *)0x40028100U) /**< \brief (PDC_USART1) Base Address */
#define USBDEV     ((USBDev *)0x40034000U) /**< \brief (USBDEV    ) Base Address */
#define ADC        ((Adc    *)0x40038000U) /**< \brief (ADC       ) Base Address */
#define PDC_ADC    ((Pdc    *)0x40038100U) /**< \brief (PDC_ADC   ) Base Address */
#define DACC       ((Dacc   *)0x4003C000U) /**< \brief (DACC      ) Base Address */
#define PDC_DACC   ((Pdc    *)0x4003C100U) /**< \brief (PDC_DACC  ) Base Address */
#define ACC        ((Acc    *)0x40040000U) /**< \brief (ACC       ) Base Address */
#define CRCCU      ((Crccu  *)0x40044000U) /**< \brief (CRCCU     ) Base Address */
#define CMCC       ((Cmcc   *)0x4007C000U) /**< \brief (CMCC      ) Base Address */
#define SMC        ((Smc    *)0x400E0000U) /**< \brief (SMC       ) Base Address */
#define MATRIX     ((Matrix *)0x400E0200U) /**< \brief (MATRIX    ) Base Address */
#define PMC        ((Pmc    *)0x400E0400U) /**< \brief (PMC       ) Base Address */
#define UART0      ((Uart   *)0x400E0600U) /**< \brief (UART0     ) Base Address */
#define PDC_UART0  ((Pdc    *)0x400E0700U) /**< \brief (PDC_UART0 ) Base Address */
#define CHIPID     ((Chipid *)0x400E0740U) /**< \brief (CHIPID    ) Base Address */
#define UART1      ((Uart   *)0x400E0800U) /**< \brief (UART1     ) Base Address */
#define PDC_UART1  ((Pdc    *)0x400E0900U) /**< \brief (PDC_UART1 ) Base Address */
#define EFC0       ((Efc    *)0x400E0A00U) /**< \brief (EFC0      ) Base Address */
#define EFC1       ((Efc    *)0x400E0C00U) /**< \brief (EFC1      ) Base Address */
#define PIOA       ((Pio    *)0x400E0E00U) /**< \brief (PIOA      ) Base Address */
#define PDC_PIOA   ((Pdc    *)0x400E0F68U) /**< \brief (PDC_PIOA  ) Base Address */
#define PIOB       ((Pio    *)0x400E1000U) /**< \brief (PIOB      ) Base Address */
#define PIOC       ((Pio    *)0x400E1200U) /**< \brief (PIOC      ) Base Address */
#define RSTC       ((Rstc   *)0x400E1400U) /**< \brief (RSTC      ) Base Address */
#define SUPC       ((Supc   *)0x400E1410U) /**< \brief (SUPC      ) Base Address */
#define RTT        ((Rtt    *)0x400E1430U) /**< \brief (RTT       ) Base Address */
#define WDT        ((Wdt    *)0x400E1450U) /**< \brief (WDT       ) Base Address */
#define RTC        ((Rtc    *)0x400E1460U) /**< \brief (RTC       ) Base Address */
#define GPBR       ((Gpbr   *)0x400E1490U) /**< \brief (GPBR      ) Base Address */
/*@}*/

/* ************************************************************************** */
/*   MEMORY MAPPING DEFINITIONS FOR SAM4SD32C */
/* ************************************************************************** */

#define IFLASH0_SIZE             (0x100000u)
#define IFLASH0_PAGE_SIZE        (512u)
#define IFLASH0_LOCK_REGION_SIZE (8192u)
#define IFLASH0_NB_OF_PAGES      (2048u)
#define IFLASH0_NB_OF_LOCK_BITS  (128u)
#define IFLASH1_SIZE             (0x100000u)
#define IFLASH1_PAGE_SIZE        (512u)
#define IFLASH1_LOCK_REGION_SIZE (8192u)
#define IFLASH1_NB_OF_PAGES      (2048u)
#define IFLASH1_NB_OF_LOCK_BITS  (128u)
#define IRAM_SIZE                (0x28000u)
#define IFLASH_SIZE              (IFLASH0_SIZE+IFLASH1_SIZE)

#define IFLASH0_ADDR (0x00400000u) /**< Internal Flash 0 base address */
#if defined IFLASH0_SIZE
#define IFLASH1_ADDR (IFLASH0_ADDR+IFLASH0_SIZE) /**< Internal Flash 1 base address */
#endif
#define IROM_ADDR    (0x00800000u) /**< Internal ROM base address */
#define IRAM_ADDR    (0x20000000u) /**< Internal RAM base address */
#define EBI_CS0_ADDR (0x60000000u) /**< EBI Chip Select 0 base address */
#define EBI_CS1_ADDR (0x61000000u) /**< EBI Chip Select 1 base address */
#define EBI_CS2_ADDR (0x62000000u) /**< EBI Chip Select 2 base address */
#define EBI_CS3_ADDR (0x63000000u) /**< EBI Chip Select 3 base address */

/* ************************************************************************** */
/*   MISCELLANEOUS DEFINITIONS FOR SAM4SD32C */
/* ************************************************************************** */

#define CHIP_JTAGID       (0x05B3203FUL)
#define CHIP_CIDR         (0x29A70EE0UL)
#define CHIP_EXID         (0x0UL)
#define NB_CH_ADC         (15UL)
#define NB_CH_DAC         (2UL)
#define USB_DEVICE_MAX_EP (8UL)

/* ************************************************************************** */
/*   ELECTRICAL DEFINITIONS FOR SAM4SD32C */
/* ************************************************************************** */

/* Device characteristics */
#define CHIP_FREQ_SLCK_RC_MIN           (20000UL)
#define CHIP_FREQ_SLCK_RC               (32000UL)
#define CHIP_FREQ_SLCK_RC_MAX           (44000UL)
#define CHIP_FREQ_MAINCK_RC_4MHZ        (4000000UL)
#define CHIP_FREQ_MAINCK_RC_8MHZ        (8000000UL)
#define CHIP_FREQ_MAINCK_RC_12MHZ       (12000000UL)
#define CHIP_FREQ_CPU_MAX               (120000000UL)
#define CHIP_FREQ_XTAL_32K              (32768UL)

/* Embedded Flash Write Wait State */
#define CHIP_FLASH_WRITE_WAIT_STATE     (6U)

#if defined __SAM4S2A__ || defined __SAM4S2B__ || defined __SAM4S2C__ || \
    defined __SAM4S4A__ || defined __SAM4S4B__ || defined __SAM4S4C__

/* Embedded Flash Read Wait State (VDDCORE set at 1.20V and VDDIO 3.3V) */
#define CHIP_FREQ_FWS_0                 (29000000UL)  /**< \brief Maximum operating frequency when FWS is 0 */
#define CHIP_FREQ_FWS_1                 (58000000UL)  /**< \brief Maximum operating frequency when FWS is 1 */
#define CHIP_FREQ_FWS_2                 (88000000UL)  /**< \brief Maximum operating frequency when FWS is 2 */
#define CHIP_FREQ_FWS_3                 (10800000UL)  /**< \brief Maximum operating frequency when FWS is 3 */
#define CHIP_FREQ_FWS_4                 (120000000UL) /**< \brief Maximum operating frequency when FWS is 4 */

#else  /* SAM4S8/S16/SA16/SD16/SD32 */

/* Embedded Flash Read Wait State (VDDCORE set at 1.20V and VDDIO 3.3V) */
#define CHIP_FREQ_FWS_0                 (20000000UL)  /**< \brief Maximum operating frequency when FWS is 0 */
#define CHIP_FREQ_FWS_1                 (40000000UL)  /**< \brief Maximum operating frequency when FWS is 1 */
#define CHIP_FREQ_FWS_2                 (60000000UL)  /**< \brief Maximum operating frequency when FWS is 2 */
#define CHIP_FREQ_FWS_3                 (80000000UL)  /**< \brief Maximum operating frequency when FWS is 3 */
#define CHIP_FREQ_FWS_4                 (100000000UL) /**< \brief Maximum operating frequency when FWS is 4 */
#define CHIP_FREQ_FWS_5                 (123000000UL) /**< \brief Maximum operating frequency when FWS is 5 */

#endif

/* HYSTeresis levels: please refer to Electrical Characteristics */
#define ACC_ACR_HYST_50MV_MAX	        (0x01UL)
#define ACC_ACR_HYST_90MV_MAX           (0x11UL)

//PMC
typedef struct {
  __O  uint32_t PMC_SCER;       /**< \brief (Pmc Offset: 0x0000) System Clock Enable Register */
  __O  uint32_t PMC_SCDR;       /**< \brief (Pmc Offset: 0x0004) System Clock Disable Register */
  __I  uint32_t PMC_SCSR;       /**< \brief (Pmc Offset: 0x0008) System Clock Status Register */
  __I  uint32_t Reserved1[1];
  __O  uint32_t PMC_PCER0;      /**< \brief (Pmc Offset: 0x0010) Peripheral Clock Enable Register 0 */
  __O  uint32_t PMC_PCDR0;      /**< \brief (Pmc Offset: 0x0014) Peripheral Clock Disable Register 0 */
  __I  uint32_t PMC_PCSR0;      /**< \brief (Pmc Offset: 0x0018) Peripheral Clock Status Register 0 */
  __I  uint32_t Reserved2[1];
  __IO uint32_t CKGR_MOR;       /**< \brief (Pmc Offset: 0x0020) Main Oscillator Register */
  __IO uint32_t CKGR_MCFR;      /**< \brief (Pmc Offset: 0x0024) Main Clock Frequency Register */
  __IO uint32_t CKGR_PLLAR;     /**< \brief (Pmc Offset: 0x0028) PLLA Register */
  __IO uint32_t CKGR_PLLBR;     /**< \brief (Pmc Offset: 0x002C) PLLB Register */
  __IO uint32_t PMC_MCKR;       /**< \brief (Pmc Offset: 0x0030) Master Clock Register */
  __I  uint32_t Reserved3[1];
  __IO uint32_t PMC_USB;        /**< \brief (Pmc Offset: 0x0038) USB Clock Register */
  __I  uint32_t Reserved4[1];
  __IO uint32_t PMC_PCK[3];     /**< \brief (Pmc Offset: 0x0040) Programmable Clock 0 Register */
  __I  uint32_t Reserved5[5];
  __O  uint32_t PMC_IER;        /**< \brief (Pmc Offset: 0x0060) Interrupt Enable Register */
  __O  uint32_t PMC_IDR;        /**< \brief (Pmc Offset: 0x0064) Interrupt Disable Register */
  __I  uint32_t PMC_SR;         /**< \brief (Pmc Offset: 0x0068) Status Register */
  __I  uint32_t PMC_IMR;        /**< \brief (Pmc Offset: 0x006C) Interrupt Mask Register */
  __IO uint32_t PMC_FSMR;       /**< \brief (Pmc Offset: 0x0070) Fast Startup Mode Register */
  __IO uint32_t PMC_FSPR;       /**< \brief (Pmc Offset: 0x0074) Fast Startup Polarity Register */
  __O  uint32_t PMC_FOCR;       /**< \brief (Pmc Offset: 0x0078) Fault Output Clear Register */
  __I  uint32_t Reserved6[26];
  __IO uint32_t PMC_WPMR;       /**< \brief (Pmc Offset: 0x00E4) Write Protection Mode Register */
  __I  uint32_t PMC_WPSR;       /**< \brief (Pmc Offset: 0x00E8) Write Protection Status Register */
  __I  uint32_t Reserved7[5];
  __O  uint32_t PMC_PCER1;      /**< \brief (Pmc Offset: 0x0100) Peripheral Clock Enable Register 1 */
  __O  uint32_t PMC_PCDR1;      /**< \brief (Pmc Offset: 0x0104) Peripheral Clock Disable Register 1 */
  __I  uint32_t PMC_PCSR1;      /**< \brief (Pmc Offset: 0x0108) Peripheral Clock Status Register 1 */
  __I  uint32_t Reserved8[1];
  __IO uint32_t PMC_OCR;        /**< \brief (Pmc Offset: 0x0110) Oscillator Calibration Register */
} Pmc;

/* -------- PMC_SCER : (PMC Offset: 0x0000) System Clock Enable Register -------- */
#define PMC_SCER_UDP (0x1u << 7) /**< \brief (PMC_SCER) USB Device Port Clock Enable */
#define PMC_SCER_PCK0 (0x1u << 8) /**< \brief (PMC_SCER) Programmable Clock 0 Output Enable */
#define PMC_SCER_PCK1 (0x1u << 9) /**< \brief (PMC_SCER) Programmable Clock 1 Output Enable */
#define PMC_SCER_PCK2 (0x1u << 10) /**< \brief (PMC_SCER) Programmable Clock 2 Output Enable */
/* -------- PMC_SCDR : (PMC Offset: 0x0004) System Clock Disable Register -------- */
#define PMC_SCDR_UDP (0x1u << 7) /**< \brief (PMC_SCDR) USB Device Port Clock Disable */
#define PMC_SCDR_PCK0 (0x1u << 8) /**< \brief (PMC_SCDR) Programmable Clock 0 Output Disable */
#define PMC_SCDR_PCK1 (0x1u << 9) /**< \brief (PMC_SCDR) Programmable Clock 1 Output Disable */
#define PMC_SCDR_PCK2 (0x1u << 10) /**< \brief (PMC_SCDR) Programmable Clock 2 Output Disable */
/* -------- PMC_SCSR : (PMC Offset: 0x0008) System Clock Status Register -------- */
#define PMC_SCSR_UDP (0x1u << 7) /**< \brief (PMC_SCSR) USB Device Port Clock Status */
#define PMC_SCSR_PCK0 (0x1u << 8) /**< \brief (PMC_SCSR) Programmable Clock 0 Output Status */
#define PMC_SCSR_PCK1 (0x1u << 9) /**< \brief (PMC_SCSR) Programmable Clock 1 Output Status */
#define PMC_SCSR_PCK2 (0x1u << 10) /**< \brief (PMC_SCSR) Programmable Clock 2 Output Status */
/* -------- PMC_PCER0 : (PMC Offset: 0x0010) Peripheral Clock Enable Register 0 -------- */
#define PMC_PCER0_PID8 (0x1u << 8) /**< \brief (PMC_PCER0) Peripheral Clock 8 Enable */
#define PMC_PCER0_PID9 (0x1u << 9) /**< \brief (PMC_PCER0) Peripheral Clock 9 Enable */
#define PMC_PCER0_PID10 (0x1u << 10) /**< \brief (PMC_PCER0) Peripheral Clock 10 Enable */
#define PMC_PCER0_PID11 (0x1u << 11) /**< \brief (PMC_PCER0) Peripheral Clock 11 Enable */
#define PMC_PCER0_PID12 (0x1u << 12) /**< \brief (PMC_PCER0) Peripheral Clock 12 Enable */
#define PMC_PCER0_PID13 (0x1u << 13) /**< \brief (PMC_PCER0) Peripheral Clock 13 Enable */
#define PMC_PCER0_PID14 (0x1u << 14) /**< \brief (PMC_PCER0) Peripheral Clock 14 Enable */
#define PMC_PCER0_PID15 (0x1u << 15) /**< \brief (PMC_PCER0) Peripheral Clock 15 Enable */
#define PMC_PCER0_PID16 (0x1u << 16) /**< \brief (PMC_PCER0) Peripheral Clock 16 Enable */
#define PMC_PCER0_PID17 (0x1u << 17) /**< \brief (PMC_PCER0) Peripheral Clock 17 Enable */
#define PMC_PCER0_PID18 (0x1u << 18) /**< \brief (PMC_PCER0) Peripheral Clock 18 Enable */
#define PMC_PCER0_PID19 (0x1u << 19) /**< \brief (PMC_PCER0) Peripheral Clock 19 Enable */
#define PMC_PCER0_PID20 (0x1u << 20) /**< \brief (PMC_PCER0) Peripheral Clock 20 Enable */
#define PMC_PCER0_PID21 (0x1u << 21) /**< \brief (PMC_PCER0) Peripheral Clock 21 Enable */
#define PMC_PCER0_PID22 (0x1u << 22) /**< \brief (PMC_PCER0) Peripheral Clock 22 Enable */
#define PMC_PCER0_PID23 (0x1u << 23) /**< \brief (PMC_PCER0) Peripheral Clock 23 Enable */
#define PMC_PCER0_PID24 (0x1u << 24) /**< \brief (PMC_PCER0) Peripheral Clock 24 Enable */
#define PMC_PCER0_PID25 (0x1u << 25) /**< \brief (PMC_PCER0) Peripheral Clock 25 Enable */
#define PMC_PCER0_PID26 (0x1u << 26) /**< \brief (PMC_PCER0) Peripheral Clock 26 Enable */
#define PMC_PCER0_PID27 (0x1u << 27) /**< \brief (PMC_PCER0) Peripheral Clock 27 Enable */
#define PMC_PCER0_PID28 (0x1u << 28) /**< \brief (PMC_PCER0) Peripheral Clock 28 Enable */
#define PMC_PCER0_PID29 (0x1u << 29) /**< \brief (PMC_PCER0) Peripheral Clock 29 Enable */
#define PMC_PCER0_PID30 (0x1u << 30) /**< \brief (PMC_PCER0) Peripheral Clock 30 Enable */
#define PMC_PCER0_PID31 (0x1u << 31) /**< \brief (PMC_PCER0) Peripheral Clock 31 Enable */
/* -------- PMC_PCDR0 : (PMC Offset: 0x0014) Peripheral Clock Disable Register 0 -------- */
#define PMC_PCDR0_PID8 (0x1u << 8) /**< \brief (PMC_PCDR0) Peripheral Clock 8 Disable */
#define PMC_PCDR0_PID9 (0x1u << 9) /**< \brief (PMC_PCDR0) Peripheral Clock 9 Disable */
#define PMC_PCDR0_PID10 (0x1u << 10) /**< \brief (PMC_PCDR0) Peripheral Clock 10 Disable */
#define PMC_PCDR0_PID11 (0x1u << 11) /**< \brief (PMC_PCDR0) Peripheral Clock 11 Disable */
#define PMC_PCDR0_PID12 (0x1u << 12) /**< \brief (PMC_PCDR0) Peripheral Clock 12 Disable */
#define PMC_PCDR0_PID13 (0x1u << 13) /**< \brief (PMC_PCDR0) Peripheral Clock 13 Disable */
#define PMC_PCDR0_PID14 (0x1u << 14) /**< \brief (PMC_PCDR0) Peripheral Clock 14 Disable */
#define PMC_PCDR0_PID15 (0x1u << 15) /**< \brief (PMC_PCDR0) Peripheral Clock 15 Disable */
#define PMC_PCDR0_PID16 (0x1u << 16) /**< \brief (PMC_PCDR0) Peripheral Clock 16 Disable */
#define PMC_PCDR0_PID17 (0x1u << 17) /**< \brief (PMC_PCDR0) Peripheral Clock 17 Disable */
#define PMC_PCDR0_PID18 (0x1u << 18) /**< \brief (PMC_PCDR0) Peripheral Clock 18 Disable */
#define PMC_PCDR0_PID19 (0x1u << 19) /**< \brief (PMC_PCDR0) Peripheral Clock 19 Disable */
#define PMC_PCDR0_PID20 (0x1u << 20) /**< \brief (PMC_PCDR0) Peripheral Clock 20 Disable */
#define PMC_PCDR0_PID21 (0x1u << 21) /**< \brief (PMC_PCDR0) Peripheral Clock 21 Disable */
#define PMC_PCDR0_PID22 (0x1u << 22) /**< \brief (PMC_PCDR0) Peripheral Clock 22 Disable */
#define PMC_PCDR0_PID23 (0x1u << 23) /**< \brief (PMC_PCDR0) Peripheral Clock 23 Disable */
#define PMC_PCDR0_PID24 (0x1u << 24) /**< \brief (PMC_PCDR0) Peripheral Clock 24 Disable */
#define PMC_PCDR0_PID25 (0x1u << 25) /**< \brief (PMC_PCDR0) Peripheral Clock 25 Disable */
#define PMC_PCDR0_PID26 (0x1u << 26) /**< \brief (PMC_PCDR0) Peripheral Clock 26 Disable */
#define PMC_PCDR0_PID27 (0x1u << 27) /**< \brief (PMC_PCDR0) Peripheral Clock 27 Disable */
#define PMC_PCDR0_PID28 (0x1u << 28) /**< \brief (PMC_PCDR0) Peripheral Clock 28 Disable */
#define PMC_PCDR0_PID29 (0x1u << 29) /**< \brief (PMC_PCDR0) Peripheral Clock 29 Disable */
#define PMC_PCDR0_PID30 (0x1u << 30) /**< \brief (PMC_PCDR0) Peripheral Clock 30 Disable */
#define PMC_PCDR0_PID31 (0x1u << 31) /**< \brief (PMC_PCDR0) Peripheral Clock 31 Disable */
/* -------- PMC_PCSR0 : (PMC Offset: 0x0018) Peripheral Clock Status Register 0 -------- */
#define PMC_PCSR0_PID8 (0x1u << 8) /**< \brief (PMC_PCSR0) Peripheral Clock 8 Status */
#define PMC_PCSR0_PID9 (0x1u << 9) /**< \brief (PMC_PCSR0) Peripheral Clock 9 Status */
#define PMC_PCSR0_PID10 (0x1u << 10) /**< \brief (PMC_PCSR0) Peripheral Clock 10 Status */
#define PMC_PCSR0_PID11 (0x1u << 11) /**< \brief (PMC_PCSR0) Peripheral Clock 11 Status */
#define PMC_PCSR0_PID12 (0x1u << 12) /**< \brief (PMC_PCSR0) Peripheral Clock 12 Status */
#define PMC_PCSR0_PID13 (0x1u << 13) /**< \brief (PMC_PCSR0) Peripheral Clock 13 Status */
#define PMC_PCSR0_PID14 (0x1u << 14) /**< \brief (PMC_PCSR0) Peripheral Clock 14 Status */
#define PMC_PCSR0_PID15 (0x1u << 15) /**< \brief (PMC_PCSR0) Peripheral Clock 15 Status */
#define PMC_PCSR0_PID16 (0x1u << 16) /**< \brief (PMC_PCSR0) Peripheral Clock 16 Status */
#define PMC_PCSR0_PID17 (0x1u << 17) /**< \brief (PMC_PCSR0) Peripheral Clock 17 Status */
#define PMC_PCSR0_PID18 (0x1u << 18) /**< \brief (PMC_PCSR0) Peripheral Clock 18 Status */
#define PMC_PCSR0_PID19 (0x1u << 19) /**< \brief (PMC_PCSR0) Peripheral Clock 19 Status */
#define PMC_PCSR0_PID20 (0x1u << 20) /**< \brief (PMC_PCSR0) Peripheral Clock 20 Status */
#define PMC_PCSR0_PID21 (0x1u << 21) /**< \brief (PMC_PCSR0) Peripheral Clock 21 Status */
#define PMC_PCSR0_PID22 (0x1u << 22) /**< \brief (PMC_PCSR0) Peripheral Clock 22 Status */
#define PMC_PCSR0_PID23 (0x1u << 23) /**< \brief (PMC_PCSR0) Peripheral Clock 23 Status */
#define PMC_PCSR0_PID24 (0x1u << 24) /**< \brief (PMC_PCSR0) Peripheral Clock 24 Status */
#define PMC_PCSR0_PID25 (0x1u << 25) /**< \brief (PMC_PCSR0) Peripheral Clock 25 Status */
#define PMC_PCSR0_PID26 (0x1u << 26) /**< \brief (PMC_PCSR0) Peripheral Clock 26 Status */
#define PMC_PCSR0_PID27 (0x1u << 27) /**< \brief (PMC_PCSR0) Peripheral Clock 27 Status */
#define PMC_PCSR0_PID28 (0x1u << 28) /**< \brief (PMC_PCSR0) Peripheral Clock 28 Status */
#define PMC_PCSR0_PID29 (0x1u << 29) /**< \brief (PMC_PCSR0) Peripheral Clock 29 Status */
#define PMC_PCSR0_PID30 (0x1u << 30) /**< \brief (PMC_PCSR0) Peripheral Clock 30 Status */
#define PMC_PCSR0_PID31 (0x1u << 31) /**< \brief (PMC_PCSR0) Peripheral Clock 31 Status */
/* -------- CKGR_MOR : (PMC Offset: 0x0020) Main Oscillator Register -------- */
#define CKGR_MOR_MOSCXTEN (0x1u << 0) /**< \brief (CKGR_MOR) Main Crystal Oscillator Enable */
#define CKGR_MOR_MOSCXTBY (0x1u << 1) /**< \brief (CKGR_MOR) Main Crystal Oscillator Bypass */
#define CKGR_MOR_WAITMODE (0x1u << 2) /**< \brief (CKGR_MOR) Wait Mode Command */
#define CKGR_MOR_MOSCRCEN (0x1u << 3) /**< \brief (CKGR_MOR) Main On-Chip RC Oscillator Enable */
#define CKGR_MOR_MOSCRCF_Pos 4
#define CKGR_MOR_MOSCRCF_Msk (0x7u << CKGR_MOR_MOSCRCF_Pos) /**< \brief (CKGR_MOR) Main On-Chip RC Oscillator Frequency Selection */
#define   CKGR_MOR_MOSCRCF_4_MHz (0x0u << 4) /**< \brief (CKGR_MOR) The Fast RC Oscillator Frequency is at 4 MHz (default) */
#define   CKGR_MOR_MOSCRCF_8_MHz (0x1u << 4) /**< \brief (CKGR_MOR) The Fast RC Oscillator Frequency is at 8 MHz */
#define   CKGR_MOR_MOSCRCF_12_MHz (0x2u << 4) /**< \brief (CKGR_MOR) The Fast RC Oscillator Frequency is at 12 MHz */
#define CKGR_MOR_MOSCXTST_Pos 8
#define CKGR_MOR_MOSCXTST_Msk (0xffu << CKGR_MOR_MOSCXTST_Pos) /**< \brief (CKGR_MOR) Main Crystal Oscillator Start-up Time */
#define CKGR_MOR_MOSCXTST(value) ((CKGR_MOR_MOSCXTST_Msk & ((value) << CKGR_MOR_MOSCXTST_Pos)))
#define CKGR_MOR_KEY_Pos 16
#define CKGR_MOR_KEY_Msk (0xffu << CKGR_MOR_KEY_Pos) /**< \brief (CKGR_MOR) Write Access Password */
#define   CKGR_MOR_KEY_PASSWD (0x37u << 16) /**< \brief (CKGR_MOR) Writing any other value in this field aborts the write operation.Always reads as 0. */
#define CKGR_MOR_MOSCSEL (0x1u << 24) /**< \brief (CKGR_MOR) Main Oscillator Selection */
#define CKGR_MOR_CFDEN (0x1u << 25) /**< \brief (CKGR_MOR) Clock Failure Detector Enable */
/* -------- CKGR_MCFR : (PMC Offset: 0x0024) Main Clock Frequency Register -------- */
#define CKGR_MCFR_MAINF_Pos 0
#define CKGR_MCFR_MAINF_Msk (0xffffu << CKGR_MCFR_MAINF_Pos) /**< \brief (CKGR_MCFR) Main Clock Frequency */
#define CKGR_MCFR_MAINF(value) ((CKGR_MCFR_MAINF_Msk & ((value) << CKGR_MCFR_MAINF_Pos)))
#define CKGR_MCFR_MAINFRDY (0x1u << 16) /**< \brief (CKGR_MCFR) Main Clock Ready */
#define CKGR_MCFR_RCMEAS (0x1u << 20) /**< \brief (CKGR_MCFR) RC Oscillator Frequency Measure (write-only) */
/* -------- CKGR_PLLAR : (PMC Offset: 0x0028) PLLA Register -------- */
#define CKGR_PLLAR_DIVA_Pos 0
#define CKGR_PLLAR_DIVA_Msk (0xffu << CKGR_PLLAR_DIVA_Pos) /**< \brief (CKGR_PLLAR) PLLA Front_End Divider */
#define CKGR_PLLAR_DIVA(value) ((CKGR_PLLAR_DIVA_Msk & ((value) << CKGR_PLLAR_DIVA_Pos)))
#define CKGR_PLLAR_PLLACOUNT_Pos 8
#define CKGR_PLLAR_PLLACOUNT_Msk (0x3fu << CKGR_PLLAR_PLLACOUNT_Pos) /**< \brief (CKGR_PLLAR) PLLA Counter */
#define CKGR_PLLAR_PLLACOUNT(value) ((CKGR_PLLAR_PLLACOUNT_Msk & ((value) << CKGR_PLLAR_PLLACOUNT_Pos)))
#define CKGR_PLLAR_MULA_Pos 16
#define CKGR_PLLAR_MULA_Msk (0x7ffu << CKGR_PLLAR_MULA_Pos) /**< \brief (CKGR_PLLAR) PLLA Multiplier */
#define CKGR_PLLAR_MULA(value) ((CKGR_PLLAR_MULA_Msk & ((value) << CKGR_PLLAR_MULA_Pos)))
#define CKGR_PLLAR_ONE (0x1u << 29) /**< \brief (CKGR_PLLAR) Must Be Set to 1 */
/* -------- CKGR_PLLBR : (PMC Offset: 0x002C) PLLB Register -------- */
#define CKGR_PLLBR_DIVB_Pos 0
#define CKGR_PLLBR_DIVB_Msk (0xffu << CKGR_PLLBR_DIVB_Pos) /**< \brief (CKGR_PLLBR) PLLB Front-End Divider */
#define CKGR_PLLBR_DIVB(value) ((CKGR_PLLBR_DIVB_Msk & ((value) << CKGR_PLLBR_DIVB_Pos)))
#define CKGR_PLLBR_PLLBCOUNT_Pos 8
#define CKGR_PLLBR_PLLBCOUNT_Msk (0x3fu << CKGR_PLLBR_PLLBCOUNT_Pos) /**< \brief (CKGR_PLLBR) PLLB Counter */
#define CKGR_PLLBR_PLLBCOUNT(value) ((CKGR_PLLBR_PLLBCOUNT_Msk & ((value) << CKGR_PLLBR_PLLBCOUNT_Pos)))
#define CKGR_PLLBR_MULB_Pos 16
#define CKGR_PLLBR_MULB_Msk (0x7ffu << CKGR_PLLBR_MULB_Pos) /**< \brief (CKGR_PLLBR) PLLB Multiplier */
#define CKGR_PLLBR_MULB(value) ((CKGR_PLLBR_MULB_Msk & ((value) << CKGR_PLLBR_MULB_Pos)))
/* -------- PMC_MCKR : (PMC Offset: 0x0030) Master Clock Register -------- */
#define PMC_MCKR_CSS_Pos 0
#define PMC_MCKR_CSS_Msk (0x3u << PMC_MCKR_CSS_Pos) /**< \brief (PMC_MCKR) Master Clock Source Selection */
#define   PMC_MCKR_CSS_SLOW_CLK (0x0u << 0) /**< \brief (PMC_MCKR) Slow Clock is selected */
#define   PMC_MCKR_CSS_MAIN_CLK (0x1u << 0) /**< \brief (PMC_MCKR) Main Clock is selected */
#define   PMC_MCKR_CSS_PLLA_CLK (0x2u << 0) /**< \brief (PMC_MCKR) PLLA Clock is selected */
#define   PMC_MCKR_CSS_PLLB_CLK (0x3u << 0) /**< \brief (PMC_MCKR) PLLBClock is selected */
#define PMC_MCKR_PRES_Pos 4
#define PMC_MCKR_PRES_Msk (0x7u << PMC_MCKR_PRES_Pos) /**< \brief (PMC_MCKR) Processor Clock Prescaler */
#define   PMC_MCKR_PRES_CLK_1 (0x0u << 4) /**< \brief (PMC_MCKR) Selected clock */
#define   PMC_MCKR_PRES_CLK_2 (0x1u << 4) /**< \brief (PMC_MCKR) Selected clock divided by 2 */
#define   PMC_MCKR_PRES_CLK_4 (0x2u << 4) /**< \brief (PMC_MCKR) Selected clock divided by 4 */
#define   PMC_MCKR_PRES_CLK_8 (0x3u << 4) /**< \brief (PMC_MCKR) Selected clock divided by 8 */
#define   PMC_MCKR_PRES_CLK_16 (0x4u << 4) /**< \brief (PMC_MCKR) Selected clock divided by 16 */
#define   PMC_MCKR_PRES_CLK_32 (0x5u << 4) /**< \brief (PMC_MCKR) Selected clock divided by 32 */
#define   PMC_MCKR_PRES_CLK_64 (0x6u << 4) /**< \brief (PMC_MCKR) Selected clock divided by 64 */
#define   PMC_MCKR_PRES_CLK_3 (0x7u << 4) /**< \brief (PMC_MCKR) Selected clock divided by 3 */
#define PMC_MCKR_PLLADIV2 (0x1u << 12) /**< \brief (PMC_MCKR) PLLA Divisor by 2 */
#define PMC_MCKR_PLLBDIV2 (0x1u << 13) /**< \brief (PMC_MCKR) PLLB Divisor by 2 */
/* -------- PMC_USB : (PMC Offset: 0x0038) USB Clock Register -------- */
#define PMC_USB_USBS (0x1u << 0) /**< \brief (PMC_USB) USB Input Clock Selection */
#define PMC_USB_USBDIV_Pos 8
#define PMC_USB_USBDIV_Msk (0xfu << PMC_USB_USBDIV_Pos) /**< \brief (PMC_USB) Divider for USB Clock */
#define PMC_USB_USBDIV(value) ((PMC_USB_USBDIV_Msk & ((value) << PMC_USB_USBDIV_Pos)))
/* -------- PMC_PCK[3] : (PMC Offset: 0x0040) Programmable Clock 0 Register -------- */
#define PMC_PCK_CSS_Pos 0
#define PMC_PCK_CSS_Msk (0x7u << PMC_PCK_CSS_Pos) /**< \brief (PMC_PCK[3]) Master Clock Source Selection */
#define   PMC_PCK_CSS_SLOW_CLK (0x0u << 0) /**< \brief (PMC_PCK[3]) Slow Clock is selected */
#define   PMC_PCK_CSS_MAIN_CLK (0x1u << 0) /**< \brief (PMC_PCK[3]) Main Clock is selected */
#define   PMC_PCK_CSS_PLLA_CLK (0x2u << 0) /**< \brief (PMC_PCK[3]) PLLA Clock is selected */
#define   PMC_PCK_CSS_PLLB_CLK (0x3u << 0) /**< \brief (PMC_PCK[3]) PLLB Clock is selected */
#define   PMC_PCK_CSS_MCK (0x4u << 0) /**< \brief (PMC_PCK[3]) Master Clock is selected */
#define PMC_PCK_PRES_Pos 4
#define PMC_PCK_PRES_Msk (0x7u << PMC_PCK_PRES_Pos) /**< \brief (PMC_PCK[3]) Programmable Clock Prescaler */
#define   PMC_PCK_PRES_CLK_1 (0x0u << 4) /**< \brief (PMC_PCK[3]) Selected clock */
#define   PMC_PCK_PRES_CLK_2 (0x1u << 4) /**< \brief (PMC_PCK[3]) Selected clock divided by 2 */
#define   PMC_PCK_PRES_CLK_4 (0x2u << 4) /**< \brief (PMC_PCK[3]) Selected clock divided by 4 */
#define   PMC_PCK_PRES_CLK_8 (0x3u << 4) /**< \brief (PMC_PCK[3]) Selected clock divided by 8 */
#define   PMC_PCK_PRES_CLK_16 (0x4u << 4) /**< \brief (PMC_PCK[3]) Selected clock divided by 16 */
#define   PMC_PCK_PRES_CLK_32 (0x5u << 4) /**< \brief (PMC_PCK[3]) Selected clock divided by 32 */
#define   PMC_PCK_PRES_CLK_64 (0x6u << 4) /**< \brief (PMC_PCK[3]) Selected clock divided by 64 */
/* -------- PMC_IER : (PMC Offset: 0x0060) Interrupt Enable Register -------- */
#define PMC_IER_MOSCXTS (0x1u << 0) /**< \brief (PMC_IER) Main Crystal Oscillator Status Interrupt Enable */
#define PMC_IER_LOCKA (0x1u << 1) /**< \brief (PMC_IER) PLLA Lock Interrupt Enable */
#define PMC_IER_LOCKB (0x1u << 2) /**< \brief (PMC_IER) PLLB Lock Interrupt Enable */
#define PMC_IER_MCKRDY (0x1u << 3) /**< \brief (PMC_IER) Master Clock Ready Interrupt Enable */
#define PMC_IER_PCKRDY0 (0x1u << 8) /**< \brief (PMC_IER) Programmable Clock Ready 0 Interrupt Enable */
#define PMC_IER_PCKRDY1 (0x1u << 9) /**< \brief (PMC_IER) Programmable Clock Ready 1 Interrupt Enable */
#define PMC_IER_PCKRDY2 (0x1u << 10) /**< \brief (PMC_IER) Programmable Clock Ready 2 Interrupt Enable */
#define PMC_IER_MOSCSELS (0x1u << 16) /**< \brief (PMC_IER) Main Oscillator Selection Status Interrupt Enable */
#define PMC_IER_MOSCRCS (0x1u << 17) /**< \brief (PMC_IER) Main On-Chip RC Status Interrupt Enable */
#define PMC_IER_CFDEV (0x1u << 18) /**< \brief (PMC_IER) Clock Failure Detector Event Interrupt Enable */
/* -------- PMC_IDR : (PMC Offset: 0x0064) Interrupt Disable Register -------- */
#define PMC_IDR_MOSCXTS (0x1u << 0) /**< \brief (PMC_IDR) Main Crystal Oscillator Status Interrupt Disable */
#define PMC_IDR_LOCKA (0x1u << 1) /**< \brief (PMC_IDR) PLLA Lock Interrupt Disable */
#define PMC_IDR_LOCKB (0x1u << 2) /**< \brief (PMC_IDR) PLLB Lock Interrupt Disable */
#define PMC_IDR_MCKRDY (0x1u << 3) /**< \brief (PMC_IDR) Master Clock Ready Interrupt Disable */
#define PMC_IDR_PCKRDY0 (0x1u << 8) /**< \brief (PMC_IDR) Programmable Clock Ready 0 Interrupt Disable */
#define PMC_IDR_PCKRDY1 (0x1u << 9) /**< \brief (PMC_IDR) Programmable Clock Ready 1 Interrupt Disable */
#define PMC_IDR_PCKRDY2 (0x1u << 10) /**< \brief (PMC_IDR) Programmable Clock Ready 2 Interrupt Disable */
#define PMC_IDR_MOSCSELS (0x1u << 16) /**< \brief (PMC_IDR) Main Oscillator Selection Status Interrupt Disable */
#define PMC_IDR_MOSCRCS (0x1u << 17) /**< \brief (PMC_IDR) Main On-Chip RC Status Interrupt Disable */
#define PMC_IDR_CFDEV (0x1u << 18) /**< \brief (PMC_IDR) Clock Failure Detector Event Interrupt Disable */
/* -------- PMC_SR : (PMC Offset: 0x0068) Status Register -------- */
#define PMC_SR_MOSCXTS (0x1u << 0) /**< \brief (PMC_SR) Main Crystal Oscillator Status */
#define PMC_SR_LOCKA (0x1u << 1) /**< \brief (PMC_SR) PLLA Lock Status */
#define PMC_SR_LOCKB (0x1u << 2) /**< \brief (PMC_SR) PLLB Lock Status */
#define PMC_SR_MCKRDY (0x1u << 3) /**< \brief (PMC_SR) Master Clock Status */
#define PMC_SR_OSCSELS (0x1u << 7) /**< \brief (PMC_SR) Slow Clock Oscillator Selection */
#define PMC_SR_PCKRDY0 (0x1u << 8) /**< \brief (PMC_SR) Programmable Clock Ready Status */
#define PMC_SR_PCKRDY1 (0x1u << 9) /**< \brief (PMC_SR) Programmable Clock Ready Status */
#define PMC_SR_PCKRDY2 (0x1u << 10) /**< \brief (PMC_SR) Programmable Clock Ready Status */
#define PMC_SR_MOSCSELS (0x1u << 16) /**< \brief (PMC_SR) Main Oscillator Selection Status */
#define PMC_SR_MOSCRCS (0x1u << 17) /**< \brief (PMC_SR) Main On-Chip RC Oscillator Status */
#define PMC_SR_CFDEV (0x1u << 18) /**< \brief (PMC_SR) Clock Failure Detector Event */
#define PMC_SR_CFDS (0x1u << 19) /**< \brief (PMC_SR) Clock Failure Detector Status */
#define PMC_SR_FOS (0x1u << 20) /**< \brief (PMC_SR) Clock Failure Detector Fault Output Status */
/* -------- PMC_IMR : (PMC Offset: 0x006C) Interrupt Mask Register -------- */
#define PMC_IMR_MOSCXTS (0x1u << 0) /**< \brief (PMC_IMR) Main Crystal Oscillator Status Interrupt Mask */
#define PMC_IMR_LOCKA (0x1u << 1) /**< \brief (PMC_IMR) PLLA Lock Interrupt Mask */
#define PMC_IMR_LOCKB (0x1u << 2) /**< \brief (PMC_IMR) PLLB Lock Interrupt Mask */
#define PMC_IMR_MCKRDY (0x1u << 3) /**< \brief (PMC_IMR) Master Clock Ready Interrupt Mask */
#define PMC_IMR_PCKRDY0 (0x1u << 8) /**< \brief (PMC_IMR) Programmable Clock Ready 0 Interrupt Mask */
#define PMC_IMR_PCKRDY1 (0x1u << 9) /**< \brief (PMC_IMR) Programmable Clock Ready 1 Interrupt Mask */
#define PMC_IMR_PCKRDY2 (0x1u << 10) /**< \brief (PMC_IMR) Programmable Clock Ready 2 Interrupt Mask */
#define PMC_IMR_MOSCSELS (0x1u << 16) /**< \brief (PMC_IMR) Main Oscillator Selection Status Interrupt Mask */
#define PMC_IMR_MOSCRCS (0x1u << 17) /**< \brief (PMC_IMR) Main On-Chip RC Status Interrupt Mask */
#define PMC_IMR_CFDEV (0x1u << 18) /**< \brief (PMC_IMR) Clock Failure Detector Event Interrupt Mask */
/* -------- PMC_FSMR : (PMC Offset: 0x0070) Fast Startup Mode Register -------- */
#define PMC_FSMR_FSTT0 (0x1u << 0) /**< \brief (PMC_FSMR) Fast Startup Input Enable 0 */
#define PMC_FSMR_FSTT1 (0x1u << 1) /**< \brief (PMC_FSMR) Fast Startup Input Enable 1 */
#define PMC_FSMR_FSTT2 (0x1u << 2) /**< \brief (PMC_FSMR) Fast Startup Input Enable 2 */
#define PMC_FSMR_FSTT3 (0x1u << 3) /**< \brief (PMC_FSMR) Fast Startup Input Enable 3 */
#define PMC_FSMR_FSTT4 (0x1u << 4) /**< \brief (PMC_FSMR) Fast Startup Input Enable 4 */
#define PMC_FSMR_FSTT5 (0x1u << 5) /**< \brief (PMC_FSMR) Fast Startup Input Enable 5 */
#define PMC_FSMR_FSTT6 (0x1u << 6) /**< \brief (PMC_FSMR) Fast Startup Input Enable 6 */
#define PMC_FSMR_FSTT7 (0x1u << 7) /**< \brief (PMC_FSMR) Fast Startup Input Enable 7 */
#define PMC_FSMR_FSTT8 (0x1u << 8) /**< \brief (PMC_FSMR) Fast Startup Input Enable 8 */
#define PMC_FSMR_FSTT9 (0x1u << 9) /**< \brief (PMC_FSMR) Fast Startup Input Enable 9 */
#define PMC_FSMR_FSTT10 (0x1u << 10) /**< \brief (PMC_FSMR) Fast Startup Input Enable 10 */
#define PMC_FSMR_FSTT11 (0x1u << 11) /**< \brief (PMC_FSMR) Fast Startup Input Enable 11 */
#define PMC_FSMR_FSTT12 (0x1u << 12) /**< \brief (PMC_FSMR) Fast Startup Input Enable 12 */
#define PMC_FSMR_FSTT13 (0x1u << 13) /**< \brief (PMC_FSMR) Fast Startup Input Enable 13 */
#define PMC_FSMR_FSTT14 (0x1u << 14) /**< \brief (PMC_FSMR) Fast Startup Input Enable 14 */
#define PMC_FSMR_FSTT15 (0x1u << 15) /**< \brief (PMC_FSMR) Fast Startup Input Enable 15 */
#define PMC_FSMR_RTTAL (0x1u << 16) /**< \brief (PMC_FSMR) RTT Alarm Enable */
#define PMC_FSMR_RTCAL (0x1u << 17) /**< \brief (PMC_FSMR) RTC Alarm Enable */
#define PMC_FSMR_USBAL (0x1u << 18) /**< \brief (PMC_FSMR) USB Alarm Enable */
#define PMC_FSMR_LPM (0x1u << 20) /**< \brief (PMC_FSMR) Low-power Mode */
#define PMC_FSMR_FLPM_Pos 21
#define PMC_FSMR_FLPM_Msk (0x3u << PMC_FSMR_FLPM_Pos) /**< \brief (PMC_FSMR) Flash Low-power Mode */
#define   PMC_FSMR_FLPM_FLASH_STANDBY (0x0u << 21) /**< \brief (PMC_FSMR) Flash is in Standby Mode when system enters Wait Mode */
#define   PMC_FSMR_FLPM_FLASH_DEEP_POWERDOWN (0x1u << 21) /**< \brief (PMC_FSMR) Flash is in Deep-power-down mode when system enters Wait Mode */
#define   PMC_FSMR_FLPM_FLASH_IDLE (0x2u << 21) /**< \brief (PMC_FSMR) Idle mode */
/* -------- PMC_FSPR : (PMC Offset: 0x0074) Fast Startup Polarity Register -------- */
#define PMC_FSPR_FSTP0 (0x1u << 0) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP1 (0x1u << 1) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP2 (0x1u << 2) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP3 (0x1u << 3) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP4 (0x1u << 4) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP5 (0x1u << 5) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP6 (0x1u << 6) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP7 (0x1u << 7) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP8 (0x1u << 8) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP9 (0x1u << 9) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP10 (0x1u << 10) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP11 (0x1u << 11) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP12 (0x1u << 12) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP13 (0x1u << 13) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP14 (0x1u << 14) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
#define PMC_FSPR_FSTP15 (0x1u << 15) /**< \brief (PMC_FSPR) Fast Startup Input Polarityx */
/* -------- PMC_FOCR : (PMC Offset: 0x0078) Fault Output Clear Register -------- */
#define PMC_FOCR_FOCLR (0x1u << 0) /**< \brief (PMC_FOCR) Fault Output Clear */
/* -------- PMC_WPMR : (PMC Offset: 0x00E4) Write Protection Mode Register -------- */
#define PMC_WPMR_WPEN (0x1u << 0) /**< \brief (PMC_WPMR) Write Protection Enable */
#define PMC_WPMR_WPKEY_Pos 8
#define PMC_WPMR_WPKEY_Msk (0xffffffu << PMC_WPMR_WPKEY_Pos) /**< \brief (PMC_WPMR) Write Protection Key */
#define   PMC_WPMR_WPKEY_PASSWD (0x504D43u << 8) /**< \brief (PMC_WPMR) Writing any other value in this field aborts the write operation of the WPEN bit. Always reads as 0. */
/* -------- PMC_WPSR : (PMC Offset: 0x00E8) Write Protection Status Register -------- */
#define PMC_WPSR_WPVS (0x1u << 0) /**< \brief (PMC_WPSR) Write Protection Violation Status */
#define PMC_WPSR_WPVSRC_Pos 8
#define PMC_WPSR_WPVSRC_Msk (0xffffu << PMC_WPSR_WPVSRC_Pos) /**< \brief (PMC_WPSR) Write Protection Violation Source */
/* -------- PMC_PCER1 : (PMC Offset: 0x0100) Peripheral Clock Enable Register 1 -------- */
#define PMC_PCER1_PID32 (0x1u << 0) /**< \brief (PMC_PCER1) Peripheral Clock 32 Enable */
#define PMC_PCER1_PID33 (0x1u << 1) /**< \brief (PMC_PCER1) Peripheral Clock 33 Enable */
#define PMC_PCER1_PID34 (0x1u << 2) /**< \brief (PMC_PCER1) Peripheral Clock 34 Enable */
/* -------- PMC_PCDR1 : (PMC Offset: 0x0104) Peripheral Clock Disable Register 1 -------- */
#define PMC_PCDR1_PID32 (0x1u << 0) /**< \brief (PMC_PCDR1) Peripheral Clock 32 Disable */
#define PMC_PCDR1_PID33 (0x1u << 1) /**< \brief (PMC_PCDR1) Peripheral Clock 33 Disable */
#define PMC_PCDR1_PID34 (0x1u << 2) /**< \brief (PMC_PCDR1) Peripheral Clock 34 Disable */
/* -------- PMC_PCSR1 : (PMC Offset: 0x0108) Peripheral Clock Status Register 1 -------- */
#define PMC_PCSR1_PID32 (0x1u << 0) /**< \brief (PMC_PCSR1) Peripheral Clock 32 Status */
#define PMC_PCSR1_PID33 (0x1u << 1) /**< \brief (PMC_PCSR1) Peripheral Clock 33 Status */
#define PMC_PCSR1_PID34 (0x1u << 2) /**< \brief (PMC_PCSR1) Peripheral Clock 34 Status */
/* -------- PMC_OCR : (PMC Offset: 0x0110) Oscillator Calibration Register -------- */
#define PMC_OCR_CAL4_Pos 0
#define PMC_OCR_CAL4_Msk (0x7fu << PMC_OCR_CAL4_Pos) /**< \brief (PMC_OCR) RC Oscillator Calibration bits for 4 MHz */
#define PMC_OCR_CAL4(value) ((PMC_OCR_CAL4_Msk & ((value) << PMC_OCR_CAL4_Pos)))
#define PMC_OCR_SEL4 (0x1u << 7) /**< \brief (PMC_OCR) Selection of RC Oscillator Calibration bits for 4 MHz */
#define PMC_OCR_CAL8_Pos 8
#define PMC_OCR_CAL8_Msk (0x7fu << PMC_OCR_CAL8_Pos) /**< \brief (PMC_OCR) RC Oscillator Calibration bits for 8 MHz */
#define PMC_OCR_CAL8(value) ((PMC_OCR_CAL8_Msk & ((value) << PMC_OCR_CAL8_Pos)))
#define PMC_OCR_SEL8 (0x1u << 15) /**< \brief (PMC_OCR) Selection of RC Oscillator Calibration bits for 8 MHz */
#define PMC_OCR_CAL12_Pos 16
#define PMC_OCR_CAL12_Msk (0x7fu << PMC_OCR_CAL12_Pos) /**< \brief (PMC_OCR) RC Oscillator Calibration bits for 12 MHz */
#define PMC_OCR_CAL12(value) ((PMC_OCR_CAL12_Msk & ((value) << PMC_OCR_CAL12_Pos)))
#define PMC_OCR_SEL12 (0x1u << 23) /**< \brief (PMC_OCR) Selection of RC Oscillator Calibration bits for 12 MHz */


//EFC
typedef struct {
	__IO uint32_t EEFC_FMR; /**< \brief (Efc Offset: 0x00) EEFC Flash Mode Register */
	__O  uint32_t EEFC_FCR; /**< \brief (Efc Offset: 0x04) EEFC Flash Command Register */
	__I  uint32_t EEFC_FSR; /**< \brief (Efc Offset: 0x08) EEFC Flash Status Register */
	__I  uint32_t EEFC_FRR; /**< \brief (Efc Offset: 0x0C) EEFC Flash Result Register */
} Efc;

#define EEFC_FMR_FRDY (0x1u << 0) /**< \brief (EEFC_FMR) Ready Interrupt Enable */
#define EEFC_FMR_FWS_Pos 8
#define EEFC_FMR_FWS_Msk (0xfu << EEFC_FMR_FWS_Pos) /**< \brief (EEFC_FMR) Flash Wait State */
#define EEFC_FMR_FWS(value) ((EEFC_FMR_FWS_Msk & ((value) << EEFC_FMR_FWS_Pos)))
#define EEFC_FMR_SCOD (0x1u << 16) /**< \brief (EEFC_FMR) Sequential Code Optimization Disable */
#define EEFC_FMR_FAM (0x1u << 24) /**< \brief (EEFC_FMR) Flash Access Mode */
#define EEFC_FMR_CLOE (0x1u << 26) /**< \brief (EEFC_FMR) Code Loop Optimization Enable */
/* -------- EEFC_FCR : (EFC Offset: 0x04) EEFC Flash Command Register -------- */
#define EEFC_FCR_FCMD_Pos 0
#define EEFC_FCR_FCMD_Msk (0xffu << EEFC_FCR_FCMD_Pos) /**< \brief (EEFC_FCR) Flash Command */
#define EEFC_FCR_FCMD_GETD (0x0u << 0) /**< \brief (EEFC_FCR) Get Flash descriptor */
#define EEFC_FCR_FCMD_WP (0x1u << 0) /**< \brief (EEFC_FCR) Write page */
#define EEFC_FCR_FCMD_WPL (0x2u << 0) /**< \brief (EEFC_FCR) Write page and lock */
#define EEFC_FCR_FCMD_EWP (0x3u << 0) /**< \brief (EEFC_FCR) Erase page and write page */
#define EEFC_FCR_FCMD_EWPL (0x4u << 0) /**< \brief (EEFC_FCR) Erase page and write page then lock */
#define EEFC_FCR_FCMD_EA (0x5u << 0) /**< \brief (EEFC_FCR) Erase all */
#define EEFC_FCR_FCMD_EPA (0x7u << 0) /**< \brief (EEFC_FCR) Erase pages */
#define EEFC_FCR_FCMD_SLB (0x8u << 0) /**< \brief (EEFC_FCR) Set lock bit */
#define EEFC_FCR_FCMD_CLB (0x9u << 0) /**< \brief (EEFC_FCR) Clear lock bit */
#define EEFC_FCR_FCMD_GLB (0xAu << 0) /**< \brief (EEFC_FCR) Get lock bit */
#define EEFC_FCR_FCMD_SGPB (0xBu << 0) /**< \brief (EEFC_FCR) Set GPNVM bit */
#define EEFC_FCR_FCMD_CGPB (0xCu << 0) /**< \brief (EEFC_FCR) Clear GPNVM bit */
#define EEFC_FCR_FCMD_GGPB (0xDu << 0) /**< \brief (EEFC_FCR) Get GPNVM bit */
#define EEFC_FCR_FCMD_STUI (0xEu << 0) /**< \brief (EEFC_FCR) Start read unique identifier */
#define EEFC_FCR_FCMD_SPUI (0xFu << 0) /**< \brief (EEFC_FCR) Stop read unique identifier */
#define EEFC_FCR_FCMD_GCALB (0x10u << 0) /**< \brief (EEFC_FCR) Get CALIB bit */
#define EEFC_FCR_FCMD_ES (0x11u << 0) /**< \brief (EEFC_FCR) Erase sector */
#define EEFC_FCR_FCMD_WUS (0x12u << 0) /**< \brief (EEFC_FCR) Write user signature */
#define EEFC_FCR_FCMD_EUS (0x13u << 0) /**< \brief (EEFC_FCR) Erase user signature */
#define EEFC_FCR_FCMD_STUS (0x14u << 0) /**< \brief (EEFC_FCR) Start read user signature */
#define EEFC_FCR_FCMD_SPUS (0x15u << 0) /**< \brief (EEFC_FCR) Stop read user signature */
#define EEFC_FCR_FARG_Pos 8
#define EEFC_FCR_FARG_Msk (0xffffu << EEFC_FCR_FARG_Pos) /**< \brief (EEFC_FCR) Flash Command Argument */
#define EEFC_FCR_FARG(value) ((EEFC_FCR_FARG_Msk & ((value) << EEFC_FCR_FARG_Pos)))
#define EEFC_FCR_FKEY_Pos 24
#define EEFC_FCR_FKEY_Msk (0xffu << EEFC_FCR_FKEY_Pos) /**< \brief (EEFC_FCR) Flash Writing Protection Key */
#define EEFC_FCR_FKEY_PASSWD (0x5Au << 24) /**< \brief (EEFC_FCR) The 0x5A value enables the command defined by the bits of the register. If the field is written with a different value, the write is not performed and no action is started. */
/* -------- EEFC_FSR : (EFC Offset: 0x08) EEFC Flash Status Register -------- */
#define EEFC_FSR_FRDY (0x1u << 0) /**< \brief (EEFC_FSR) Flash Ready Status */
#define EEFC_FSR_FCMDE (0x1u << 1) /**< \brief (EEFC_FSR) Flash Command Error Status */
#define EEFC_FSR_FLOCKE (0x1u << 2) /**< \brief (EEFC_FSR) Flash Lock Error Status */
#define EEFC_FSR_FLERR (0x1u << 3) /**< \brief (EEFC_FSR) Flash Error Status */
/* -------- EEFC_FRR : (EFC Offset: 0x0C) EEFC Flash Result Register -------- */
#define EEFC_FRR_FVALUE_Pos 0
#define EEFC_FRR_FVALUE_Msk (0xffffffffu << EEFC_FRR_FVALUE_Pos) /**< \brief (EEFC_FRR) Flash Result Value */<Paste>

/** \brief  Structure type to access the System Control Block (SCB).
 */
typedef struct
{
  __I  uint32_t CPUID;                   /*!< Offset: 0x000 (R/ )  CPUID Base Register                                   */
  __IO uint32_t ICSR;                    /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register                  */
  __IO uint32_t VTOR;                    /*!< Offset: 0x008 (R/W)  Vector Table Offset Register                          */
  __IO uint32_t AIRCR;                   /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register      */
  __IO uint32_t SCR;                     /*!< Offset: 0x010 (R/W)  System Control Register                               */
  __IO uint32_t CCR;                     /*!< Offset: 0x014 (R/W)  Configuration Control Register                        */
  __IO uint8_t  SHP[12];                 /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
  __IO uint32_t SHCSR;                   /*!< Offset: 0x024 (R/W)  System Handler Control and State Register             */
  __IO uint32_t CFSR;                    /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register                    */
  __IO uint32_t HFSR;                    /*!< Offset: 0x02C (R/W)  HardFault Status Register                             */
  __IO uint32_t DFSR;                    /*!< Offset: 0x030 (R/W)  Debug Fault Status Register                           */
  __IO uint32_t MMFAR;                   /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register                      */
  __IO uint32_t BFAR;                    /*!< Offset: 0x038 (R/W)  BusFault Address Register                             */
  __IO uint32_t AFSR;                    /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register                       */
  __I  uint32_t PFR[2];                  /*!< Offset: 0x040 (R/ )  Processor Feature Register                            */
  __I  uint32_t DFR;                     /*!< Offset: 0x048 (R/ )  Debug Feature Register                                */
  __I  uint32_t ADR;                     /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register                            */
  __I  uint32_t MMFR[4];                 /*!< Offset: 0x050 (R/ )  Memory Model Feature Register                         */
  __I  uint32_t ISAR[5];                 /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register                   */
       uint32_t RESERVED0[5];
  __IO uint32_t CPACR;                   /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register                   */
} SCB_Type;

/* SCB CPUID Register Definitions */
#define SCB_CPUID_IMPLEMENTER_Pos          24                                             /*!< SCB CPUID: IMPLEMENTER Position */
#define SCB_CPUID_IMPLEMENTER_Msk          (0xFFUL << SCB_CPUID_IMPLEMENTER_Pos)          /*!< SCB CPUID: IMPLEMENTER Mask */

#define SCB_CPUID_VARIANT_Pos              20                                             /*!< SCB CPUID: VARIANT Position */
#define SCB_CPUID_VARIANT_Msk              (0xFUL << SCB_CPUID_VARIANT_Pos)               /*!< SCB CPUID: VARIANT Mask */

#define SCB_CPUID_ARCHITECTURE_Pos         16                                             /*!< SCB CPUID: ARCHITECTURE Position */
#define SCB_CPUID_ARCHITECTURE_Msk         (0xFUL << SCB_CPUID_ARCHITECTURE_Pos)          /*!< SCB CPUID: ARCHITECTURE Mask */

#define SCB_CPUID_PARTNO_Pos                4                                             /*!< SCB CPUID: PARTNO Position */
#define SCB_CPUID_PARTNO_Msk               (0xFFFUL << SCB_CPUID_PARTNO_Pos)              /*!< SCB CPUID: PARTNO Mask */

#define SCB_CPUID_REVISION_Pos              0                                             /*!< SCB CPUID: REVISION Position */
#define SCB_CPUID_REVISION_Msk             (0xFUL /*<< SCB_CPUID_REVISION_Pos*/)          /*!< SCB CPUID: REVISION Mask */

/* SCB Interrupt Control State Register Definitions */
#define SCB_ICSR_NMIPENDSET_Pos            31                                             /*!< SCB ICSR: NMIPENDSET Position */
#define SCB_ICSR_NMIPENDSET_Msk            (1UL << SCB_ICSR_NMIPENDSET_Pos)               /*!< SCB ICSR: NMIPENDSET Mask */

#define SCB_ICSR_PENDSVSET_Pos             28                                             /*!< SCB ICSR: PENDSVSET Position */
#define SCB_ICSR_PENDSVSET_Msk             (1UL << SCB_ICSR_PENDSVSET_Pos)                /*!< SCB ICSR: PENDSVSET Mask */

#define SCB_ICSR_PENDSVCLR_Pos             27                                             /*!< SCB ICSR: PENDSVCLR Position */
#define SCB_ICSR_PENDSVCLR_Msk             (1UL << SCB_ICSR_PENDSVCLR_Pos)                /*!< SCB ICSR: PENDSVCLR Mask */

#define SCB_ICSR_PENDSTSET_Pos             26                                             /*!< SCB ICSR: PENDSTSET Position */
#define SCB_ICSR_PENDSTSET_Msk             (1UL << SCB_ICSR_PENDSTSET_Pos)                /*!< SCB ICSR: PENDSTSET Mask */

#define SCB_ICSR_PENDSTCLR_Pos             25                                             /*!< SCB ICSR: PENDSTCLR Position */
#define SCB_ICSR_PENDSTCLR_Msk             (1UL << SCB_ICSR_PENDSTCLR_Pos)                /*!< SCB ICSR: PENDSTCLR Mask */

#define SCB_ICSR_ISRPREEMPT_Pos            23                                             /*!< SCB ICSR: ISRPREEMPT Position */
#define SCB_ICSR_ISRPREEMPT_Msk            (1UL << SCB_ICSR_ISRPREEMPT_Pos)               /*!< SCB ICSR: ISRPREEMPT Mask */

#define SCB_ICSR_ISRPENDING_Pos            22                                             /*!< SCB ICSR: ISRPENDING Position */
#define SCB_ICSR_ISRPENDING_Msk            (1UL << SCB_ICSR_ISRPENDING_Pos)               /*!< SCB ICSR: ISRPENDING Mask */

#define SCB_ICSR_VECTPENDING_Pos           12                                             /*!< SCB ICSR: VECTPENDING Position */
#define SCB_ICSR_VECTPENDING_Msk           (0x1FFUL << SCB_ICSR_VECTPENDING_Pos)          /*!< SCB ICSR: VECTPENDING Mask */

#define SCB_ICSR_RETTOBASE_Pos             11                                             /*!< SCB ICSR: RETTOBASE Position */
#define SCB_ICSR_RETTOBASE_Msk             (1UL << SCB_ICSR_RETTOBASE_Pos)                /*!< SCB ICSR: RETTOBASE Mask */

#define SCB_ICSR_VECTACTIVE_Pos             0                                             /*!< SCB ICSR: VECTACTIVE Position */
#define SCB_ICSR_VECTACTIVE_Msk            (0x1FFUL /*<< SCB_ICSR_VECTACTIVE_Pos*/)       /*!< SCB ICSR: VECTACTIVE Mask */

/* SCB Vector Table Offset Register Definitions */
#define SCB_VTOR_TBLOFF_Pos                 7                                             /*!< SCB VTOR: TBLOFF Position */
#define SCB_VTOR_TBLOFF_Msk                (0x1FFFFFFUL << SCB_VTOR_TBLOFF_Pos)           /*!< SCB VTOR: TBLOFF Mask */

/* SCB Application Interrupt and Reset Control Register Definitions */
#define SCB_AIRCR_VECTKEY_Pos              16                                             /*!< SCB AIRCR: VECTKEY Position */
#define SCB_AIRCR_VECTKEY_Msk              (0xFFFFUL << SCB_AIRCR_VECTKEY_Pos)            /*!< SCB AIRCR: VECTKEY Mask */

#define SCB_AIRCR_VECTKEYSTAT_Pos          16                                             /*!< SCB AIRCR: VECTKEYSTAT Position */
#define SCB_AIRCR_VECTKEYSTAT_Msk          (0xFFFFUL << SCB_AIRCR_VECTKEYSTAT_Pos)        /*!< SCB AIRCR: VECTKEYSTAT Mask */

#define SCB_AIRCR_ENDIANESS_Pos            15                                             /*!< SCB AIRCR: ENDIANESS Position */
#define SCB_AIRCR_ENDIANESS_Msk            (1UL << SCB_AIRCR_ENDIANESS_Pos)               /*!< SCB AIRCR: ENDIANESS Mask */

#define SCB_AIRCR_PRIGROUP_Pos              8                                             /*!< SCB AIRCR: PRIGROUP Position */
#define SCB_AIRCR_PRIGROUP_Msk             (7UL << SCB_AIRCR_PRIGROUP_Pos)                /*!< SCB AIRCR: PRIGROUP Mask */

#define SCB_AIRCR_SYSRESETREQ_Pos           2                                             /*!< SCB AIRCR: SYSRESETREQ Position */
#define SCB_AIRCR_SYSRESETREQ_Msk          (1UL << SCB_AIRCR_SYSRESETREQ_Pos)             /*!< SCB AIRCR: SYSRESETREQ Mask */

#define SCB_AIRCR_VECTCLRACTIVE_Pos         1                                             /*!< SCB AIRCR: VECTCLRACTIVE Position */
#define SCB_AIRCR_VECTCLRACTIVE_Msk        (1UL << SCB_AIRCR_VECTCLRACTIVE_Pos)           /*!< SCB AIRCR: VECTCLRACTIVE Mask */

#define SCB_AIRCR_VECTRESET_Pos             0                                             /*!< SCB AIRCR: VECTRESET Position */
#define SCB_AIRCR_VECTRESET_Msk            (1UL /*<< SCB_AIRCR_VECTRESET_Pos*/)           /*!< SCB AIRCR: VECTRESET Mask */

/* SCB System Control Register Definitions */
#define SCB_SCR_SEVONPEND_Pos               4                                             /*!< SCB SCR: SEVONPEND Position */
#define SCB_SCR_SEVONPEND_Msk              (1UL << SCB_SCR_SEVONPEND_Pos)                 /*!< SCB SCR: SEVONPEND Mask */

#define SCB_SCR_SLEEPDEEP_Pos               2                                             /*!< SCB SCR: SLEEPDEEP Position */
#define SCB_SCR_SLEEPDEEP_Msk              (1UL << SCB_SCR_SLEEPDEEP_Pos)                 /*!< SCB SCR: SLEEPDEEP Mask */

#define SCB_SCR_SLEEPONEXIT_Pos             1                                             /*!< SCB SCR: SLEEPONEXIT Position */
#define SCB_SCR_SLEEPONEXIT_Msk            (1UL << SCB_SCR_SLEEPONEXIT_Pos)               /*!< SCB SCR: SLEEPONEXIT Mask */

/* SCB Configuration Control Register Definitions */
#define SCB_CCR_STKALIGN_Pos                9                                             /*!< SCB CCR: STKALIGN Position */
#define SCB_CCR_STKALIGN_Msk               (1UL << SCB_CCR_STKALIGN_Pos)                  /*!< SCB CCR: STKALIGN Mask */

#define SCB_CCR_BFHFNMIGN_Pos               8                                             /*!< SCB CCR: BFHFNMIGN Position */
#define SCB_CCR_BFHFNMIGN_Msk              (1UL << SCB_CCR_BFHFNMIGN_Pos)                 /*!< SCB CCR: BFHFNMIGN Mask */

#define SCB_CCR_DIV_0_TRP_Pos               4                                             /*!< SCB CCR: DIV_0_TRP Position */
#define SCB_CCR_DIV_0_TRP_Msk              (1UL << SCB_CCR_DIV_0_TRP_Pos)                 /*!< SCB CCR: DIV_0_TRP Mask */

#define SCB_CCR_UNALIGN_TRP_Pos             3                                             /*!< SCB CCR: UNALIGN_TRP Position */
#define SCB_CCR_UNALIGN_TRP_Msk            (1UL << SCB_CCR_UNALIGN_TRP_Pos)               /*!< SCB CCR: UNALIGN_TRP Mask */

#define SCB_CCR_USERSETMPEND_Pos            1                                             /*!< SCB CCR: USERSETMPEND Position */
#define SCB_CCR_USERSETMPEND_Msk           (1UL << SCB_CCR_USERSETMPEND_Pos)              /*!< SCB CCR: USERSETMPEND Mask */

#define SCB_CCR_NONBASETHRDENA_Pos          0                                             /*!< SCB CCR: NONBASETHRDENA Position */
#define SCB_CCR_NONBASETHRDENA_Msk         (1UL /*<< SCB_CCR_NONBASETHRDENA_Pos*/)        /*!< SCB CCR: NONBASETHRDENA Mask */

/* SCB System Handler Control and State Register Definitions */
#define SCB_SHCSR_USGFAULTENA_Pos          18                                             /*!< SCB SHCSR: USGFAULTENA Position */
#define SCB_SHCSR_USGFAULTENA_Msk          (1UL << SCB_SHCSR_USGFAULTENA_Pos)             /*!< SCB SHCSR: USGFAULTENA Mask */

#define SCB_SHCSR_BUSFAULTENA_Pos          17                                             /*!< SCB SHCSR: BUSFAULTENA Position */
#define SCB_SHCSR_BUSFAULTENA_Msk          (1UL << SCB_SHCSR_BUSFAULTENA_Pos)             /*!< SCB SHCSR: BUSFAULTENA Mask */

#define SCB_SHCSR_MEMFAULTENA_Pos          16                                             /*!< SCB SHCSR: MEMFAULTENA Position */
#define SCB_SHCSR_MEMFAULTENA_Msk          (1UL << SCB_SHCSR_MEMFAULTENA_Pos)             /*!< SCB SHCSR: MEMFAULTENA Mask */

#define SCB_SHCSR_SVCALLPENDED_Pos         15                                             /*!< SCB SHCSR: SVCALLPENDED Position */
#define SCB_SHCSR_SVCALLPENDED_Msk         (1UL << SCB_SHCSR_SVCALLPENDED_Pos)            /*!< SCB SHCSR: SVCALLPENDED Mask */

#define SCB_SHCSR_BUSFAULTPENDED_Pos       14                                             /*!< SCB SHCSR: BUSFAULTPENDED Position */
#define SCB_SHCSR_BUSFAULTPENDED_Msk       (1UL << SCB_SHCSR_BUSFAULTPENDED_Pos)          /*!< SCB SHCSR: BUSFAULTPENDED Mask */

#define SCB_SHCSR_MEMFAULTPENDED_Pos       13                                             /*!< SCB SHCSR: MEMFAULTPENDED Position */
#define SCB_SHCSR_MEMFAULTPENDED_Msk       (1UL << SCB_SHCSR_MEMFAULTPENDED_Pos)          /*!< SCB SHCSR: MEMFAULTPENDED Mask */

#define SCB_SHCSR_USGFAULTPENDED_Pos       12                                             /*!< SCB SHCSR: USGFAULTPENDED Position */
#define SCB_SHCSR_USGFAULTPENDED_Msk       (1UL << SCB_SHCSR_USGFAULTPENDED_Pos)          /*!< SCB SHCSR: USGFAULTPENDED Mask */

#define SCB_SHCSR_SYSTICKACT_Pos           11                                             /*!< SCB SHCSR: SYSTICKACT Position */
#define SCB_SHCSR_SYSTICKACT_Msk           (1UL << SCB_SHCSR_SYSTICKACT_Pos)              /*!< SCB SHCSR: SYSTICKACT Mask */

#define SCB_SHCSR_PENDSVACT_Pos            10                                             /*!< SCB SHCSR: PENDSVACT Position */
#define SCB_SHCSR_PENDSVACT_Msk            (1UL << SCB_SHCSR_PENDSVACT_Pos)               /*!< SCB SHCSR: PENDSVACT Mask */

#define SCB_SHCSR_MONITORACT_Pos            8                                             /*!< SCB SHCSR: MONITORACT Position */
#define SCB_SHCSR_MONITORACT_Msk           (1UL << SCB_SHCSR_MONITORACT_Pos)              /*!< SCB SHCSR: MONITORACT Mask */

#define SCB_SHCSR_SVCALLACT_Pos             7                                             /*!< SCB SHCSR: SVCALLACT Position */
#define SCB_SHCSR_SVCALLACT_Msk            (1UL << SCB_SHCSR_SVCALLACT_Pos)               /*!< SCB SHCSR: SVCALLACT Mask */

#define SCB_SHCSR_USGFAULTACT_Pos           3                                             /*!< SCB SHCSR: USGFAULTACT Position */
#define SCB_SHCSR_USGFAULTACT_Msk          (1UL << SCB_SHCSR_USGFAULTACT_Pos)             /*!< SCB SHCSR: USGFAULTACT Mask */

#define SCB_SHCSR_BUSFAULTACT_Pos           1                                             /*!< SCB SHCSR: BUSFAULTACT Position */
#define SCB_SHCSR_BUSFAULTACT_Msk          (1UL << SCB_SHCSR_BUSFAULTACT_Pos)             /*!< SCB SHCSR: BUSFAULTACT Mask */

#define SCB_SHCSR_MEMFAULTACT_Pos           0                                             /*!< SCB SHCSR: MEMFAULTACT Position */
#define SCB_SHCSR_MEMFAULTACT_Msk          (1UL /*<< SCB_SHCSR_MEMFAULTACT_Pos*/)         /*!< SCB SHCSR: MEMFAULTACT Mask */

/* SCB Configurable Fault Status Registers Definitions */
#define SCB_CFSR_USGFAULTSR_Pos            16                                             /*!< SCB CFSR: Usage Fault Status Register Position */
#define SCB_CFSR_USGFAULTSR_Msk            (0xFFFFUL << SCB_CFSR_USGFAULTSR_Pos)          /*!< SCB CFSR: Usage Fault Status Register Mask */

#define SCB_CFSR_BUSFAULTSR_Pos             8                                             /*!< SCB CFSR: Bus Fault Status Register Position */
#define SCB_CFSR_BUSFAULTSR_Msk            (0xFFUL << SCB_CFSR_BUSFAULTSR_Pos)            /*!< SCB CFSR: Bus Fault Status Register Mask */

#define SCB_CFSR_MEMFAULTSR_Pos             0                                             /*!< SCB CFSR: Memory Manage Fault Status Register Position */
#define SCB_CFSR_MEMFAULTSR_Msk            (0xFFUL /*<< SCB_CFSR_MEMFAULTSR_Pos*/)        /*!< SCB CFSR: Memory Manage Fault Status Register Mask */

/* SCB Hard Fault Status Registers Definitions */
#define SCB_HFSR_DEBUGEVT_Pos              31                                             /*!< SCB HFSR: DEBUGEVT Position */
#define SCB_HFSR_DEBUGEVT_Msk              (1UL << SCB_HFSR_DEBUGEVT_Pos)                 /*!< SCB HFSR: DEBUGEVT Mask */

#define SCB_HFSR_FORCED_Pos                30                                             /*!< SCB HFSR: FORCED Position */
#define SCB_HFSR_FORCED_Msk                (1UL << SCB_HFSR_FORCED_Pos)                   /*!< SCB HFSR: FORCED Mask */

#define SCB_HFSR_VECTTBL_Pos                1                                             /*!< SCB HFSR: VECTTBL Position */
#define SCB_HFSR_VECTTBL_Msk               (1UL << SCB_HFSR_VECTTBL_Pos)                  /*!< SCB HFSR: VECTTBL Mask */

/* SCB Debug Fault Status Register Definitions */
#define SCB_DFSR_EXTERNAL_Pos               4                                             /*!< SCB DFSR: EXTERNAL Position */
#define SCB_DFSR_EXTERNAL_Msk              (1UL << SCB_DFSR_EXTERNAL_Pos)                 /*!< SCB DFSR: EXTERNAL Mask */

#define SCB_DFSR_VCATCH_Pos                 3                                             /*!< SCB DFSR: VCATCH Position */
#define SCB_DFSR_VCATCH_Msk                (1UL << SCB_DFSR_VCATCH_Pos)                   /*!< SCB DFSR: VCATCH Mask */

#define SCB_DFSR_DWTTRAP_Pos                2                                             /*!< SCB DFSR: DWTTRAP Position */
#define SCB_DFSR_DWTTRAP_Msk               (1UL << SCB_DFSR_DWTTRAP_Pos)                  /*!< SCB DFSR: DWTTRAP Mask */

#define SCB_DFSR_BKPT_Pos                   1                                             /*!< SCB DFSR: BKPT Position */
#define SCB_DFSR_BKPT_Msk                  (1UL << SCB_DFSR_BKPT_Pos)                     /*!< SCB DFSR: BKPT Mask */

#define SCB_DFSR_HALTED_Pos                 0                                             /*!< SCB DFSR: HALTED Position */
#define SCB_DFSR_HALTED_Msk                (1UL /*<< SCB_DFSR_HALTED_Pos*/)               /*!< SCB DFSR: HALTED Mask */

/*@} end of group CMSIS_SCB */

/* Memory mapping of Cortex-M4 Hardware */
#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address  */
#define ITM_BASE            (0xE0000000UL)                            /*!< ITM Base Address                   */
#define DWT_BASE            (0xE0001000UL)                            /*!< DWT Base Address                   */
#define TPI_BASE            (0xE0040000UL)                            /*!< TPI Base Address                   */
#define CoreDebug_BASE      (0xE000EDF0UL)                            /*!< Core Debug Base Address            */
#define SysTick_BASE        (SCS_BASE +  0x0010UL)                    /*!< SysTick Base Address               */
#define NVIC_BASE           (SCS_BASE +  0x0100UL)                    /*!< NVIC Base Address                  */
#define SCB_BASE            (SCS_BASE +  0x0D00UL)                    /*!< System Control Block Base Address  */

#define SCnSCB              ((SCnSCB_Type    *)     SCS_BASE      )   /*!< System control Register not in SCB */
#define SCB                 ((SCB_Type       *)     SCB_BASE      )   /*!< SCB configuration struct           */
#define SysTick             ((SysTick_Type   *)     SysTick_BASE  )   /*!< SysTick configuration struct       */
#define NVIC                ((NVIC_Type      *)     NVIC_BASE     )   /*!< NVIC configuration struct          */
#define ITM                 ((ITM_Type       *)     ITM_BASE      )   /*!< ITM configuration struct           */
#define DWT                 ((DWT_Type       *)     DWT_BASE      )   /*!< DWT configuration struct           */
#define TPI                 ((TPI_Type       *)     TPI_BASE      )   /*!< TPI configuration struct           */
#define CoreDebug           ((CoreDebug_Type *)     CoreDebug_BASE)   /*!< Core Debug configuration struct    */

#if (__MPU_PRESENT == 1)
  #define MPU_BASE          (SCS_BASE +  0x0D90UL)                    /*!< Memory Protection Unit             */
  #define MPU               ((MPU_Type       *)     MPU_BASE      )   /*!< Memory Protection Unit             */
#endif

#if (__FPU_PRESENT == 1)
  #define FPU_BASE          (SCS_BASE +  0x0F30UL)                    /*!< Floating Point Unit                */
  #define FPU               ((FPU_Type       *)     FPU_BASE      )   /*!< Floating Point Unit                */
#endif

/* Memory mapping of Cortex-M4 Hardware */
#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address  */
#define ITM_BASE            (0xE0000000UL)                            /*!< ITM Base Address                   */
#define DWT_BASE            (0xE0001000UL)                            /*!< DWT Base Address                   */
#define TPI_BASE            (0xE0040000UL)                            /*!< TPI Base Address                   */
#define CoreDebug_BASE      (0xE000EDF0UL)                            /*!< Core Debug Base Address            */
#define SysTick_BASE        (SCS_BASE +  0x0010UL)                    /*!< SysTick Base Address               */
#define NVIC_BASE           (SCS_BASE +  0x0100UL)                    /*!< NVIC Base Address                  */
#define SCB_BASE            (SCS_BASE +  0x0D00UL)                    /*!< System Control Block Base Address  */

#define SCnSCB              ((SCnSCB_Type    *)     SCS_BASE      )   /*!< System control Register not in SCB */
#define SCB                 ((SCB_Type       *)     SCB_BASE      )   /*!< SCB configuration struct           */
#define SysTick             ((SysTick_Type   *)     SysTick_BASE  )   /*!< SysTick configuration struct       */
#define NVIC                ((NVIC_Type      *)     NVIC_BASE     )   /*!< NVIC configuration struct          */
#define ITM                 ((ITM_Type       *)     ITM_BASE      )   /*!< ITM configuration struct           */
#define DWT                 ((DWT_Type       *)     DWT_BASE      )   /*!< DWT configuration struct           */
#define TPI                 ((TPI_Type       *)     TPI_BASE      )   /*!< TPI configuration struct           */
#define CoreDebug           ((CoreDebug_Type *)     CoreDebug_BASE)   /*!< Core Debug configuration struct    */

#if (__MPU_PRESENT == 1)
  #define MPU_BASE          (SCS_BASE +  0x0D90UL)                    /*!< Memory Protection Unit             */
  #define MPU               ((MPU_Type       *)     MPU_BASE      )   /*!< Memory Protection Unit             */
#endif

#if (__FPU_PRESENT == 1)
  #define FPU_BASE          (SCS_BASE +  0x0F30UL)                    /*!< Floating Point Unit                */
  #define FPU               ((FPU_Type       *)     FPU_BASE      )   /*!< Floating Point Unit                */
#endif

/* Memory mapping of Cortex-M4 Hardware */
#define SCS_BASE            (0xE000E000UL)                            /*!< System Control Space Base Address  */
#define ITM_BASE            (0xE0000000UL)                            /*!< ITM Base Address                   */
#define DWT_BASE            (0xE0001000UL)                            /*!< DWT Base Address                   */
#define TPI_BASE            (0xE0040000UL)                            /*!< TPI Base Address                   */
#define CoreDebug_BASE      (0xE000EDF0UL)                            /*!< Core Debug Base Address            */
#define SysTick_BASE        (SCS_BASE +  0x0010UL)                    /*!< SysTick Base Address               */
#define NVIC_BASE           (SCS_BASE +  0x0100UL)                    /*!< NVIC Base Address                  */
#define SCB_BASE            (SCS_BASE +  0x0D00UL)                    /*!< System Control Block Base Address  */

#define SCnSCB              ((SCnSCB_Type    *)     SCS_BASE      )   /*!< System control Register not in SCB */
#define SCB                 ((SCB_Type       *)     SCB_BASE      )   /*!< SCB configuration struct           */
#define SysTick             ((SysTick_Type   *)     SysTick_BASE  )   /*!< SysTick configuration struct       */
#define NVIC                ((NVIC_Type      *)     NVIC_BASE     )   /*!< NVIC configuration struct          */
#define ITM                 ((ITM_Type       *)     ITM_BASE      )   /*!< ITM configuration struct           */
#define DWT                 ((DWT_Type       *)     DWT_BASE      )   /*!< DWT configuration struct           */
#define TPI                 ((TPI_Type       *)     TPI_BASE      )   /*!< TPI configuration struct           */
#define CoreDebug           ((CoreDebug_Type *)     CoreDebug_BASE)   /*!< Core Debug configuration struct    */

#if (__MPU_PRESENT == 1)
  #define MPU_BASE          (SCS_BASE +  0x0D90UL)                    /*!< Memory Protection Unit             */
  #define MPU               ((MPU_Type       *)     MPU_BASE      )   /*!< Memory Protection Unit             */
#endif

#if (__FPU_PRESENT == 1)
  #define FPU_BASE          (SCS_BASE +  0x0F30UL)                    /*!< Floating Point Unit                */
  #define FPU               ((FPU_Type       *)     FPU_BASE      )   /*!< Floating Point Unit                */
#endif

/* ============================================================================= */
/**  SOFTWARE API DEFINITION FOR Universal Asynchronous Receiver Transmitter */
/* ============================================================================= */
/** \addtogroup SAM4S_UART Universal Asynchronous Receiver Transmitter */
/*@{*/

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
/** \brief Uart hardware registers */
typedef struct {
  __O  uint32_t UART_CR;       /**< \brief (Uart Offset: 0x0000) Control Register */
  __IO uint32_t UART_MR;       /**< \brief (Uart Offset: 0x0004) Mode Register */
  __O  uint32_t UART_IER;      /**< \brief (Uart Offset: 0x0008) Interrupt Enable Register */
  __O  uint32_t UART_IDR;      /**< \brief (Uart Offset: 0x000C) Interrupt Disable Register */
  __I  uint32_t UART_IMR;      /**< \brief (Uart Offset: 0x0010) Interrupt Mask Register */
  __I  uint32_t UART_SR;       /**< \brief (Uart Offset: 0x0014) Status Register */
  __I  uint32_t UART_RHR;      /**< \brief (Uart Offset: 0x0018) Receive Holding Register */
  __O  uint32_t UART_THR;      /**< \brief (Uart Offset: 0x001C) Transmit Holding Register */
  __IO uint32_t UART_BRGR;     /**< \brief (Uart Offset: 0x0020) Baud Rate Generator Register */
  __I  uint32_t Reserved1[55];
  __IO uint32_t UART_RPR;      /**< \brief (Uart Offset: 0x100) Receive Pointer Register */
  __IO uint32_t UART_RCR;      /**< \brief (Uart Offset: 0x104) Receive Counter Register */
  __IO uint32_t UART_TPR;      /**< \brief (Uart Offset: 0x108) Transmit Pointer Register */
  __IO uint32_t UART_TCR;      /**< \brief (Uart Offset: 0x10C) Transmit Counter Register */
  __IO uint32_t UART_RNPR;     /**< \brief (Uart Offset: 0x110) Receive Next Pointer Register */
  __IO uint32_t UART_RNCR;     /**< \brief (Uart Offset: 0x114) Receive Next Counter Register */
  __IO uint32_t UART_TNPR;     /**< \brief (Uart Offset: 0x118) Transmit Next Pointer Register */
  __IO uint32_t UART_TNCR;     /**< \brief (Uart Offset: 0x11C) Transmit Next Counter Register */
  __O  uint32_t UART_PTCR;     /**< \brief (Uart Offset: 0x120) Transfer Control Register */
  __I  uint32_t UART_PTSR;     /**< \brief (Uart Offset: 0x124) Transfer Status Register */
} Uart;
#endif /* !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */
/* -------- UART_CR : (UART Offset: 0x0000) Control Register -------- */
#define UART_CR_RSTRX (0x1u << 2) /**< \brief (UART_CR) Reset Receiver */
#define UART_CR_RSTTX (0x1u << 3) /**< \brief (UART_CR) Reset Transmitter */
#define UART_CR_RXEN (0x1u << 4) /**< \brief (UART_CR) Receiver Enable */
#define UART_CR_RXDIS (0x1u << 5) /**< \brief (UART_CR) Receiver Disable */
#define UART_CR_TXEN (0x1u << 6) /**< \brief (UART_CR) Transmitter Enable */
#define UART_CR_TXDIS (0x1u << 7) /**< \brief (UART_CR) Transmitter Disable */
#define UART_CR_RSTSTA (0x1u << 8) /**< \brief (UART_CR) Reset Status Bits */
/* -------- UART_MR : (UART Offset: 0x0004) Mode Register -------- */
#define UART_MR_PAR_Pos 9
#define UART_MR_PAR_Msk (0x7u << UART_MR_PAR_Pos) /**< \brief (UART_MR) Parity Type */
#define   UART_MR_PAR_EVEN (0x0u << 9) /**< \brief (UART_MR) Even Parity */
#define   UART_MR_PAR_ODD (0x1u << 9) /**< \brief (UART_MR) Odd Parity */
#define   UART_MR_PAR_SPACE (0x2u << 9) /**< \brief (UART_MR) Space: parity forced to 0 */
#define   UART_MR_PAR_MARK (0x3u << 9) /**< \brief (UART_MR) Mark: parity forced to 1 */
#define   UART_MR_PAR_NO (0x4u << 9) /**< \brief (UART_MR) No parity */
#define UART_MR_CHMODE_Pos 14
#define UART_MR_CHMODE_Msk (0x3u << UART_MR_CHMODE_Pos) /**< \brief (UART_MR) Channel Mode */
#define   UART_MR_CHMODE_NORMAL (0x0u << 14) /**< \brief (UART_MR) Normal mode */
#define   UART_MR_CHMODE_AUTOMATIC (0x1u << 14) /**< \brief (UART_MR) Automatic echo */
#define   UART_MR_CHMODE_LOCAL_LOOPBACK (0x2u << 14) /**< \brief (UART_MR) Local loopback */
#define   UART_MR_CHMODE_REMOTE_LOOPBACK (0x3u << 14) /**< \brief (UART_MR) Remote loopback */
/* -------- UART_IER : (UART Offset: 0x0008) Interrupt Enable Register -------- */
#define UART_IER_RXRDY (0x1u << 0) /**< \brief (UART_IER) Enable RXRDY Interrupt */
#define UART_IER_TXRDY (0x1u << 1) /**< \brief (UART_IER) Enable TXRDY Interrupt */
#define UART_IER_ENDRX (0x1u << 3) /**< \brief (UART_IER) Enable End of Receive Transfer Interrupt */
#define UART_IER_ENDTX (0x1u << 4) /**< \brief (UART_IER) Enable End of Transmit Interrupt */
#define UART_IER_OVRE (0x1u << 5) /**< \brief (UART_IER) Enable Overrun Error Interrupt */
#define UART_IER_FRAME (0x1u << 6) /**< \brief (UART_IER) Enable Framing Error Interrupt */
#define UART_IER_PARE (0x1u << 7) /**< \brief (UART_IER) Enable Parity Error Interrupt */
#define UART_IER_TXEMPTY (0x1u << 9) /**< \brief (UART_IER) Enable TXEMPTY Interrupt */
#define UART_IER_TXBUFE (0x1u << 11) /**< \brief (UART_IER) Enable Buffer Empty Interrupt */
#define UART_IER_RXBUFF (0x1u << 12) /**< \brief (UART_IER) Enable Buffer Full Interrupt */
/* -------- UART_IDR : (UART Offset: 0x000C) Interrupt Disable Register -------- */
#define UART_IDR_RXRDY (0x1u << 0) /**< \brief (UART_IDR) Disable RXRDY Interrupt */
#define UART_IDR_TXRDY (0x1u << 1) /**< \brief (UART_IDR) Disable TXRDY Interrupt */
#define UART_IDR_ENDRX (0x1u << 3) /**< \brief (UART_IDR) Disable End of Receive Transfer Interrupt */
#define UART_IDR_ENDTX (0x1u << 4) /**< \brief (UART_IDR) Disable End of Transmit Interrupt */
#define UART_IDR_OVRE (0x1u << 5) /**< \brief (UART_IDR) Disable Overrun Error Interrupt */
#define UART_IDR_FRAME (0x1u << 6) /**< \brief (UART_IDR) Disable Framing Error Interrupt */
#define UART_IDR_PARE (0x1u << 7) /**< \brief (UART_IDR) Disable Parity Error Interrupt */
#define UART_IDR_TXEMPTY (0x1u << 9) /**< \brief (UART_IDR) Disable TXEMPTY Interrupt */
#define UART_IDR_TXBUFE (0x1u << 11) /**< \brief (UART_IDR) Disable Buffer Empty Interrupt */
#define UART_IDR_RXBUFF (0x1u << 12) /**< \brief (UART_IDR) Disable Buffer Full Interrupt */
/* -------- UART_IMR : (UART Offset: 0x0010) Interrupt Mask Register -------- */
#define UART_IMR_RXRDY (0x1u << 0) /**< \brief (UART_IMR) Mask RXRDY Interrupt */
#define UART_IMR_TXRDY (0x1u << 1) /**< \brief (UART_IMR) Disable TXRDY Interrupt */
#define UART_IMR_ENDRX (0x1u << 3) /**< \brief (UART_IMR) Mask End of Receive Transfer Interrupt */
#define UART_IMR_ENDTX (0x1u << 4) /**< \brief (UART_IMR) Mask End of Transmit Interrupt */
#define UART_IMR_OVRE (0x1u << 5) /**< \brief (UART_IMR) Mask Overrun Error Interrupt */
#define UART_IMR_FRAME (0x1u << 6) /**< \brief (UART_IMR) Mask Framing Error Interrupt */
#define UART_IMR_PARE (0x1u << 7) /**< \brief (UART_IMR) Mask Parity Error Interrupt */
#define UART_IMR_TXEMPTY (0x1u << 9) /**< \brief (UART_IMR) Mask TXEMPTY Interrupt */
#define UART_IMR_TXBUFE (0x1u << 11) /**< \brief (UART_IMR) Mask TXBUFE Interrupt */
#define UART_IMR_RXBUFF (0x1u << 12) /**< \brief (UART_IMR) Mask RXBUFF Interrupt */
/* -------- UART_SR : (UART Offset: 0x0014) Status Register -------- */
#define UART_SR_RXRDY (0x1u << 0) /**< \brief (UART_SR) Receiver Ready */
#define UART_SR_TXRDY (0x1u << 1) /**< \brief (UART_SR) Transmitter Ready */
#define UART_SR_ENDRX (0x1u << 3) /**< \brief (UART_SR) End of Receiver Transfer */
#define UART_SR_ENDTX (0x1u << 4) /**< \brief (UART_SR) End of Transmitter Transfer */
#define UART_SR_OVRE (0x1u << 5) /**< \brief (UART_SR) Overrun Error */
#define UART_SR_FRAME (0x1u << 6) /**< \brief (UART_SR) Framing Error */
#define UART_SR_PARE (0x1u << 7) /**< \brief (UART_SR) Parity Error */
#define UART_SR_TXEMPTY (0x1u << 9) /**< \brief (UART_SR) Transmitter Empty */
#define UART_SR_TXBUFE (0x1u << 11) /**< \brief (UART_SR) Transmission Buffer Empty */
#define UART_SR_RXBUFF (0x1u << 12) /**< \brief (UART_SR) Receive Buffer Full */
/* -------- UART_RHR : (UART Offset: 0x0018) Receive Holding Register -------- */
#define UART_RHR_RXCHR_Pos 0
#define UART_RHR_RXCHR_Msk (0xffu << UART_RHR_RXCHR_Pos) /**< \brief (UART_RHR) Received Character */
/* -------- UART_THR : (UART Offset: 0x001C) Transmit Holding Register -------- */
#define UART_THR_TXCHR_Pos 0
#define UART_THR_TXCHR_Msk (0xffu << UART_THR_TXCHR_Pos) /**< \brief (UART_THR) Character to be Transmitted */
#define UART_THR_TXCHR(value) ((UART_THR_TXCHR_Msk & ((value) << UART_THR_TXCHR_Pos)))
/* -------- UART_BRGR : (UART Offset: 0x0020) Baud Rate Generator Register -------- */
#define UART_BRGR_CD_Pos 0
#define UART_BRGR_CD_Msk (0xffffu << UART_BRGR_CD_Pos) /**< \brief (UART_BRGR) Clock Divisor */
#define UART_BRGR_CD(value) ((UART_BRGR_CD_Msk & ((value) << UART_BRGR_CD_Pos)))
/* -------- UART_RPR : (UART Offset: 0x100) Receive Pointer Register -------- */
#define UART_RPR_RXPTR_Pos 0
#define UART_RPR_RXPTR_Msk (0xffffffffu << UART_RPR_RXPTR_Pos) /**< \brief (UART_RPR) Receive Pointer Register */
#define UART_RPR_RXPTR(value) ((UART_RPR_RXPTR_Msk & ((value) << UART_RPR_RXPTR_Pos)))
/* -------- UART_RCR : (UART Offset: 0x104) Receive Counter Register -------- */
#define UART_RCR_RXCTR_Pos 0
#define UART_RCR_RXCTR_Msk (0xffffu << UART_RCR_RXCTR_Pos) /**< \brief (UART_RCR) Receive Counter Register */
#define UART_RCR_RXCTR(value) ((UART_RCR_RXCTR_Msk & ((value) << UART_RCR_RXCTR_Pos)))
/* -------- UART_TPR : (UART Offset: 0x108) Transmit Pointer Register -------- */
#define UART_TPR_TXPTR_Pos 0
#define UART_TPR_TXPTR_Msk (0xffffffffu << UART_TPR_TXPTR_Pos) /**< \brief (UART_TPR) Transmit Counter Register */
#define UART_TPR_TXPTR(value) ((UART_TPR_TXPTR_Msk & ((value) << UART_TPR_TXPTR_Pos)))
/* -------- UART_TCR : (UART Offset: 0x10C) Transmit Counter Register -------- */
#define UART_TCR_TXCTR_Pos 0
#define UART_TCR_TXCTR_Msk (0xffffu << UART_TCR_TXCTR_Pos) /**< \brief (UART_TCR) Transmit Counter Register */
#define UART_TCR_TXCTR(value) ((UART_TCR_TXCTR_Msk & ((value) << UART_TCR_TXCTR_Pos)))
/* -------- UART_RNPR : (UART Offset: 0x110) Receive Next Pointer Register -------- */
#define UART_RNPR_RXNPTR_Pos 0
#define UART_RNPR_RXNPTR_Msk (0xffffffffu << UART_RNPR_RXNPTR_Pos) /**< \brief (UART_RNPR) Receive Next Pointer */
#define UART_RNPR_RXNPTR(value) ((UART_RNPR_RXNPTR_Msk & ((value) << UART_RNPR_RXNPTR_Pos)))
/* -------- UART_RNCR : (UART Offset: 0x114) Receive Next Counter Register -------- */
#define UART_RNCR_RXNCTR_Pos 0
#define UART_RNCR_RXNCTR_Msk (0xffffu << UART_RNCR_RXNCTR_Pos) /**< \brief (UART_RNCR) Receive Next Counter */
#define UART_RNCR_RXNCTR(value) ((UART_RNCR_RXNCTR_Msk & ((value) << UART_RNCR_RXNCTR_Pos)))
/* -------- UART_TNPR : (UART Offset: 0x118) Transmit Next Pointer Register -------- */
#define UART_TNPR_TXNPTR_Pos 0
#define UART_TNPR_TXNPTR_Msk (0xffffffffu << UART_TNPR_TXNPTR_Pos) /**< \brief (UART_TNPR) Transmit Next Pointer */
#define UART_TNPR_TXNPTR(value) ((UART_TNPR_TXNPTR_Msk & ((value) << UART_TNPR_TXNPTR_Pos)))
/* -------- UART_TNCR : (UART Offset: 0x11C) Transmit Next Counter Register -------- */
#define UART_TNCR_TXNCTR_Pos 0
#define UART_TNCR_TXNCTR_Msk (0xffffu << UART_TNCR_TXNCTR_Pos) /**< \brief (UART_TNCR) Transmit Counter Next */
#define UART_TNCR_TXNCTR(value) ((UART_TNCR_TXNCTR_Msk & ((value) << UART_TNCR_TXNCTR_Pos)))
/* -------- UART_PTCR : (UART Offset: 0x120) Transfer Control Register -------- */
#define UART_PTCR_RXTEN (0x1u << 0) /**< \brief (UART_PTCR) Receiver Transfer Enable */
#define UART_PTCR_RXTDIS (0x1u << 1) /**< \brief (UART_PTCR) Receiver Transfer Disable */
#define UART_PTCR_TXTEN (0x1u << 8) /**< \brief (UART_PTCR) Transmitter Transfer Enable */
#define UART_PTCR_TXTDIS (0x1u << 9) /**< \brief (UART_PTCR) Transmitter Transfer Disable */
/* -------- UART_PTSR : (UART Offset: 0x124) Transfer Status Register -------- */
#define UART_PTSR_RXTEN (0x1u << 0) /**< \brief (UART_PTSR) Receiver Transfer Enable */
#define UART_PTSR_TXTEN (0x1u << 8) /**< \brief (UART_PTSR) Transmitter Transfer Enable */



/* ============================================================================= */
/**  SOFTWARE API DEFINITION FOR Timer Counter */
/* ============================================================================= */
/** \addtogroup SAM4S_TC Timer Counter */

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
/** \brief TcChannel hardware registers */
typedef struct {
  __O  uint32_t TC_CCR;       /**< \brief (TcChannel Offset: 0x0) Channel Control Register */
  __IO uint32_t TC_CMR;       /**< \brief (TcChannel Offset: 0x4) Channel Mode Register */
  __IO uint32_t TC_SMMR;      /**< \brief (TcChannel Offset: 0x8) Stepper Motor Mode Register */
  __I  uint32_t Reserved1[1];
  __I  uint32_t TC_CV;        /**< \brief (TcChannel Offset: 0x10) Counter Value */
  __IO uint32_t TC_RA;        /**< \brief (TcChannel Offset: 0x14) Register A */
  __IO uint32_t TC_RB;        /**< \brief (TcChannel Offset: 0x18) Register B */
  __IO uint32_t TC_RC;        /**< \brief (TcChannel Offset: 0x1C) Register C */
  __I  uint32_t TC_SR;        /**< \brief (TcChannel Offset: 0x20) Status Register */
  __O  uint32_t TC_IER;       /**< \brief (TcChannel Offset: 0x24) Interrupt Enable Register */
  __O  uint32_t TC_IDR;       /**< \brief (TcChannel Offset: 0x28) Interrupt Disable Register */
  __I  uint32_t TC_IMR;       /**< \brief (TcChannel Offset: 0x2C) Interrupt Mask Register */
  __I  uint32_t Reserved2[4];
} TcChannel;
/** \brief Tc hardware registers */
#define TCCHANNEL_NUMBER 3
typedef struct {
       TcChannel TC_CHANNEL[TCCHANNEL_NUMBER]; /**< \brief (Tc Offset: 0x0) channel = 0 .. 2 */
  __O  uint32_t  TC_BCR;                       /**< \brief (Tc Offset: 0xC0) Block Control Register */
  __IO uint32_t  TC_BMR;                       /**< \brief (Tc Offset: 0xC4) Block Mode Register */
  __O  uint32_t  TC_QIER;                      /**< \brief (Tc Offset: 0xC8) QDEC Interrupt Enable Register */
  __O  uint32_t  TC_QIDR;                      /**< \brief (Tc Offset: 0xCC) QDEC Interrupt Disable Register */
  __I  uint32_t  TC_QIMR;                      /**< \brief (Tc Offset: 0xD0) QDEC Interrupt Mask Register */
  __I  uint32_t  TC_QISR;                      /**< \brief (Tc Offset: 0xD4) QDEC Interrupt Status Register */
  __IO uint32_t  TC_FMR;                       /**< \brief (Tc Offset: 0xD8) Fault Mode Register */
  __I  uint32_t  Reserved1[2];
  __IO uint32_t  TC_WPMR;                      /**< \brief (Tc Offset: 0xE4) Write Protection Mode Register */
} Tc;
#endif /* !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */
/* -------- TC_CCR : (TC Offset: N/A) Channel Control Register -------- */
#define TC_CCR_CLKEN (0x1u << 0) /**< \brief (TC_CCR) Counter Clock Enable Command */
#define TC_CCR_CLKDIS (0x1u << 1) /**< \brief (TC_CCR) Counter Clock Disable Command */
#define TC_CCR_SWTRG (0x1u << 2) /**< \brief (TC_CCR) Software Trigger Command */
/* -------- TC_CMR : (TC Offset: N/A) Channel Mode Register -------- */
#define TC_CMR_TCCLKS_Pos 0
#define TC_CMR_TCCLKS_Msk (0x7u << TC_CMR_TCCLKS_Pos) /**< \brief (TC_CMR) Clock Selection */
#define   TC_CMR_TCCLKS_TIMER_CLOCK1 (0x0u << 0) /**< \brief (TC_CMR) Clock selected: internal TIMER_CLOCK1 clock signal (from PMC) */
#define   TC_CMR_TCCLKS_TIMER_CLOCK2 (0x1u << 0) /**< \brief (TC_CMR) Clock selected: internal TIMER_CLOCK2 clock signal (from PMC) */
#define   TC_CMR_TCCLKS_TIMER_CLOCK3 (0x2u << 0) /**< \brief (TC_CMR) Clock selected: internal TIMER_CLOCK3 clock signal (from PMC) */
#define   TC_CMR_TCCLKS_TIMER_CLOCK4 (0x3u << 0) /**< \brief (TC_CMR) Clock selected: internal TIMER_CLOCK4 clock signal (from PMC) */
#define   TC_CMR_TCCLKS_TIMER_CLOCK5 (0x4u << 0) /**< \brief (TC_CMR) Clock selected: internal TIMER_CLOCK5 clock signal (from PMC) */
#define   TC_CMR_TCCLKS_XC0 (0x5u << 0) /**< \brief (TC_CMR) Clock selected: XC0 */
#define   TC_CMR_TCCLKS_XC1 (0x6u << 0) /**< \brief (TC_CMR) Clock selected: XC1 */
#define   TC_CMR_TCCLKS_XC2 (0x7u << 0) /**< \brief (TC_CMR) Clock selected: XC2 */
#define TC_CMR_CLKI (0x1u << 3) /**< \brief (TC_CMR) Clock Invert */
#define TC_CMR_BURST_Pos 4
#define TC_CMR_BURST_Msk (0x3u << TC_CMR_BURST_Pos) /**< \brief (TC_CMR) Burst Signal Selection */
#define   TC_CMR_BURST_NONE (0x0u << 4) /**< \brief (TC_CMR) The clock is not gated by an external signal. */
#define   TC_CMR_BURST_XC0 (0x1u << 4) /**< \brief (TC_CMR) XC0 is ANDed with the selected clock. */
#define   TC_CMR_BURST_XC1 (0x2u << 4) /**< \brief (TC_CMR) XC1 is ANDed with the selected clock. */
#define   TC_CMR_BURST_XC2 (0x3u << 4) /**< \brief (TC_CMR) XC2 is ANDed with the selected clock. */
#define TC_CMR_LDBSTOP (0x1u << 6) /**< \brief (TC_CMR) Counter Clock Stopped with RB Loading */
#define TC_CMR_LDBDIS (0x1u << 7) /**< \brief (TC_CMR) Counter Clock Disable with RB Loading */
#define TC_CMR_ETRGEDG_Pos 8
#define TC_CMR_ETRGEDG_Msk (0x3u << TC_CMR_ETRGEDG_Pos) /**< \brief (TC_CMR) External Trigger Edge Selection */
#define   TC_CMR_ETRGEDG_NONE (0x0u << 8) /**< \brief (TC_CMR) The clock is not gated by an external signal. */
#define   TC_CMR_ETRGEDG_RISING (0x1u << 8) /**< \brief (TC_CMR) Rising edge */
#define   TC_CMR_ETRGEDG_FALLING (0x2u << 8) /**< \brief (TC_CMR) Falling edge */
#define   TC_CMR_ETRGEDG_EDGE (0x3u << 8) /**< \brief (TC_CMR) Each edge */
#define TC_CMR_ABETRG (0x1u << 10) /**< \brief (TC_CMR) TIOA or TIOB External Trigger Selection */
#define TC_CMR_CPCTRG (0x1u << 14) /**< \brief (TC_CMR) RC Compare Trigger Enable */
#define TC_CMR_WAVE (0x1u << 15) /**< \brief (TC_CMR) Waveform Mode */
#define TC_CMR_LDRA_Pos 16
#define TC_CMR_LDRA_Msk (0x3u << TC_CMR_LDRA_Pos) /**< \brief (TC_CMR) RA Loading Edge Selection */
#define   TC_CMR_LDRA_NONE (0x0u << 16) /**< \brief (TC_CMR) None */
#define   TC_CMR_LDRA_RISING (0x1u << 16) /**< \brief (TC_CMR) Rising edge of TIOA */
#define   TC_CMR_LDRA_FALLING (0x2u << 16) /**< \brief (TC_CMR) Falling edge of TIOA */
#define   TC_CMR_LDRA_EDGE (0x3u << 16) /**< \brief (TC_CMR) Each edge of TIOA */
#define TC_CMR_LDRB_Pos 18
#define TC_CMR_LDRB_Msk (0x3u << TC_CMR_LDRB_Pos) /**< \brief (TC_CMR) RB Loading Edge Selection */
#define   TC_CMR_LDRB_NONE (0x0u << 18) /**< \brief (TC_CMR) None */
#define   TC_CMR_LDRB_RISING (0x1u << 18) /**< \brief (TC_CMR) Rising edge of TIOA */
#define   TC_CMR_LDRB_FALLING (0x2u << 18) /**< \brief (TC_CMR) Falling edge of TIOA */
#define   TC_CMR_LDRB_EDGE (0x3u << 18) /**< \brief (TC_CMR) Each edge of TIOA */
#define TC_CMR_CPCSTOP (0x1u << 6) /**< \brief (TC_CMR) Counter Clock Stopped with RC Compare */
#define TC_CMR_CPCDIS (0x1u << 7) /**< \brief (TC_CMR) Counter Clock Disable with RC Compare */
#define TC_CMR_EEVTEDG_Pos 8
#define TC_CMR_EEVTEDG_Msk (0x3u << TC_CMR_EEVTEDG_Pos) /**< \brief (TC_CMR) External Event Edge Selection */
#define   TC_CMR_EEVTEDG_NONE (0x0u << 8) /**< \brief (TC_CMR) None */
#define   TC_CMR_EEVTEDG_RISING (0x1u << 8) /**< \brief (TC_CMR) Rising edge */
#define   TC_CMR_EEVTEDG_FALLING (0x2u << 8) /**< \brief (TC_CMR) Falling edge */
#define   TC_CMR_EEVTEDG_EDGE (0x3u << 8) /**< \brief (TC_CMR) Each edge */
#define TC_CMR_EEVT_Pos 10
#define TC_CMR_EEVT_Msk (0x3u << TC_CMR_EEVT_Pos) /**< \brief (TC_CMR) External Event Selection */
#define   TC_CMR_EEVT_TIOB (0x0u << 10) /**< \brief (TC_CMR) TIOB */
#define   TC_CMR_EEVT_XC0 (0x1u << 10) /**< \brief (TC_CMR) XC0 */
#define   TC_CMR_EEVT_XC1 (0x2u << 10) /**< \brief (TC_CMR) XC1 */
#define   TC_CMR_EEVT_XC2 (0x3u << 10) /**< \brief (TC_CMR) XC2 */
#define TC_CMR_ENETRG (0x1u << 12) /**< \brief (TC_CMR) External Event Trigger Enable */
#define TC_CMR_WAVSEL_Pos 13
#define TC_CMR_WAVSEL_Msk (0x3u << TC_CMR_WAVSEL_Pos) /**< \brief (TC_CMR) Waveform Selection */
#define   TC_CMR_WAVSEL_UP (0x0u << 13) /**< \brief (TC_CMR) UP mode without automatic trigger on RC Compare */
#define   TC_CMR_WAVSEL_UPDOWN (0x1u << 13) /**< \brief (TC_CMR) UPDOWN mode without automatic trigger on RC Compare */
#define   TC_CMR_WAVSEL_UP_RC (0x2u << 13) /**< \brief (TC_CMR) UP mode with automatic trigger on RC Compare */
#define   TC_CMR_WAVSEL_UPDOWN_RC (0x3u << 13) /**< \brief (TC_CMR) UPDOWN mode with automatic trigger on RC Compare */
#define TC_CMR_ACPA_Pos 16
#define TC_CMR_ACPA_Msk (0x3u << TC_CMR_ACPA_Pos) /**< \brief (TC_CMR) RA Compare Effect on TIOA */
#define   TC_CMR_ACPA_NONE (0x0u << 16) /**< \brief (TC_CMR) None */
#define   TC_CMR_ACPA_SET (0x1u << 16) /**< \brief (TC_CMR) Set */
#define   TC_CMR_ACPA_CLEAR (0x2u << 16) /**< \brief (TC_CMR) Clear */
#define   TC_CMR_ACPA_TOGGLE (0x3u << 16) /**< \brief (TC_CMR) Toggle */
#define TC_CMR_ACPC_Pos 18
#define TC_CMR_ACPC_Msk (0x3u << TC_CMR_ACPC_Pos) /**< \brief (TC_CMR) RC Compare Effect on TIOA */
#define   TC_CMR_ACPC_NONE (0x0u << 18) /**< \brief (TC_CMR) None */
#define   TC_CMR_ACPC_SET (0x1u << 18) /**< \brief (TC_CMR) Set */
#define   TC_CMR_ACPC_CLEAR (0x2u << 18) /**< \brief (TC_CMR) Clear */
#define   TC_CMR_ACPC_TOGGLE (0x3u << 18) /**< \brief (TC_CMR) Toggle */
#define TC_CMR_AEEVT_Pos 20
#define TC_CMR_AEEVT_Msk (0x3u << TC_CMR_AEEVT_Pos) /**< \brief (TC_CMR) External Event Effect on TIOA */
#define   TC_CMR_AEEVT_NONE (0x0u << 20) /**< \brief (TC_CMR) None */
#define   TC_CMR_AEEVT_SET (0x1u << 20) /**< \brief (TC_CMR) Set */
#define   TC_CMR_AEEVT_CLEAR (0x2u << 20) /**< \brief (TC_CMR) Clear */
#define   TC_CMR_AEEVT_TOGGLE (0x3u << 20) /**< \brief (TC_CMR) Toggle */
#define TC_CMR_ASWTRG_Pos 22
#define TC_CMR_ASWTRG_Msk (0x3u << TC_CMR_ASWTRG_Pos) /**< \brief (TC_CMR) Software Trigger Effect on TIOA */
#define   TC_CMR_ASWTRG_NONE (0x0u << 22) /**< \brief (TC_CMR) None */
#define   TC_CMR_ASWTRG_SET (0x1u << 22) /**< \brief (TC_CMR) Set */
#define   TC_CMR_ASWTRG_CLEAR (0x2u << 22) /**< \brief (TC_CMR) Clear */
#define   TC_CMR_ASWTRG_TOGGLE (0x3u << 22) /**< \brief (TC_CMR) Toggle */
#define TC_CMR_BCPB_Pos 24
#define TC_CMR_BCPB_Msk (0x3u << TC_CMR_BCPB_Pos) /**< \brief (TC_CMR) RB Compare Effect on TIOB */
#define   TC_CMR_BCPB_NONE (0x0u << 24) /**< \brief (TC_CMR) None */
#define   TC_CMR_BCPB_SET (0x1u << 24) /**< \brief (TC_CMR) Set */
#define   TC_CMR_BCPB_CLEAR (0x2u << 24) /**< \brief (TC_CMR) Clear */
#define   TC_CMR_BCPB_TOGGLE (0x3u << 24) /**< \brief (TC_CMR) Toggle */
#define TC_CMR_BCPC_Pos 26
#define TC_CMR_BCPC_Msk (0x3u << TC_CMR_BCPC_Pos) /**< \brief (TC_CMR) RC Compare Effect on TIOB */
#define   TC_CMR_BCPC_NONE (0x0u << 26) /**< \brief (TC_CMR) None */
#define   TC_CMR_BCPC_SET (0x1u << 26) /**< \brief (TC_CMR) Set */
#define   TC_CMR_BCPC_CLEAR (0x2u << 26) /**< \brief (TC_CMR) Clear */
#define   TC_CMR_BCPC_TOGGLE (0x3u << 26) /**< \brief (TC_CMR) Toggle */
#define TC_CMR_BEEVT_Pos 28
#define TC_CMR_BEEVT_Msk (0x3u << TC_CMR_BEEVT_Pos) /**< \brief (TC_CMR) External Event Effect on TIOB */
#define   TC_CMR_BEEVT_NONE (0x0u << 28) /**< \brief (TC_CMR) None */
#define   TC_CMR_BEEVT_SET (0x1u << 28) /**< \brief (TC_CMR) Set */
#define   TC_CMR_BEEVT_CLEAR (0x2u << 28) /**< \brief (TC_CMR) Clear */
#define   TC_CMR_BEEVT_TOGGLE (0x3u << 28) /**< \brief (TC_CMR) Toggle */
#define TC_CMR_BSWTRG_Pos 30
#define TC_CMR_BSWTRG_Msk (0x3u << TC_CMR_BSWTRG_Pos) /**< \brief (TC_CMR) Software Trigger Effect on TIOB */
#define   TC_CMR_BSWTRG_NONE (0x0u << 30) /**< \brief (TC_CMR) None */
#define   TC_CMR_BSWTRG_SET (0x1u << 30) /**< \brief (TC_CMR) Set */
#define   TC_CMR_BSWTRG_CLEAR (0x2u << 30) /**< \brief (TC_CMR) Clear */
#define   TC_CMR_BSWTRG_TOGGLE (0x3u << 30) /**< \brief (TC_CMR) Toggle */
/* -------- TC_SMMR : (TC Offset: N/A) Stepper Motor Mode Register -------- */
#define TC_SMMR_GCEN (0x1u << 0) /**< \brief (TC_SMMR) Gray Count Enable */
#define TC_SMMR_DOWN (0x1u << 1) /**< \brief (TC_SMMR) DOWN Count */
/* -------- TC_CV : (TC Offset: N/A) Counter Value -------- */
#define TC_CV_CV_Pos 0
#define TC_CV_CV_Msk (0xffffffffu << TC_CV_CV_Pos) /**< \brief (TC_CV) Counter Value */
/* -------- TC_RA : (TC Offset: N/A) Register A -------- */
#define TC_RA_RA_Pos 0
#define TC_RA_RA_Msk (0xffffffffu << TC_RA_RA_Pos) /**< \brief (TC_RA) Register A */
#define TC_RA_RA(value) ((TC_RA_RA_Msk & ((value) << TC_RA_RA_Pos)))
/* -------- TC_RB : (TC Offset: N/A) Register B -------- */
#define TC_RB_RB_Pos 0
#define TC_RB_RB_Msk (0xffffffffu << TC_RB_RB_Pos) /**< \brief (TC_RB) Register B */
#define TC_RB_RB(value) ((TC_RB_RB_Msk & ((value) << TC_RB_RB_Pos)))
/* -------- TC_RC : (TC Offset: N/A) Register C -------- */
#define TC_RC_RC_Pos 0
#define TC_RC_RC_Msk (0xffffffffu << TC_RC_RC_Pos) /**< \brief (TC_RC) Register C */
#define TC_RC_RC(value) ((TC_RC_RC_Msk & ((value) << TC_RC_RC_Pos)))
/* -------- TC_SR : (TC Offset: N/A) Status Register -------- */
#define TC_SR_COVFS (0x1u << 0) /**< \brief (TC_SR) Counter Overflow Status */
#define TC_SR_LOVRS (0x1u << 1) /**< \brief (TC_SR) Load Overrun Status */
#define TC_SR_CPAS (0x1u << 2) /**< \brief (TC_SR) RA Compare Status */
#define TC_SR_CPBS (0x1u << 3) /**< \brief (TC_SR) RB Compare Status */
#define TC_SR_CPCS (0x1u << 4) /**< \brief (TC_SR) RC Compare Status */
#define TC_SR_LDRAS (0x1u << 5) /**< \brief (TC_SR) RA Loading Status */
#define TC_SR_LDRBS (0x1u << 6) /**< \brief (TC_SR) RB Loading Status */
#define TC_SR_ETRGS (0x1u << 7) /**< \brief (TC_SR) External Trigger Status */
#define TC_SR_CLKSTA (0x1u << 16) /**< \brief (TC_SR) Clock Enabling Status */
#define TC_SR_MTIOA (0x1u << 17) /**< \brief (TC_SR) TIOA Mirror */
#define TC_SR_MTIOB (0x1u << 18) /**< \brief (TC_SR) TIOB Mirror */
/* -------- TC_IER : (TC Offset: N/A) Interrupt Enable Register -------- */
#define TC_IER_COVFS (0x1u << 0) /**< \brief (TC_IER) Counter Overflow */
#define TC_IER_LOVRS (0x1u << 1) /**< \brief (TC_IER) Load Overrun */
#define TC_IER_CPAS (0x1u << 2) /**< \brief (TC_IER) RA Compare */
#define TC_IER_CPBS (0x1u << 3) /**< \brief (TC_IER) RB Compare */
#define TC_IER_CPCS (0x1u << 4) /**< \brief (TC_IER) RC Compare */
#define TC_IER_LDRAS (0x1u << 5) /**< \brief (TC_IER) RA Loading */
#define TC_IER_LDRBS (0x1u << 6) /**< \brief (TC_IER) RB Loading */
#define TC_IER_ETRGS (0x1u << 7) /**< \brief (TC_IER) External Trigger */
/* -------- TC_IDR : (TC Offset: N/A) Interrupt Disable Register -------- */
#define TC_IDR_COVFS (0x1u << 0) /**< \brief (TC_IDR) Counter Overflow */
#define TC_IDR_LOVRS (0x1u << 1) /**< \brief (TC_IDR) Load Overrun */
#define TC_IDR_CPAS (0x1u << 2) /**< \brief (TC_IDR) RA Compare */
#define TC_IDR_CPBS (0x1u << 3) /**< \brief (TC_IDR) RB Compare */
#define TC_IDR_CPCS (0x1u << 4) /**< \brief (TC_IDR) RC Compare */
#define TC_IDR_LDRAS (0x1u << 5) /**< \brief (TC_IDR) RA Loading */
#define TC_IDR_LDRBS (0x1u << 6) /**< \brief (TC_IDR) RB Loading */
#define TC_IDR_ETRGS (0x1u << 7) /**< \brief (TC_IDR) External Trigger */
/* -------- TC_IMR : (TC Offset: N/A) Interrupt Mask Register -------- */
#define TC_IMR_COVFS (0x1u << 0) /**< \brief (TC_IMR) Counter Overflow */
#define TC_IMR_LOVRS (0x1u << 1) /**< \brief (TC_IMR) Load Overrun */
#define TC_IMR_CPAS (0x1u << 2) /**< \brief (TC_IMR) RA Compare */
#define TC_IMR_CPBS (0x1u << 3) /**< \brief (TC_IMR) RB Compare */
#define TC_IMR_CPCS (0x1u << 4) /**< \brief (TC_IMR) RC Compare */
#define TC_IMR_LDRAS (0x1u << 5) /**< \brief (TC_IMR) RA Loading */
#define TC_IMR_LDRBS (0x1u << 6) /**< \brief (TC_IMR) RB Loading */
#define TC_IMR_ETRGS (0x1u << 7) /**< \brief (TC_IMR) External Trigger */
/* -------- TC_BCR : (TC Offset: 0xC0) Block Control Register -------- */
#define TC_BCR_SYNC (0x1u << 0) /**< \brief (TC_BCR) Synchro Command */
/* -------- TC_BMR : (TC Offset: 0xC4) Block Mode Register -------- */
#define TC_BMR_TC0XC0S_Pos 0
#define TC_BMR_TC0XC0S_Msk (0x3u << TC_BMR_TC0XC0S_Pos) /**< \brief (TC_BMR) External Clock Signal 0 Selection */
#define   TC_BMR_TC0XC0S_TCLK0 (0x0u << 0) /**< \brief (TC_BMR) Signal connected to XC0: TCLK0 */
#define   TC_BMR_TC0XC0S_TIOA1 (0x2u << 0) /**< \brief (TC_BMR) Signal connected to XC0: TIOA1 */
#define   TC_BMR_TC0XC0S_TIOA2 (0x3u << 0) /**< \brief (TC_BMR) Signal connected to XC0: TIOA2 */
#define TC_BMR_TC1XC1S_Pos 2
#define TC_BMR_TC1XC1S_Msk (0x3u << TC_BMR_TC1XC1S_Pos) /**< \brief (TC_BMR) External Clock Signal 1 Selection */
#define   TC_BMR_TC1XC1S_TCLK1 (0x0u << 2) /**< \brief (TC_BMR) Signal connected to XC1: TCLK1 */
#define   TC_BMR_TC1XC1S_TIOA0 (0x2u << 2) /**< \brief (TC_BMR) Signal connected to XC1: TIOA0 */
#define   TC_BMR_TC1XC1S_TIOA2 (0x3u << 2) /**< \brief (TC_BMR) Signal connected to XC1: TIOA2 */
#define TC_BMR_TC2XC2S_Pos 4
#define TC_BMR_TC2XC2S_Msk (0x3u << TC_BMR_TC2XC2S_Pos) /**< \brief (TC_BMR) External Clock Signal 2 Selection */
#define   TC_BMR_TC2XC2S_TCLK2 (0x0u << 4) /**< \brief (TC_BMR) Signal connected to XC2: TCLK2 */
#define   TC_BMR_TC2XC2S_TIOA0 (0x2u << 4) /**< \brief (TC_BMR) Signal connected to XC2: TIOA0 */
#define   TC_BMR_TC2XC2S_TIOA1 (0x3u << 4) /**< \brief (TC_BMR) Signal connected to XC2: TIOA1 */
#define TC_BMR_QDEN (0x1u << 8) /**< \brief (TC_BMR) Quadrature Decoder ENabled */
#define TC_BMR_POSEN (0x1u << 9) /**< \brief (TC_BMR) POSition ENabled */
#define TC_BMR_SPEEDEN (0x1u << 10) /**< \brief (TC_BMR) SPEED ENabled */
#define TC_BMR_QDTRANS (0x1u << 11) /**< \brief (TC_BMR) Quadrature Decoding TRANSparent */
#define TC_BMR_EDGPHA (0x1u << 12) /**< \brief (TC_BMR) EDGe on PHA count mode */
#define TC_BMR_INVA (0x1u << 13) /**< \brief (TC_BMR) INVerted phA */
#define TC_BMR_INVB (0x1u << 14) /**< \brief (TC_BMR) INVerted phB */
#define TC_BMR_INVIDX (0x1u << 15) /**< \brief (TC_BMR) INVerted InDeX */
#define TC_BMR_SWAP (0x1u << 16) /**< \brief (TC_BMR) SWAP PHA and PHB */
#define TC_BMR_IDXPHB (0x1u << 17) /**< \brief (TC_BMR) InDeX pin is PHB pin */
#define TC_BMR_FILTER (0x1u << 19) /**< \brief (TC_BMR) Glitch Filter */
#define TC_BMR_MAXFILT_Pos 20
#define TC_BMR_MAXFILT_Msk (0x3fu << TC_BMR_MAXFILT_Pos) /**< \brief (TC_BMR) MAXimum FILTer */
#define TC_BMR_MAXFILT(value) ((TC_BMR_MAXFILT_Msk & ((value) << TC_BMR_MAXFILT_Pos)))
/* -------- TC_QIER : (TC Offset: 0xC8) QDEC Interrupt Enable Register -------- */
#define TC_QIER_IDX (0x1u << 0) /**< \brief (TC_QIER) InDeX */
#define TC_QIER_DIRCHG (0x1u << 1) /**< \brief (TC_QIER) DIRection CHanGe */
#define TC_QIER_QERR (0x1u << 2) /**< \brief (TC_QIER) Quadrature ERRor */
/* -------- TC_QIDR : (TC Offset: 0xCC) QDEC Interrupt Disable Register -------- */
#define TC_QIDR_IDX (0x1u << 0) /**< \brief (TC_QIDR) InDeX */
#define TC_QIDR_DIRCHG (0x1u << 1) /**< \brief (TC_QIDR) DIRection CHanGe */
#define TC_QIDR_QERR (0x1u << 2) /**< \brief (TC_QIDR) Quadrature ERRor */
/* -------- TC_QIMR : (TC Offset: 0xD0) QDEC Interrupt Mask Register -------- */
#define TC_QIMR_IDX (0x1u << 0) /**< \brief (TC_QIMR) InDeX */
#define TC_QIMR_DIRCHG (0x1u << 1) /**< \brief (TC_QIMR) DIRection CHanGe */
#define TC_QIMR_QERR (0x1u << 2) /**< \brief (TC_QIMR) Quadrature ERRor */
/* -------- TC_QISR : (TC Offset: 0xD4) QDEC Interrupt Status Register -------- */
#define TC_QISR_IDX (0x1u << 0) /**< \brief (TC_QISR) InDeX */
#define TC_QISR_DIRCHG (0x1u << 1) /**< \brief (TC_QISR) DIRection CHanGe */
#define TC_QISR_QERR (0x1u << 2) /**< \brief (TC_QISR) Quadrature ERRor */
#define TC_QISR_DIR (0x1u << 8) /**< \brief (TC_QISR) DIRection */
/* -------- TC_FMR : (TC Offset: 0xD8) Fault Mode Register -------- */
#define TC_FMR_ENCF0 (0x1u << 0) /**< \brief (TC_FMR) ENable Compare Fault Channel 0 */
#define TC_FMR_ENCF1 (0x1u << 1) /**< \brief (TC_FMR) ENable Compare Fault Channel 1 */
/* -------- TC_WPMR : (TC Offset: 0xE4) Write Protection Mode Register -------- */
#define TC_WPMR_WPEN (0x1u << 0) /**< \brief (TC_WPMR) Write Protect Enable */
#define TC_WPMR_WPKEY_Pos 8
#define TC_WPMR_WPKEY_Msk (0xffffffu << TC_WPMR_WPKEY_Pos) /**< \brief (TC_WPMR) Write Protect KEY */
#define   TC_WPMR_WPKEY_PASSWD (0x54494Du << 8) /**< \brief (TC_WPMR) Writing any other value in this field aborts the write operation of the WPEN bit.Always reads as 0. */


// ----- Functions -----

void *memset( void *addr, int val, unsigned int len );
void *memcpy( void *dst, const void *src, unsigned int len );
int memcmp( const void *a, const void *b, unsigned int len );


// ----- Interrupts -----

/**< Interrupt Number Definition */
typedef enum IRQn
{
/******  Cortex-M4 Processor Exceptions Numbers ******************************/
  NonMaskableInt_IRQn   = -14, /**<  2 Non Maskable Interrupt                */
  HardFault_IRQn        = -13, /**<  3 HardFault Interrupt                   */
  MemoryManagement_IRQn = -12, /**<  4 Cortex-M4 Memory Management Interrupt */
  BusFault_IRQn         = -11, /**<  5 Cortex-M4 Bus Fault Interrupt         */
  UsageFault_IRQn       = -10, /**<  6 Cortex-M4 Usage Fault Interrupt       */
  SVCall_IRQn           = -5,  /**< 11 Cortex-M4 SV Call Interrupt           */
  DebugMonitor_IRQn     = -4,  /**< 12 Cortex-M4 Debug Monitor Interrupt     */
  PendSV_IRQn           = -2,  /**< 14 Cortex-M4 Pend SV Interrupt           */
  SysTick_IRQn          = -1,  /**< 15 Cortex-M4 System Tick Interrupt       */
/******  SAM4SD32C specific Interrupt Numbers *********************************/

  SUPC_IRQn            =  0, /**<  0 SAM4SD32C Supply Controller (SUPC) */
  RSTC_IRQn            =  1, /**<  1 SAM4SD32C Reset Controller (RSTC) */
  RTC_IRQn             =  2, /**<  2 SAM4SD32C Real Time Clock (RTC) */
  RTT_IRQn             =  3, /**<  3 SAM4SD32C Real Time Timer (RTT) */
  WDT_IRQn             =  4, /**<  4 SAM4SD32C Watchdog Timer (WDT) */
  PMC_IRQn             =  5, /**<  5 SAM4SD32C Power Management Controller (PMC) */
  EFC0_IRQn            =  6, /**<  6 SAM4SD32C Enhanced Embedded Flash Controller 0 (EFC0) */
  EFC1_IRQn            =  7, /**<  7 SAM4SD32C Enhanced Embedded Flash Controller 1 (EFC1) */
  UART0_IRQn           =  8, /**<  8 SAM4SD32C UART 0 (UART0) */
  UART1_IRQn           =  9, /**<  9 SAM4SD32C UART 1 (UART1) */
  PIOA_IRQn            = 11, /**< 11 SAM4SD32C Parallel I/O Controller A (PIOA) */
  PIOB_IRQn            = 12, /**< 12 SAM4SD32C Parallel I/O Controller B (PIOB) */
  PIOC_IRQn            = 13, /**< 13 SAM4SD32C Parallel I/O Controller C (PIOC) */
  USART0_IRQn          = 14, /**< 14 SAM4SD32C USART 0 (USART0) */
  USART1_IRQn          = 15, /**< 15 SAM4SD32C USART 1 (USART1) */
  HSMCI_IRQn           = 18, /**< 18 SAM4SD32C Multimedia Card Interface (HSMCI) */
  TWI0_IRQn            = 19, /**< 19 SAM4SD32C Two Wire Interface 0 (TWI0) */
  TWI1_IRQn            = 20, /**< 20 SAM4SD32C Two Wire Interface 1 (TWI1) */
  SPI0_IRQn            = 21, /**< 21 SAM4SD32C Serial Peripheral Interface (SPI0) */
  SSC_IRQn             = 22, /**< 22 SAM4SD32C Synchronous Serial Controller (SSC) */
  TC0_IRQn             = 23, /**< 23 SAM4SD32C Timer/Counter 0 (TC0) */
  TC1_IRQn             = 24, /**< 24 SAM4SD32C Timer/Counter 1 (TC1) */
  TC2_IRQn             = 25, /**< 25 SAM4SD32C Timer/Counter 2 (TC2) */
  TC3_IRQn             = 26, /**< 26 SAM4SD32C Timer/Counter 3 (TC3) */
  TC4_IRQn             = 27, /**< 27 SAM4SD32C Timer/Counter 4 (TC4) */
  TC5_IRQn             = 28, /**< 28 SAM4SD32C Timer/Counter 5 (TC5) */
  ADC_IRQn             = 29, /**< 29 SAM4SD32C Analog To Digital Converter (ADC) */
  DACC_IRQn            = 30, /**< 30 SAM4SD32C Digital To Analog Converter (DACC) */
  PWM_IRQn             = 31, /**< 31 SAM4SD32C Pulse Width Modulation (PWM) */
  CRCCU_IRQn           = 32, /**< 32 SAM4SD32C CRC Calculation Unit (CRCCU) */
  ACC_IRQn             = 33, /**< 33 SAM4SD32C Analog Comparator (ACC) */
  USBDEV_IRQn          = 34, /**< 34 SAM4SD32C USB Device Port (USBDEV) */

  PERIPH_COUNT_IRQn    = 35  /**< Number of peripheral IDs */
} IRQn_Type;

typedef struct _DeviceVectors
{
  /* Stack pointer */
  void* pvStack;

  /* Cortex-M handlers */
  void* pfnReset_Handler;
  void* pfnNMI_Handler;
  void* pfnHardFault_Handler;
  void* pfnMemManage_Handler;
  void* pfnBusFault_Handler;
  void* pfnUsageFault_Handler;
  void* pfnReserved1_Handler;
  void* pfnReserved2_Handler;
  void* pfnReserved3_Handler;
  void* pfnReserved4_Handler;
  void* pfnSVC_Handler;
  void* pfnDebugMon_Handler;
  void* pfnReserved5_Handler;
  void* pfnPendSV_Handler;
  void* pfnSysTick_Handler;

  /* Peripheral handlers */
  void* pfnSUPC_Handler;   /*  0 Supply Controller */
  void* pfnRSTC_Handler;   /*  1 Reset Controller */
  void* pfnRTC_Handler;    /*  2 Real Time Clock */
  void* pfnRTT_Handler;    /*  3 Real Time Timer */
  void* pfnWDT_Handler;    /*  4 Watchdog Timer */
  void* pfnPMC_Handler;    /*  5 Power Management Controller */
  void* pfnEFC0_Handler;   /*  6 Enhanced Embedded Flash Controller 0 */
  void* pfnEFC1_Handler;   /*  7 Enhanced Embedded Flash Controller 1 */
  void* pfnUART0_Handler;  /*  8 UART 0 */
  void* pfnUART1_Handler;  /*  9 UART 1 */
  void* pvReserved10;
  void* pfnPIOA_Handler;   /* 11 Parallel I/O Controller A */
  void* pfnPIOB_Handler;   /* 12 Parallel I/O Controller B */
  void* pfnPIOC_Handler;   /* 13 Parallel I/O Controller C */
  void* pfnUSART0_Handler; /* 14 USART 0 */
  void* pfnUSART1_Handler; /* 15 USART 1 */
  void* pvReserved16;
  void* pvReserved17;
  void* pfnHSMCI_Handler;  /* 18 Multimedia Card Interface */
  void* pfnTWI0_Handler;   /* 19 Two Wire Interface 0 */
  void* pfnTWI1_Handler;   /* 20 Two Wire Interface 1 */
  void* pfnSPI0_Handler;   /* 21 Serial Peripheral Interface */
  void* pfnSSC_Handler;    /* 22 Synchronous Serial Controller */
  void* pfnTC0_Handler;    /* 23 Timer/Counter 0 */
  void* pfnTC1_Handler;    /* 24 Timer/Counter 1 */
  void* pfnTC2_Handler;    /* 25 Timer/Counter 2 */
  void* pfnTC3_Handler;    /* 26 Timer/Counter 3 */
  void* pfnTC4_Handler;    /* 27 Timer/Counter 4 */
  void* pfnTC5_Handler;    /* 28 Timer/Counter 5 */
  void* pfnADC_Handler;    /* 29 Analog To Digital Converter */
  void* pfnDACC_Handler;   /* 30 Digital To Analog Converter */
  void* pfnPWM_Handler;    /* 31 Pulse Width Modulation */
  void* pfnCRCCU_Handler;  /* 32 CRC Calculation Unit */
  void* pfnACC_Handler;    /* 33 Analog Comparator */
  void* pfnUSBDEV_Handler; /* 34 USB Device Port */
} DeviceVectors;

/* Cortex-M4 core handlers */
void ResetHandler      ( void );
void NMI_Handler        ( void );
void HardFault_Handler  ( void );
void MemManage_Handler  ( void );
void BusFault_Handler   ( void );
void UsageFault_Handler ( void );
void SVC_Handler        ( void );
void DebugMon_Handler   ( void );
void PendSV_Handler     ( void );
void SysTick_Handler    ( void );

/* Peripherals handlers */
void ACC_Handler        ( void );
void ADC_Handler        ( void );
void CRCCU_Handler      ( void );
void DACC_Handler       ( void );
void EFC0_Handler       ( void );
void EFC1_Handler       ( void );
void HSMCI_Handler      ( void );
void PIOA_Handler       ( void );
void PIOB_Handler       ( void );
void PIOC_Handler       ( void );
void PMC_Handler        ( void );
void PWM_Handler        ( void );
void RSTC_Handler       ( void );
void RTC_Handler        ( void );
void RTT_Handler        ( void );
void SPI0_Handler       ( void );
void SSC_Handler        ( void );
void SUPC_Handler       ( void );
void TC0_Handler        ( void );
void TC1_Handler        ( void );
void TC2_Handler        ( void );
void TC3_Handler        ( void );
void TC4_Handler        ( void );
void TC5_Handler        ( void );
void TWI0_Handler       ( void );
void TWI1_Handler       ( void );
void UART0_Handler      ( void );
void UART1_Handler      ( void );
void USBDEV_Handler     ( void );
void USART0_Handler     ( void );
void USART1_Handler     ( void );
void WDT_Handler        ( void );
