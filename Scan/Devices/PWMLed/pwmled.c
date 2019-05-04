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

// ASF Includes
#include <common/services/clock/sysclk.h>
#include <sam/drivers/pmc/pmc.h>
#include <sam/drivers/pwm/pwm.h>

// Project Includes
#include <Lib/gpio.h>
#include <print.h>
#include <kll_defs.h>

// Local Includes
#include "pwmled.h"



// ----- Defines -----

// ----- Structs -----

// ----- Variables -----

static pwm_channel_t pwm_channel[PWMLed_Channels_define];



// ----- Functions -----

void PWMLED_setup()
{
	// Enable peripheral clock for PWM
	pmc_enable_periph_clk(ID_PWM);

	// Disable channels until properly configured
	pwm_channel_disable(PWM, PWM_CHANNEL_0);
	pwm_channel_disable(PWM, PWM_CHANNEL_1);
	pwm_channel_disable(PWM, PWM_CHANNEL_2);
	pwm_channel_disable(PWM, PWM_CHANNEL_3);

	// TODO
	// Figure out PWM startup procedure
	// 1) Disable lock
	// 2) Select clock generator
	// 3) Select clock for each channel
	// 4) Configure waveform alignment per channel
	// 5) If CALG = 1, select counter event selection (CES)
	// 6) Configure output waveform per channel
	// 7) ???

	const GPIO_ConfigPin pwm_pin1 = PWMLed_Ch1_pin_define;
	PIO_Setup(pwm_pin1);
#if PWMLed_Channels_define >= 2
	const GPIO_ConfigPin pwm_pin2 = PWMLed_Ch2_pin_define;
	PIO_Setup(pwm_pin2);
#endif
#if PWMLed_Channels_define >= 3
	const GPIO_ConfigPin pwm_pin3 = PWMLed_Ch3_pin_define;
	PIO_Setup(pwm_pin3);
#endif
#if PWMLed_Channels_define >= 4
	const GPIO_ConfigPin pwm_pin4 = PWMLed_Ch4_pin_define;
	//PIO_Setup(pwm_pin4);
#endif

#if PWMLed_Ctrls_define >= 1
	const GPIO_ConfigPin pwm_ctrl = PWMLed_Ctrl_pin_define;
	PIO_Setup(pwm_ctrl);
#endif
	// PWM clock configuration
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
	// TODO
	pwm_channel[0].channel = PWM_CHANNEL_0;
	pwm_channel[1].channel = PWM_CHANNEL_1;
	pwm_channel[2].channel = PWM_CHANNEL_2;
	pwm_channel[3].channel = PWM_CHANNEL_3;
	for (uint8_t c = 0; c < 4; c++)
	{
	pwm_channel[c].ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_channel[c].polarity = PWM_HIGH;
	pwm_channel[c].alignment = PWM_ALIGN_LEFT;
	pwm_channel[c].ul_period = 20;
	pwm_channel[c].ul_duty = 5;
	pwm_channel[c].b_sync_ch = false,
	pwm_channel[c].b_deadtime_generator = false,
	pwm_channel[c].us_deadtime_pwmh = 0,
	pwm_channel[c].us_deadtime_pwml = 0,
	pwm_channel[c].output_selection.b_override_pwmh = false,
	pwm_channel[c].output_selection.b_override_pwml = false,

	// Apply the channel configuration
	pwm_channel_init(PWM, &pwm_channel[c]);
	}

	// Configuration is complete, enable the channel
	pwm_channel_enable(PWM, PWM_CHANNEL_0);
	pwm_channel_enable(PWM, PWM_CHANNEL_1);
	pwm_channel_enable(PWM, PWM_CHANNEL_2);
	pwm_channel_enable(PWM, PWM_CHANNEL_3);
}

void PWMLED_scan()
{
}

