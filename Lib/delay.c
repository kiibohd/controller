
#include "delay.h"
#include "mk20dx.h"

// the systick interrupt is supposed to increment this at 1 kHz rate
volatile uint32_t systick_millis_count = 0;

void yield(void) {};

uint32_t micros(void)
{
	uint32_t count, current, istatus;

	__disable_irq();
	current = SYST_CVR;
	count = systick_millis_count;
	istatus = SCB_ICSR; // bit 26 indicates if systick exception pending
	__enable_irq();
	if ((istatus & SCB_ICSR_PENDSTSET) && current > ((F_CPU / 1000) - 50)) count++;
	current = ((F_CPU / 1000) - 1) - current;
	return count * 1000 + current / (F_CPU / 1000000);
}

void delay(uint32_t ms)
{
	uint32_t start = micros();

	while (1) {
		if ((micros() - start) >= 1000) {
			ms--;
			if (ms == 0) break;
			start += 1000;
		}
		yield();
	}
}

