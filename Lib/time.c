/* Copyright (C) 2017-2018 by Jacob Alexander
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

#if F_CPU == 120000000
const char* Time_ticksPer_ns_str = "8.3333 ns";
const uint32_t Time_ticksPer_ns_x1000 = 8333;
#elif F_CPU == 72000000
const char* Time_ticksPer_ns_str = "13.889 ns";
const uint32_t Time_ticksPer_ns_x1000 = 13889;
#elif F_CPU == 48000000
const char* Time_ticksPer_ns_str = "20.833 ns";
const uint32_t Time_ticksPer_ns_x1000 = 20833;
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
	Time time = {
		.ms    = systick_millis_count,
		.ticks = DWT->CYCCNT,
	};
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

// Add time amount to a given variable
// Returns 1 if there was an ms rollover
uint8_t Time_add( Time *current, Time add )
{
	Time data = Time_init();
	data.ms = 0;
	data.ticks = current->ticks + add.ticks;

	// First determine if the ticks need to rollover
	if ( data.ticks >= Time_maxTicks )
	{
		data.ticks -= Time_maxTicks;
		data.ms++;
	}

	// It's ok if ms rolls over (it will eventually anyways)
	uint8_t rollover = 0;
	data.ms += current->ms + add.ms;
	if ( data.ms + current->ms + add.ms <= current->ms )
	{
		rollover = 1;
	}

	// Set the time
	*current = data;

	return rollover;
}

// Compares two Time variables
// -1 if compare is less than base
//  0 if compare is the same
//  1 if compare is more than base
int8_t Time_compare( Time base, Time compare )
{
	// First compare ms
	if ( base.ms > compare.ms )
	{
		return -1;
	}
	else if ( base.ms < compare.ms )
	{
		return 1;
	}

	// Next compare ticks (ms is the same)
	if ( base.ticks > compare.ticks )
	{
		return -1;
	}
	else if ( base.ticks < compare.ticks )
	{
		return 1;
	}

	// Otherwise base and compare are identical
	return 0;
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

	uint32_t us = time.ms * 1000 + time.ticks / (Time_maxTicks / 1000);
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

	// XXX (HaaTa): Since most MCUs don't run at 1 GHz, and we don't have floating point numbers
	//              converting to ns is more interesting.
	//              For example, 120 MHz /w 400 ticks is 8.333..ns * 400 ticks = 3333.2 ns
	//              To do this with integers only, we multiply the ns constant to us, then divide by 1000 at the end.
	//               400 * 8333 / 1000 = 3333 ns
	//              Not exact, but pretty close.
	uint32_t ns = time.ms * 1000000 + (time.ticks * Time_ticksPer_ns_x1000 / 1000);
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

Time Time_duration_rollover( Time now, Time since )
{
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

	// If ticks have exceeded 1 ms, increment ms
	if ( duration.ticks >= Time_maxTicks )
	{
		duration.ms++;
		duration.ticks -= Time_maxTicks;
	}

	return duration;
}

Time Time_duration_rollover_now( Time since )
{
	Time now = Time_now();
	return Time_duration_rollover( now, since );
}

Time Time_duration( Time since )
{
	return Time_duration_rollover_now( since );
}

uint32_t Time_duration_days( Time since )
{
	Time duration = Time_duration_rollover_now( since );
	return Time_days( duration );
}

uint32_t Time_duration_hours( Time since )
{
	Time duration = Time_duration_rollover_now( since );
	return Time_hours( duration );
}

uint32_t Time_duration_minutes( Time since )
{
	Time duration = Time_duration_rollover_now( since );
	return Time_minutes( duration );
}

uint32_t Time_duration_seconds( Time since )
{
	Time duration = Time_duration_rollover_now( since );
	return Time_seconds( duration );
}

uint32_t Time_duration_ms( Time since )
{
	Time duration = Time_duration_rollover_now( since );
	return Time_ms( duration );
}

uint32_t Time_duration_us( Time since )
{
	Time duration = Time_duration_rollover_now( since );
	return Time_us( duration );
}

uint32_t Time_duration_ns( Time since )
{
	Time duration = Time_duration_rollover_now( since );
	return Time_ns( duration );
}

// Number of ticks since
uint32_t Time_duration_ticks( Time since )
{
	Time duration = Time_duration_rollover_now( since );
	return Time_ticks( duration );
}


// -- Tick Functions --
//
// Functions that operate around the TickStore struct
//
// To start a tick sequence call Time_tick_start.
// Then call Time_tick_update to refresh the status/state of the TickStore.
// Time_tick_update will return how many ticks have incremented since the last update.
//
// If no tick has incremented, or the store has exceeded the max_tick defined,
// Time_tick_update will return 0

void Time_tick_start( TickStore *store, Time duration, uint32_t max_ticks )
{
	// Set the duration and max_tick
	store->tick_duration = duration;
	store->max_ticks = max_ticks;

	// Reset last_tick and ticks_since_start
	Time_tick_reset( store );
}

void Time_tick_reset( TickStore *store )
{
	// Reset last_tick and ticks_since_start
	store->last_tick = Time_now();
	store->ticks_since_start = 0;

	// Mark as a fresh TickStore
	store->fresh_store = 1;
}

uint32_t Time_tick_update( TickStore *store )
{
	// TODO (HaaTa) Handle rollover case (only happens once every 49 days while the keyboard is on)
	// Check if we've already gotten to the max tick threshold
	if ( store->ticks_since_start > store->max_ticks )
	{
		return 0;
	}

	// Total ticks since last update
	uint32_t ticks = 0;

	// Query current time
	Time now = Time_now();

	// Previous time
	Time prev = store->last_tick;

	// Otherwise just increment until we've gone too far
	while ( Time_compare( now, store->last_tick ) <= 0 )
	{
		prev = store->last_tick;
		Time_add( &store->last_tick, store->tick_duration );
		ticks++;
	}

	// Reverse back to previous tick
	ticks -= 1;
	store->last_tick = prev;

	// Add ticks to store
	store->ticks_since_start += ticks;

	return ticks;
}

