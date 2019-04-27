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

// KLL Include
#include <kll.h>

// Local Includes
#include "solenoid.h"



// ----- Defines -----

// ----- Structs -----

// ----- Variables -----

// ----- Functions -----

void Solenoid_Pulse_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only activate on press event
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Solenoid_Pulse_capability(us_duration)");
		return;
	default:
		return;
	}

	// Get argument
	uint16_t us_duration = *(uint16_t*)(&args[0]);

#if Solenoid_PWM_Mode_define == 0
	// GPIO Mode
	// Pulse on
	const GPIO_Pin gpio_pin = Solenoid_Pin_GPIO_define;
	GPIO_Ctrl(gpio_pin, GPIO_Type_DriveHigh, GPIO_Config_None);

	// Wait for duration
	delay_us(us_duration);

	// Pulse off
	GPIO_Ctrl(gpio_pin, GPIO_Type_DriveLow, GPIO_Config_None);
#elif Solenoid_PWM_Mode_define == 1
	// TODO
#warning "Solenoid PWM Mode not yet implemented!"
#endif
}

void Solenoid_setup()
{
#if Solenoid_PWM_Mode_define == 0
	// GPIO Mode
	// Setup GPIO pin and default to low
	const GPIO_Pin gpio_pin = Solenoid_Pin_GPIO_define;
	GPIO_Ctrl(gpio_pin, GPIO_Type_DriveSetup, GPIO_Config_None);
	GPIO_Ctrl(gpio_pin, GPIO_Type_DriveLow, GPIO_Config_None);
#elif Solenoid_PWM_Mode_define == 1
	// PWM Mode
	// Configure PWM pin
	const GPIO_ConfigPin pwm_pin = Solenoid_Pin_PWM_define;
	PIO_Setup(pwm_pin);

	// Enable peripheral clock for PWM
	pmc_enable_periph_clk(ID_PWM);

	// Disable channel until properly configured
	pwm_channel_disable(PWM, Solenoid_PWM_Channel_define);

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
#endif

}

