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

// ----- Includes -----

// Compiler Includes
#include <stdint.h>

// Host Includes
#if defined(_host_)
#include <string.h>
#endif

// KLL Include
#include <kll.h>

// Local Includes
#include "latency.h"



// ----- Variables -----

static LatencyMeasurement latency_measurements[LatencyMeasurementCount_define];
static uint8_t latency_resources;



// ----- Functions -----

// Initialize latency module
// Call before adding resources
void Latency_init()
{
	// Zero out all measurements
	memset( &latency_measurements, 0, sizeof(LatencyMeasurement) * LatencyMeasurementCount_define );

	// Set used resources to 0
	latency_resources = 0;
}

// Number of latency resources used
uint8_t Latency_resources()
{
	return latency_resources;
}

// Add latency tracking resource
//
// return: resource index
uint8_t Latency_add_resource( const char* name, LatencyOption option )
{
	// Add identifier name
	uint8_t index = latency_resources++;

	// Make sure there are resources left to allocate
	if ( index >= LatencyMeasurementCount_define )
	{
		erro_print("No more latency resources available...");
		return 0;
	}

	// Set name
	latency_measurements[index].name = name;

	// Set option
	latency_measurements[index].option = option;

	// Max out min latency
	latency_measurements[index].min_latency = 0xFFFFFFFF;

	return index;
}

// Query latency
// type:     type of query
// resource: index of resource
//
// return: latency query result
uint32_t Latency_query( LatencyQuery type, uint8_t resource )
{
	switch ( type )
	{
	case LatencyQuery_Min:
		return latency_measurements[resource].min_latency;

	case LatencyQuery_Max:
		return latency_measurements[resource].max_latency;

	case LatencyQuery_Average:
		return latency_measurements[resource].average_latency;

	case LatencyQuery_Last:
		return latency_measurements[resource].last_latency;

	case LatencyQuery_Count:
		return latency_measurements[resource].count;

	default:
		return 0;
	}
}

// Resource Lookup
// resource: index of resource
//
// return: Name of resource
const char* Latency_query_name( uint8_t resource )
{
	return latency_measurements[resource].name;
}

// Resource start time
//
// resource: index of resource
void Latency_start_time( uint8_t resource )
{
	latency_measurements[resource].start_time = Time_now();
}

// Measure latency, and store
//
// resource: index of resource
void Latency_end_time( uint8_t resource )
{
	uint32_t measured;
	switch ( latency_measurements[resource].option )
	{
	case LatencyOption_ms:
		measured = Time_duration_ms( latency_measurements[resource].start_time );
		break;

	case LatencyOption_us:
		measured = Time_duration_us( latency_measurements[resource].start_time );
		break;

	case LatencyOption_ns:
		measured = Time_duration_ns( latency_measurements[resource].start_time );
		break;

	default:
		measured = Time_duration_ticks( latency_measurements[resource].start_time );
		break;
	}

	// Check if min or max latencies need to change
	if ( measured < latency_measurements[resource].min_latency )
	{
		latency_measurements[resource].min_latency = measured;
	}
	if ( measured > latency_measurements[resource].max_latency )
	{
		latency_measurements[resource].max_latency = measured;
	}

	// Calculate average, places emphasis on recent values
	uint32_t old_avg = latency_measurements[resource].average_latency;
	if ( old_avg == 0 )
	{
		// Use measured value if this is the first calculation
		old_avg = measured;
	}
	latency_measurements[resource].average_latency = ( old_avg / 2 ) + ( measured / 2 ) + ( old_avg & measured & 1 );

	// Set last average
	latency_measurements[resource].last_latency = measured;

	// Latency check count
	latency_measurements[resource].count++;
}

