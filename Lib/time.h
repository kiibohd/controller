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

// ----- Includes -----

// System Includes
#include <stdint.h>



// ----- Structs -----

typedef struct Time {
	uint32_t ms;
	uint32_t ticks;
} Time;



// ----- Variables -----

extern const uint32_t Time_maxTicks;
extern const uint32_t Time_maxTicks_ms;
extern const char* Time_ticksPer_ns_str;



// ----- Functions -----

// Get current time
Time Time_now();
Time Time_init();


// Conversions
uint32_t Time_days( Time time );
uint32_t Time_hours( Time time );
uint32_t Time_minutes( Time time );
uint32_t Time_seconds( Time time );
uint32_t Time_ms( Time time );
uint32_t Time_us( Time time );
uint32_t Time_ns( Time time );
uint32_t Time_ticks( Time time );

Time Time_from_days( uint32_t days );
Time Time_from_hours( uint32_t hours );
Time Time_from_minutes( uint32_t minutes );
Time Time_from_seconds( uint32_t seconds );
Time Time_from_ms( uint32_t ms );


// Durations
Time Time_duration( Time since );
Time Time_duration_rollover( Time since );

uint32_t Time_duration_ticks( Time since );
uint32_t Time_duration_days( Time since );
uint32_t Time_duration_hours( Time since );
uint32_t Time_duration_minutes( Time since );
uint32_t Time_duration_seconds( Time since );
uint32_t Time_duration_ms( Time since );
uint32_t Time_duration_us( Time since );
uint32_t Time_duration_ns( Time since );

