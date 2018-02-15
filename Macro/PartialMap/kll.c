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

