/**
 * \file
 *
 * \brief SAM4S Xplained Pro board definition and driver
 *
 * Copyright (C) 2012-2016 Atmel Corporation. All rights reserved.
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
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef SAM4S_XPLAINED_PRO_H
#define SAM4S_XPLAINED_PRO_H

#include <conf_board.h>

/**
 * \ingroup group_common_boards
 * \defgroup sam4s_xplained_pro_group SAM4S Xplained Pro board
 * @{
 */


/**
 * \defgroup sam4s_xplained_pro_config_group Configuration
 *
 * Symbols to use for configuring the board and its initialization.
 *
 * @{
 */
#ifdef __DOXYGEN__

//! \name Initialization
//@{

/**
 * \def CONF_BOARD_KEEP_WATCHDOG_AT_INIT
 * \brief Let watchdog remain enabled
 *
 * If this symbol is defined, the watchdog is left running with its current
 * configuration. Otherwise, it gets disabled during board initialization.
 */
# ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
#  define CONF_BOARD_KEEP_WATCHDOG_AT_INIT
# endif

//@}

#endif // __DOXYGEN__
/** @} */


/**
 * \defgroup sam4s_xplained_pro_features_group Features
 *
 * Symbols that describe features and capabilities of the board.
 *
 * @{
 */

//! Name string macro
#define BOARD_NAME                "SAM4S_XPLAINED_PRO"

//! \name Resonator definitions
//@{
#define BOARD_FREQ_SLCK_XTAL      (32768U)
#define BOARD_FREQ_SLCK_BYPASS    (32768U)
#define BOARD_FREQ_MAINCK_XTAL    (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS  (12000000U)
#define BOARD_MCK                 CHIP_FREQ_CPU_MAX
#define BOARD_OSC_STARTUP_US      15625
//@}

//! \name LED0 definitions
//@{
#define LED0_PIN                  IOPORT_CREATE_PIN(PIOC, 23)
#define LED0_ACTIVE               false
#define LED0_INACTIVE             !LED0_ACTIVE
#define LED0				      LED0_GPIO
#define LED0_GPIO				  LED0_PIN
//@}

//! \name SW0 definitions
//@{
#define SW0_PIN                   IOPORT_CREATE_PIN(PIOA, 2)
#define SW0_ACTIVE                false
#define SW0_INACTIVE              !SW0_ACTIVE
#define SW0_SUPC_INPUT            2
//@}

/**
 * \name LED #0 definitions
 *
 * Wrapper macros for LED0, to ensure common naming across all Xplained Pro
 * boards.
 */
//@{
#define LED_0_NAME                "LED0 (yellow)"
#define LED_0_PIN                 LED0_PIN
#define LED_0_ACTIVE              LED0_ACTIVE
#define LED_0_INACTIVE            LED0_INACTIVE
//@}

//! Number of on-board LEDs
#define LED_COUNT 1

/**
 * \name Button #0 definitions
 *
 * Wrapper macros for SW0, to ensure common naming across all Xplained Pro
 * boards.
 */
//@{
#define BUTTON_0_NAME             "SW0"
#define BUTTON_0_PIN              SW0_PIN
#define BUTTON_0_ACTIVE           SW0_ACTIVE
#define BUTTON_0_INACTIVE         SW0_INACTIVE
#define BUTTON_0_SUPC_INPUT       SW0_SUPC_INPUT
#define GPIO_PUSH_BUTTON_0        BUTTON_0_PIN 

#define PIN_PUSHBUTTON_0_MASK     PIO_PA2
#define PIN_PUSHBUTTON_0_PIO      PIOA
#define PIN_PUSHBUTTON_0_ID       ID_PIOA
#define PIN_PUSHBUTTON_0_TYPE     PIO_INPUT
#define PIN_PUSHBUTTON_0_ATTR     (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)
//@}

//! Number of on-board buttons
#define BUTTON_COUNT              1

//! \name Extension header #1 pin definitions
//@{
#define EXT1_PIN_3                IOPORT_CREATE_PIN(PIOA, 17)
#define EXT1_PIN_4                IOPORT_CREATE_PIN(PIOA, 18)
#define EXT1_PIN_5                IOPORT_CREATE_PIN(PIOA, 24)
#define EXT1_PIN_6                IOPORT_CREATE_PIN(PIOA, 25)
#define EXT1_PIN_7                IOPORT_CREATE_PIN(PIOA, 23)
#define EXT1_PIN_8                IOPORT_CREATE_PIN(PIOA, 19)
#define EXT1_PIN_9                IOPORT_CREATE_PIN(PIOA, 1)
#define EXT1_PIN_10               IOPORT_CREATE_PIN(PIOA, 6)
#define EXT1_PIN_11               IOPORT_CREATE_PIN(PIOA, 3)
#define EXT1_PIN_12               IOPORT_CREATE_PIN(PIOA, 4)
#define EXT1_PIN_13               IOPORT_CREATE_PIN(PIOA, 21)
#define EXT1_PIN_14               IOPORT_CREATE_PIN(PIOA, 22)
#define EXT1_PIN_15               IOPORT_CREATE_PIN(PIOA, 11)
#define EXT1_PIN_16               IOPORT_CREATE_PIN(PIOA, 13)
#define EXT1_PIN_17               IOPORT_CREATE_PIN(PIOA, 12)
#define EXT1_PIN_18               IOPORT_CREATE_PIN(PIOA, 14)
//@}

//! \name Extension header #1 pin definitions by function
//@{
#define EXT1_PIN_ADC_0            EXT1_PIN_3
#define EXT1_PIN_ADC_1            EXT1_PIN_4
#define EXT1_PIN_GPIO_0           EXT1_PIN_5
#define EXT1_PIN_GPIO_1           EXT1_PIN_6
#define EXT1_PIN_PWM_0            EXT1_PIN_7
#define EXT1_PIN_PWM_1            EXT1_PIN_8
#define EXT1_PIN_IRQ              EXT1_PIN_9
#define EXT1_PIN_TWI_SDA          EXT1_PIN_11
#define EXT1_PIN_TWI_SCL          EXT1_PIN_12
#define EXT1_PIN_UART_RX          EXT1_PIN_13
#define EXT1_PIN_UART_TX          EXT1_PIN_14
#define EXT1_PIN_SPI_SS_1         EXT1_PIN_10
#define EXT1_PIN_SPI_SS_0         EXT1_PIN_15
#define EXT1_PIN_SPI_MOSI         EXT1_PIN_16
#define EXT1_PIN_SPI_MISO         EXT1_PIN_17
#define EXT1_PIN_SPI_SCK          EXT1_PIN_18
//@}

//! \name Extension header #1 ADC definitions
//@{
#define EXT1_ADC_MODULE           ADC
#define EXT1_ADC_0_CHANNEL        0
#define EXT1_ADC_1_CHANNEL        1
//@}

//! \name Extension header #1 PWM definitions
//@{
#define EXT1_PWM_MODULE           PWM
#define EXT1_PWM_CHANNEL          0
//@}

//! \name Extension header #1 IRQ/External interrupt definitions
//@{
#define EXT1_IRQ_MODULE           SUPC
#define EXT1_IRQ_INPUT            1
//@}

//! \name Extension header #1 TWI definitions
//@{
#define EXT1_TWI_MODULE           TWI0
//@}

//! \name Extension header #1 UART definitions
//@{
#define EXT1_UART_MODULE          USART1
//@}

//! \name Extension header #1 SPI definitions
//@{
#define EXT1_SPI_MODULE           SPI
//@}

//! \name Extension header #2 pin definitions
//@{
#define EXT2_PIN_3                IOPORT_CREATE_PIN(PIOB, 0)
#define EXT2_PIN_4                IOPORT_CREATE_PIN(PIOB, 1)
#define EXT2_PIN_5                IOPORT_CREATE_PIN(PIOC, 24)
#define EXT2_PIN_6                IOPORT_CREATE_PIN(PIOC, 25)
#define EXT2_PIN_7                IOPORT_CREATE_PIN(PIOC, 19)
#define EXT2_PIN_8                IOPORT_CREATE_PIN(PIOA, 20)
#define EXT2_PIN_9                IOPORT_CREATE_PIN(PIOC, 26)
#define EXT2_PIN_10               IOPORT_CREATE_PIN(PIOC, 27)
#define EXT2_PIN_11               IOPORT_CREATE_PIN(PIOA, 3)
#define EXT2_PIN_12               IOPORT_CREATE_PIN(PIOA, 4)
#define EXT2_PIN_13               IOPORT_CREATE_PIN(PIOA, 21)
#define EXT2_PIN_14               IOPORT_CREATE_PIN(PIOA, 22)
#define EXT2_PIN_15               IOPORT_CREATE_PIN(PIOA, 9)
#define EXT2_PIN_16               IOPORT_CREATE_PIN(PIOA, 13)
#define EXT2_PIN_17               IOPORT_CREATE_PIN(PIOA, 12)
#define EXT2_PIN_18               IOPORT_CREATE_PIN(PIOA, 14)
//@}

//! \name Extension header #2 pin definitions by function
//@{
#define EXT2_PIN_ADC_0            EXT2_PIN_3
#define EXT2_PIN_ADC_1            EXT2_PIN_4
#define EXT2_PIN_GPIO_0           EXT2_PIN_5
#define EXT2_PIN_GPIO_1           EXT2_PIN_6
#define EXT2_PIN_PWM_0            EXT2_PIN_7
#define EXT2_PIN_PWM_1            EXT2_PIN_8
#define EXT2_PIN_TWI_SDA          EXT2_PIN_11
#define EXT2_PIN_TWI_SCL          EXT2_PIN_12
#define EXT2_PIN_UART_RX          EXT2_PIN_13
#define EXT2_PIN_UART_TX          EXT2_PIN_14
#define EXT2_PIN_SPI_SS_1         EXT2_PIN_10
#define EXT2_PIN_SPI_SS_0         EXT2_PIN_15
#define EXT2_PIN_SPI_MOSI         EXT2_PIN_16
#define EXT2_PIN_SPI_MISO         EXT2_PIN_17
#define EXT2_PIN_SPI_SCK          EXT2_PIN_18
//@}

//! \name Extension header #2 ADC definitions
//@{
#define EXT2_ADC_MODULE           ADC
#define EXT2_ADC_0_CHANNEL        4
#define EXT2_ADC_1_CHANNEL        5
//@}

//! \name Extension header #2 PWM definitions
//@{
#define EXT2_PWM_MODULE           PWM
#define EXT2_PWM_CHANNEL          1
//@}

//! \name Extension header #2 TWI definitions
//@{
#define EXT2_TWI_MODULE           TWI0
//@}

//! \name Extension header #2 UART definitions
//@{
#define EXT2_UART_MODULE          USART1
//@}

//! \name Extension header #2 SPI definitions
//@{
#define EXT2_SPI_MODULE           SPI
//@}

//! \name Extension header #3 pin definitions
//@{
#define EXT3_PIN_3                IOPORT_CREATE_PIN(PIOC, 29)
#define EXT3_PIN_4                IOPORT_CREATE_PIN(PIOC, 30)
#define EXT3_PIN_5                IOPORT_CREATE_PIN(PIOC, 21)
#define EXT3_PIN_6                IOPORT_CREATE_PIN(PIOC, 22)
#define EXT3_PIN_7                IOPORT_CREATE_PIN(PIOC, 20)
#define EXT3_PIN_8                IOPORT_CREATE_PIN(PIOA, 16)
#define EXT3_PIN_9                IOPORT_CREATE_PIN(PIOA, 0)
#define EXT3_PIN_10               IOPORT_CREATE_PIN(PIOC, 31)
#define EXT3_PIN_11               IOPORT_CREATE_PIN(PIOB, 4)
#define EXT3_PIN_12               IOPORT_CREATE_PIN(PIOB, 5)
#define EXT3_PIN_13               IOPORT_CREATE_PIN(PIOB, 2)
#define EXT3_PIN_14               IOPORT_CREATE_PIN(PIOB, 3)
#define EXT3_PIN_15               IOPORT_CREATE_PIN(PIOA, 10)
#define EXT3_PIN_16               IOPORT_CREATE_PIN(PIOA, 13)
#define EXT3_PIN_17               IOPORT_CREATE_PIN(PIOA, 12)
#define EXT3_PIN_18               IOPORT_CREATE_PIN(PIOA, 14)
//@}

//! \name Extension header #3 pin definitions by function
//@{
#define EXT3_PIN_ADC_0            EXT3_PIN_3
#define EXT3_PIN_ADC_1            EXT3_PIN_4
#define EXT3_PIN_GPIO_0           EXT3_PIN_5
#define EXT3_PIN_GPIO_1           EXT3_PIN_6
#define EXT3_PIN_PWM_0            EXT3_PIN_7
#define EXT3_PIN_PWM_1            EXT3_PIN_8
#define EXT3_PIN_IRQ              EXT3_PIN_9
#define EXT3_PIN_TWI_SDA          EXT3_PIN_11
#define EXT3_PIN_TWI_SCL          EXT3_PIN_12
#define EXT3_PIN_UART_RX          EXT3_PIN_13
#define EXT3_PIN_UART_TX          EXT3_PIN_14
#define EXT3_PIN_SPI_SS_1         EXT3_PIN_10
#define EXT3_PIN_SPI_SS_0         EXT3_PIN_15
#define EXT3_PIN_SPI_MOSI         EXT3_PIN_16
#define EXT3_PIN_SPI_MISO         EXT3_PIN_17
#define EXT3_PIN_SPI_SCK          EXT3_PIN_18
//@}

//! \name Extension header #3 ADC definitions
//@{
#define EXT3_ADC_MODULE           ADC
#define EXT3_ADC_0_CHANNEL        13
#define EXT3_ADC_1_CHANNEL        14
//@}

//! \name Extension header #3 PWM definitions
//@{
#define EXT3_PWM_MODULE           PWM
#define EXT3_PWM_CHANNEL          2
//@}

//! \name Extension header #3 IRQ/External interrupt definitions
//@{
#define EXT3_IRQ_MODULE           SUPC
#define EXT3_IRQ_INPUT            0
//@}

//! \name Extension header #3 TWI definitions
//@{
#define EXT3_TWI_MODULE           TWI1
//@}

//! \name Extension header #3 UART definitions
//@{
#define EXT3_UART_MODULE          UART1
//@}

//! \name Extension header #3 SPI definitions
//@{
#define EXT3_SPI_MODULE           SPI
//@}

//! \name Embedded debugger GPIO interface definitions
//@{
#define EDBG_GPIO0_PIN            IOPORT_CREATE_PIN(PIOA, 6)
#define EDBG_GPIO1_PIN            IOPORT_CREATE_PIN(PIOC, 22)
#define EDBG_GPIO2_PIN            IOPORT_CREATE_PIN(PIOC, 24)
#define EDBG_GPIO3_PIN            IOPORT_CREATE_PIN(PIOC, 25)
//@}

//! \name Embedded debugger USART interface definitions
//@{
#define EDBG_UART_MODULE          UART1
//@}

//! \name Embedded debugger TWI interface definitions
//@{
#define EDBG_TWI_MODULE           TWI0
//@}

//! \name Embedded debugger SPI interface definitions
//@{
#define EDBG_SPI_MODULE           SPI
//@}

#define CONSOLE_UART              UART1
#define CONSOLE_UART_ID           ID_UART1

/** UART1 pins (UTXD1 and URXD1) definitions, PB2,PB3. */
#define PINS_UART1                (PIO_PB2A_URXD1 | PIO_PB3A_UTXD1)
#define PINS_UART1_FLAGS          (PIO_PERIPH_A | PIO_DEFAULT)
#define PINS_UART1_PIO            PIOB

//! \name SPI
//@{
/** SPI MISO pin definition. */
#define SPI_MISO_GPIO         (PIO_PA12_IDX)
#define SPI_MISO_FLAGS       (PIO_PERIPH_A | PIO_PULLUP)
/** SPI MOSI pin definition. */
#define SPI_MOSI_GPIO         (PIO_PA13_IDX)
#define SPI_MOSI_FLAGS       (PIO_PERIPH_A | PIO_PULLUP)
/** SPI SPCK pin definition. */
#define SPI_SPCK_GPIO         (PIO_PA14_IDX)
#define SPI_SPCK_FLAGS       (PIO_PERIPH_A | PIO_PULLUP)

/** SPI chip select 0 pin definition. (Only one configuration is possible) */
#define SPI_NPCS0_GPIO         (PIO_PA11_IDX)
#define SPI_NPCS0_FLAGS           (PIO_PERIPH_A | PIO_DEFAULT)
/** SPI chip select 1 pin definition. (multiple configurations are possible) */
#define SPI_NPCS1_PA9_GPIO     (PIO_PA9_IDX)
#define SPI_NPCS1_PA9_FLAGS       (PIO_PERIPH_B | PIO_DEFAULT)
#define SPI_NPCS1_PA31_GPIO    (PIO_PA31_IDX)
#define SPI_NPCS1_PA31_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)
#define SPI_NPCS1_PB14_GPIO    (PIO_PB14_IDX)
#define SPI_NPCS1_PB14_FLAGS      (PIO_PERIPH_A | PIO_DEFAULT)
#define SPI_NPCS1_PC4_GPIO     (PIO_PC4_IDX)
#define SPI_NPCS1_PC4_FLAGS       (PIO_PERIPH_B | PIO_DEFAULT)
/** SPI chip select 2 pin definition. (multiple configurations are possible) */
#define SPI_NPCS2_PA10_GPIO    (PIO_PA10_IDX)
#define SPI_NPCS2_PA10_FLAGS      (PIO_PERIPH_B | PIO_PULLUP)
#define SPI_NPCS2_PA30_GPIO    (PIO_PA30_IDX)
#define SPI_NPCS2_PA30_FLAGS      (PIO_PERIPH_B | PIO_DEFAULT)
#define SPI_NPCS2_PB2_GPIO     (PIO_PB2_IDX)
#define SPI_NPCS2_PB2_FLAGS       (PIO_PERIPH_B | PIO_DEFAULT)
/** SPI chip select 3 pin definition. (multiple configurations are possible) */
#define SPI_NPCS3_PA3_GPIO     (PIO_PA3_IDX)
#define SPI_NPCS3_PA3_FLAGS       (PIO_PERIPH_B | PIO_DEFAULT)
#define SPI_NPCS3_PA5_GPIO     (PIO_PA5_IDX)
#define SPI_NPCS3_PA5_FLAGS       (PIO_PERIPH_B | PIO_DEFAULT)
#define SPI_NPCS3_PA22_GPIO    (PIO_PA22_IDX)
#define SPI_NPCS3_PA22_FLAGS      (PIO_PERIPH_B | PIO_DEFAULT)
//@}

//! \name OLED
//@{
#define UG_2832HSWEG04_DATA_CMD_GPIO      (PIO_PC21_IDX)
#define UG_2832HSWEG04_DATA_CMD_FLAGS    (PIO_OUTPUT_1 | PIO_PULLUP)
#define UG_2832HSWEG04_RESET_GPIO      (PIO_PC31_IDX)
#define UG_2832HSWEG04_RESET_FLAGS    (PIO_OUTPUT_1 | PIO_PULLUP)

//#define UG_2832HSWEG04_SPI              &USARTC0
//#define UG_2832HSWEG04_DATA_CMD         IOPORT_CREATE_PIN(PIOC, 0)
//#define UG_2832HSWEG04_RESET            IOPORT_CREATE_PIN(PIOC, 31)
//#define UG_2832HSWEG04_SPI_SCK          IOPORT_CREATE_PIN(PIOA, 14)
//#define UG_2832HSWEG04_SPI_MOSI         IOPORT_CREATE_PIN(PIOA, 13)
#define UG_2832HSWEG04_SS               2
#define UG_2832HSWEG04_BAUDRATE         5000000
//! \name OLED dimensions
//@{
#define LCD_WIDTH_PIXELS                (128)
#define LCD_HEIGHT_PIXELS               (32)
//@}
//@}

//! \name TWI
//@{
/** TWI0 pin definitions */
#define TWI0_DATA_GPIO   PIO_PA3_IDX
#define TWI0_DATA_FLAGS  (PIO_PERIPH_A | PIO_PULLUP)
#define TWI0_CLK_GPIO    PIO_PA4_IDX
#define TWI0_CLK_FLAGS   (PIO_PERIPH_A | PIO_PULLUP)
//@}



//! \name OLED Xplained Pro LED definitions */
//@{
#define IO1_LED1_PIN                  EXT3_PIN_7
#define IO1_LED1_ACTIVE               false
#define IO1_LED1_INACTIVE             !IO1_LED1_ACTIVE

#define IO1_LED2_PIN                  EXT3_PIN_8
#define IO1_LED2_ACTIVE               false
#define IO1_LED2_INACTIVE             !IO1_LED2_ACTIVE

#define IO1_LED3_PIN                  EXT3_PIN_6
#define IO1_LED3_ACTIVE               false
#define IO1_LED3_INACTIVE             !IO1_LED3_ACTIVE
//@}

//! \name OLED Xplained Pro button definitions */
//@{
/** Push button #1 definition. Attributes = pull-up + debounce + interrupt on rising edge. */
#define PUSHBUTTON_1_NAME    "BUTTON 1"
#define GPIO_PUSH_BUTTON_1   (PIO_PA0_IDX)
#define GPIO_PUSH_BUTTON_1_FLAGS    (PIO_INPUT | PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)

#define PIN_PUSHBUTTON_1    {PIO_PA0, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE}
#define PIN_PUSHBUTTON_1_MASK PIO_PA0
#define PIN_PUSHBUTTON_1_PIO PIOA
#define PIN_PUSHBUTTON_1_ID ID_PIOA
#define PIN_PUSHBUTTON_1_TYPE PIO_INPUT
#define PIN_PUSHBUTTON_1_ATTR (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)

/** Push button #2 definition. Attributes = pull-up + debounce + interrupt on rising edge. */
#define PUSHBUTTON_2_NAME    "BUTTON 2"
#define GPIO_PUSH_BUTTON_2   (PIO_PC29_IDX)
#define GPIO_PUSH_BUTTON_2_FLAGS    (PIO_INPUT | PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)

#define PIN_PUSHBUTTON_2    {PIO_PC29, PIOC, ID_PIOC, PIO_INPUT, PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE}
#define PIN_PUSHBUTTON_2_MASK PIO_PC29
#define PIN_PUSHBUTTON_2_PIO PIOC
#define PIN_PUSHBUTTON_2_ID ID_PIOC
#define PIN_PUSHBUTTON_2_TYPE PIO_INPUT
#define PIN_PUSHBUTTON_2_ATTR (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)

/** Push button #3 definition. Attributes = pull-up + debounce + interrupt on rising edge. */
#define PUSHBUTTON_3_NAME    "BUTTON 3"
#define GPIO_PUSH_BUTTON_3   (PIO_PC30_IDX)
#define GPIO_PUSH_BUTTON_3_FLAGS    (PIO_INPUT | PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)

#define PIN_PUSHBUTTON_3    {PIO_PC30, PIOC, ID_PIOC, PIO_INPUT, PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE}
#define PIN_PUSHBUTTON_3_MASK PIO_PC30
#define PIN_PUSHBUTTON_3_PIO PIOC
#define PIN_PUSHBUTTON_3_ID ID_PIOC
#define PIN_PUSHBUTTON_3_TYPE PIO_INPUT
#define PIN_PUSHBUTTON_3_ATTR (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE)
//@}

//! \name Light Sensor
//@{
#define LIGHT_SENSOR_GPIO   PIO_PB0_IDX
#define LIGHT_SENSOR_FLAGS  PIO_INPUT
//@}

//! \name SD Card
//@{
#define SD_MMC_0_CD_GPIO            (PIO_PC12_IDX)
#define SD_MMC_0_CD_PIO_ID          ID_PIOC
#define SD_MMC_0_CD_FLAGS           (PIO_INPUT | PIO_PULLUP)

#define SD_MMC_0_CD_DETECT_VALUE    0

#define SD_MMC_0_CD    {PIO_PC12, PIOC, ID_PIOC, PIO_INPUT, PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE}
#define SD_MMC_0_CD_MASK PIO_PC12
#define SD_MMC_0_CD_PIO PIOC
#define SD_MMC_0_CD_ID ID_PIOC
#define SD_MMC_0_CD_TYPE PIO_INPUT
#define SD_MMC_0_CD_ATTR (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_EDGE)


/*! \name Connections of the AT86RFX transceiver
 */
/** @{ */
#define AT86RFX_SPI                  SPI
#define AT86RFX_RST_PIN              EXT1_PIN_7
#define AT86RFX_IRQ_PIN              EXT1_PIN_9
#define AT86RFX_SLP_PIN              EXT1_PIN_10
#define AT86RFX_SPI_CS               0
#define AT86RFX_SPI_CS_PIN           SPI_NPCS0_GPIO
#define AT86RFX_SPI_CS_FLAGS         SPI_NPCS0_FLAGS
#define AT86RFX_SPI_MOSI             EXT1_PIN_16
#define AT86RFX_SPI_MISO             EXT1_PIN_17
#define AT86RFX_SPI_SCK              EXT1_PIN_18
#define AT86RFX_CSD     		     EXT1_PIN_5
#define AT86RFX_CPS 	             EXT1_PIN_8


void at86rfx_isr(uint32_t, uint32_t);

#define AT86RFX_INTC_INIT()         ioport_set_pin_dir(AT86RFX_IRQ_PIN, IOPORT_DIR_INPUT);\
									ioport_set_pin_sense_mode(AT86RFX_IRQ_PIN, IOPORT_SENSE_RISING);\
									pmc_enable_periph_clk(ID_PIOA);\
									pio_set_debounce_filter(PIOA, PIO_PA1, 10);\
									pio_handler_set(PIOA, ID_PIOA, PIO_PA1, PIO_IT_RISE_EDGE, at86rfx_isr);\
									NVIC_EnableIRQ((IRQn_Type) ID_PIOA);\
									pio_enable_interrupt(PIOA, PIO_PA1);									

#define AT86RFX_ISR()               void at86rfx_isr(uint32_t arg1, uint32_t arg2)

/** Enables the transceiver main interrupt. */
#define ENABLE_TRX_IRQ()            pio_enable_pin_interrupt(AT86RFX_IRQ_PIN)

/** Disables the transceiver main interrupt. */
#define DISABLE_TRX_IRQ()           pio_disable_pin_interrupt(AT86RFX_IRQ_PIN)

/** Clears the transceiver main interrupt. */
#define CLEAR_TRX_IRQ()             NVIC_ClearPendingIRQ(PIOA_IRQn);
/*
 * This macro saves the trx interrupt status and disables the trx interrupt.
 */
#define ENTER_TRX_REGION()          pio_disable_pin_interrupt(AT86RFX_IRQ_PIN);

/*
 *  This macro restores the transceiver interrupt status
 */
#define LEAVE_TRX_REGION()         pio_enable_pin_interrupt(AT86RFX_IRQ_PIN)

/** @} */

//! \name NandFlash
//@{
/** NandFlash pins definition: OE. */
#define PIN_EBI_NANDOE    (PIO_PC9_IDX)
#define PIN_EBI_NANDOE_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

/** NandFlash pins definition: WE. */
#define PIN_EBI_NANDWE    (PIO_PC10_IDX)
#define PIN_EBI_NANDWE_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

/** NandFlash pins definition: CLE. */
#define PIN_EBI_NANDCLE    (PIO_PC17_IDX)
#define PIN_EBI_NANDCLE_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

/** NandFlash pins definition: ALE. */
#define PIN_EBI_NANDALE    (PIO_PC16_IDX)
#define PIN_EBI_NANDALE_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

/** NandFlash pins definition: DATA. */
#define PIN_EBI_NANDIO_0    (PIO_PC0_IDX)
#define PIN_EBI_NANDIO_0_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

#define PIN_EBI_NANDIO_1    (PIO_PC1_IDX)
#define PIN_EBI_NANDIO_1_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

#define PIN_EBI_NANDIO_2    (PIO_PC2_IDX)
#define PIN_EBI_NANDIO_2_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

#define PIN_EBI_NANDIO_3    (PIO_PC3_IDX)
#define PIN_EBI_NANDIO_3_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

#define PIN_EBI_NANDIO_4    (PIO_PC4_IDX)
#define PIN_EBI_NANDIO_4_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

#define PIN_EBI_NANDIO_5    (PIO_PC5_IDX)
#define PIN_EBI_NANDIO_5_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

#define PIN_EBI_NANDIO_6    (PIO_PC6_IDX)
#define PIN_EBI_NANDIO_6_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

#define PIN_EBI_NANDIO_7    (PIO_PC7_IDX)
#define PIN_EBI_NANDIO_7_FLAGS    (PIO_PERIPH_A | PIO_PULLUP)

/** Nandflash chip enable pin definition. */
#define PIN_NF_CE_IDX    (PIO_PC14_IDX)
#define PIN_NF_CE_FLAGS    (PIO_TYPE_PIO_OUTPUT_1 | PIO_DEFAULT)

/** Nandflash ready/busy pin definition. */
#define PIN_NF_RB_IDX    (PIO_PC13_IDX)
#define PIN_NF_RB_FLAGS    (PIO_INPUT | PIO_PULLUP)

/* Chip select number for nand */
#define BOARD_NAND_CS    0

/*----------------------------------------------------------------------------*/
/**
 * \page sam4s_ek_mem "SAM4S-XPLAINED-PRO - Memories"
 * This page lists definitions related to internal & external on-board memories.
 *
 * \section NandFlash
 * - \ref BOARD_NF_COMMAND_ADDR
 * - \ref BOARD_NF_ADDRESS_ADDR
 * - \ref BOARD_NF_DATA_ADDR
 */

/** Address for transferring command bytes to the nandflash. */
#define BOARD_NF_COMMAND_ADDR   0x60400000
/** Address for transferring address bytes to the nandflash. */
#define BOARD_NF_ADDRESS_ADDR   0x60200000
/** Address for transferring data bytes to the nandflash. */
#define BOARD_NF_DATA_ADDR      0x60000000
/* Bus width for NAND */
#define CONF_NF_BUSWIDTH    8
/* Access timing for NAND */
#define CONF_NF_SETUP_TIMING (SMC_SETUP_NWE_SETUP(0) \
		| SMC_SETUP_NCS_WR_SETUP(1) \
		| SMC_SETUP_NRD_SETUP(0) \
		| SMC_SETUP_NCS_RD_SETUP(1))
#define CONF_NF_PULSE_TIMING (SMC_PULSE_NWE_PULSE(2) \
		| SMC_PULSE_NCS_WR_PULSE(3) \
		| SMC_PULSE_NRD_PULSE(4) \
		| SMC_PULSE_NCS_RD_PULSE(4))
#define CONF_NF_CYCLE_TIMING (SMC_CYCLE_NWE_CYCLE(4) \
		| SMC_CYCLE_NRD_CYCLE(7))
//@}

//! \name HSMCI pins definition
//@{
/*! Number of slot connected on HSMCI interface */
#define SD_MMC_HSMCI_MEM_CNT      1
#define SD_MMC_HSMCI_SLOT_0_SIZE  4
#define PINS_HSMCI   {0x3fUL << 26, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_PULLUP}
/** HSMCI MCCDA pin definition. */
#define PIN_HSMCI_MCCDA_GPIO            (PIO_PA28_IDX)
#define PIN_HSMCI_MCCDA_FLAGS           (PIO_PERIPH_C | PIO_DEFAULT)
/** HSMCI MCCK pin definition. */
#define PIN_HSMCI_MCCK_GPIO             (PIO_PA29_IDX)
#define PIN_HSMCI_MCCK_FLAGS            (PIO_PERIPH_C | PIO_DEFAULT)
/** HSMCI MCDA0 pin definition. */
#define PIN_HSMCI_MCDA0_GPIO            (PIO_PA30_IDX)
#define PIN_HSMCI_MCDA0_FLAGS           (PIO_PERIPH_C | PIO_DEFAULT)
/** HSMCI MCDA1 pin definition. */
#define PIN_HSMCI_MCDA1_GPIO            (PIO_PA31_IDX)
#define PIN_HSMCI_MCDA1_FLAGS           (PIO_PERIPH_C | PIO_DEFAULT)
/** HSMCI MCDA2 pin definition. */
#define PIN_HSMCI_MCDA2_GPIO            (PIO_PA26_IDX)
#define PIN_HSMCI_MCDA2_FLAGS           (PIO_PERIPH_C | PIO_DEFAULT)
/** HSMCI MCDA3 pin definition. */
#define PIN_HSMCI_MCDA3_GPIO            (PIO_PA27_IDX)
#define PIN_HSMCI_MCDA3_FLAGS           (PIO_PERIPH_C | PIO_DEFAULT)
/** SD/MMC card detect pin definition. */
//@}

/** @} */

/** @} */

#endif /* SAM4S_XPLAINED_PRO_H */
