/* Copyright (C) 2014-2019 by Jacob Alexander
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

// Project Includes
#include <Lib/mcu_compat.h>
#if defined(_kinetis_)
#include <Lib/kinetis.h>
#elif defined(_sam_)
#include <Lib/sam.h>
#endif

// Local Includes
#include "gpio.h"



// ----- Defines -----

// ----- Function Declarations -----

// ----- Variables -----

// ----- Functions -----

// Pin action (Strobe, Sense, Strobe Setup, Sense Setup)
// GPIO_Config is only set with DriveSetup and ReadSetup, otherwise it is ignored
uint8_t GPIO_Ctrl( GPIO_Pin gpio, GPIO_Type type, GPIO_Config config )
{
#if defined(_kinetis_)
	// NOTE: This function is highly dependent upon the organization of the register map
	//       Only guaranteed to work with Freescale Kinetis MCUs
	// Register width is defined as size of a pointer
	unsigned int gpio_offset = gpio.port * 0x40   / sizeof(unsigned int*);
	unsigned int port_offset = gpio.port * 0x1000 / sizeof(unsigned int*) + gpio.pin;

	// Assumes 0x40 between GPIO Port registers and 0x1000 between PORT pin registers
	// See Lib/kinetis.h
	volatile unsigned int *GPIO_PDDR = (unsigned int*)(&GPIOA_PDDR) + gpio_offset;
	volatile unsigned int *GPIO_PSOR = (unsigned int*)(&GPIOA_PSOR) + gpio_offset;
	volatile unsigned int *GPIO_PCOR = (unsigned int*)(&GPIOA_PCOR) + gpio_offset;
	volatile unsigned int *GPIO_PTOR = (unsigned int*)(&GPIOA_PTOR) + gpio_offset;
	volatile unsigned int *GPIO_PDIR = (unsigned int*)(&GPIOA_PDIR) + gpio_offset;
	volatile unsigned int *PORT_PCR  = (unsigned int*)(&PORTA_PCR0) + port_offset;

	// Operation depends on Type
	switch ( type )
	{
	case GPIO_Type_DriveHigh:
		*GPIO_PSOR |= (1 << gpio.pin);
		break;

	case GPIO_Type_DriveLow:
		*GPIO_PCOR |= (1 << gpio.pin);
		break;

	case GPIO_Type_DriveToggle:
		*GPIO_PTOR |= (1 << gpio.pin);
		break;

	case GPIO_Type_DriveSetup:
		// Set as output pin
		*GPIO_PDDR |= (1 << gpio.pin);

		// Configure pin with slow slew, high drive strength and GPIO mux
		*PORT_PCR = PORT_PCR_SRE | PORT_PCR_DSE | PORT_PCR_MUX(1);

		// Enabling open-drain if specified
		switch ( config )
		{
		case GPIO_Config_Opendrain:
			*PORT_PCR |= PORT_PCR_ODE;
			break;

		// Do nothing otherwise
		default:
			break;
		}
		break;

	case GPIO_Type_Read:
		return *GPIO_PDIR & (1 << gpio.pin) ? 1 : 0;

	case GPIO_Type_ReadSetup:
		// Set as input pin
		*GPIO_PDDR &= ~(1 << gpio.pin);

		// Configure pin with passive filter and GPIO mux
		*PORT_PCR = PORT_PCR_PFE | PORT_PCR_MUX(1);

		// Pull resistor config
		switch ( config )
		{
		case GPIO_Config_Pullup:
			*PORT_PCR |= PORT_PCR_PE | PORT_PCR_PS;
			break;

		case GPIO_Config_Pulldown:
			*PORT_PCR |= PORT_PCR_PE;
			break;

		// Do nothing otherwise
		default:
			break;
		}
		break;
	}
#elif defined(_sam_)
#if defined(_sam4s_c_)
	volatile Pio *ports[] = {PIOA, PIOB, PIOC};
#else
	volatile Pio *ports[] = {PIOA, PIOB};
#endif
	volatile Pio *pio = ports[gpio.port];

	// Operation depends on Type
	switch ( type )
	{
	case GPIO_Type_DriveHigh:
		pio->PIO_SODR |= (1 << gpio.pin);
		break;

	case GPIO_Type_DriveLow:
		pio->PIO_CODR |= (1 << gpio.pin);
		break;

	case GPIO_Type_DriveToggle:
		// Toggle pin opposite to the current state
		if ( pio->PIO_ODSR & (1 << gpio.pin) )
		{
			pio->PIO_CODR |= (1 << gpio.pin);
		}
		else
		{
			pio->PIO_SODR |= (1 << gpio.pin);
		}
		break;

	case GPIO_Type_DriveSetup:
		// Enabling open-drain if specified
		switch ( config )
		{
		case GPIO_Config_Opendrain:
			pio->PIO_MDER |= (1 << gpio.pin);
			break;

		// Do nothing otherwise
		default:
			//pio->PIO_MDDR = (1 << gpio.pin);
			break;
		}

		// Set as output pin
		pio->PIO_OER |= (1 << gpio.pin);
		pio->PIO_PER |= (1 << gpio.pin);
		break;

	case GPIO_Type_Read:
		return (pio->PIO_PDSR >> gpio.pin) & 1;

	case GPIO_Type_ReadSetup:
		// Pull resistor config
		switch ( config )
		{
		case GPIO_Config_Pullup:
			pio->PIO_PPDDR |= (1 << gpio.pin);
			pio->PIO_PUER |= (1 << gpio.pin);
			break;

		case GPIO_Config_Pulldown:
			pio->PIO_PUDR |= (1 << gpio.pin);
			pio->PIO_PPDER |= (1 << gpio.pin);
			break;

		// Do nothing otherwise
		default:
			break;
		}

		// Set as input pin
		pio->PIO_IFER |= (1 << gpio.pin); // glitch filter
		pio->PIO_ODR |= (1 << gpio.pin);
		pio->PIO_PER |= (1 << gpio.pin);
		break;
	}
#endif

	return 0;
}

void PIO_Setup(GPIO_ConfigPin config)
{
#if defined(_sam_)
	Pio *pio;
	switch (config.port)
	{
	case GPIO_Port_A:
		pio = PIOA;
		break;
	case GPIO_Port_B:
		switch (config.pin)
		{
		case GPIO_Pin_4: // TDI->PB4
			MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;
			break;
		case GPIO_Pin_5: // TDO/TRACESWO->PB5
			MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO5;
			break;
		case GPIO_Pin_6: // TMS/SWDIO->PB6
			MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO6;
			break;
		case GPIO_Pin_7: // TCK/SWCLK->PB7
			MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO7;
			break;
		case GPIO_Pin_10: // DDM->PB10
			MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO10;
			break;
		case GPIO_Pin_11: // DDP->PB11
			MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO11;
			break;
		case GPIO_Pin_12: // ERASE->PB12
			MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO12;
			break;

		default:
			break;
		}
		pio = PIOB;
		break;
#if defined(_sam4s_c_)
	case GPIO_Port_C:
		pio = PIOC;
		break;
#endif
	default:
		return;
	}

	pio_set_peripheral(pio, config.peripheral, (1 << config.pin));
#endif
}

