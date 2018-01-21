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


// ----- Includes -----

// Compiler Includes
#include <stddef.h>

// System Includes
#include <Lib/mcu_compat.h>
#include <Lib/Interrupts.h>

// Debug Includes
#if !defined(_bootloader_)
#include <print.h>
#else
#include <debug.h>
#endif

// Local Includes
#include "delay.h"
#include "time.h"



// ----- Variables -----

#if defined(_host_)
extern volatile uint32_t systick_millis_count;
extern volatile uint32_t ns_since_systick_count;
#endif

#if defined(F_CPU)
// Ticks per ms
const uint32_t Time_maxTicks = F_CPU / 1000;
const uint32_t Time_maxTicks_ms = 0xFFFFFFFF / ( F_CPU / 1000 );
#elif defined(_host_)
// TODO (HaaTa) This is variable, should be set in KLL?
const uint32_t Time_maxTicks = 1;
const uint32_t Time_maxTicks_ms = 1;
#endif

#if F_CPU == 72000000
const char* Time_ticksPer_ns_str = "13.889 ns";
#elif F_CPU == 48000000
const char* Time_ticksPer_ns_str = "20.833 ns";
#elif defined(_host_)
const char* Time_ticksPer_ns_str = "<USERDEFINED>";
#else
const char* Time_ticksPer_ns_str = "<UNKNOWN>";
#endif



// ----- Function Declarations -----

// ----- Functions -----

// Get current time
Time Time_now()
{
#if defined(_kinetis_)
	Time time = {
		.ms    = systick_millis_count,
		.ticks = ARM_DWT_CYCCNT,
	};
#elif defined(_sam_)
	//SAM TODO
	Time time = Time_init();
#elif defined(_host_)
	Time time = {
		.ms    = systick_millis_count,
		.ticks = ns_since_systick_count,
	};
#else
	// No time facilities...
	Time time = Time_init();
#endif

	return time;
}

// Get zero'd Time
#if !defined(_host_)
inline
#endif
Time Time_init()
{
	Time time = {
		.ms    = 0,
		.ticks = 0,
	};
	return time;
}


// -- Time Conversion Functions --

#if !defined(_host_)
inline
#endif
uint32_t Time_days( Time time )
{
	uint32_t days = (time.ms) / (1000 * 60 * 60 * 24);
	return days;
}

#if !defined(_host_)
inline
#endif
uint32_t Time_hours( Time time )
{
	uint32_t hours = time.ms / (1000 * 60 * 60);
	return hours;
}

#if !defined(_host_)
inline
#endif
uint32_t Time_minutes( Time time )
{
	uint32_t minutes = time.ms / (1000 * 60);
	return minutes;
}

#if !defined(_host_)
inline
#endif
uint32_t Time_seconds( Time time )
{
	uint32_t seconds = time.ms / 1000;
	return seconds;
}

#if !defined(_host_)
inline
#endif
uint32_t Time_ms( Time time )
{
	return time.ms;
}

#if !defined(_host_)
inline
#endif
uint32_t Time_us( Time time )
{
	// Return max uint32_t if ms count is too high
	if ( time.ms >= (0xFFFFFFFF / 1000) )
	{
		return 0xFFFFFFFF;
	}

	uint32_t us = ( time.ms + time.ticks * Time_maxTicks ) * 1000;
	return us;
}

#if !defined(_host_)
inline
#endif
uint32_t Time_ns( Time time )
{
	// Return max uint32_t if ms count is too high
	if ( time.ms >= (0xFFFFFFFF / 1000000) )
	{
		return 0xFFFFFFFF;
	}

	uint32_t ns = ( time.ms + time.ticks * Time_maxTicks ) * 1000000;
	return ns;
}

#if !defined(_host_)
inline
#endif
uint32_t Time_ticks( Time time )
{
	// Return max uint32_t if ms count is too high
	if ( time.ms >= Time_maxTicks_ms )
	{
		return 0xFFFFFFFF;
	}

	uint32_t ticks = time.ms * Time_maxTicks;
	ticks += time.ticks;
	return ticks;
}

#if !defined(_host_)
inline
#endif
Time Time_from_days( uint32_t days )
{
	Time time = {
		.ms    = days * 1000 * 3600 * 24,
		.ticks = 0,
	};
	return time;
}

#if !defined(_host_)
inline
#endif
Time Time_from_hours( uint32_t hours )
{
	Time time = {
		.ms    = hours * 1000 * 3600,
		.ticks = 0,
	};
	return time;
}

#if !defined(_host_)
inline
#endif
Time Time_from_minutes( uint32_t minutes )
{
	Time time = {
		.ms    = minutes * 60 * 1000,
		.ticks = 0,
	};
	return time;
}

#if !defined(_host_)
inline
#endif
Time Time_from_seconds( uint32_t seconds )
{
	Time time = {
		.ms    = seconds * 1000,
		.ticks = 0,
	};
	return time;
}

#if !defined(_host_)
inline
#endif
Time Time_from_ms( uint32_t ms )
{
	Time time = {
		.ms    = ms,
		.ticks = 0,
	};
	return time;
}


// -- Time Duration Functions --
//
// Time since the given time
// If the given time value is lower, then assume a single register wrap
// This is around 49 days (plenty of time for most tasks)

Time Time_duration_rollover( Time since )
{
	Time now = Time_now();
	Time duration;

	// Check if ms have done a rollover
	if ( now.ms < since.ms )
	{
		duration.ms = now.ms + ( 0xFFFFFFFF - since.ms );
	}
	// Standard case
	else
	{
		duration.ms = now.ms - since.ms;
	}

	// Ticks do not rollover, they are computed from the last increment of ms
	// Depending on the set clock speed, the maximum number of ticks per ms may vary
	duration.ticks = now.ticks + ( Time_maxTicks - since.ticks );

	return duration;
}

Time Time_duration( Time since )
{
	return Time_duration_rollover( since );
}

uint32_t Time_duration_days( Time since )
{
	Time duration = Time_duration_rollover( since );
	return Time_days( duration );
}

uint32_t Time_duration_hours( Time since )
{
	Time duration = Time_duration_rollover( since );
	return Time_hours( duration );
}

uint32_t Time_duration_minutes( Time since )
{
	Time duration = Time_duration_rollover( since );
	return Time_minutes( duration );
}

uint32_t Time_duration_seconds( Time since )
{
	Time duration = Time_duration_rollover( since );
	return Time_seconds( duration );
}

uint32_t Time_duration_ms( Time since )
{
	Time duration = Time_duration_rollover( since );
	return Time_ms( duration );
}

uint32_t Time_duration_us( Time since )
{
	Time duration = Time_duration_rollover( since );
	return Time_us( duration );
}

uint32_t Time_duration_ns( Time since )
{
	Time duration = Time_duration_rollover( since );
	return Time_ns( duration );
}

// Number of ticks since
uint32_t Time_duration_ticks( Time since )
{
	Time duration = Time_duration_rollover( since );
	return Time_ticks( duration );
}

