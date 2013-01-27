
#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>

// Convenience Macros, for delay compatibility with AVR-GCC
#define _delay_ms(val) delay( val )
#define _delay_us(val) delayMicroseconds( val )


// the systick interrupt is supposed to increment this at 1 kHz rate
extern volatile uint32_t systick_millis_count;

static inline uint32_t millis(void) __attribute__((always_inline, unused));
static inline uint32_t millis(void)
{
	return systick_millis_count; // single aligned 32 bit is atomic;
}


static inline void delayMicroseconds(uint32_t) __attribute__((always_inline, unused));
static inline void delayMicroseconds(uint32_t usec)
{
#if F_CPU == 96000000
	uint32_t n = usec << 5;
#elif F_CPU == 48000000
	uint32_t n = usec << 4;
#elif F_CPU == 24000000
	uint32_t n = usec << 3;
#endif
	asm volatile(
		"L_%=_delayMicroseconds:"		"\n\t"
		"subs   %0, #1"				"\n\t"
		"bne    L_%=_delayMicroseconds"		"\n"
		: "+r" (n) :
	);
}


void yield(void) __attribute__ ((weak));

uint32_t micros(void);

void delay(uint32_t ms);

#endif

