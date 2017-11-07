/* Copyright (C) 2017 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// ----- Includes -----

// Compiler Includes
#include <Lib/MainLib.h>

// System Includes
#include <Lib/time.h>



// ----- Defines -----

// ----- Enumerations -----

typedef enum LatencyQuery {
	LatencyQuery_Min = 0,
	LatencyQuery_Max = 1,
	LatencyQuery_Average = 2,
	LatencyQuery_Last = 3,
	LatencyQuery_Count = 4,
} LatencyQuery;

typedef enum LatencyOption {
	LatencyOption_Ticks,
	LatencyOption_ns,
	LatencyOption_us,
	LatencyOption_ms,
} LatencyOption;



// ----- Structs -----

typedef struct LatencyMeasurement {
	const char* name;
	Time start_time;
	LatencyOption option;
	uint32_t min_latency;
	uint32_t max_latency;
	uint32_t average_latency;
	uint32_t last_latency;
	uint32_t count;
} LatencyMeasurement;



// ----- Variables -----

// ----- Functions -----

void Latency_init();
void Latency_start_time( uint8_t resource );
void Latency_end_time( uint8_t resource );

const char* Latency_query_name( uint8_t resource );

uint8_t Latency_add_resource( const char* name, LatencyOption option );
uint8_t Latency_resources();

uint32_t  Latency_query( LatencyQuery type, uint8_t resource );

