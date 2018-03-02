/* Copyright (C) 2018 by Jacob Alexander
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

#include "kll.h"



// ----- Variables -----

// ----- Functions -----

// Based on the type and index, lookup the full index
var_uint_t KLL_TriggerIndex_loopkup( TriggerType type, uint8_t index )
{
	var_uint_t fullindex = index;

	// Modify index
	switch ( type )
	{
	case TriggerType_Switch2:
	case TriggerType_Analog2:
	case TriggerType_Layer2:
		fullindex += 255;
		break;

	case TriggerType_Switch3:
	case TriggerType_Analog3:
	case TriggerType_Layer3:
		fullindex += 511;
		break;

	case TriggerType_Switch4:
	case TriggerType_Analog4:
	case TriggerType_Layer4:
		fullindex += 767;
		break;

	// Otherwise, just pass the index as is
	case TriggerType_Switch1:
	case TriggerType_Analog1:
	case TriggerType_LED1:
	case TriggerType_Layer1:
	default:
		break;
	}

	return fullindex;
}


// Given a state and state type determine a CapabilityState
CapabilityState KLL_CapabilityState( ScheduleState state, TriggerType type )
{
	// Lookup trigger type
	switch ( type )
	{
	// Switches
	case TriggerType_Switch1:
	case TriggerType_Switch2:
	case TriggerType_Switch3:
	case TriggerType_Switch4:
		switch ( state )
		{
		// Press
		case ScheduleType_P:
			return CapabilityState_Initial;

		// Hold
		case ScheduleType_H:
			return CapabilityState_Any;

		// Release
		case ScheduleType_R:
			return CapabilityState_Last;

		// Off / Invalid
		default:
			break;
		}
		break;

	// LEDs
	case TriggerType_LED1:
		switch ( state )
		{
		// Activate
		case ScheduleType_A:
			return CapabilityState_Initial;

		// On
		case ScheduleType_On:
			return CapabilityState_Any;

		// Deactivate
		case ScheduleType_D:
			return CapabilityState_Last;

		// Off / Invalid
		default:
			break;
		}
		break;

	// Analog
	case TriggerType_Analog1:
	case TriggerType_Analog2:
	case TriggerType_Analog3:
	case TriggerType_Analog4:
		// XXX There is no "Press" state per say
		//     0x02 is reserved for Press
		//     0x01 is reserved for Release
		//     However these are reserved for working with state scheduling events
		//     By default, all analog values have to be handled manually (unless state scheduled)
		switch ( state )
		{
		// Release (reserved value)
		case 0x01:
			return CapabilityState_Last;

		// Press (reserved value)
		case 0x02:
			return CapabilityState_Initial;

		// Off
		case 0x00:
			break;

		// Other values
		default:
			return CapabilityState_Any;
		}
		break;

	// Layer
	case TriggerType_Layer1:
	case TriggerType_Layer2:
	case TriggerType_Layer3:
	case TriggerType_Layer4:
		switch ( state )
		{
		// Activate
		case ScheduleType_A:
			return CapabilityState_Initial;

		// On
		case ScheduleType_On:
			return CapabilityState_Any;

		// Deactivate
		case ScheduleType_D:
			return CapabilityState_Last;

		// Off / Invalid
		default:
			break;
		}
		break;

	// Animation
	case TriggerType_Animation1:
	case TriggerType_Animation2:
	case TriggerType_Animation3:
	case TriggerType_Animation4:
		switch ( state )
		{
		// Done
		case ScheduleType_Done:
			return CapabilityState_Any;

		// Repeat
		case ScheduleType_Repeat:
			return CapabilityState_Any;

		// Invalid
		default:
			break;
		}
		break;

	// Debug
	case TriggerType_Debug:
		// State must also be 0xFF to trigger debug
		if ( state == 0xFF )
		{
			return CapabilityState_Debug;
		}
		break;

	// Invalid
	default:
		break;
	}

	// Ignore otherwise
	return CapabilityState_None;
}

