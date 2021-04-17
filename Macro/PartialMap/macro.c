/* Copyright (C) 2014-2020 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/MacroLib.h>

// Project Includes
#include <cli.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <scan_loop.h>

// Keymaps
#include <usb_hid.h> // Generated using kll (and hid-io/layouts) at compile time, in build directory
#include <generatedKeymap.h> // Generated using kll at compile time, in build directory

// Connect Includes
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// PixelMap Includes
#if defined(Pixel_MapEnabled_define)
#include <pixel.h>
#endif

// Local Includes
#include "layer.h"
#include "trigger.h"
#include "result.h"
#include "macro.h"



// ----- Function Declarations -----

void cliFunc_capDebug  ( char* args );
void cliFunc_capList   ( char* args );
void cliFunc_capSelect ( char* args );
void cliFunc_keyHold   ( char* args );
void cliFunc_keyPress  ( char* args );
void cliFunc_keyRelease( char* args );
void cliFunc_layerDebug( char* args );
void cliFunc_layerList ( char* args );
void cliFunc_layerState( char* args );
void cliFunc_macroDebug( char* args );
void cliFunc_macroList ( char* args );
void cliFunc_macroProc ( char* args );
void cliFunc_macroShow ( char* args );
void cliFunc_macroStep ( char* args );
void cliFunc_posList   ( char* args );
void cliFunc_voteDebug ( char* args );

void Macro_showScheduleType( ScheduleState state );
void Macro_showTriggerType( TriggerType type );



// ----- Variables -----

// Macro Module command dictionary
CLIDict_Entry( capDebug,    "Prints capability debug message before calling, with trigger conditions." );
CLIDict_Entry( capList,     "Prints an indexed list of all non USB keycode capabilities." );
CLIDict_Entry( capSelect,   "Triggers the specified capabilities. First two args are state and stateType." NL "\t\t\033[35mK11\033[0m Keyboard Capability 0x0B" );
CLIDict_Entry( keyHold,     "Send key-hold events to the macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( keyPress,    "Send key-press events to the macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( keyRelease,  "Send key-release event to macro module. Duplicates have undefined behaviour." NL "\t\t\033[35mS10\033[0m Scancode 0x0A" );
CLIDict_Entry( layerDebug,  "Layer debug mode. Shows layer stack and any changes." );
CLIDict_Entry( layerList,   "List available layers." );
CLIDict_Entry( layerState,  "Modify specified indexed layer state <layer> <state byte>." NL "\t\t\033[35mL2\033[0m Indexed Layer 0x02" NL "\t\t0 Off, 1 Shift, 2 Latch, 4 Lock States" );
CLIDict_Entry( macroDebug,  "Disables/Enables sending USB keycodes to the Output Module and prints U/K codes." );
CLIDict_Entry( macroList,   "List the defined trigger and result macros." );
CLIDict_Entry( macroProc,   "Pause/Resume macro processing." );
CLIDict_Entry( macroShow,   "Show the macro corresponding to the given index." NL "\t\t\033[35mT16\033[0m Indexed Trigger Macro 0x10, \033[35mR12\033[0m Indexed Result Macro 0x0C" );
CLIDict_Entry( macroStep,   "Do N macro processing steps. Defaults to 1." );
CLIDict_Entry( posList,     "List physical key positions by ScanCode." );
CLIDict_Entry( voteDebug,   "Show results of TriggerEvent voting." );

CLIDict_Def( macroCLIDict, "Macro Module Commands" ) = {
	CLIDict_Item( capDebug ),
	CLIDict_Item( capList ),
	CLIDict_Item( capSelect ),
	CLIDict_Item( keyHold ),
	CLIDict_Item( keyPress ),
	CLIDict_Item( keyRelease ),
	CLIDict_Item( layerDebug ),
	CLIDict_Item( layerList ),
	CLIDict_Item( layerState ),
	CLIDict_Item( macroDebug ),
	CLIDict_Item( macroList ),
	CLIDict_Item( macroProc ),
	CLIDict_Item( macroShow ),
	CLIDict_Item( macroStep ),
	CLIDict_Item( posList ),
	CLIDict_Item( voteDebug ),
	{ 0, 0, 0 } // Null entry for dictionary end
};


// Capability debug flag - If set, shows the name of the capability when before it is called
extern uint8_t capDebugMode;

// Layer debug flag - If set, displays any changes to layers and the full layer stack on change
extern uint8_t layerDebugMode;

// Macro debug flag - If set, clears the USB Buffers after signalling processing completion
// 1 - Disable USB output, show debug
// 2 - Enabled USB output, show debug
// 3 - Disable USB output
uint8_t macroDebugMode;

// Vote debug flag - If set show the result of each
uint8_t voteDebugMode;

// Trigger pending debug flag - If set show pending triggers before evaluating
uint8_t triggerPendingDebugMode;

// Macro pause flag - If set, the macro module pauses processing, unless unset, or the step counter is non-zero
uint8_t macroPauseMode;

// Macro step counter - If non-zero, the step counter counts down every time the macro module does one processing loop
uint16_t macroStepCounter;

// Macro rotation store - Each store is indexed, and is initialized to 0
static uint8_t Macro_rotation_store[256]; // TODO (HaaTa): Use KLL to determine max usage (or dynamically size)


// Latency resource
static uint8_t macroLatencyResource;


// Incoming Trigger Event Buffer
TriggerEvent macroTriggerEventBuffer[ MaxScanCode_KLL + 1 ];
var_uint_t macroTriggerEventBufferSize;

extern ResultsPending macroResultMacroPendingList;
extern index_uint_t macroTriggerMacroPendingList[];
extern index_uint_t macroTriggerMacroPendingListSize;

// Interconnect ScanCode Cache
#if defined(ConnectEnabled_define) || defined(PressReleaseCache_define)
// TODO This can be shrunk by the size of the max node 0 ScanCode
TriggerEvent macroInterconnectCache[ MaxScanCode_KLL + 1 ];
uint8_t macroInterconnectCacheSize = 0;
#endif

// Dynamically Sized Type Widths
#if defined(_host_)
const uint8_t StateWordSize = StateWordSize_define;
const uint8_t IndexWordSize = IndexWordSize_define;
const uint8_t ScheduleStateSize = ScheduleStateSize_define;
#endif



// ----- Capabilities -----

// Rotation capability
// Maintains state and fires of a rotation event trigger
void Macro_rotate_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Macro_rotate()");
		return;
	default:
		return;
	}

	// Get storage index from arguments
	uint8_t index = args[0];
	// Get increment from arguments
	int8_t increment = (int8_t)args[1];

	// Trigger rotation
	Macro_rotationState( index, increment );
}

// No-op capability (None)
void Macro_none_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Debug:
		// Display capability name
		print("Macro_none()");
		return;
	default:
		break;
	}

}

// Test Thread-safe Capability
// Capability used to test a thread-safe result
void Macro_testThreadSafe_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Debug:
		// Display capability name
		print("Macro_testThreadSafe()");
		return;
	default:
		break;
	}

	// Show trigger information
	print("ThreadSafe: ");
	Macro_showTriggerType( (TriggerType)stateType );
	print(" ");
	Macro_showScheduleType( (ScheduleState)state );
	print(" - ");
	printHex32( (intptr_t)trigger );
	print(NL);
}


// Test Thread-unsafe Capability
// Capability used to test a thread-unsafe result
void Macro_testThreadUnsafe_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Debug:
		// Display capability name
		print("Macro_testThreadUnsafe()");
		return;
	default:
		break;
	}

	// Show trigger information
	print("ThreadUnsafe: ");
	Macro_showTriggerType( (TriggerType)stateType );
	print(" ");
	Macro_showScheduleType( (ScheduleState)state );
	print(" - ");
	printHex32( (intptr_t)trigger );
	print(NL);
}



// ----- Debug Functions -----

// Shows a ScheduleType
void Macro_showScheduleType( ScheduleState state )
{
	// State types
	switch ( state & 0x0F )
	{
	case ScheduleType_P:
	//case ScheduleType_A:
		print("\033[1;33mP\033[0m");
		break;

	case ScheduleType_H:
	//case ScheduleType_On:
		print("\033[1;32mH\033[0m");
		break;

	case ScheduleType_R:
	//case ScheduleType_D:
		print("\033[1;35mR\033[0m");
		break;

	case ScheduleType_O:
	//case ScheduleType_Off:
		print("\033[1mO\033[0m");
		break;

	case ScheduleType_UP:
		print("UP");
		break;

	case ScheduleType_UR:
		print("UR");
		break;

	case ScheduleType_Done:
		print("Done");
		break;

	case ScheduleType_Repeat:
		print("Repeat");
		break;

	case ScheduleType_Debug:
		print("Debug");
		break;

	default:
		print("\033[1;31mINVALID\033[0m");
		printInt8( state & 0x0F );
		break;
	}

	// Check for Shift/Latch/Lock type
	switch ( state & 0xF0 )
	{
	case ScheduleType_Shift:
		print("Sh");
		break;

	case ScheduleType_Latch:
		print("La");
		break;

	case ScheduleType_Lock:
		print("Lo");
		break;

	default:
		break;
	}
}

// Shows a Schedule
void Macro_showSchedule( Schedule *param, uint8_t analog )
{
	// Analog
	if ( analog )
	{
		printInt8( param->analog );
	}
	// Everything else
	else
	{
		Macro_showScheduleType( param->state );
	}

	// Time
	print(":");
	printInt32( param->time.ms );
	print(".");
	printInt32( param->time.ticks );
}

// Shows a TriggerType
void Macro_showTriggerType( TriggerType type )
{
	// Type
	switch ( type )
	{
	// Switches
	case TriggerType_Switch1:
	case TriggerType_Switch2:
	case TriggerType_Switch3:
	case TriggerType_Switch4:
		print("Sw");
		break;

	// LEDs
	case TriggerType_LED1:
		print("LED");
		break;

	// Analog
	case TriggerType_Analog1:
	case TriggerType_Analog2:
	case TriggerType_Analog3:
	case TriggerType_Analog4:
		print("An");
		break;

	// Layer
	case TriggerType_Layer1:
	case TriggerType_Layer2:
	case TriggerType_Layer3:
	case TriggerType_Layer4:
		print("Layer");
		break;

	// Animation
	case TriggerType_Animation1:
	case TriggerType_Animation2:
	case TriggerType_Animation3:
	case TriggerType_Animation4:
		print("Animation");
		break;

	// Sleep
	case TriggerType_Sleep1:
		print("Sleep");
		break;

	// Resume
	case TriggerType_Resume1:
		print("Resume");
		break;

	// Inactive
	case TriggerType_Inactive1:
		print("Inactive");
		break;

	// Active
	case TriggerType_Active1:
		print("Active");
		break;

	// Rotation
	case TriggerType_Rotation1:
		print("Rotation");
		break;

	// Dial
	case TriggerType_Dial1:
		print("Dial");
		break;

	// Invalid
	default:
		print("INVALID");
		break;

	// Debug
	case TriggerType_Debug:
		print("Debug");
		break;
	}
}

// Shows a TriggerEvent
void Macro_showTriggerEvent( TriggerEvent *event )
{
	// Decode type
	Macro_showTriggerType( event->type );
	print(" ");

	// Show state
	switch ( event->type )
	{
	case TriggerType_Analog1:
	case TriggerType_Analog2:
	case TriggerType_Analog3:
	case TriggerType_Analog4:
	case TriggerType_Rotation1:
	case TriggerType_Dial1:
		printInt8( event->state );
		break;

	default:
		Macro_showScheduleType( event->state );
		break;
	}
	print(" ");

	// Show index number
	printInt8( event->type );
	print(":");
	printInt8( event->index );
}

// Shows a TriggerGuide
void Macro_showTriggerGuide( TriggerGuide *guide )
{
}



// ----- Functions -----

#if defined(_host_)
// Callback for host-side kll
extern int Output_callback( char* command, char* args );
const uint32_t LayerNum_host = LayerNum;
#endif


// Add an interconnect ScanCode
// These are handled differently (less information is sent, hold/off states must be assumed)
// Returns 1 if added, 0 if the ScanCode is already in the buffer
// Returns 2 if there's an error
#if defined(ConnectEnabled_define) || defined(PressReleaseCache_define)
uint8_t Macro_pressReleaseAdd( void *trigger_ptr )
{
	TriggerEvent *trigger = (TriggerEvent*)trigger_ptr;

	// Error checking
	uint8_t error = 0;
	switch ( trigger->type )
	{
	case TriggerType_Switch1:
	case TriggerType_Switch2:
	case TriggerType_Switch3:
	case TriggerType_Switch4:
	case TriggerType_LED1:
		switch ( trigger->state )
		{
		case ScheduleType_P:
		case ScheduleType_H:
		case ScheduleType_R:
		case ScheduleType_O:
			break;
		default:
			erro_print("Invalid key state - ");
			error = 1;
			break;
		}
		break;

	case TriggerType_Analog1:
	case TriggerType_Analog2:
	case TriggerType_Analog3:
	case TriggerType_Analog4:
	case TriggerType_Rotation1:
	case TriggerType_Dial1:
		break;

	case TriggerType_Animation1:
	case TriggerType_Animation2:
	case TriggerType_Animation3:
	case TriggerType_Animation4:
	case TriggerType_Sleep1:
	case TriggerType_Resume1:
	case TriggerType_Active1:
	case TriggerType_Inactive1:
		break;

	// Invalid TriggerGuide type for Interconnect
	default:
		erro_print("Invalid type - ");
		error = 1;
		break;
	}

	// Check if ScanCode is out of range
	if ( trigger->index > MaxScanCode_KLL )
	{
		warn_print("ScanCode is out of range/not defined - ");
		error = 1;
	}

	// Display TriggerGuide
	if ( error )
	{
		printHex( trigger->type );
		print(" ");
		printHex( trigger->state );
		print(" ");
		printHex( trigger->index );
		print( NL );
		return 2;
	}

	// Add trigger to the Interconnect Cache
	// During each processing loop, a scancode may be re-added depending on it's state
	for ( var_uint_t c = 0; c < macroInterconnectCacheSize; c++ )
	{
		// Check if the same ScanCode
		if ( macroInterconnectCache[ c ].index == trigger->index )
		{
			// Update the state
			macroInterconnectCache[ c ].state = trigger->state;
			return 0;
		}
	}

	// If not in the list, add it
	macroInterconnectCache[ macroInterconnectCacheSize++ ] = *trigger;

	return 1;
}
#endif


// Update the scancode key state
// States:
//   * 0x00 - Off
//   * 0x01 - Pressed
//   * 0x02 - Held
//   * 0x03 - Released
//   * 0x04 - Unpressed (this is currently ignored)
void Macro_keyState( uint16_t scanCode, uint8_t state )
{
#if defined(ConnectEnabled_define)
	// Only compile in if a Connect node module is available
	if ( !Connect_master )
	{
		// ScanCodes are only added if there was a state change (on/off)
		switch ( state )
		{
		case ScheduleType_O: // Off
		case ScheduleType_H: // Held
			return;
		}
	}
#endif

	// Lookup done based on size of scanCode
	uint8_t index = 0;
	TriggerType type = TriggerType_Switch1;

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case ScheduleType_P: // Pressed
	case ScheduleType_H: // Held
	case ScheduleType_R: // Released
		// Check if ScanCode is out of range
		if ( scanCode > MaxScanCode_KLL )
		{
			warn_print("ScanCode is out of range/not defined: ");
			printInt16( scanCode );
			print( NL );
			return;
		}

		// Determine which type
		if ( scanCode < 256 )
		{
			index = scanCode;
		}
		else if ( scanCode < 512 )
		{
			index = scanCode - 256;
			type = TriggerType_Switch2;
		}
		else if ( scanCode < 768 )
		{
			index = scanCode - 512;
			type = TriggerType_Switch3;
		}
		else if ( scanCode < 1024 )
		{
			index = scanCode - 768;
			type = TriggerType_Switch4;
		}

		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


// Update the scancode analog state
// States:
//   * 0x00      - Off
//   * 0x01      - Released
//   * 0x02-0xFF - Analog value (low to high)
void Macro_analogState( uint16_t scanCode, uint8_t state )
{
	// Only add to macro trigger list if non-off
	if ( state == 0x00 )
		return;

	// Lookup done based on size of scanCode
	uint8_t index = 0;
	TriggerType type = TriggerType_Analog1;

	// Determine which type
	if ( scanCode < 256 )
	{
		index = scanCode;
	}
	else if ( scanCode < 512 )
	{
		index = scanCode - 256;
		type = TriggerType_Analog2;
	}
	else if ( scanCode < 768 )
	{
		index = scanCode - 512;
		type = TriggerType_Analog3;
	}
	else if ( scanCode < 1024 )
	{
		index = scanCode - 768;
		type = TriggerType_Analog4;
	}

	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
	macroTriggerEventBufferSize++;
}


// Update led state
// States:
//   * 0x00 - Off
//   * 0x01 - Activate
//   * 0x02 - On
//   * 0x03 - Deactivate
void Macro_ledState( uint16_t ledCode, uint8_t state )
{
	// Lookup done based on size of scanCode
	uint8_t index = ledCode;
	TriggerType type = TriggerType_LED1;

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case ScheduleType_A:  // Activate
	case ScheduleType_On: // On
	case ScheduleType_D:  // Deactivate
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


// Update animation state
// States:
//   * 0x00 - Off
//   * 0x06 - Done
//   * 0x07 - Repeat
void Macro_animationState( uint16_t animationIndex, uint8_t state )
{
	// Lookup done based on size of layerIndex
	uint8_t index = 0;
	TriggerType type = TriggerType_Animation1;

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case ScheduleType_Done:   // Activate
	case ScheduleType_Repeat: // On
		// Check if animation index is out of range
		if ( animationIndex > AnimationNum_KLL )
		{
			warn_print("AnimationIndex is out of range/not defined: ");
			printInt16( animationIndex );
			print( NL );
			return;
		}

		// Determine which type
		if ( animationIndex < 256 )
		{
			index = animationIndex;
		}
		else if ( animationIndex < 512 )
		{
			index = animationIndex - 256;
			type = TriggerType_Animation2;
		}
		else if ( animationIndex < 768 )
		{
			index = animationIndex - 512;
			type = TriggerType_Animation3;
		}
		else if ( animationIndex < 1024 )
		{
			index = animationIndex - 768;
			type = TriggerType_Animation4;
		}

		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


// Update layer state
// States:
//   * 0x00 - Off
//   * 0x01 - Activate
//   * 0x02 - On
//   * 0x03 - Deactivate
void Macro_layerState( uint16_t layerIndex, uint8_t state )
{
	// Lookup done based on size of layerIndex
	uint8_t index = 0;
	TriggerType type = TriggerType_Layer1;

	// Only add to macro trigger list if one of three states
	// Mask around Shift/Latch/Lock state
	switch ( state & ScheduleType_D )
	{
	case ScheduleType_A:  // Activate
	case ScheduleType_On: // On
	case ScheduleType_D:  // Deactivate
		// Check if layer is out of range
		if ( layerIndex > LayerNum_KLL )
		{
			warn_print("LayerIndex is out of range/not defined: ");
			printInt16( layerIndex );
			print( NL );
			return;
		}

		// Determine which type
		if ( layerIndex < 256 )
		{
			index = layerIndex;
		}
		else if ( layerIndex < 512 )
		{
			index = layerIndex - 256;
			type = TriggerType_Layer2;
		}
		else if ( layerIndex < 768 )
		{
			index = layerIndex - 512;
			type = TriggerType_Layer3;
		}
		else if ( layerIndex < 1024 )
		{
			index = layerIndex - 768;
			type = TriggerType_Layer4;
		}

		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


// Update Time State trigger
// Only valid with Sleep/Resume and Inactive/Active triggers
// States:
//   * 0x00 - Off
//   * 0x01 - Activate
//   * 0x02 - On
//   * 0x03 - Deactivate
void Macro_timeState( uint8_t type, uint16_t cur_time, uint8_t state )
{
	// Make sure this is a valid trigger type
	switch ( type )
	{
	case TriggerType_Sleep1:
	case TriggerType_Resume1:
	case TriggerType_Inactive1:
	case TriggerType_Active1:
		break;

	// Ignore if not the correct type
	default:
		warn_print("Invalid time state trigger update: ");
		printHex( type );
		print(NL);
		return;
	}

	// cur_time is controlled by the caller
	// When this function called the trigger is active
	if ( cur_time > 0xFF )
	{
		warn_print("Only 255 time instances are accepted for a time state trigger: ");
		printInt16( cur_time );
		print(NL);
		return;
	}
	uint8_t index = cur_time;

	// Only add to macro trigger list if one of three states
	switch ( state )
	{
	case ScheduleType_A:  // Activate
	case ScheduleType_On: // On
	case ScheduleType_D:  // Deactivate
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


// Rotation state update
// Queues up a rotation trigger event
// States:
//   * 0x00 - Off
//   * 0x01 - Activate
//   * 0x02 - On
//   * 0x03 - Deactivate
void Macro_rotationState( uint8_t index, int8_t increment )
{
	// For now, always Rotation1
	uint8_t type = TriggerType_Rotation1;

	// If index is invalid, ignore
	if ( index > RotationNum )
	{
		return;
	}

	// State is used as the increment position
	int16_t position = Macro_rotation_store[index] + increment;

	// If first starting, the first rotation is 0
	if ( Macro_rotation_store[index] == 255 )
	{
		position = 0;
	}

	// Wrap-around
	// May have to wrap-around multiple times
	while ( position > Rotation_MaxParameter[index] )
	{
		position -= Rotation_MaxParameter[index] + 1;
	}

	// Reverse Wrap-around
	if ( position < 0 )
	{
		// May have to wrap-around multiple times
		while ( position * -1 > Rotation_MaxParameter[index] )
		{
			position += Rotation_MaxParameter[index] - 1;
		}

		// Do wrap-around
		position += Rotation_MaxParameter[index] - 1;

	}
	Macro_rotation_store[index] = position;

	// Queue event
	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = position;
	macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
	macroTriggerEventBufferSize++;
}


// Dial state update
// Queues up a dial trigger event
// States:
//  * 0x00 - Off
//  * 0x01 - Increase
//  * 0x02 - Decrease
void Macro_dialState( uint8_t index, uint8_t state )
{
	uint8_t type = TriggerType_Dial1;

	// Queue event
	switch ( state )
	{
	case ScheduleType_Inc: // Increment
	case ScheduleType_Dec: // Decrement
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].index = index;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].state = state;
		macroTriggerEventBuffer[ macroTriggerEventBufferSize ].type  = type;
		macroTriggerEventBufferSize++;
		break;
	}
}


// [In]Activity detected, do TickStore update and signal generation
// Returns 1 if a signal is sent
uint8_t Macro_tick_update( TickStore *store, uint8_t type )
{
	uint32_t ticks = Time_tick_update( store );
	uint8_t signal_sent = 0;

	// Check for a fresh store (separate signal)
	if ( store->fresh_store )
	{
		Time_tick_reset( store );

		// Unset fresh store
		store->fresh_store = 0;

		// Send initial activity signal
		Macro_timeState( type, 0, ScheduleType_A );
		signal_sent = 1;
		store->ticks_since_start++;
	}

	// No need to update if there were no ticks
	if ( ticks == 0 )
	{
		goto done;
	}

	// Check to see if we need to signal
	for (
		uint16_t signal = store->ticks_since_start - ticks;
		signal < store->ticks_since_start;
		signal++
	)
	{
		// Send queued up signals
		Macro_timeState( type, signal, ScheduleStateSize_define );
	}

done:
	return signal_sent;
}


// Macro Processing Loop, called often
// Generally takes care of thread-unsafe calls to capabilities that must be synchronized
void Macro_poll()
{
	// Process delayed capabilities
	Result_process_delayed();
}


// Macro Processing Loop, called from the periodic execution thread
// Called once per USB buffer send
void Macro_periodic()
{
	// Latency measurement
	Latency_start_time( macroLatencyResource );

#if defined(ConnectEnabled_define)
	// Only compile in if a Connect node module is available
	// If this is a interconnect slave node, send all scancodes to master node
	if ( !Connect_master )
	{
		if ( macroTriggerEventBufferSize > 0 )
		{
			Connect_send_ScanCode( Connect_id, macroTriggerEventBuffer, macroTriggerEventBufferSize );
			macroTriggerEventBufferSize = 0;
		}
		return;
	}
#endif

#if defined(ConnectEnabled_define) || defined(PressReleaseCache_define)
#if defined(ConnectEnabled_define)
	// Check if there are any ScanCodes in the interconnect cache to process
	if ( Connect_master && macroInterconnectCacheSize > 0 )
#endif
	{
		// Iterate over all the cache ScanCodes
		uint8_t currentInterconnectCacheSize = macroInterconnectCacheSize;
		macroInterconnectCacheSize = 0;
		for ( uint8_t c = 0; c < currentInterconnectCacheSize; c++ )
		{
			// Add to the trigger list
			macroTriggerEventBuffer[ macroTriggerEventBufferSize++ ] = macroInterconnectCache[ c ];

			// TODO Handle other TriggerGuide types (e.g. analog)
			switch ( macroInterconnectCache[ c ].type )
			{
			// Normal (Press/Hold/Release)
			case TriggerType_Switch1:
			case TriggerType_Switch2:
			case TriggerType_Switch3:
			case TriggerType_Switch4:
			case TriggerType_LED1:
				// Decide what to do based on the current state
				switch ( macroInterconnectCache[ c ].state )
				{
				// Re-add to interconnect cache in hold state
				case ScheduleType_P: // Press
				//case ScheduleType_H: // Hold // XXX Why does this not work? -HaaTa
					macroInterconnectCache[ c ].state = ScheduleType_H;
					macroInterconnectCache[ macroInterconnectCacheSize++ ] = macroInterconnectCache[ c ];
					break;

				case ScheduleType_R: // Release
					break;

				// Otherwise, do not re-add
				default:
					break;
				}
				break;

			case TriggerType_Rotation1:
			case TriggerType_Dial1:
			case TriggerType_Animation1:
			case TriggerType_Animation2:
			case TriggerType_Animation3:
			case TriggerType_Animation4:
			case TriggerType_Sleep1:
			case TriggerType_Resume1:
			case TriggerType_Active1:
			case TriggerType_Inactive1:
				// Do not re-add
				break;

			// Not implemented
			default:
				erro_print("Interconnect Trigger Event Type - Not Implemented ");
				printHex( macroInterconnectCache[ c ].type );
				print( NL );
				break;
			}
		}
	}
#endif
	// Macro incoming state debug
	switch ( macroDebugMode )
	{
	case 1:
	case 2:
		// Iterate over incoming triggers
		for ( uint16_t trigger = 0; trigger < macroTriggerEventBufferSize; trigger++ )
		{
			// Show debug info about incoming trigger
			Macro_showTriggerEvent( &macroTriggerEventBuffer[trigger] );
			print( NL );
		}

	case 3:
	default:
		break;
	}

	// Check macroTriggerEventBufferSize to make sure no overflow
	if ( macroTriggerEventBufferSize >= MaxScanCode_KLL )
	{
		// No scancodes defined
		if ( MaxScanCode_KLL == 0 )
		{
#if NoneScanModule_define == 0
			warn_printNL("No scancodes defined! Check your BaseMap!");
#endif
		}
		// Bug!
		else
		{
			erro_print("Macro Trigger Event Overflow! Serious Bug! ");
			printInt16( macroTriggerEventBufferSize );
			print( NL );
			macroTriggerEventBufferSize = 0;
		}
	}

	// If the pause flag is set, only process if the step counter is non-zero
	if ( macroPauseMode )
	{
		if ( macroStepCounter == 0 )
			return;

		// Proceed, decrementing the step counter
		macroStepCounter--;
		dbug_printNL("Macro Step");
	}

	// Process Trigger Macros
	Trigger_process();


	// Store events processed
	var_uint_t macroTriggerEventBufferSize_processed = macroTriggerEventBufferSize;

	// Reset TriggerList buffer
	macroTriggerEventBufferSize = 0;


	// Process result macros
	Result_process();

	// Signal buffer that we've used it
	Scan_finishedWithMacro( macroTriggerEventBufferSize_processed );

#if defined(_host_)
	// Signal host to read layer state
	Output_callback( "layerState", "" );
#endif

	// Latency measurement
	Latency_end_time( macroLatencyResource );

	// If Macro debug mode is set, clear the USB Buffer
#if defined(Output_USBEnabled_define)
	if ( macroDebugMode == 1 || macroDebugMode == 3 )
	{
		USBKeys_primary.changed = 0;
	}
#endif
}


inline void Macro_setup()
{
	// Register Macro CLI dictionary
	//CLI_registerDictionary( macroCLIDict, macroCLIDictName );

	// Disable Macro debug mode
	macroDebugMode = 0;

	// Disable Macro pause flag
	macroPauseMode = 0;

	// Set Macro step counter to zero
	macroStepCounter = 0;

	// Disable Macro Vote debug mode
	voteDebugMode = 0;

	// Disable Trigger Pending debug mode
	triggerPendingDebugMode = 0;

	// Make sure macro trigger event buffer is empty
	macroTriggerEventBufferSize = 0;

	// Initial rotation store to 255s
	memset( Macro_rotation_store, 255, sizeof(Macro_rotate_capability) );

	// Setup Layers
	Layer_setup();

	// Setup Triggers
	Trigger_setup();

	// Setup Results
	Result_setup();

	// Allocate resource for latency measurement
	macroLatencyResource = Latency_add_resource("PartialMap", LatencyOption_Ticks);
}


// ----- CLI Command Functions -----

void cliFunc_capDebug( char* args )
{
	// Toggle layer debug mode
	capDebugMode = capDebugMode ? 0 : 1;

	print( NL );
	info_print("Capability Debug Mode: ");
	printInt8( capDebugMode );
}

void cliFunc_capList( char* args )
{
	print( NL );
	info_print("Capabilities List ");
	printHex( CapabilitiesNum );

	// Iterate through all of the capabilities and display them
	for ( var_uint_t cap = 0; cap < CapabilitiesNum; cap++ )
	{
		print( NL "\t" );
		printHex( cap );
		print(" - ");

		// Display/Lookup Capability Name (utilize debug mode of capability)
		void (*capability)(TriggerMacro*, uint8_t, uint8_t, uint8_t*) = \
			(void(*)(TriggerMacro*, uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ cap ].func);
		capability( 0, 0xFF, 0xFF, 0 );
	}
}

void cliFunc_capSelect( char* args )
{
	// Parse code from argument
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Total number of args to scan (must do a lookup if a keyboard capability is selected)
	var_uint_t totalArgs = 2; // Always at least two args
	var_uint_t cap = 0;

	// Arguments used for keyboard capability function
	var_uint_t argSetCount = 0;
	uint8_t *argSet = (uint8_t*)args;

	// Process all args
	for ( var_uint_t c = 0; argSetCount < totalArgs; c++ )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		// Extra arguments are ignored
		if ( *arg1Ptr == '\0' )
			break;

		// For the first argument, choose the capability
		if ( c == 0 ) switch ( arg1Ptr[0] )
		{
		// Keyboard Capability
		case 'K':
			// Determine capability index
			cap = numToInt( &arg1Ptr[1] );

			// Lookup the number of args
			totalArgs += CapabilitiesList[ cap ].argCount;
			continue;
		}

		// Because allocating memory isn't doable, and the argument count is arbitrary
		// The argument pointer is repurposed as the argument list (much smaller anyways)
		argSet[ argSetCount++ ] = (uint8_t)numToInt( arg1Ptr );

		// Once all the arguments are prepared, call the keyboard capability function
		if ( argSetCount == totalArgs )
		{
			// Indicate that the capability was called
			print( NL );
			info_print("K");
			printInt8( cap );
			print(" - ");
			printHex( argSet[0] );
			print(" - ");
			printHex( argSet[1] );
			print(" - ");
			printHex( argSet[2] );
			print( "..." NL );

			// Make sure this isn't the reload capability
			// If it is, and the remote reflash define is not set, ignore
			if ( flashModeEnabled_define == 0 ) for ( uint32_t cap = 0; cap < CapabilitiesNum; cap++ )
			{
				if ( CapabilitiesList[ cap ].func == (const void*)Output_flashMode_capability )
				{
					print( NL );
					warn_printNL("flashModeEnabled not set, cancelling firmware reload...");
					info_print("Set flashModeEnabled to 1 in your kll configuration.");
					return;
				}
			}

			void (*capability)(TriggerMacro*, uint8_t, uint8_t, uint8_t*) = \
				(void(*)(TriggerMacro*, uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ cap ].func);
			capability( 0, argSet[0], argSet[1], &argSet[2] );
		}
	}
}

void cliFunc_keyHold( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Ignore non-Scancode numbers
		switch ( arg1Ptr[0] )
		{
		// Scancode
		case 'S':
			Macro_keyState( (uint8_t)numToInt( &arg1Ptr[1] ), 0x02 ); // Hold scancode
			break;
		}
	}
}

void cliFunc_keyPress( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Ignore non-Scancode numbers
		switch ( arg1Ptr[0] )
		{
		// Scancode
		case 'S':
			Macro_keyState( (uint8_t)numToInt( &arg1Ptr[1] ), 0x01 ); // Press scancode
			break;
		}
	}
}

void cliFunc_keyRelease( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Ignore non-Scancode numbers
		switch ( arg1Ptr[0] )
		{
		// Scancode
		case 'S':
			Macro_keyState( (uint8_t)numToInt( &arg1Ptr[1] ), 0x03 ); // Release scancode
			break;
		}
	}
}

void cliFunc_layerDebug( char *args )
{
	// Toggle layer debug mode
	layerDebugMode = layerDebugMode ? 0 : 1;

	print( NL );
	info_print("Layer Debug Mode: ");
	printInt8( layerDebugMode );
}

void cliFunc_layerList( char* args )
{
	print( NL );
	info_print("Layer List");

	// Iterate through all of the layers and display them
	for ( index_uint_t layer = 0; layer < LayerNum; layer++ )
	{
		print( NL "\t" );
		printHex( layer );
		print(" - ");

		// Display layer name
		dPrint( (char*)LayerIndex[ layer ].name );

		// Default map
		if ( layer == 0 )
			print(" \033[1m(default)\033[0m");

		// Layer State
		print( NL "\t\t Layer State: " );
		printHex( LayerState[ layer ] );

		// First -> Last Indices
		print(" First -> Last Indices: ");
		printHex( LayerIndex[ layer ].first );
		print(" -> ");
		printHex( LayerIndex[ layer ].last );
	}
}

void cliFunc_layerState( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	uint8_t arg1 = 0;
	uint8_t arg2 = 0;

	// Process first two args
	for ( uint8_t c = 0; c < 2; c++ )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		switch ( c )
		{
		// First argument (e.g. L1)
		case 0:
			if ( arg1Ptr[0] != 'L' )
				return;

			arg1 = (uint8_t)numToInt( &arg1Ptr[1] );
			break;
		// Second argument (e.g. 4)
		case 1:
			arg2 = (uint8_t)numToInt( arg1Ptr );

			// Display operation (to indicate that it worked)
			print( NL );
			info_print("Setting Layer L");
			printInt8( arg1 );
			print(" to - ");
			printHex( arg2 );

			// Set the layer state
			LayerState[ arg1 ] = arg2;
			break;
		}
	}
}

void cliFunc_macroDebug( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set the macro debug flag depending on the argument
	switch ( arg1Ptr[0] )
	{
	// 3 as argument
	case '3':
		macroDebugMode = macroDebugMode != 3 ? 3 : 0;
		break;

	// 2 as argument
	case '2':
		macroDebugMode = macroDebugMode != 2 ? 2 : 0;
		break;

	// No argument
	case '1':
	case '\0':
		macroDebugMode = macroDebugMode != 1 ? 1 : 0;
		break;

	// Invalid argument
	default:
		return;
	}

	print( NL );
	info_print("Macro Debug Mode: ");
	printInt8( macroDebugMode );
}

void cliFunc_macroList( char* args )
{
	// Show pending key events
	print( NL );
	info_print("Pending Key Events: ");
	printInt16( (uint16_t)macroTriggerEventBufferSize );
	print(" : ");
	for ( var_uint_t key = 0; key < macroTriggerEventBufferSize; key++ )
	{
		printHex( macroTriggerEventBuffer[ key ].index );
		print(" ");
	}

	// Show pending trigger macros
	print( NL );
	info_print("Pending Trigger Macros: ");
	printInt16( (uint16_t)macroTriggerMacroPendingListSize );
	print(" : ");
	for ( var_uint_t macro = 0; macro < macroTriggerMacroPendingListSize; macro++ )
	{
		printHex( macroTriggerMacroPendingList[ macro ] );
		print(" ");
	}

	// Show pending result macros
	print( NL );
	info_print("Pending Result Macros: ");
	printInt16( (uint16_t)macroResultMacroPendingList.size );
	print(" : ");
	for ( var_uint_t macro = 0; macro < macroResultMacroPendingList.size; macro++ )
	{
		printHex( macroResultMacroPendingList.data[ macro ].index );
		print(" ");
	}

	// Show available trigger macro indices
	print( NL );
	info_print("Trigger Macros Range: T0 -> T");
	printInt16( (uint16_t)TriggerMacroNum - 1 ); // Hopefully large enough :P (can't assume 32-bit)

	// Show available result macro indices
	print( NL );
	info_print("Result  Macros Range: R0 -> R");
	printInt16( (uint16_t)ResultMacroNum - 1 ); // Hopefully large enough :P (can't assume 32-bit)

	// Show Trigger to Result Macro Links
	print( NL );
	info_print("Trigger : Result Macro Pairs");
	for ( var_uint_t macro = 0; macro < TriggerMacroNum; macro++ )
	{
		print( NL );
		print("\tT");
		printInt16( (uint16_t)macro ); // Hopefully large enough :P (can't assume 32-bit)
		print(" : R");
		printInt16( (uint16_t)TriggerMacroList[ macro ].result ); // Hopefully large enough :P (can't assume 32-bit)
	}
}

void cliFunc_macroProc( char* args )
{
	// Toggle macro pause mode
	macroPauseMode = macroPauseMode ? 0 : 1;

	print( NL );
	info_print("Macro Processing Mode: ");
	printInt8( macroPauseMode );
}

void macroDebugShowTrigger( var_uint_t index )
{
	// Only proceed if the macro exists
	if ( index >= TriggerMacroNum )
		return;

	// Trigger Macro Show
	const TriggerMacro *macro = &TriggerMacroList[ index ];
	TriggerMacroRecord *record = &TriggerMacroRecordList[ index ];

	print( NL );
	info_print("Trigger Macro Index: ");
	printInt16( (uint16_t)index ); // Hopefully large enough :P (can't assume 32-bit)
	print( NL );

	// Read the comboLength for combo in the sequence (sequence of combos)
	var_uint_t pos = 0;
	uint8_t comboLength = macro->guide[ pos ];

	// Iterate through and interpret the guide
	while ( comboLength != 0 )
	{
		// Initial position of the combo
		var_uint_t comboPos = ++pos;

		// Iterate through the combo
		while ( pos < comboLength * TriggerGuideSize + comboPos )
		{
			// Assign TriggerGuide element (key type, state and scancode)
			TriggerGuide *guide = (TriggerGuide*)(&macro->guide[ pos ]);

			// Display guide information about trigger key
			printHex( guide->scanCode );
			print("|");
			printHex( guide->type );
			print("|");
			printHex( guide->state );

			// Increment position
			pos += TriggerGuideSize;

			// Only show combo separator if there are combos left in the sequence element
			if ( pos < comboLength * TriggerGuideSize + comboPos )
				print("+");
		}

		// Read the next comboLength
		comboLength = macro->guide[ pos ];

		// Only show sequence separator if there is another combo to process
		if ( comboLength != 0 )
			print(";");
	}

	// Display current position
	print( NL "Position: " );
	printInt16( (uint16_t)record->pos ); // Hopefully large enough :P (can't assume 32-bit)
	print(" ");
	printInt16( (uint16_t)record->prevPos );

	// Display result macro index
	print( NL "Result Macro Index: " );
	printInt16( (uint16_t)macro->result ); // Hopefully large enough :P (can't assume 32-bit)

	// Display trigger macro state
	print( NL "Trigger Macro State: " );
	switch ( record->state )
	{
	case TriggerMacro_Press:        print("Press");   break;
	case TriggerMacro_Release:      print("Release"); break;
	case TriggerMacro_PressRelease: print("Press|Release"); break;
	case TriggerMacro_Waiting:      print("Waiting"); break;
	}
}

void macroDebugShowResult( var_uint_t index )
{
	// Only proceed if the macro exists
	if ( index >= ResultMacroNum )
		return;

	// Trigger Macro Show
	const ResultMacro *macro = &ResultMacroList[ index ];

	print( NL );
	info_print("Result Macro Index: ");
	printInt16( (uint16_t)index ); // Hopefully large enough :P (can't assume 32-bit)
	print( NL );

	// Read the comboLength for combo in the sequence (sequence of combos)
	var_uint_t pos = 0;
	uint8_t comboLength = macro->guide[ pos++ ];

	// Iterate through and interpret the guide
	while ( comboLength != 0 )
	{
		// Function Counter, used to keep track of the combos processed
		var_uint_t funcCount = 0;

		// Iterate through the combo
		while ( funcCount < comboLength )
		{
			// Assign TriggerGuide element (key type, state and scancode)
			ResultGuide *guide = (ResultGuide*)(&macro->guide[ pos ]);

			// Display Function Index
			printHex( guide->index );
			print("|");

			// Display Function Ptr Address
			printHex( (nat_ptr_t)CapabilitiesList[ guide->index ].func );
			print("|");

			// Display/Lookup Capability Name (utilize debug mode of capability)
			void (*capability)(TriggerMacro*, uint8_t, uint8_t, uint8_t*) = \
				(void(*)(TriggerMacro*, uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ guide->index ].func);
			capability( 0, 0xFF, 0xFF, 0 );

			// Display Argument(s)
			print("(");
			for ( var_uint_t arg = 0; arg < CapabilitiesList[ guide->index ].argCount; arg++ )
			{
				// Arguments are only 8 bit values
				printHex( (&guide->args)[ arg ] );

				// Only show arg separator if there are args left
				if ( arg + 1 < CapabilitiesList[ guide->index ].argCount )
					print(",");
			}
			print(")");

			// Increment position
			pos += ResultGuideSize( guide );

			// Increment function count
			funcCount++;

			// Only show combo separator if there are combos left in the sequence element
			if ( funcCount < comboLength )
				print("+");
		}

		// Read the next comboLength
		comboLength = macro->guide[ pos++ ];

		// Only show sequence separator if there is another combo to process
		if ( comboLength != 0 )
			print(";");
	}

	/* XXX (HaaTa) Fix for ring-buffer record list
	ResultMacroRecord *record = &ResultMacroRecordList[ index ];

	// Display current position
	print( NL "Position: " );
	printInt16( (uint16_t)record->pos ); // Hopefully large enough :P (can't assume 32-bit)
	print(" ");
	printInt16( (uint16_t)record->prevPos );

	// Display final trigger state/type
	print( NL "Final Trigger State (State/Type): " );
	printHex( record->state );
	print("/");
	printHex( record->stateType );
	*/
}

void cliFunc_macroShow( char* args )
{
	// Parse codes from arguments
	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process all args
	for ( ;; )
	{
		curArgs = arg2Ptr;
		CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

		// Stop processing args if no more are found
		if ( *arg1Ptr == '\0' )
			break;

		// Ignore invalid codes
		switch ( arg1Ptr[0] )
		{
		// Indexed Trigger Macro
		case 'T':
			macroDebugShowTrigger( numToInt( &arg1Ptr[1] ) );
			break;
		// Indexed Result Macro
		case 'R':
			macroDebugShowResult( numToInt( &arg1Ptr[1] ) );
			break;
		}
	}
}

void cliFunc_macroStep( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Default to 1, if no argument given
	var_uint_t count = (var_uint_t)numToInt( arg1Ptr );

	if ( count == 0 )
		count = 1;

	// Set the macro step counter, negative int's are cast to uint
	macroStepCounter = count;
}

// Convenience Macro
#define Key_PositionPrint( key, name ) \
	printInt16( Key_Position[ key ].name.i ); \
	print("."); \
	printInt16( Key_Position[ key ].name.f )

void cliFunc_posList( char* args )
{
	print( NL );

	/* TODO Add printFloat function
	// List out physical key positions by scan code
	for ( uint8_t key = 0; key < MaxScanCode_KLL; key++ )
	{
		printInt8( key + 1 );
		print(": [");
		Key_PositionPrint( key, x );
		print(", ");
		Key_PositionPrint( key, y );
		print(", ");
		Key_PositionPrint( key, z );
		print("] r[");
		Key_PositionPrint( key, rx );
		print(", ");
		Key_PositionPrint( key, ry );
		print(", ");
		Key_PositionPrint( key, rz );
		print("]");
		print( NL );
	}
	*/
}

void cliFunc_voteDebug( char* args )
{
	// Parse number from argument
	//  NOTE: Only first argument is used
	char* arg1Ptr;
	char* arg2Ptr;
	CLI_argumentIsolation( args, &arg1Ptr, &arg2Ptr );

	// Set the vote debug flag depending on the argument
	switch ( arg1Ptr[0] )
	{
	// No argument
	case 1:
	case '\0':
		voteDebugMode = voteDebugMode != 1 ? 1 : 0;
		break;

	// Invalid argument
	default:
		return;
	}

	print( NL );
	info_print("Vote Debug Mode: ");
	printInt8( voteDebugMode );
}

