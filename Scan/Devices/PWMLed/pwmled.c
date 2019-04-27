/*
 * Copyright (C) 2019 Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files ( the "Software" ), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// ----- Includes ----

// Compiler Includes
#include <Lib/ScanLib.h>

// Project Includes
#include <Lib/gpio.h>
#include <print.h>
#include <kll_defs.h>

// Local Includes
#include "pwmled.h"



// ----- Defines -----

// Using Lib/gpio.h enums, but redefining gpio() macro
#undef gpio
#define gpio( port, pin ) GPIO_Port_##port, GPIO_Pin_##pin



// ----- Structs -----

// ----- Variables -----

// ----- Functions -----

void pwmled_setup()
{
	//connect peripheral B to pin A23
	// TODO how to put this is KLL
	pio_configure_pin(PWM_DAC, PIO_TYPE_PIO_PERIPH_B);

	// Enable peripheral clock for PWM
	pmc_enable_periph_clk(ID_PWM);

	// Disable channels until properly configured
	pwm_channel_disable(PWM, PWM_CHANNEL_0);
	pwm_channel_disable(PWM, PWM_CHANNEL_1);
	pwm_channel_disable(PWM, PWM_CHANNEL_2);
	pwm_channel_disable(PWM, PWM_CHANNEL_3);

	// PWM clock configuration
	// TODO
	pwm_clock_t PWMLed_clock_config =
	{
		.ul_clka = 1000000,
		.ul_clkb = 0,
		.ul_mck = sysclk_get_cpu_hz()
	};
	pwm_init(PWM, &PWMLed_clock_config);

	// Setting up synchronous PWM
	// Except for duty cycle, all options are written to Channel 0
	// TODO

	//see the article for details
	pwm_channel_instance.channel = PWM_CHANNEL_0;
	pwm_channel_instance.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_channel_instance.polarity = PWM_HIGH;
	pwm_channel_instance.alignment = PWM_ALIGN_LEFT;
	pwm_channel_instance.ul_period = 20;
	pwm_channel_instance.ul_duty = 5;

	//apply the channel configuration
	pwm_channel_init(PWM, &pwm_channel_instance);

	//configuration is complete, so enable the channel
	pwm_channel_enable(PWM, PWM_CHANNEL_0);
}

