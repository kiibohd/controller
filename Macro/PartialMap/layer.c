/* Copyright (C) 2014-2018 by Jacob Alexander
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
#include <Lib/MacroLib.h>

// Project Includes
#include <cli.h>
#include <led.h>
#include <print.h>

// Connect Includes
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// Generated Includes
#include <kll_defs.h>

// Local Includes
#include "kll.h"
#include "trigger.h"



// ----- Function Declarations -----

// ----- Variables -----

// Layer debug flag - If set, displays any changes to layers and the full layer stack on change
uint8_t layerDebugMode;

// Incoming Trigger Event Buffer
var_uint_t macroTriggerEventLayerCache[ MaxScanCode_KLL + 1 ];

// Layer Index Stack
//  * When modifying layer state and the state is non-0x0, the stack must be adjusted
index_uint_t macroLayerIndexStack[ LayerNum ] = { 0 };
index_uint_t macroLayerIndexStackSize = 0;

// Current rotated layer
uint16_t Layer_rotationLayer;

// Layer State Information
extern LayerStateType LayerState[];

// Layer Index List
extern const Layer LayerIndex[];



// ----- Capabilities -----

// Sets the given layer with the specified layerState
void Layer_layerStateSet( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint16_t layer, uint8_t layerState )
{
	// Ignore if layer does not exist or trying to manipulate layer 0/Default layer
	if ( layer >= LayerNum || layer == 0 )
		return;

	// Check current layer state
	uint8_t oldState = LayerState[ layer ];

	// Is layer in the LayerIndexStack?
	uint8_t inLayerIndexStack = 0;
	uint16_t stackItem = 0;
	while ( stackItem < macroLayerIndexStackSize )
	{
		// Flag if layer is already in the LayerIndexStack
		if ( macroLayerIndexStack[ stackItem ] == layer )
		{
			inLayerIndexStack = 1;
			break;
		}

		// Increment to next item
		stackItem++;
	}

	// Toggle Layer State Byte
	if ( LayerState[ layer ] & layerState )
	{
		// Unset
		LayerState[ layer ] &= ~layerState;
	}
	else
	{
		// Set
		LayerState[ layer ] |= layerState;
	}

	// If the layer was not in the LayerIndexStack add it
	if ( !inLayerIndexStack )
	{
		macroLayerIndexStack[ macroLayerIndexStackSize++ ] = layer;
	}

	// If the layer is in the LayerIndexStack and the state is 0x00, remove
	uint8_t newState = LayerState[ layer ];
	if ( newState == LayerStateType_Off && inLayerIndexStack )
	{
		// Remove the layer from the LayerIndexStack
		// Using the already positioned stackItem variable from the loop above
		while ( stackItem < macroLayerIndexStackSize )
		{
			macroLayerIndexStack[ stackItem ] = macroLayerIndexStack[ stackItem + 1 ];
			stackItem++;
		}

		// Reduce LayerIndexStack size
		macroLayerIndexStackSize--;
	}

	// Determine what signal to send about layer
	if ( oldState && newState )
	{
		// On -> On (Layer still active)
		Macro_layerState( layer, ScheduleType_On );
	}
	else if ( !oldState && newState )
	{
		// Off -> On (Activate)
		Macro_layerState( layer, ScheduleType_A );
	}
	else if ( oldState && !newState )
	{
		// On -> Off (Deactivate)
		Macro_layerState( layer, ScheduleType_D );
	}

	// Layer Debug Mode
	if ( layerDebugMode )
	{
		print("\033[1;36mL\033[0m ");

		// Iterate over each of the layers displaying the state as a hex value
		for ( index_uint_t index = 0; index < LayerNum; index++ )
		{
			printHex_op( LayerState[ index ], 0 );
		}

		// Always show the default layer (it's always 0)
		print(" 0");

		// Iterate over the layer stack starting from the bottom of the stack
		for ( index_uint_t index = macroLayerIndexStackSize; index > 0; index-- )
		{
			print(":");
			printHex_op( macroLayerIndexStack[ index - 1 ], 0 );
		}

		print( NL );
	}
}

// Modifies the specified Layer control byte
// Argument #1: Layer Index -> uint16_t
// Argument #2: Layer State -> uint8_t
void Layer_layerState_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
	case CapabilityState_Last:
		// Only use capability on press or release
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Layer_layerState(layerIndex,layerState)");
		return;
	default:
		return;
	}

	// Get layer index from arguments
	// Cast pointer to uint8_t to uint16_t then access that memory location
	uint16_t layer = *(uint16_t*)(&args[0]);

	// Get layer toggle byte
	uint8_t layerState = args[ sizeof(uint16_t) ];

	Layer_layerStateSet( trigger, state, stateType, layer, layerState );
}


// Latches given layer
// Argument #1: Layer Index -> uint16_t
void Layer_layerLatch_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Last:
		// Only use capability on release
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Layer_layerLatch(layerIndex)");
		return;
	default:
		return;
	}

	// Get layer index from arguments
	// Cast pointer to uint8_t to uint16_t then access that memory location
	uint16_t layer = *(uint16_t*)(&args[0]);

	Layer_layerStateSet( trigger, state, stateType, layer, LayerStateType_Latch );
}


// Locks given layer
// Argument #1: Layer Index -> uint16_t
void Layer_layerLock_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Layer_layerLock(layerIndex)");
		return;
	default:
		return;
	}

	// Get layer index from arguments
	// Cast pointer to uint8_t to uint16_t then access that memory location
	uint16_t layer = *(uint16_t*)(&args[0]);

	Layer_layerStateSet( trigger, state, stateType, layer, LayerStateType_Lock );
}


// Shifts given layer
// Argument #1: Layer Index -> uint16_t
void Layer_layerShift_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Get layer index from arguments
	// Cast pointer to uint8_t to uint16_t then access that memory location
	uint16_t layer = *(uint16_t*)(&args[0]);

	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Press
		// Only set the layer if it is disabled
		if ( LayerState[ layer ] != LayerStateType_Off )
		{
			return;
		}
		break;
	case CapabilityState_Last:
		// Release
		// Only unset the layer if it is enabled
		if ( LayerState[ layer ] == LayerStateType_Off )
		{
			return;
		}
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Layer_layerShift(layerIndex)");
	default:
		return;
	}

	Layer_layerStateSet( trigger, state, stateType, layer, LayerStateType_Shift );
}


// Rotate layer to next/previous
// Uses state variable to keep track of the current layer position
// Layers are still evaluated using the layer stack
void Layer_layerRotate_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Layer_layerRotate(previous)");
		return;
	default:
		return;
	}

	// Unset previous rotation layer if not 0
	if ( Layer_rotationLayer != 0 )
	{
		Layer_layerStateSet( trigger, state, stateType, Layer_rotationLayer, LayerStateType_Lock );
	}

	// Get direction of rotation, 0, next, non-zero previous
	uint8_t direction = *args;

	// Next
	if ( !direction )
	{
		Layer_rotationLayer++;

		// Invalid layer
		if ( Layer_rotationLayer >= LayerNum )
			Layer_rotationLayer = 0;
	}
	// Previous
	else
	{
		Layer_rotationLayer--;

		// Layer wrap
		if ( Layer_rotationLayer >= LayerNum )
			Layer_rotationLayer = LayerNum - 1;
	}

	// Toggle the computed layer rotation
	Layer_layerStateSet( trigger, state, stateType, Layer_rotationLayer, LayerStateType_Lock );
}



// ----- Functions -----

// Clears the current layer state
void Layer_clearLayers()
{
	// Clear layer stack
	macroLayerIndexStackSize = 0;

	// Clear layer states
	memset( &LayerState, 0, sizeof(LayerStateType) * LayerNum );
}


// Setup layers
void Layer_setup()
{
	// Cleanup layers
	Layer_clearLayers();

	// Set the current rotated layer to 0
	Layer_rotationLayer = 0;

	// Layer debug mode
	layerDebugMode = 0;
}


// Looks up the trigger list for the given scan code (from the active layer)
// NOTE: Calling function must handle the NULL pointer case
nat_ptr_t *Layer_layerLookup( TriggerEvent *event, uint8_t latch_expire )
{
	uint8_t index = event->index;

	// Cached Lookup (for handling layer latches)
	uint8_t cache_lookup = 0;
	CapabilityState cstate = KLL_CapabilityState( event->state, event->type );
	switch ( cstate )
	{
	case CapabilityState_Any:
	case CapabilityState_Last:
		// Ignore press, off and debug
		cache_lookup = 1;

	case CapabilityState_Initial:
	default:
		break;
	}

	// Do a cached lookup if necessary
	if ( cache_lookup )
	{
		// Cached layer
		var_uint_t cachedLayer = macroTriggerEventLayerCache[ index ];

		// Lookup map, then layer
		nat_ptr_t **map = (nat_ptr_t**)LayerIndex[ cachedLayer ].triggerMap;
		const Layer *layer = &LayerIndex[ cachedLayer ];

		// Cache trigger list before attempting to expire latch
		nat_ptr_t *trigger_list = map[ index - layer->first ];

		// Check if latch has been pressed for this layer
		uint8_t latch = LayerState[ cachedLayer ] & LayerStateType_Latch;
		if ( latch && latch_expire )
		{
			Layer_layerStateSet( 0, 0, 0, cachedLayer, LayerStateType_Latch );
#if defined(ConnectEnabled_define) && defined(LCDEnabled_define)
			// Evaluate the layerStack capability if available (LCD + Interconnect)
			extern void LCD_layerStack_capability(
				TriggerMacro *trigger,
				uint8_t state,
				uint8_t stateType,
				uint8_t *args
			);
			LCD_layerStack_capability( 0, 0, 0, 0 );
#endif
		}

		return trigger_list;
	}

	// If no trigger macro is defined at the given layer, fallthrough to the next layer
	for ( uint16_t layerIndex = macroLayerIndexStackSize; layerIndex != 0xFFFF; layerIndex-- )
	{
		// Lookup Layer
		const Layer *layer = &LayerIndex[ macroLayerIndexStack[ layerIndex ] ];

		// Lookup each of the states
		uint8_t shift = LayerState[ macroLayerIndexStack[ layerIndex ] ] & LayerStateType_Shift;
		uint8_t latch = LayerState[ macroLayerIndexStack[ layerIndex ] ] & LayerStateType_Latch;
		uint8_t lock = LayerState[ macroLayerIndexStack[ layerIndex ] ] & LayerStateType_Lock;

		// Check if latch has been pressed for this layer
		// XXX Regardless of whether a key is found, the latch is removed on first lookup
		if ( latch && latch_expire )
		{
			Layer_layerStateSet( 0, 0, 0, macroLayerIndexStack[ layerIndex ], LayerStateType_Latch );
		}

		// Only use layer, if state is valid
		// XOR each of the state bits
		// If only two are enabled, do not use this state
		if ( (shift) ^ (latch>>1) ^ (lock>>2) )
		{
			// Lookup layer
			nat_ptr_t **map = (nat_ptr_t**)layer->triggerMap;

			// Determine if layer has key defined
			// Make sure index is between layer first and last scancodes
			if ( map != 0
				&& index <= layer->last
				&& index >= layer->first
				&& *map[ index - layer->first ] != 0 )
			{
				// Set the layer cache
				macroTriggerEventLayerCache[ index ] = macroLayerIndexStack[ layerIndex ];

				return map[ index - layer->first ];
			}
		}
	}

	// Do lookup on default layer
	nat_ptr_t **map = (nat_ptr_t**)LayerIndex[0].triggerMap;

	// Lookup default layer
	const Layer *layer = &LayerIndex[0];

	// Make sure index is between layer first and last scancodes
	if ( map != 0
		&& index <= layer->last
		&& index >= layer->first
		&& *map[ index - layer->first ] != 0 )
	{
		// Set the layer cache to default map
		macroTriggerEventLayerCache[ index ] = 0;

		return map[ index - layer->first ];
	}

	// Otherwise no defined Trigger Macro
	erro_msg("Index has no defined Trigger Macro: ");
	printHex( index );
	print( NL );
	return 0;
}

