/* Copyright (C) 2015-2019 by Jacob Alexander
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
#include <Lib/storage.h>
#include <cli.h>
#include <kll_defs.h>
#include <latency.h>
#include <led.h>
#include <print.h>
#include <output_com.h>

// Interconnect module if compiled in
#if defined(ConnectEnabled_define)
#include <connect_scan.h>
#endif

// Local Includes
#include "pixel.h"



// ----- Function Declarations -----

void Pixel_loadConfig();
void Pixel_saveConfig();
void Pixel_printConfig();

void cliFunc_aniAdd     ( char* args );
void cliFunc_aniDel     ( char* args );
void cliFunc_aniStack   ( char* args );
void cliFunc_chanTest   ( char* args );
void cliFunc_pixelList  ( char* args );
void cliFunc_pixelSCTest( char* args );
void cliFunc_pixelTest  ( char* args );
void cliFunc_pixelXYTest( char* args );
void cliFunc_rectDisp   ( char* args );



// ----- Enums -----

typedef enum PixelTest {
	PixelTest_Off                 = 0,   // Disabled
	PixelTest_Chan_Single         = 1,
	PixelTest_Chan_SingleReverse  = 2,
	PixelTest_Chan_All            = 3,   // Enable all positions
	PixelTest_Chan_Roll           = 4,   // Iterate over all positions
	PixelTest_Chan_Full           = 5,   // Turn on all pixels
	PixelTest_Chan_Off            = 6,   // Turn off all pixels
	PixelTest_Pixel_Single        = 10,
	PixelTest_Pixel_SingleReverse = 11,
	PixelTest_Pixel_All           = 12,  // Enable all positions
	PixelTest_Pixel_Roll          = 13,  // Iterate over all positions
	PixelTest_Pixel_Full          = 14,  // Turn on all pixels
	PixelTest_Pixel_Off           = 15,  // Turn off all pixels
	PixelTest_Scan_Single         = 20,
	PixelTest_Scan_SingleReverse  = 21,
	PixelTest_Scan_All            = 22,
	PixelTest_Scan_Roll           = 23,
	PixelTest_XY_Single           = 30,
	PixelTest_XY_SingleReverse    = 31,
	PixelTest_XY_All              = 32,
	PixelTest_XY_Roll             = 33,
} PixelTest;

typedef enum PixelFadeControl {
	PixelFadeControl_Reset                = 0, // Resets fade profile to defaults (arg ignored)
	PixelFadeControl_Reset_All            = 1, // Resets all fade profiles to defaults (profile, arg ignored)
	PixelFadeControl_Brightness_Set       = 2, // Sets fade profile to a given brightness
	PixelFadeControl_Brightness_Increment = 3, // Increment brightness by given amount
	PixelFadeControl_Brightness_Decrement = 4, // Decrement brightness by given amount
	PixelFadeControl_Brightness_Default   = 5, // Set profile brightness to default
	PixelFadeControl_LAST,
} PixelFadeControl;



// ----- Variables -----

typedef struct {
	uint8_t index;
	uint8_t pos;
} PixelConfigElem;

typedef struct {
	PixelConfigElem animations[Pixel_AnimationStackSize];
	PixelPeriodConfig fade_periods[4][4];
	uint8_t fade_brightness[4];
} PixelConfig;

static PixelConfig settings;

#if Storage_Enable_define == 1
static PixelConfig defaults;

static StorageModule PixelStorage = {
	.name = "Pixel Map",
	.settings = &settings,
	.defaults = &defaults,
	.size = sizeof(PixelConfig),
	.onLoad = Pixel_loadConfig,
	.onSave = Pixel_saveConfig,
	.display = Pixel_printConfig
};
#endif

// Macro Module command dictionary
CLIDict_Entry( aniAdd,       "Add the given animation id to the stack" );
CLIDict_Entry( aniDel,       "Remove the given stack index animation" );
CLIDict_Entry( aniStack,     "Displays the animation stack contents" );
CLIDict_Entry( chanTest,     "Channel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelList,    "Prints out pixel:channel mappings." );
CLIDict_Entry( pixelSCTest,  "Scancode pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( pixelTest,    "Pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop, f - full" );
CLIDict_Entry( pixelXYTest,  "XY pixel test. No arg - next pixel. # - pixel, r - roll-through. a - all, s - stop" );
CLIDict_Entry( rectDisp,     "Show the current output of the MCU pixel buffer." );

CLIDict_Def( pixelCLIDict, "Pixel Module Commands" ) = {
	CLIDict_Item( aniAdd ),
	CLIDict_Item( aniDel ),
	CLIDict_Item( aniStack ),
	CLIDict_Item( chanTest ),
	CLIDict_Item( pixelList ),
	CLIDict_Item( pixelSCTest ),
	CLIDict_Item( pixelTest ),
	CLIDict_Item( pixelXYTest ),
	CLIDict_Item( rectDisp ),
	{ 0, 0, 0 } // Null entry for dictionary end
};

// Gamma correction
extern const uint8_t gamma_table[];
static uint8_t gamma_enabled;

// Debug states
PixelTest Pixel_testMode;
volatile uint16_t  Pixel_testPos = 0;

// Frame State
//  Indicates to pixel and output modules current state of the buffer
FrameState Pixel_FrameState;

// Animation Stack
AnimationStack Pixel_AnimationStack;

// Animation Control
AnimationControl Pixel_animationControl;

// Memory Stor for Animation Elements
// Animation elements may be called multiple times, thus memory must be allocated per instance
AnimationStackElement Pixel_AnimationElement_Stor[Pixel_AnimationStackSize];

#if defined(_host_)
uint16_t Pixel_AnimationStack_HostSize = Pixel_AnimationStackSize;
uint8_t  Pixel_Buffers_HostLen = Pixel_BuffersLen_KLL;
uint8_t  Pixel_MaxChannelPerPixel_Host = Pixel_MaxChannelPerPixel;
uint16_t Pixel_Mapping_HostLen = 128; // TODO Define
uint8_t  Pixel_AnimationStackElement_HostSize = sizeof( AnimationStackElement );
#endif

// Pixel Fade Profile Mapping
// Assigned per pixel (rather than channel)
// 0 - Disabled
// 1 - Profile 1 - Keys
// 2 - Profile 2 - Underlighting
// 3 - Profile 3 - Indicator LEDs
// 4 - Profile 4 - Current active layer (defaultmap is excluded)
static uint8_t Pixel_pixel_fade_profile[Pixel_TotalPixels_KLL];

// Pixel Fade Profile Parameters
// TODO (HaaTa): Use KLL to determine number of profiles (currently only 4)
static PixelFadeProfile Pixel_pixel_fade_profile_entries[4];

// Latency Measurement Resource
static uint8_t pixelLatencyResource;



// ----- Function Declarations -----

uint8_t Pixel_animationProcess( AnimationStackElement *elem );
uint8_t Pixel_addAnimation( AnimationStackElement *element, CapabilityState cstate );
uint8_t Pixel_determineLastTriggerScanCode( TriggerMacro *trigger );

void Pixel_pixelSet( PixelElement *elem, uint32_t value );
void Pixel_clearAnimations();

void Pixel_SecondaryProcessing_profile_init();

PixelBuf *Pixel_bufferMap( uint16_t channel );

AnimationStackElement *Pixel_lookupAnimation( uint16_t index, uint16_t prev );



// ----- Capabilities -----
//
void Pixel_GammaControl_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_GammaControl_capability(func)");
		return;
	default:
		return;
	}

	uint8_t arg  = *(uint8_t*)(&args[0]);

	// Interconnect broadcasting
#if defined(ConnectEnabled_define)
	// By default send to the *next* node, which will determine where to go next
	extern uint8_t Connect_id; // connect_scan.c
	uint8_t addr = Connect_id + 1;

	// Send interconnect remote capability packet
	// generatedKeymap.h
	extern const Capability CapabilitiesList[];

	// Broadcast layerStackExact remote capability (0xFF is the broadcast id)
	Connect_send_RemoteCapability(
		addr,
		Pixel_GammaControl_capability_index,
		state,
		stateType,
		CapabilitiesList[ Pixel_GammaControl_capability_index ].argCount,
		args
	);
#endif

	// Decide how to handle function
	switch ( arg )
	{
	case 0: // Disabled
		gamma_enabled = 0;
		break;
	case 1: // Enabled
		gamma_enabled = 1;
		break;
	default: // Toggle
		gamma_enabled = !gamma_enabled;
		break;
	}
}

void Pixel_AnimationIndex_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
	case CapabilityState_Last:
		// Mainly used on press
		// Except some configurations may also use release
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_AnimationIndex_capability(settingindex)");
		return;
	default:
		return;
	}

	// Interconnect broadcasting
#if defined(ConnectEnabled_define)
	// By default send to the *next* node, which will determine where to go next
	extern uint8_t Connect_id; // connect_scan.c
	uint8_t addr = Connect_id + 1;

	// Send interconnect remote capability packet
	// generatedKeymap.h
	extern const Capability CapabilitiesList[];

	// Broadcast layerStackExact remote capability (0xFF is the broadcast id)
	Connect_send_RemoteCapability(
		addr,
		Pixel_AnimationIndex_capability_index,
		state,
		stateType,
		CapabilitiesList[ Pixel_AnimationIndex_capability_index ].argCount,
		args
	);
#endif

	// Lookup animation settings
	uint16_t index = *(uint16_t*)(&args[0]);

	// Check if a valid setting
	if ( index >= Pixel_AnimationSettingsNum_KLL )
	{
		warn_msg("Invalid AnimationSetting index: ");
		printInt16( index );
		print( NL );
		return;
	}

	AnimationStackElement element = Pixel_AnimationSettings[ index ];
	element.trigger = trigger;

	Pixel_addAnimation( &element, cstate );
}

// XXX (HaaTa): It's not recommended to use this capability, use AnimationIndex instead
void Pixel_Animation_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_Animation_capability(index,loops,pfunc,framedelay,frameoption,replace)");
		return;
	default:
		return;
	}

	AnimationStackElement element;
	element.trigger = trigger;
	element.pos = 0; // TODO (HaaTa) Start at specific frame
	element.subpos = 0;
	element.index = *(uint16_t*)(&args[0]);
	element.loops = *(uint8_t*)(&args[2]);
	element.pfunc = *(uint8_t*)(&args[3]);
	element.framedelay = *(uint8_t*)(&args[4]);
	element.frameoption = *(uint8_t*)(&args[5]);
	element.replace = *(uint8_t*)(&args[6]);

	Pixel_addAnimation( &element, cstate );
}

// XXX (HaaTa): TODO
void Pixel_Pixel_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_Pixel_capability(pixel,chan,value)");
		return;
	default:
		return;
	}

	/*
	PixelChange change = *(PixelChange*)(&args[0]);
	uint16_t channel = *(uint16_t*)(&args[1]);
	uint32_t value = *(uint32_t*)(&args[3]);
	*/

	// TODO (HaaTa) Apply the channel modification
}

void Pixel_AnimationControl_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_AnimationControl_capability(func)");
		return;
	default:
		return;
	}

	// Interconnect broadcasting
#if defined(ConnectEnabled_define)
	// By default send to the *next* node, which will determine where to go next
	extern uint8_t Connect_id; // connect_scan.c
	uint8_t addr = Connect_id + 1;

	// Send interconnect remote capability packet
	// generatedKeymap.h
	extern const Capability CapabilitiesList[];

	// Broadcast layerStackExact remote capability (0xFF is the broadcast id)
	Connect_send_RemoteCapability(
		addr,
		Pixel_AnimationControl_capability_index,
		state,
		stateType,
		CapabilitiesList[ Pixel_AnimationControl_capability_index ].argCount,
		args
	);
#endif

	uint8_t arg  = *(uint8_t*)(&args[0]);

	// Decide how to handle function
	switch ( arg )
	{
	case 0: // Pause/Resume
		// Determine how to handle Pause/Resume
		switch ( Pixel_animationControl )
		{
		case AnimationControl_Forward:
		case AnimationControl_ForwardOne:
			Pixel_animationControl = AnimationControl_Pause;
			break;
		case AnimationControl_Pause:
		default:
			Pixel_animationControl = AnimationControl_Forward;
			break;
		}
		break;
	case 1: // Forward one frame
		Pixel_animationControl = AnimationControl_ForwardOne;
		break;
	case 2: // Forward
		Pixel_animationControl = AnimationControl_Forward;
		break;
	case 3: // Stop (clears all animations)
		Pixel_animationControl = AnimationControl_Stop;
		break;
	case 4: // Reset (restarts animations)
		Pixel_animationControl = AnimationControl_Reset;
		break;
	case 5: // Pauses animations and clears display
		Pixel_animationControl = AnimationControl_WipePause;
		break;
	case 6: // Pauses animation
		Pixel_animationControl = AnimationControl_Pause;
		break;
	case 7: // Clears pixels (no pause and no stop)
		Pixel_animationControl = AnimationControl_Clear;
		break;
	}
}

void Pixel_FadeSet_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only use capability on press
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_FadeSet_capability(profile,config,period)");
		return;
	default:
		return;
	}

	// Interconnect broadcasting
#if defined(ConnectEnabled_define)
	// By default send to the *next* node, which will determine where to go next
	extern uint8_t Connect_id; // connect_scan.c
	uint8_t addr = Connect_id + 1;

	// Send interconnect remote capability packet
	// generatedKeymap.h
	extern const Capability CapabilitiesList[];

	// Broadcast layerStackExact remote capability (0xFF is the broadcast id)
	Connect_send_RemoteCapability(
		addr,
		Pixel_FadeSet_capability_index,
		state,
		stateType,
		CapabilitiesList[ Pixel_FadeSet_capability_index ].argCount,
		args
	);
#endif

	// Get arguments
	uint8_t profile = *(uint8_t*)(&args[0]);
	uint8_t config = *(uint8_t*)(&args[1]);
	uint8_t period = *(uint8_t*)(&args[2]);

	// Get period configuation
	const PixelPeriodConfig *period_config = &Pixel_LED_FadePeriods[period];

	// Set period configuration
	Pixel_pixel_fade_profile_entries[profile].conf[config].start = period_config->start;
	Pixel_pixel_fade_profile_entries[profile].conf[config].end = period_config->end;

	// Reset the current period being processed
	Pixel_pixel_fade_profile_entries[profile].pos = 0;
	Pixel_pixel_fade_profile_entries[profile].period_conf = PixelPeriodIndex_Off_to_On;
}

void Pixel_FadeLayerHighlight_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Scan the layer for keys
		break;
	case CapabilityState_Last:
		// Refresh the fade profiles
		Pixel_SecondaryProcessing_profile_init();
		return;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_FadeLayerHighlight_capability(layer)");
		return;
	default:
		return;
	}

	// Interconnect broadcasting
#if defined(ConnectEnabled_define)
	// By default send to the *next* node, which will determine where to go next
	extern uint8_t Connect_id; // connect_scan.c
	uint8_t addr = Connect_id + 1;

	// Send interconnect remote capability packet
	// generatedKeymap.h
	extern const Capability CapabilitiesList[];

	// Broadcast layerStackExact remote capability (0xFF is the broadcast id)
	Connect_send_RemoteCapability(
		addr,
		Pixel_FadeLayerHighlight_capability_index,
		state,
		stateType,
		CapabilitiesList[ Pixel_FadeLayerHighlight_capability_index ].argCount,
		args
	);
#endif

	// Get argument
	uint16_t layer = *(uint16_t*)(&args[0]);

	// Ignore if an invalid layer
	if ( layer >= LayerNum )
	{
		return;
	}

	// Lookup layer
	const Layer *layer_map = &LayerIndex[layer];

	// Lookup list of keys in layer
	for ( uint8_t key = layer_map->first; key <= layer_map->last; key++ )
	{
		uint8_t index = key - layer_map->first;

		// Skip 0 index, as scancodes start at 1
		if ( index == 0 )
		{
			continue;
		}

		// If the first entry in trigger list is a 0, ignore (otherwise, key is in layer)
		if ( layer_map->triggerMap[index][0] == 0 )
		{
			continue;
		}

		// Lookup pixel associated with scancode (remember -1 as all pixels and scancodes start at 1, not 0)
		uint16_t pixel = Pixel_ScanCodeToPixel[key - 1];

		// If pixel is 0, ignore
		if ( pixel == 0 )
		{
			continue;
		}

		// Set pixel to group #4
		Pixel_pixel_fade_profile[pixel - 1] = 4;
	}
}

void Pixel_FadeControl_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only activate on press event
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_FadeControl_capability(test)");
		return;
	default:
		return;
	}

	// Get arguments
	uint8_t profile = args[0];
	uint8_t command = args[1];
	uint8_t arg = args[2];

	// Make sure profile is valid
	if ( profile >= sizeof(Pixel_pixel_fade_profile_entries) )
	{
		return;
	}

	// Process command
	uint8_t tmp;
	switch ( command )
	{
	case PixelFadeControl_Reset:
		for ( uint8_t config = 0; config < 4; config++ )
		{
			Pixel_pixel_fade_profile_entries[profile].conf[config] = \
				Pixel_LED_FadePeriods[Pixel_LED_FadePeriod_Defaults[profile][config]];
		}
		Pixel_pixel_fade_profile_entries[profile].pos = 0;
		Pixel_pixel_fade_profile_entries[profile].period_conf = PixelPeriodIndex_Off_to_On;
		Pixel_pixel_fade_profile_entries[profile].brightness = Pixel_LED_FadeBrightness[profile];
		break;

	case PixelFadeControl_Reset_All:
		// Setup fade defaults
		for ( uint8_t pr = 0; pr < 4; pr++ )
		{
			for ( uint8_t config = 0; config < 4; config++ )
			{
				Pixel_pixel_fade_profile_entries[pr].conf[config] = \
					Pixel_LED_FadePeriods[Pixel_LED_FadePeriod_Defaults[pr][config]];
			}
			Pixel_pixel_fade_profile_entries[pr].pos = 0;
			Pixel_pixel_fade_profile_entries[pr].period_conf = PixelPeriodIndex_Off_to_On;
			Pixel_pixel_fade_profile_entries[profile].brightness = Pixel_LED_FadeBrightness[pr];
		}
		break;

	case PixelFadeControl_Brightness_Set:
		// Set brightness
		Pixel_pixel_fade_profile_entries[profile].brightness = arg;
		break;

	case PixelFadeControl_Brightness_Increment:
		// Increment with no rollover
		tmp = Pixel_pixel_fade_profile_entries[profile].brightness;
		if ( tmp + arg < tmp )
		{
			Pixel_pixel_fade_profile_entries[profile].brightness = 0xFF;
			break;
		}
		Pixel_pixel_fade_profile_entries[profile].brightness += arg;
		break;

	case PixelFadeControl_Brightness_Decrement:
		// Decrement with no rollover
		tmp = Pixel_pixel_fade_profile_entries[profile].brightness;
		if ( tmp - arg > tmp )
		{
			Pixel_pixel_fade_profile_entries[profile].brightness = 0x00;
			break;
		}
		Pixel_pixel_fade_profile_entries[profile].brightness -= arg;
		break;

	case PixelFadeControl_Brightness_Default:
		Pixel_pixel_fade_profile_entries[profile].brightness = Pixel_LED_FadeBrightness[profile];
		break;

	default:
		return;
	}
}

void Pixel_LEDTest_capability( TriggerMacro *trigger, uint8_t state, uint8_t stateType, uint8_t *args )
{
	CapabilityState cstate = KLL_CapabilityState( state, stateType );

	switch ( cstate )
	{
	case CapabilityState_Initial:
		// Only activate on press event
		break;
	case CapabilityState_Debug:
		// Display capability name
		print("Pixel_LEDTest_capability(test)");
		return;
	default:
		return;
	}

	// Get arguments
	PixelTest test = *(PixelTest*)(&args[0]);
	uint16_t index = *(uint16_t*)(&args[1]);

	// If index is not set to 0xFFFF, make sure to update the test position
	if ( index != 0xFFFF )
	{
		Pixel_testPos = index;
	}

	// Set the test mode
	Pixel_testMode = test;
}



// ----- Functions -----

// -- Debug Functions --

// Debug info for PixelElement
void Pixel_showPixelElement( PixelElement *elem )
{
	print("W:");
	printInt8( elem->width );
	print(" C:");
	printInt8( elem->channels );
	print(" I:");
	printInt16( elem->indices[0] );
	for ( uint8_t c = 1; c < elem->channels; c++ )
	{
		print(",");
		printInt16( elem->indices[c] );

	}
}


// -- Utility Functions --

// TODO Support non-8bit channels
uint8_t Pixel_8bitInterpolation( uint8_t start, uint8_t end, uint8_t dist )
{
	return (start * (256 - dist) + end * dist) >> 8;
}

void Pixel_pixelInterpolate( PixelElement *elem, uint8_t position, uint8_t intensity )
{
	// Toggle each of the channels of the pixel

	for ( uint8_t ch = 0; ch < elem->channels; ch++ )
	{
		uint16_t ch_pos = elem->indices[ch];
		PixelBuf *pixbuf = Pixel_bufferMap( ch_pos );
		PixelBuf16( pixbuf, ch_pos ) = Pixel_8bitInterpolation( 0, intensity, position * (ch + 1) );
	}
}



// -- Animation Stack --

// Locates animation memory slot using default settings for the animation
// Initiates animation to process on the next cycle
// Returns 1 on success, 0 on failure to allocate
uint8_t Pixel_addDefaultAnimation( uint32_t index )
{
	if ( index >= Pixel_AnimationSettingsNum_KLL )
	{
		warn_msg("Invalid AnimationSetting index: ");
		printInt32( index );
		print( NL );
		return 0;
	}

	return Pixel_addAnimation( (AnimationStackElement*)&Pixel_AnimationSettings[ index ], CapabilityState_None );
}

// Allocates animaton memory slot
// Initiates animation to process on the next cycle
// Returns 1 on success, 0 on failure to allocate
uint8_t Pixel_addAnimation( AnimationStackElement *element, CapabilityState cstate )
{
	AnimationStackElement *found;
	switch ( element->replace )
	{
	case AnimationReplaceType_Basic:
	case AnimationReplaceType_All:
		found = Pixel_lookupAnimation( element->index, 0 );

		// If found, modify stack element
		if ( found != NULL && ( found->trigger == element->trigger || element->replace == AnimationReplaceType_All ) )
		{
			found->pos = element->pos;
			found->subpos = element->subpos;
			found->loops = element->loops;
			found->pfunc = element->pfunc;
			found->ffunc = element->ffunc;
			found->framedelay = element->framedelay;
			found->frameoption = element->frameoption;
			found->replace = element->replace;
			found->state = element->state;
			return 0;
		}

	// Replace on press and release
	// Press starts the animation
	// Release stops the animation
	case AnimationReplaceType_State:
		found = Pixel_lookupAnimation( element->index, 0 );

		switch ( cstate )
		{
		// Press
		case CapabilityState_Initial:
			// If found, modify stack element
			if ( found )
			{
				found->pos = element->pos;
				found->subpos = element->subpos;
				found->loops = element->loops;
				found->pfunc = element->pfunc;
				found->ffunc = element->ffunc;
				found->framedelay = element->framedelay;
				found->frameoption = element->frameoption;
				found->replace = element->replace;
				found->state = element->state;
				return 0;
			}
			break;

		// Release
		case CapabilityState_Last:
			// Only need to do something if the animation was found (which is stop)
			if ( found )
			{
				found->state = AnimationPlayState_Stop;
			}
			return 0;

		default:
			break;
		}

	// Clear all current animations from stack before adding new animation
	case AnimationReplaceType_Clear:
		Pixel_clearAnimations();
		break;

	// Clear all current animations from stack before adding new animation
	// Unless it's paused, and if it's paused do a replace if necessary
	case AnimationReplaceType_ClearActive:
		found = Pixel_lookupAnimation( element->index, 0 );
		// If found, modify stack element
		if ( found )
		{
			found->pos = element->pos;
			found->subpos = element->subpos;
			found->loops = element->loops;
			found->pfunc = element->pfunc;
			found->ffunc = element->ffunc;
			found->framedelay = element->framedelay;
			found->frameoption = element->frameoption;
			found->replace = element->replace;
			found->state = element->state;
			return 0;
		}

		// Iterate through stack, stopping animations that are not paused
		// and ignoring the found animation
		for ( uint16_t pos = 0; pos < Pixel_AnimationStack.size; pos++ )
		{
			// Ignore found animation
			if ( Pixel_AnimationStack.stack[pos] == found )
			{
				continue;
			}

			// Ignore paused animations (single will be paused on the next frame)
			if (
				Pixel_AnimationStack.stack[pos]->state == AnimationPlayState_Pause ||
				Pixel_AnimationStack.stack[pos]->state == AnimationPlayState_Single
			)
			{
				continue;
			}

			// Otherwise stop
			Pixel_AnimationStack.stack[pos]->state = AnimationPlayState_Stop;
		}
		break;

	default:
		break;
	}

	// Make sure there is room left on the stack
	if ( Pixel_AnimationStack.size >= Pixel_AnimationStackSize )
	{
		warn_print("Animation stack is full...");
		return 0;
	}

	// Add to animation stack
	// Processing is done from bottom to top of the stack
	// First find a free memory slot, this is not ordered, so we have to search for it
	uint16_t pos = 0;
	for ( ; pos < Pixel_AnimationStackSize; pos++ )
	{
		// Check if memory is unused
		if ( Pixel_AnimationElement_Stor[pos].index == 0xFFFF )
		{
			break;
		}
	}

	// No memory left
	// XXX This shouldn't happen
	if ( pos >= Pixel_AnimationStackSize )
	{
		erro_print("Animation Stack memory leak...this is a bug!");
		return 0;
	}

	// Set stack location
	Pixel_AnimationStack.stack[Pixel_AnimationStack.size++] = &Pixel_AnimationElement_Stor[pos];

	// Copy animation settings
	memcpy( &Pixel_AnimationElement_Stor[pos], element, sizeof(AnimationStackElement) );

	return 1;
}

// Removes the first index of an animation
// Will be popped from the stack on the next animation processing loop
uint8_t Pixel_delAnimation( uint16_t index, uint8_t finish )
{
	// Find animation by index, look for the *first* one
	uint16_t pos = 0;
	for ( ; pos < Pixel_AnimationStackSize; pos++ )
	{
		// Check if memory is unused
		if ( Pixel_AnimationElement_Stor[pos].index == index )
		{
			// Let animation finish it's last frame
			if ( finish )
			{
				Pixel_AnimationElement_Stor[pos].loops = 1;
			}
			else
			{
				Pixel_AnimationElement_Stor[pos].index = 0xFFFF;
			}
			return 1;
		}
	}

	return 0;
}

// Cleans/resets animation stack. Removes all running animations.
// NOTE: Does not clear the output buffer
void Pixel_clearAnimations()
{
	// Set stack size to 0
	Pixel_AnimationStack.size = 0;

	// Set indices to max value to indicate un-allocated
	for ( uint16_t pos = 0; pos < Pixel_AnimationStackSize; pos++ )
	{
		Pixel_AnimationElement_Stor[pos].index = 0xFFFF;
	}
}

// Clears all pixels
void Pixel_clearPixels()
{
	// Update all positions
	for ( uint16_t px = 0; px < Pixel_TotalPixels_KLL; px++ )
	{
		// Unset pixel
		Pixel_pixelSet( (PixelElement*)&Pixel_Mapping[ px ], 0 );
	}
}

// Animation ID lookup
// - Does lookup from bottom to top
// - Set previous element to look for the next one
// - Set prev to 0 to start from the beginning
// - Returns NULL/0 if not found
AnimationStackElement *Pixel_lookupAnimation( uint16_t index, uint16_t prev )
{
	uint16_t pos = prev;

	// Look for next instance of index
	for ( ; pos < Pixel_AnimationStack.size; pos++ )
	{
		// Check if index matches
		if ( Pixel_AnimationStack.stack[pos]->index == index )
		{
			return Pixel_AnimationStack.stack[pos];
		}
	}

	// Could not find, return NULL
	return NULL;
}

// Iterate over each element in the Animation Stack
// If the animation is complete, do not re-add to the stack
// - The stack is re-built each time.
// - Ignores any stack element indices set to 0xFFFF/-1 (max)
void Pixel_stackProcess()
{
	uint16_t pos = 0;
	uint16_t size = Pixel_AnimationStack.size;

	// We reset the stack size, and rebuild the stack on the fly
	Pixel_AnimationStack.size = 0;

	// Process each element of the stack
	for ( ; pos < size; pos++ )
	{
		// Lookup animation stack element
		AnimationStackElement *elem = Pixel_AnimationStack.stack[pos];

		// Ignore animation if index is 0xFFFF (max)
		if ( elem->index == 0xFFFF )
		{
			continue;
		}

		// Store index, in case we need to send an event
		uint16_t cur_index = elem->index;

		// Process animation element
		if ( Pixel_animationProcess( elem ) )
		{
			// Re-add animation to stack
			Pixel_AnimationStack.stack[Pixel_AnimationStack.size++] = elem;
		}
		else
		{
			// Signal that animation finished
			Macro_animationState( cur_index, ScheduleType_Done );
		}
	}
}



// -- Pixel Control --

// PixelBuf lookup
// - Determines which buffer a channel resides in
PixelBuf *Pixel_bufferMap( uint16_t channel )
{
	// TODO Generate based on keyboard
#if ISSI_Chip_31FL3731_define == 1 || ISSI_Chip_31FL3732_define == 1
	if      ( channel < 144 ) return &Pixel_Buffers[0];
	else if ( channel < 288 ) return &Pixel_Buffers[1];
	else if ( channel < 432 ) return &Pixel_Buffers[2];
	else if ( channel < 576 ) return &Pixel_Buffers[3];
#elif ISSI_Chip_31FL3733_define == 1
	if      ( channel < 192 ) return &Pixel_Buffers[0];
	else if ( channel < 384 ) return &Pixel_Buffers[1];
	else if ( channel < 576 ) return &Pixel_Buffers[2];
#else
	if      ( channel < 192 ) return &Pixel_Buffers[0];
	else if ( channel < 384 ) return &Pixel_Buffers[1];
	else if ( channel < 576 ) return &Pixel_Buffers[2];
#endif

	// Invalid channel, return first channel and display error
	erro_msg("Invalid channel: ");
	printHex( channel );
	print( NL );
	return 0;
}

// PixelBuf lookup (LED_Buffers)
// - Determines which buffer a channel resides in
PixelBuf *LED_bufferMap( uint16_t channel )
{
	// TODO Generate based on keyboard
#if ISSI_Chip_31FL3731_define == 1 || ISSI_Chip_31FL3732_define == 1
	if      ( channel < 144 ) return &LED_Buffers[0];
	else if ( channel < 288 ) return &LED_Buffers[1];
	else if ( channel < 432 ) return &LED_Buffers[2];
	else if ( channel < 576 ) return &LED_Buffers[3];
#elif ISSI_Chip_31FL3733_define == 1
	if      ( channel < 192 ) return &LED_Buffers[0];
	else if ( channel < 384 ) return &LED_Buffers[1];
	else if ( channel < 576 ) return &LED_Buffers[2];
#else
	if      ( channel < 192 ) return &LED_Buffers[0];
	else if ( channel < 384 ) return &LED_Buffers[1];
	else if ( channel < 576 ) return &LED_Buffers[2];
#endif

	// Invalid channel, return first channel and display error
	erro_msg("Invalid channel (LED): ");
	printHex( channel );
	print( NL );
	return 0;
}

#define PixelChange_Expansion(pixbuf, ch_pos, mod_value, op) \
	/* Lookup buffer to data width mapping */ \
	switch ( pixbuf->width ) \
	{ \
	case 8:  /*  8 bit mapping */ \
		PixelBuf8( pixbuf, ch_pos ) op (uint8_t)mod_value; break; \
	case 16: /* 16 bit mapping */ \
		PixelBuf16( pixbuf, ch_pos ) op (uint16_t)mod_value; break; \
	case 32: /* 32 bit mapping */ \
		PixelBuf32( pixbuf, ch_pos ) op (uint32_t)mod_value; break; \
	default: \
		warn_print("Invalid width mapping for "#op ); \
		break; \
	}

// Pixel Evaluation
// - Iterates over each of the Pixel channels and applies modifications
void Pixel_pixelEvaluation( PixelModElement *mod, PixelElement *elem )
{
	// Ignore if no element
	if ( elem == 0 )
	{
		return;
	}

	// Lookup number of channels in pixel
	uint8_t channels = elem->channels;

	// Data position iterator
	uint8_t position_iter = 0;

	// Apply operation to each channel of the pixel
	for ( uint8_t ch = 0; ch < channels; ch++ )
	{
		// Lookup channel position
		uint16_t ch_pos = elem->indices[ch];

		// Determine which buffer we are in
		PixelBuf *pixbuf = Pixel_bufferMap( ch_pos );

		// Invalid channel, stop
		if ( pixbuf == 0 )
		{
			break;
		}

		// Change Type (first 8 bits of each channel of data, see pixel.h for layout)
		PixelChange change = (PixelChange)mod->data[ position_iter++ ];

		// Modification Value
		uint32_t mod_value = 0;

		// Lookup modification value
		switch ( elem->width )
		{
		case 8:
			mod_value = mod->data[ position_iter++ ];
			break;

		case 16:
			mod_value = mod->data[ position_iter + 1 ] |
				( mod->data[ position_iter + 2 ] << 8 );
			position_iter += 2;
			break;

		case 32:
			mod_value = mod->data[ position_iter + 1 ] |
				( mod->data[ position_iter + 2 ] << 8 ) |
				( mod->data[ position_iter + 3 ] << 16 ) |
				( mod->data[ position_iter + 4 ] << 24 );
			position_iter += 4;
			break;

		default:
			warn_print("Invalid PixelElement width mapping");
			break;
		}

		// Operation
		switch ( change )
		{
		case PixelChange_Set:             // =
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, = );
			break;

		case PixelChange_Add:             // +
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, += );
			break;

		case PixelChange_Subtract:        // -
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, -= );
			break;

		case PixelChange_LeftShift:       // <<
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, <<= );
			break;

		case PixelChange_RightShift:      // >>
			PixelChange_Expansion( pixbuf, ch_pos, mod_value, >>= );
			break;

		case PixelChange_NoRoll_Add:      // +:
			// Lookup buffer to data width mapping
			switch ( pixbuf->width )
			{
			case 8:  //  8  bit mapping
			{
				uint8_t prev = PixelBuf8( pixbuf, ch_pos );
				PixelBuf8( pixbuf, ch_pos ) += (uint8_t)mod_value;
				if ( prev > PixelBuf8( pixbuf, ch_pos ) )
					PixelBuf8( pixbuf, ch_pos ) = 0xFF;
				break;
			}
			case 16: // 16  bit mapping
			{
				// TODO Fix for 16 on 8 bit (i.e. early K-Type)
				//uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) += (uint16_t)mod_value;
				/*
				if ( prev > PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0xFFFF;
				*/
				if ( 0xFF < PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0xFF;
				break;
			}
			case 32: // 32  bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) += (uint32_t)mod_value;
				if ( prev > PixelBuf32( pixbuf, ch_pos ) )
					PixelBuf32( pixbuf, ch_pos ) = 0xFFFFFFFF;
				break;
			}

			default:
				warn_print("Invalid width mapping on set");
				break;
			}
			break;

		case PixelChange_NoRoll_Subtract: // -:
			// Lookup buffer to data width mapping
			switch ( pixbuf->width )
			{
			case 8:  //  8  bit mapping
			{
				uint8_t prev = PixelBuf8( pixbuf, ch_pos );
				PixelBuf8( pixbuf, ch_pos ) -= (uint8_t)mod_value;
				if ( prev < PixelBuf8( pixbuf, ch_pos ) )
					PixelBuf8( pixbuf, ch_pos ) = 0;
				break;
			}
			case 16: // 16  bit mapping
			{
				uint16_t prev = PixelBuf16( pixbuf, ch_pos );
				PixelBuf16( pixbuf, ch_pos ) -= (uint16_t)mod_value;
				if ( prev < PixelBuf16( pixbuf, ch_pos ) )
					PixelBuf16( pixbuf, ch_pos ) = 0;
				break;
			}
			case 32: // 32  bit mapping
			{
				uint32_t prev = PixelBuf32( pixbuf, ch_pos );
				PixelBuf32( pixbuf, ch_pos ) -= (uint32_t)mod_value;
				if ( prev < PixelBuf32( pixbuf, ch_pos ) )
					PixelBuf32( pixbuf, ch_pos ) = 0;
				break;
			}

			default:
				warn_print("Invalid width mapping on set");
				break;
			}
			break;

		default:
			warn_print("Unimplemented pixel modifier");
			break;
		}
	}
}



// -- Fill Algorithms --

// Fill Algorithm Pixel Lookup
// - **elem stores a pointer to the PixelElement which can be used to lookup the channel buffer location
// - Determines which pixel element to work on next
// - If type:index has more than one pixel, non-0 is returned
// - The return value signifies the next value to set the prev argument
// - Once the function returns 0, all pixels have been processed
uint16_t Pixel_fillPixelLookup(
	PixelModElement *mod,
	PixelElement **elem,
	uint16_t prev,
	AnimationStackElement *stack_elem,
	uint16_t *valid
)
{
	// Used to determine next pixel in column or row (fill)
	uint16_t cur = prev;
	uint16_t index = 0;

	// Assume invalid (i.e. do not evaluate pixel unless we are sure it's valid)
	*valid = 0;

	// Default to nothing found
	*elem = 0;

	// Lookup fill algorith
	switch ( mod->type )
	{
	case PixelAddressType_Index:
		// Lookup pixel by absolute index
		*elem = (PixelElement*)&Pixel_Mapping[mod->index] - 1;
		if ( mod->index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}
		break;

	case PixelAddressType_Rect:
		// Make sure row,column exists
		if ( mod->rect.col >= Pixel_DisplayMapping_Cols_KLL
			&& mod->rect.row >= Pixel_DisplayMapping_Rows_KLL )
		{
			erro_msg("Invalid row,column index: ");
			printInt16( mod->rect.row );
			print(",");
			printInt16( mod->rect.col );
			print( NL );
			break;
		}

		// Lookup pixel in rectangle display organization
		*elem = (PixelElement*)&Pixel_Mapping[
			Pixel_DisplayMapping[
				mod->rect.row * Pixel_DisplayMapping_Cols_KLL + mod->rect.col
			] - 1
		];
		*valid = 1;
		break;

	case PixelAddressType_ColumnFill:
		// Lookup pixel until either, non-0 index or we reached the last row
		do {
			// Check if we've processed all rows
			if ( cur >= Pixel_DisplayMapping_Rows_KLL )
			{
				return 0;
			}

			// Pixel index
			index = Pixel_DisplayMapping[ cur * Pixel_DisplayMapping_Cols_KLL + mod->rect.col ];

			cur++;
		} while ( index == 0 );

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;

	case PixelAddressType_RowFill:
		// Lookup pixel until either, non-0 index or we reached the last column
		do {
			// Check if we've processed all rows
			if ( cur >= Pixel_DisplayMapping_Cols_KLL )
			{
				return 0;
			}

			// Pixel index
			index = Pixel_DisplayMapping[ mod->rect.row * Pixel_DisplayMapping_Cols_KLL + cur ];

			cur++;
		} while ( index == 0 );

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;

	case PixelAddressType_ScanCode:
		// Make sure ScanCode exists
		if ( mod->index > MaxScanCode_KLL )
		{
			erro_msg("Invalid ScanCode: ");
			printInt16( mod->index );
			print( NL );
			break;
		}
		*valid = 1;

		// Lookup ScanCode - Indices are 1-indexed in both arrays (hence the -1)
		uint16_t pixel = Pixel_ScanCodeToPixel[ mod->index - 1 ];
		*elem = (PixelElement*)&Pixel_Mapping[ pixel - 1 ];
		break;

	case PixelAddressType_RelativeIndex:
		// TODO
		break;

	case PixelAddressType_RelativeRect:
	{
		// Determine scancode to be relative from
		uint8_t scan_code = Pixel_determineLastTriggerScanCode( stack_elem->trigger );

		// Lookup display position of scancode
		uint16_t position = Pixel_ScanCodeToDisplay[ scan_code - 1 ];

		// Calculate rectangle offset
		position += (int16_t)mod->rect.row * Pixel_DisplayMapping_Cols_KLL + (int16_t)mod->rect.col;

		// Make sure position exists
		if ( position >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
		{
			break;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		index = Pixel_DisplayMapping[ position ];

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL && index != 0 )
		{
			*valid = 1;
			*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		}
		break;
	}
	case PixelAddressType_RelativeColumnFill:
	{
		// Determine scancode to be relative from
		uint8_t scan_code = Pixel_determineLastTriggerScanCode( stack_elem->trigger );

		// Lookup display position of scancode
		uint16_t position = Pixel_ScanCodeToDisplay[ scan_code - 1 ];

		// Calculate rectangle offset
		position += (int16_t)mod->rect.col;

		// Make sure column exists
		if ( position >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
		{
			erro_msg("Invalid position index (relcol): ");
			printInt16( position );
			print( NL );
			break;
		}

		// Determine first row in column
		position %= Pixel_DisplayMapping_Cols_KLL;

		// Lookup pixel until either, non-0 index or we reached the last row
		do {
			// Current position
			uint16_t curpos = cur++ * Pixel_DisplayMapping_Cols_KLL + position;

			// Check if we've gone too far (and finished all rows)
			if ( curpos >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
			{
				return 0;
			}

			// Pixel index
			index = Pixel_DisplayMapping[ curpos ];
		} while ( index == 0 );

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;
	}
	case PixelAddressType_RelativeRowFill:
	{
		// Determine scancode to be relative from
		uint8_t scan_code = Pixel_determineLastTriggerScanCode( stack_elem->trigger );

		// Lookup display position of scancode
		uint16_t position = Pixel_ScanCodeToDisplay[ scan_code - 1 ];

		// Calculate rectangle offset
		position += (int16_t)mod->rect.row * Pixel_DisplayMapping_Rows_KLL;

		// Make sure column exists
		if ( position >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
		{
			erro_msg("Invalid position index (relrow): ");
			printInt16( position );
			print( NL );
			break;
		}

		// Determine which row we are in
		position /= Pixel_DisplayMapping_Cols_KLL;

		// Lookup pixel until either, non-0 index or we reached the last row
		do {
			// Current position
			uint16_t curpos = cur++ + Pixel_DisplayMapping_Cols_KLL * position;

			// Check if we've gone too far (and finished all rows)
			if ( cur >= Pixel_DisplayMapping_Cols_KLL )
			{
				return 0;
			}

			// Pixel index
			index = Pixel_DisplayMapping[ curpos ];
		} while ( index == 0 );

		// Validate index is actually a valid evaluation
		if ( index <= Pixel_TotalPixels_KLL )
		{
			*valid = 1;
		}

		// Lookup pixel, pixels are 1 indexed, hence the -1
		*elem = (PixelElement*)&Pixel_Mapping[ index - 1 ];
		return cur;
	}
	// Skip
	default:
		break;
	}

	return 0;
}



// -- Pixel Tweening --

uint16_t Pixel_pixelTweenNextPos( PixelElement *elem, PixelElement *prev )
{
	// No elem found
	// XXX (HaaTa) This is actually a hard problem for relative animations
	//             We may not find any element initially, so we don't know anothing to increment the position
	//             The best solution may be to just find a relatively nearby pixel and use that info...
	//             Or waste enough more flash...

	uint16_t ret = 0;

#if Pixel_HardCode_ChanWidth_define != 0 && Pixel_HardCode_Channels_define != 0
	// More efficient tweening, as we know the number of channels and width at compile time in all cases.
	ret = (
		( Pixel_HardCode_ChanWidth_define / 8 + sizeof( PixelChange ) )
		* Pixel_HardCode_Channels_define
	) + sizeof( PixelModElement );

#else
	// Determine tweening using nearby pixel definitions
	// First try the next element
	if ( elem != 0 )
	{
		ret = ( ( elem->width / 8 + sizeof( PixelChange ) ) * elem->channels ) + sizeof( PixelModElement );
		return ret;
	}

	// Next try the previous element
	if ( prev != 0 )
	{
		ret = ( ( prev->width / 8 + sizeof( PixelChange ) ) * prev->channels ) + sizeof( PixelModElement );
		return ret;
	}

	// BAD BAD BAD
	// TODO - This is BAD, will break in most cases, except for K-Type like keyboards.
	erro_print("Pixel Tween Bug!");
	ret = ( ( 8 / 8 + sizeof( PixelChange ) ) * 3 ) + sizeof( PixelModElement );
#endif

	return ret;
}

// Standard Pixel Pixel Function (standard lookup, no additonal processing)
void Pixel_pixelTweenStandard( const uint8_t *frame, AnimationStackElement *stack_elem )
{
	// Iterate over all of the Pixel Modifier elements of the Animation Frame
	uint16_t pos = 0;
	PixelModElement *mod = (PixelModElement*)&frame[pos];
	while ( mod->type != PixelAddressType_End )
	{
		// Lookup type of pixel, choose fill algorith and query all sub-pixels
		uint16_t next = 0;
		uint16_t valid = 0;
		PixelElement *prev_pixel_elem = 0;
		PixelElement *elem = 0;
		do {
			// Last element
			prev_pixel_elem = elem;

			// Lookup pixel, and check if there are any more pixels left
			next = Pixel_fillPixelLookup( mod, &elem, next, stack_elem, &valid );

			// Apply operation to pixel
			Pixel_pixelEvaluation( mod, elem );
		} while ( next );

		// Determine next position
		pos += Pixel_pixelTweenNextPos( elem, prev_pixel_elem );

		// Lookup next mod element
		mod = (PixelModElement*)&frame[pos];
	}
}

// Basic interpolation Pixel Pixel Function
// TODO - Only works with Colummn and Row fill currently
void Pixel_pixelTweenInterpolation( const uint8_t *frame, AnimationStackElement *stack_elem )
{
	// Iterate over all of the Pixel Modifier elements of the Animation Frame
	uint16_t pos = 0;
	PixelModElement *prev = 0;
	PixelModElement *mod = (PixelModElement*)&frame[pos];
	while ( mod->type != PixelAddressType_End )
	{
		// By default, no interpolation
		int32_t start = mod->index;
		int32_t end = mod->index;

		// Calculate interpolation position
		// TODO Depends on addressing type
		// TODO Work with dis-similar PixelModElement address types
		switch ( mod->type )
		{
		case PixelAddressType_ColumnFill:
			// If this is the first pixel, just set start at the same spot
			start = prev != 0 ? prev->rect.col : mod->rect.col;
			end = mod->rect.col;
			break;

		case PixelAddressType_RowFill:
			// If this is the first pixel, just set start at the same spot
			start = prev != 0 ? prev->rect.row : mod->rect.row;
			end = mod->rect.row;
			break;

		case PixelAddressType_Rect:
			// TODO - This is not correct (just a quick and dirty hack)
			if ( prev != 0 )
			{
				// TODO - Diagonal interpolation?
				if ( prev->rect.col != mod->rect.col )
				{
					start = prev->rect.col;
				}
				if ( prev->rect.row != mod->rect.row )
				{
					start = prev->rect.row;
				}
			}
			else
			{
				start = mod->rect.col;
			}
			//end = ( mod->rect.col + mod->rect.row ) / 2;
			end = mod->rect.col;
			break;

		case PixelAddressType_ScanCode:
			// If this is the first pixel, just set start at the same spot
			start = prev != 0 ? prev->index : mod->index;
			end = mod->index;
			break;

		case PixelAddressType_Index:
			// If this is the first pixel, just set start at the same spot
			start = prev != 0 ? prev->index : mod->index;
			end = mod->index;
			break;

		default:
			break;
		}

		// Lookup prev and mod PixelElements
		PixelElement *prev_elem = 0;
		uint16_t valid = 0;
		if ( prev != 0 )
		{
			Pixel_fillPixelLookup( prev, &prev_elem, 0, stack_elem, &valid );
		}
		PixelElement *mod_elem = 0;
		Pixel_fillPixelLookup( mod, &mod_elem, 0, stack_elem, &valid );

		// Data variable for PixelModElement
		// TODO (HaaTa) Allow for smaller bit widths than 8, and sizes larger than 24-bits
		const uint8_t data_max_size = sizeof( PixelModElement ) + ( sizeof( PixelModDataElement ) + 1 ) * 3;
		uint8_t interp_data[ data_max_size ];

		// Make sure mod_elem is pointing to something, if not, this could be a blank
		// In which case continue to the next definition
		if ( mod_elem == 0 )
		{
			goto next;
		}
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
		PixelElement *prev_pixel_elem = 0;
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
		PixelElement *elem = 0;

		// Prepare tweened PixelModElement
#if Pixel_HardCode_ChanWidth_define != 0 && Pixel_HardCode_Channels_define != 0
		// More efficient tweening, as we know the number of channels and width at compile time in all cases.
		uint8_t data_size = (
			( Pixel_HardCode_ChanWidth_define / 8 + sizeof( PixelChange ) )
			* Pixel_HardCode_Channels_define
		) + sizeof( PixelModElement );
#else
		uint8_t data_size = prev_elem != 0
			? sizeof( PixelModElement ) + ( sizeof( PixelModDataElement ) + prev_elem->width / 8 ) * prev_elem->channels
			: sizeof( PixelModElement ) + ( sizeof( PixelModDataElement ) + mod_elem->width / 8 ) * mod_elem->channels;
#endif
		if ( data_size > data_max_size )
		{
			warn_msg("Bad data size for this frame: ");
			printInt8( data_size );
			print(" instead of ");
			printInt8( data_max_size );
			print(NL);
			data_size = data_max_size;
		}
		PixelModElement *interp_mod = (PixelModElement*)&interp_data;
		memcpy( interp_mod, mod, data_size );

		// Calculate slice mulitplier size
		// TODO Non-8bit
		// XXX Division...
		uint16_t slice = prev != 0 ? 256 / (end - start + 1) : 0;

		// Iterate over tween-pixels
		for ( int32_t cur = 0; cur < end - start + 1; cur++ )
		{
			// Determine where the tween pixel is
			switch ( mod->type )
			{
			case PixelAddressType_ColumnFill:
				interp_mod->rect.col = start + cur;
				// Ignore invalid columns
				if ( interp_mod->rect.col >= Pixel_DisplayMapping_Cols_KLL || interp_mod->rect.col < 0 )
				{
					continue;
				}
				break;

			case PixelAddressType_RowFill:
				interp_mod->rect.row = start + cur;
				// Ignore invalid rows
				if ( interp_mod->rect.row >= Pixel_DisplayMapping_Rows_KLL || interp_mod->rect.row < 0 )
				{
					continue;
				}
				break;

			case PixelAddressType_Rect:
				// TODO - This is not correct (just a quick and dirty hack)
				interp_mod->rect.col = 0;
				interp_mod->rect.row = 0;
				if ( prev != 0 )
				{
					interp_mod->rect.col = prev->rect.col + cur;
					interp_mod->rect.row = 0;
					//interp_mod->rect.row = prev->rect.row + cur;
				}
				break;

			case PixelAddressType_ScanCode:
				interp_mod->index = start + cur;

				// Ignore un-assigned ScanCodes
				if ( Pixel_ScanCodeToDisplay[interp_mod->index - 1] == 0 )
				{
					continue;
				}
				break;

			case PixelAddressType_Index:
				interp_mod->index = start + cur;

				// Ignore unused pixels (this is uncommon)
				if ( Pixel_Mapping[interp_mod->index - 1].width == 0 || Pixel_Mapping[interp_mod->index - 1].channels == 0 )
				{
					continue;
				}
				break;

			default:
				break;
			}

			// Calculate interpolation pixel value
			// Uses prev to current PixelMods as the base
			// TODO Non-8bit
			if ( prev != 0 )
			{
				int32_t distance = slice * cur;
				for ( uint8_t ch = 0; ch < mod_elem->channels; ch++ )
				{
					uint8_t pos = ch * 2 + 1; // TODO Only works with 8 bit channels
					interp_mod->data[pos] = Pixel_8bitInterpolation(
						prev->data[pos],
						mod->data[pos],
						distance
					);
				}
			}

			// Lookup type of pixel, choose fill algorith and query all sub-pixels
			uint16_t next = 0;
			uint16_t valid = 0;
			do {
				// Previous element
				prev_pixel_elem = elem;

				// Lookup pixel, and check if there are any more pixels left
				next = Pixel_fillPixelLookup( interp_mod, &elem, next, stack_elem, &valid );

				// Apply operation to pixel if at a valid location
				if ( valid )
				{
					Pixel_pixelEvaluation( interp_mod, elem );
				}
			} while ( next );
		}

next:
		// This may have been a valid frame in an invalid position
		// Store it so we can still use it for interpolation purposes
		prev = mod;

		// Determine next position
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
		pos += Pixel_pixelTweenNextPos( elem, prev_pixel_elem );
#ifndef __clang__
#pragma GCC diagnostic pop
#endif

		// Lookup next mod element
		mod = (PixelModElement*)&frame[pos];
	}
}



// -- Frame Tweening --

// Standard Pixel Frame Function (no additional processing)
void Pixel_frameTweenStandard( const uint8_t *data, AnimationStackElement *elem )
{
	// Do nothing during sub-frames, skip
	if ( elem->subpos != 0 )
	{
		// But only if frame strech isn't set
		if ( !( elem->frameoption & PixelFrameOption_FrameStretch ) )
		{
			return;
		}
	}

	// Lookup Pixel Tweening Function
	switch ( elem->pfunc )
	{
	case PixelPixelFunction_PointInterpolation:
		Pixel_pixelTweenInterpolation( data, elem );
		break;

	// Generic, no addition processing necessary
	case PixelPixelFunction_Off:
	case PixelPixelFunction_PointInterpolationKLL:
		Pixel_pixelTweenStandard( data, elem );
		break;
	}
}

// Pixel Frame Interpolation Tweening
// Do averaging between key frames
void Pixel_frameTweenInterpolation( const uint8_t *data, AnimationStackElement *elem )
{
	// TODO
}



// -- Animation Control --

// Process the animation stack element
// - Returns 1 if the animation should be re-added to the stack
// - Returns 0 if the animation is finished (clean-up memory)
uint8_t Pixel_animationProcess( AnimationStackElement *elem )
{
	// Skip if index has been invalidate (unset)
	if ( elem->index == 0xFFFF )
	{
		return 0;
	}

	// Check the play state
	switch ( elem->state )
	{
	// Pause animation (paused animations will take up animation stack memory)
	case AnimationPlayState_Pause:
		return 1;

	// Stopping animation (frees animation from stack memory)
	case AnimationPlayState_Stop:
		// Indicate animation slot is free
		elem->index = 0xFFFF;
		return 0;

	// Single frame of the animation
	// Set to paused afterwards
	case AnimationPlayState_Single:
		elem->state = AnimationPlayState_Pause;
		break;

	// Do nothing
	case AnimationPlayState_Start:
	default:
		break;
	}

	// Lookup animation frame to make sure we have something to do
	// TODO Make sure animation index exists -HaaTa
	const uint8_t *data = Pixel_Animations[elem->index][elem->pos];

	// If there is no frame data, that means we either stop, or restart
	if ( data == 0 )
	{
		// Check if we still have more loops, one signifies stop, 0 is infinite
		if ( elem->loops == 0 || elem->loops-- > 1 )
		{
			elem->pos = 0;

			// Signal that animation is repeating
			Macro_animationState( elem->index, ScheduleType_Repeat );

			return Pixel_animationProcess( elem );
		}
		// Stop animation
		else
		{
			// Indicate animation slot is free
			elem->index = 0xFFFF;
			return 0;
		}
	}

	// Lookup Frame Tweening Function
	switch ( elem->ffunc )
	{
	case PixelFrameFunction_Interpolation:
		Pixel_frameTweenInterpolation( data, elem );
		break;

	// Generic, no additonal processing necessary
	case PixelFrameFunction_Off:
	case PixelFrameFunction_InterpolationKLL:
		Pixel_frameTweenStandard( data, elem );
		break;
	}

	// Increment positions
	// framedelay case
	if ( elem->framedelay > 0 )
	{
		// Roll-over subpos for framedelay
		if ( elem->subpos == elem->framedelay )
		{
			elem->subpos = 0;
			elem->pos++;
		}
		// Increment subposition
		else
		{
			elem->subpos++;
		}
	}
	// Full-speed
	else
	{
		elem->pos++;
	}

	return 1;
}



// -- Pixel Control --

// Debug function, used by cli only XXX
void Pixel_channelSet( uint16_t channel, uint32_t value )
{
	// Determine which buffer we are in
	PixelBuf *pixbuf = Pixel_bufferMap( channel );

	// Toggle channel accordingly
	switch ( pixbuf->width )
	{
	// Invalid width, default to 8
	default:
		warn_msg("ChanSet Unknown width: ");
		printInt8( pixbuf->width );
		print(" Ch: ");
		printHex( channel );
		print( NL );
		// Falls through on purpose

	// 0bit width - ignore/blank
	case 0:
		break;

	// 8bit width
	case 8:
		PixelBuf8( pixbuf, channel ) = (uint8_t)value;
		break;

	// 16bit width
	case 16:
		PixelBuf16( pixbuf, channel ) = (uint16_t)value;
		break;
	}
}

// Toggle the given channel
// Debug function, used by cli only XXX
void Pixel_channelToggle( uint16_t channel )
{
	// Determine which buffer we are in
	PixelBuf *pixbuf = Pixel_bufferMap( channel );

	// Toggle channel accordingly
	switch ( pixbuf->width )
	{
	// Invalid width, default to 8
	default:
		warn_msg("ChanToggle Unknown width: ");
		printInt8( pixbuf->width );
		print(" Ch: ");
		printHex( channel );
		print( NL );
		// Falls through on purpose

	// 0bit width - ignore/blank
	case 0:
		break;

	// 8bit width
	case 8:
		PixelBuf8( pixbuf, channel ) ^= 128;
		break;

	// 16bit width
	case 16:
		PixelBuf16( pixbuf, channel ) ^= 128;
		break;
	}
}

// Set each of the channels to a specific value
void Pixel_pixelSet( PixelElement *elem, uint32_t value )
{
	// Set each of the channels of the pixel
	for ( uint8_t ch = 0; ch < elem->channels; ch++ )
	{
		Pixel_channelSet( elem->indices[ch], value );
	}
}

// Toggle given pixel element
// Debug function, used by cli only XXX
void Pixel_pixelToggle( PixelElement *elem )
{
	// Toggle each of the channels of the pixel
	for ( uint8_t ch = 0; ch < elem->channels; ch++ )
	{
		Pixel_channelToggle( elem->indices[ch] );
	}
}



// -- Secondary Processing --

void Pixel_SecondaryProcessing_profile_init()
{
	// TODO (HaaTa): Only 3 profiles for now, may need more groups in the future
	for ( uint8_t group = 0; group < 3; group++ )
	{
		const PixelLEDGroupEntry entry = Pixel_LED_DefaultFadeGroups[group];

		// Iterate over each pixel
		for ( uint16_t pxin = 0; pxin < entry.size; pxin++ )
		{
			// For each pixel in the default settings, apply index
			// 0 specifies disabled, so all groups are +1
			Pixel_pixel_fade_profile[entry.pixels[pxin] - 1] = group + 1;
		}
	}
}

void Pixel_SecondaryProcessing_setup()
{
	// Set default gamma setting
	gamma_enabled = Pixel_gamma_default_define;

	// Disable all fade profiles (active defaults afterwards)
	memset( Pixel_pixel_fade_profile, 0, Pixel_TotalPixels_KLL );

	// Setup each of the default profiles
	Pixel_SecondaryProcessing_profile_init();

	// Setup default profile parameters
	for ( uint8_t pf = 0; pf < 4; pf++ )
	{
		// Each of the periods
		for ( uint8_t pr = 0; pr < 4; pr++ )
		{
			// Set period to profile
			PixelPeriodConfig conf = settings.fade_periods[pf][pr];
			Pixel_pixel_fade_profile_entries[pf].conf[pr] = conf;
		}

		// Reset state
		Pixel_pixel_fade_profile_entries[pf].pos = 0;
		Pixel_pixel_fade_profile_entries[pf].period_conf = PixelPeriodIndex_Off_to_On;
		Pixel_pixel_fade_profile_entries[pf].brightness = settings.fade_brightness[pf];
	}
}

// Given a starting value and profile, calculate the resulting brightness
// The returned value is always equal to or less than val
static inline uint32_t Pixel_ApplyFadeBrightness( uint8_t brightness, uint32_t val )
{
	// No need to calculate if brightness is max or 0
	if ( brightness == 255 )
	{
		return val;
	}
	if ( brightness == 0 )
	{
		return 0;
	}
	uint32_t result = (val * brightness) >> 8; // val * brightness / 255
	return result;
}

void Pixel_SecondaryProcessing()
{
	// Copy KLL buffer into LED buffer
	for ( uint8_t buf = 0; buf < Pixel_BuffersLen_KLL; buf++ )
	{
		memcpy(
			LED_Buffers[buf].data,
			Pixel_Buffers[buf].data,
			Pixel_Buffers[buf].size * ( Pixel_Buffers[buf].width >> 3 ) // Size may not be multiples bytes
		);
	}

	// Iterate over each of the pixels, applying the appropriate profile to each one
	for ( uint16_t pxin = 0; pxin < Pixel_TotalPixels_KLL; pxin++ )
	{
		// Select profile
		uint8_t profile_in = Pixel_pixel_fade_profile[pxin];

		// Nothing to do
		if ( profile_in == 0 )
		{
			continue;
		}

		// All profiles start from 1
		PixelFadeProfile *profile = &Pixel_pixel_fade_profile_entries[profile_in - 1];
		PixelPeriodConfig *period = &profile->conf[profile->period_conf];

		// Lookup channels of the pixel
		const PixelElement *elem = &Pixel_Mapping[pxin];
		for ( uint8_t ch = 0; ch < elem->channels; ch++ )
		{
			// Lookup PixelBuf containing the channel
			uint16_t chan = elem->indices[ch];
			PixelBuf *buf = LED_bufferMap( chan );

			// Lookup memory location
			// Then apply fade depending on the current position
			//
			// Percentage calculation using 32-bit integer instead of float
			// This is just a: pos / end * current value of LED
			// Ignores rounding
			// For 8-bit values, the maximum percentage spread must be no greater than 25-bits
			// e.g. 1 << 24
			uint32_t val;
			switch (buf->width)
			{
			// TODO (HaaTa): Handle non-16bit arrays of 8-bit values
			case 16:
				switch ( profile->period_conf )
				{
				// Off -> On
				case PixelPeriodIndex_Off_to_On:
				// On -> Off
				case PixelPeriodIndex_On_to_Off:
					// If start and end are set to 0, ignore
					if ( period->end == 0 && period->start == 0 )
					{
						val = (uint8_t)((uint16_t*)buf->data)[chan - buf->offset];
						val = Pixel_ApplyFadeBrightness(profile->brightness, val);
						if (gamma_enabled) {
							val = gamma_table[val];
						}
						((uint16_t*)buf->data)[chan - buf->offset] = (uint8_t)val;
						break;
					}

					val = (uint8_t)((uint16_t*)buf->data)[chan - buf->offset];
					val = Pixel_ApplyFadeBrightness(profile->brightness, val);
					if (gamma_enabled) {
						val = gamma_table[val];
					}
					val *= profile->pos;
					val >>= period->end;
					((uint16_t*)buf->data)[chan - buf->offset] = (uint8_t)val;
					break;
				// On hold time
				case PixelPeriodIndex_On:
					val = (uint8_t)((uint16_t*)buf->data)[chan - buf->offset];
					val = Pixel_ApplyFadeBrightness(profile->brightness, val);
					if (gamma_enabled) {
						val = gamma_table[val];
					}
					((uint16_t*)buf->data)[chan - buf->offset] = (uint8_t)val;
					break;
				// Off hold time
				case PixelPeriodIndex_Off:
				{
					PixelPeriodConfig *prev = &profile->conf[PixelPeriodIndex_On_to_Off];

					// If the previous config was disabled, do not set to 0
					if ( prev->start == 0 && prev->end == 0 )
					{
						val = (uint8_t)((uint16_t*)buf->data)[chan - buf->offset];
						val = Pixel_ApplyFadeBrightness(profile->brightness, val);
						if (gamma_enabled) {
							val = gamma_table[val];
						}
						((uint16_t*)buf->data)[chan - buf->offset] = (uint8_t)val;
						break;
					}

					// If the previous On->Off change didn't go to fully off
					// Calculate the value based off the previous config
					val = 0;
					if ( prev->start != 0 )
					{
						val = (uint8_t)((uint16_t*)buf->data)[chan - buf->offset];
						val = Pixel_ApplyFadeBrightness(profile->brightness, val);
						if (gamma_enabled) {
							val = gamma_table[val];
						}
						val *= (1 << prev->start) - 1;
						val >>= prev->end;
					}

					// Set to 0
					((uint16_t*)buf->data)[chan - buf->offset] = (uint8_t)val;
					break;
				}
				}
				break;
			default:
				erro_print("Unsupported buffer width");
				break;
			}
		}
	}

	// Increment positions of each of the active profiles
	for ( uint8_t proin = 0; proin < 4; proin++ )
	{
		// Lookup profile and current period
		PixelFadeProfile *profile = &Pixel_pixel_fade_profile_entries[proin];
		PixelPeriodConfig *period = &profile->conf[profile->period_conf];

		switch ( profile->period_conf )
		{
		case PixelPeriodIndex_Off_to_On:
			profile->pos++;
			// Check if we need to move onto the next conf
			if ( profile->pos >= (1 << period->end) )
			{
				profile->pos = (1 << profile->conf[PixelPeriodIndex_On].start) - 1;
				profile->period_conf = PixelPeriodIndex_On;
			}
			break;
		case PixelPeriodIndex_On:
			profile->pos++;
			// Check if we need to move onto the next conf
			if ( profile->pos >= (1 << period->end) )
			{
				profile->pos = (1 << profile->conf[PixelPeriodIndex_On_to_Off].end);
				profile->period_conf = PixelPeriodIndex_On_to_Off;
			}
			break;
		case PixelPeriodIndex_On_to_Off:
			profile->pos--;
			// Check if we need to move onto the next conf
			if ( profile->pos == (1 << period->start) - 1 )
			{
				profile->pos = (1 << profile->conf[PixelPeriodIndex_Off].start) - 1;
				profile->period_conf = PixelPeriodIndex_Off;
			}
			break;
		case PixelPeriodIndex_Off:
			profile->pos++;
			// Check if we need to move onto the next conf
			if ( profile->pos >= (1 << period->end) )
			{
				profile->pos = (1 << profile->conf[PixelPeriodIndex_Off_to_On].start) - 1;
				profile->period_conf = PixelPeriodIndex_Off_to_On;
			}
			break;
		}
	}
}



// -- General --

// Looks up the final scancode in a trigger macro
uint8_t Pixel_determineLastTriggerScanCode( TriggerMacro *trigger )
{
	// Ignore (set to zero) if unset
	if ( trigger == 0 )
	{
		return 0;
	}

	// Iterate over each TriggerGuide element
	uint8_t curScanCode = 0;
	for ( var_uint_t pos = 0; ; pos += trigger->guide[ pos ] * TriggerGuideSize + 1 )
	{
		// Length of this combo
		uint8_t comboLength = trigger->guide[ pos ] * TriggerGuideSize;

		// Determine scancode, use first TriggerGuide scanCode
		TriggerGuide *guide = (TriggerGuide*)&trigger->guide[ pos + 1 ];
		curScanCode = guide->scanCode;

		// If this combo has zero length, we are at the end
		if ( trigger->guide[ pos + comboLength + 1 ] == 0 )
		{
			return curScanCode;
		}
	}
}

// External Animation Control
void Pixel_setAnimationControl( AnimationControl control )
{
	Pixel_animationControl = control;
}

// Starting Animation setup
void Pixel_initializeStartAnimations()
{
	// Animations
	for ( uint8_t pos = 0; pos < Pixel_AnimationStackSize; pos++ )
	{
		uint8_t index = settings.animations[pos].index;
		if (index != 255) {
			AnimationStackElement element = Pixel_AnimationSettings[ index ];

			// Only update state if not already defined
			if ( element.state == AnimationPlayState_Pause )
			{
				element.state = AnimationPlayState_Start;
			}

			element.pos = settings.animations[pos].pos;
			Pixel_addAnimation( &element, CapabilityState_None );
		}
	}
}

// Pixel Procesing Loop
inline void Pixel_process()
{
	// Start latency measurement
	Latency_start_time( pixelLatencyResource );

	// Only update frame when ready
	switch( Pixel_FrameState )
	{
	case FrameState_Update:
	case FrameState_Pause:
		break;
	default:
		goto pixel_process_final;
	}

	// Pause animation if set
	switch ( Pixel_animationControl )
	{
	case AnimationControl_Forward:    // Ok
	case AnimationControl_ForwardOne: // Ok + 1, then stop
		Pixel_FrameState = FrameState_Update;
		break;
	case AnimationControl_Stop:       // Clear animations, then proceed forward
		Pixel_FrameState = FrameState_Update;
		Pixel_clearAnimations();
		Pixel_clearPixels();
		Pixel_animationControl = AnimationControl_Forward;
		break;
	case AnimationControl_Reset:      // Clear animations, then restart initial
		Pixel_FrameState = FrameState_Update;
		Pixel_clearAnimations();
		Pixel_clearPixels();
		Pixel_SecondaryProcessing_setup(); // Reset fade and gamma correction
		Pixel_animationControl = AnimationControl_Forward;
		Pixel_initializeStartAnimations();
		break;
	case AnimationControl_WipePause:  // Pauses animations, clears the display
		Pixel_animationControl = AnimationControl_Pause; // Update one more time
		Pixel_clearPixels();
		goto pixel_process_done;
	case AnimationControl_Clear: // Clears the display, animations continue
		Pixel_FrameState = FrameState_Update;
		Pixel_clearPixels();
		break;
	default: // Pause
		Pixel_FrameState = FrameState_Pause;
		goto pixel_process_final;
	}

	// First check if we are in a test mode
	switch ( Pixel_testMode )
	{
	// Single channel control
	case PixelTest_Chan_Single:
		// Increment channel
		if ( Pixel_testPos >= Pixel_TotalChannels_KLL )
			Pixel_testPos = 0;
		Pixel_testPos++;

		// Toggle channel
		Pixel_channelToggle( Pixel_testPos - 1 );

		// Disable test mode
		Pixel_testMode = PixelTest_Off;

		goto pixel_process_done;

	// Single channel control reverse
	case PixelTest_Chan_SingleReverse:
		// Make sure we don't start as 0
		if ( Pixel_testPos == 0 )
			Pixel_testPos++;

		// Decrement channel
		if ( Pixel_testPos == 1 )
			Pixel_testPos = Pixel_TotalChannels_KLL + 1;
		Pixel_testPos--;

		// Toggle channel
		Pixel_channelToggle( Pixel_testPos - 1 );

		// Disable test mode
		Pixel_testMode = PixelTest_Off;

		goto pixel_process_done;

	// Toggle current position, then increment
	case PixelTest_Chan_Roll:
		// Toggle channel
		Pixel_channelToggle( Pixel_testPos );

		// Increment channel
		Pixel_testPos++;
		if ( Pixel_testPos >= Pixel_TotalChannels_KLL )
			Pixel_testPos = 0;

		goto pixel_process_done;

	// Blink all channels
	case PixelTest_Chan_All:
		// Update all positions
		for ( uint16_t ch = 0; ch < Pixel_TotalChannels_KLL; ch++ )
		{
			// Toggle channel
			Pixel_channelToggle( ch );
		}

		goto pixel_process_done;

	// Enable all channels
	case PixelTest_Chan_Full:
		// Update all positions
		for ( uint16_t ch = 0; ch < Pixel_TotalChannels_KLL; ch++ )
		{
			// Toggle channel
			Pixel_channelSet( ch, 255 );
		}

		goto pixel_process_done;

	// Turn off all channels
	case PixelTest_Chan_Off:
		// Update all positions
		for ( uint16_t ch = 0; ch < Pixel_TotalChannels_KLL; ch++ )
		{
			// Toggle channel
			Pixel_channelSet( ch, 0 );
		}

		goto pixel_process_done;

	// Single pixel control
	case PixelTest_Pixel_Single:
		// Increment channel
		if ( Pixel_testPos >= Pixel_TotalPixels_KLL )
			Pixel_testPos = 0;
		Pixel_testPos++;

		// Toggle channel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ Pixel_testPos - 1 ] );

		// Disable test mode
		Pixel_testMode = PixelTest_Off;

		goto pixel_process_done;

	// Single pixel control reverse
	case PixelTest_Pixel_SingleReverse:
		// Make sure we don't start as 0
		if ( Pixel_testPos == 0 )
			Pixel_testPos++;

		// Decrement channel
		if ( Pixel_testPos == 1 )
			Pixel_testPos = Pixel_TotalPixels_KLL + 1;
		Pixel_testPos--;

		// Toggle channel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ Pixel_testPos - 1 ] );

		// Disable test mode
		Pixel_testMode = PixelTest_Off;

		goto pixel_process_done;

	// Toggle current position, then increment
	case PixelTest_Pixel_Roll:
		// Toggle pixel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ Pixel_testPos ] );

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= Pixel_TotalPixels_KLL )
			Pixel_testPos = 0;

		goto pixel_process_done;

	// Toggle all positions
	case PixelTest_Pixel_All:
		// Update all positions
		for ( uint16_t px = Pixel_testPos; px < Pixel_TotalPixels_KLL; px++ )
		{
			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ px ] );
		}

		goto pixel_process_done;

	// Enable all positions
	case PixelTest_Pixel_Full:
		// Update all positions
		for ( uint16_t px = 0; px < Pixel_TotalPixels_KLL; px++ )
		{
			// Toggle pixel
			// XXX (only works for 8 bit atm)
			Pixel_pixelSet( (PixelElement*)&Pixel_Mapping[ px ], 255 );
			Pixel_testMode = PixelTest_Off;
		}

		goto pixel_process_done;

	// Disable all positions
	case PixelTest_Pixel_Off:
		// Update all positions
		for ( uint16_t px = 0; px < Pixel_TotalPixels_KLL; px++ )
		{
			// Unset pixel
			Pixel_pixelSet( (PixelElement*)&Pixel_Mapping[ px ], 0 );
			Pixel_testMode = PixelTest_Off;
		}

		goto pixel_process_done;

	// Single scan control
	case PixelTest_Scan_Single:
	{
		// Lookup pixel
		uint16_t pixel = Pixel_ScanCodeToPixel[ Pixel_testPos ];

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= MaxScanCode_KLL )
			Pixel_testPos = 0;

		// Ignore if pixel set to 0
		if ( pixel == 0 )
		{
			goto pixel_process_final;
		}

		// Toggle channel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );

		// Disable test mode
		Pixel_testMode = PixelTest_Off;

		goto pixel_process_done;
	}
	// Single scan control reverse
	case PixelTest_Scan_SingleReverse:
	{
		// Lookup pixel
		uint16_t pixel = Pixel_ScanCodeToPixel[ Pixel_testPos ];

		// Increment pixel
		if ( Pixel_testPos == 0 )
			Pixel_testPos = MaxScanCode_KLL;
		Pixel_testPos--;

		// Ignore if pixel set to 0
		if ( pixel == 0 )
		{
			goto pixel_process_final;
		}

		// Toggle channel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );

		// Disable test mode
		Pixel_testMode = PixelTest_Off;

		goto pixel_process_done;
	}
	// Toggle current position, then increment
	case PixelTest_Scan_Roll:
	{
		// Lookup pixel
		uint16_t pixel = Pixel_ScanCodeToPixel[ Pixel_testPos ];

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= MaxScanCode_KLL )
			Pixel_testPos = 0;

		// Ignore if pixel set to 0
		if ( pixel == 0 )
		{
			goto pixel_process_final;
		}

		// Toggle channel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );

		goto pixel_process_done;
	}
	// Toggle all positions
	case PixelTest_Scan_All:
		for ( uint16_t px = Pixel_testPos; px < MaxScanCode_KLL; px++ )
		{
			// Lookup pixel
			uint16_t pixel = Pixel_ScanCodeToPixel[ px ];

			// Ignore if pixel set to 0
			if ( pixel == 0 )
			{
				continue;
			}

			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
		}

		goto pixel_process_done;

	// Toggle current position, then increment
	case PixelTest_XY_Roll:
	{
		// Lookup pixel
		uint16_t pixel = Pixel_DisplayMapping[ Pixel_testPos ];

		// Increment pixel
		Pixel_testPos++;
		if ( Pixel_testPos >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
			Pixel_testPos = 0;

		// Ignore if pixel set to 0
		if ( pixel == 0 )
		{
			goto pixel_process_final;
		}

		// Toggle channel
		Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );

		goto pixel_process_done;
	}
	// Toggle all positions
	case PixelTest_XY_All:
		for ( uint16_t px = Pixel_testPos; px < Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL; px++ )
		{
			// Lookup pixel
			uint16_t pixel = Pixel_DisplayMapping[ px ];

			// Ignore if pixel set to 0
			if ( pixel == 0 )
			{
				continue;
			}

			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
		}

		goto pixel_process_done;

	// Otherwise ignore
	default:
		break;
	}

	// Process Animation Stack
	Pixel_stackProcess();

	// Pause if necessary
	switch( Pixel_animationControl )
	{
	case AnimationControl_ForwardOne:
		Pixel_animationControl = AnimationControl_Pause;
		break;
	default:
		break;
	}

pixel_process_done:
	// Apply secondary LED processing
	// XXX (HaaTa): Disabling IRQ as a hack, some interrupt is causing corruption during the buffer handling
	Pixel_SecondaryProcessing();

	// Frame is now ready to send
	Pixel_FrameState = FrameState_Ready;

pixel_process_final:
	// End latency measurement
	Latency_end_time( pixelLatencyResource );
}


inline void Pixel_setup()
{
	// Register Pixel CLI dictionary
	CLI_registerDictionary( pixelCLIDict, pixelCLIDictName );

	// Iterate over starting animations
	uint8_t add_animations = 0;
	for ( uint32_t index = 0; index < Pixel_AnimationSettingsNum_KLL; index++ )
	{
		// Check if a starting animation
		if ( Pixel_AnimationSettings[ index ].state == AnimationPlayState_Start )
		{
			// Default animations are noted by the TriggerMacro *trigger pointer being set to 1
			if ( (uintptr_t)(Pixel_AnimationSettings[ index ].trigger) == 1 )
			{
				// Add animation to the defaults stack
#if Storage_Enable_define == 1
				defaults.animations[add_animations].index = index;
				defaults.animations[add_animations].pos = Pixel_AnimationSettings[index].pos;
#endif
				settings.animations[add_animations].index = index;
				settings.animations[add_animations].pos = Pixel_AnimationSettings[index].pos;
				add_animations++;
			}
		}
	}

	// Fill in rest of stack
	for ( uint8_t animation = add_animations; animation < Pixel_AnimationStackSize; animation++ )
	{
#if Storage_Enable_define == 1
		defaults.animations[animation].index = 255;
		defaults.animations[animation].pos = 0;
#endif
		settings.animations[animation].index = 255;
		settings.animations[animation].pos = 0;
	}

	// Setup fade defaults
	for ( uint8_t profile = 0; profile < 4; profile++ )
	{
		for ( uint8_t config = 0; config < 4; config++ )
		{
#if Storage_Enable_define == 1
			defaults.fade_periods[profile][config] =
				Pixel_LED_FadePeriods[Pixel_LED_FadePeriod_Defaults[profile][config]];
#endif
			settings.fade_periods[profile][config] =
				Pixel_LED_FadePeriods[Pixel_LED_FadePeriod_Defaults[profile][config]];
		}
#if Storage_Enable_define == 1
		defaults.fade_brightness[profile] = Pixel_LED_FadeBrightness[profile];
#endif
		settings.fade_brightness[profile] = Pixel_LED_FadeBrightness[profile];
	}

	// Register storage module
#if Storage_Enable_define == 1
	Storage_registerModule(&PixelStorage);
#endif

	// Set frame state to update
	Pixel_FrameState = FrameState_Update;

	// Disable test modes by default, start at position 0
	Pixel_testMode = Pixel_Test_Mode_define;

	// Clear animation stack and Pixels
	Pixel_clearAnimations();
	Pixel_clearPixels();

	// Set default animation control
	Pixel_animationControl = AnimationControl_Forward;

	// Add initial animations
	Pixel_initializeStartAnimations();

	// Initialize secondary buffer processing
	Pixel_SecondaryProcessing_setup();

	// Allocate latency resource
	pixelLatencyResource = Latency_add_resource("PixelMap", LatencyOption_Ticks);
}


// ----- CLI Command Functions -----

void cliFunc_pixelList( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'b':
	case 'B':
		info_msg("Buffer List");

		// List all buffers
		for ( uint8_t buf = 0; buf < Pixel_BuffersLen_KLL; buf++ )
		{
			print( NL "\t" );
			printInt8( buf );
			print(":");
			printHex32( (uint32_t)(uintptr_t)(Pixel_Buffers[ buf ].data) );
			print(":width(");
			printInt8( Pixel_Buffers[ buf ].width );
			print("):size(");
			printInt8( Pixel_Buffers[ buf ].size );
			print(")");
		}
		break;

	default:
		info_msg("Pixel List - <num>[<ch1>,...]<width>:...");

		// List all pixels
		for ( uint16_t pixel = 0; pixel < Pixel_TotalPixels_KLL; pixel++ )
		{
			// NL occaisionally
			if ( pixel % 5 == 0 )
				print( NL );

			PixelElement *elem = (PixelElement*)&Pixel_Mapping[ pixel ];

			printHex_op( pixel, 2 );
			print(":");
			printInt8( elem->width );
			print("[");

			// Display each of the channels
			printHex_op( elem->indices[0], 2 );
			for ( uint8_t ch = 1; ch < elem->channels; ch++ )
			{
				print(",");
				printHex_op( elem->indices[ch], 2 );
			}

			print("]");
			print("  ");
		}

		break;
	}
}

void cliFunc_pixelTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'a':
	case 'A':
		info_msg("All pixel test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Pixel_All;
		return;

	case 'r':
	case 'R':
		info_msg("Pixel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Pixel_Roll;
		return;

	case 's':
	case 'S':
		info_msg("Stopping pixel test");
		Pixel_testMode = PixelTest_Off;
		return;

	case 'f':
	case 'F':
		info_msg("Enable all pixels");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Pixel_Full;
		return;

	case 'o':
	case 'O':
		info_msg("Disable all pixels");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Pixel_Off;
		return;

	default:
		Pixel_testMode = PixelTest_Pixel_Single;
		break;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}

	// If 0, ignore
	if ( Pixel_testPos == 0 )
	{
		Pixel_testMode = PixelTest_Off;
		return;
	}

	// Debug info
	print( NL );
	info_msg("Pixel: ");
	printInt16( Pixel_testPos );
	print(" ");
}

void cliFunc_chanTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'a':
	case 'A':
		info_msg("All channel test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_All;
		return;

	case 'r':
	case 'R':
		info_msg("Channel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_Roll;
		return;

	case 's':
	case 'S':
		info_msg("Stopping channel test");
		Pixel_testMode = PixelTest_Off;
		return;

	case 'f':
	case 'F':
		info_msg("Enable all pixels");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_Full;
		return;

	case 'o':
	case 'O':
		info_msg("Disable all pixels");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Chan_Off;
		return;

	default:
		Pixel_testMode = PixelTest_Chan_Single;
		break;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}

	// Debug info
	print( NL );
	info_msg("Channel: ");
	printInt16( Pixel_testPos );
	print( NL );
}

void cliFunc_pixelSCTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'a':
	case 'A':
		info_msg("All scancode pixel test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Scan_All;
		return;

	case 'r':
	case 'R':
		info_msg("Scancode pixel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_Scan_Roll;
		return;

	case 's':
	case 'S':
		info_msg("Stopping scancode pixel test");
		Pixel_testMode = PixelTest_Off;
		return;

	default:
		Pixel_testMode = PixelTest_Scan_Single;
		break;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}

	// If 0, ignore
	if ( Pixel_testPos == 0 )
	{
		Pixel_testMode = PixelTest_Off;
		return;
	}

	// Lookup pixel
	uint16_t pixel = Pixel_ScanCodeToPixel[ Pixel_testPos ];

	// Debug info
	print( NL );
	info_msg("ScanCode: ");
	printInt16( Pixel_testPos + 1 );
	print(" Pixel: ");
	printInt16( pixel );
	print(" ");
}

void cliFunc_pixelXYTest( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	char* curArgs;
	char* arg1Ptr;
	char* arg2Ptr = args;

	// Process argument if given
	curArgs = arg2Ptr;
	CLI_argumentIsolation( curArgs, &arg1Ptr, &arg2Ptr );

	// Check for special args
	switch ( *arg1Ptr )
	{
	case 'a':
	case 'A':
		info_msg("All x,y pixel test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_XY_All;
		return;

	case 'r':
	case 'R':
		info_msg("x,y pixel roll test");
		Pixel_testPos = 0;
		Pixel_testMode = PixelTest_XY_Roll;
		return;

	case 's':
	case 'S':
		info_msg("Stopping x,y pixel test");
		Pixel_testMode = PixelTest_Off;
		return;

	case 'h':
	case 'H':
		// Make sure we aren't too far already
		if ( Pixel_testPos >= Pixel_DisplayMapping_Rows_KLL )
		{
			Pixel_testPos = 0;
		}

		info_msg("Horizontal: ");
		printInt16( Pixel_testPos );

		// Iterate over the row
		for ( uint16_t pos = Pixel_DisplayMapping_Cols_KLL * Pixel_testPos;
			pos < Pixel_DisplayMapping_Cols_KLL * ( Pixel_testPos + 1);
			pos++
		)
		{
			uint16_t pixel = Pixel_DisplayMapping[ pos ];
			print(" ");
			printInt16( pixel );

			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
		}
		Pixel_testPos++;

		return;

	case 'v':
	case 'V':
		// Make sure we aren't too far already
		if ( Pixel_testPos >= Pixel_DisplayMapping_Cols_KLL )
		{
			Pixel_testPos = 0;
		}

		info_msg("Vertical: ");
		printInt16( Pixel_testPos );

		// Iterate over the column
		for ( uint16_t pos = 0; pos < Pixel_DisplayMapping_Rows_KLL; pos++ )
		{
			uint16_t pos_calc = pos * Pixel_DisplayMapping_Cols_KLL + Pixel_testPos;
			print(" ");
			printInt16( pos_calc );
			uint16_t pixel = Pixel_DisplayMapping[ pos_calc ];
			print(":");
			printInt16( pixel );

			// Toggle pixel
			Pixel_pixelToggle( (PixelElement*)&Pixel_Mapping[ pixel - 1 ] );
		}
		Pixel_testPos++;

		return;
	}

	// Check for specific position
	if ( *arg1Ptr != '\0' )
	{
		Pixel_testPos = numToInt( arg1Ptr );
	}

	// Lookup pixel
	uint16_t pixel = Pixel_DisplayMapping[ Pixel_testPos ];

	// Debug info
	print( NL );
	info_msg("Position (x,y): ");
	printInt16( Pixel_testPos % Pixel_DisplayMapping_Cols_KLL );
	print(",");
	printInt16( Pixel_testPos / Pixel_DisplayMapping_Cols_KLL );
	print(":");
	printInt16( Pixel_testPos );
	print(" Pixel: ");
	printInt16( pixel );
	print(" ");

	// Lookup pixel element
	PixelElement *elem = (PixelElement*)&Pixel_Mapping[ pixel - 1 ];
	Pixel_showPixelElement( elem );
	print( NL );

	// Increment pixel
	Pixel_testPos++;
	if ( Pixel_testPos >= Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL )
		Pixel_testPos = 0;

	// Ignore if pixel set to 0
	if ( pixel == 0 )
	{
		return;
	}

	// Toggle pixel
	Pixel_pixelToggle( elem );
}
void cliFunc_aniAdd( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// TODO
	/*
	uint16_t index = Pixel_AnimationStack.size;
	Pixel_AnimationStack.stack[index].index = 1;
	Pixel_AnimationStack.stack[index].pos = 1;
	Pixel_AnimationStack.stack[index].loops = 1;
	Pixel_AnimationStack.stack[index].divider = 0;
	Pixel_AnimationStack.stack[index].modifier = AnimationModifier_None;
	Pixel_AnimationStack.size++;
	*/
}

void cliFunc_aniDel( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// TODO
	Pixel_AnimationStack.size--;
}

void cliFunc_aniStack( char* args )
{
	print(NL);
	info_msg("Stack Size: ");
	printInt16( Pixel_AnimationStack.size );
	for ( uint8_t pos = 0; pos < Pixel_AnimationStack.size; pos++ )
	{
		print(NL);
		AnimationStackElement *elem = Pixel_AnimationStack.stack[pos];
		print(" index(");
		printInt16( elem->index );
		print(") pos(");
		printInt16( elem->pos );
		print(") loops(");
		printInt8( elem->loops );
		print(") framedelay(");
		printInt8( elem->framedelay );
		print(") frameoption(");
		printInt8( elem->frameoption );
		print(") ffunc(");
		printInt8( elem->ffunc );
		print(") pfunc(");
		printInt8( elem->pfunc );
		print(")");
	}
}

void Pixel_dispBuffer()
{
	uint8_t row = 0;
	uint8_t col = 0;
	for ( uint16_t px = 0; px < Pixel_DisplayMapping_Cols_KLL * Pixel_DisplayMapping_Rows_KLL; px++ )
	{
		// Display a + if it's a blank pixel
		if ( Pixel_DisplayMapping[px] == 0 )
		{
			print("+");
		}
		// Lookup pixel
		else
		{
			// Determine number of channels
			// TODO Adjust output if single channel

			PixelElement *elem = (PixelElement*)&Pixel_Mapping[ Pixel_DisplayMapping[px] - 1 ];

			// Lookup channel data
			// TODO account for different channel size mappings
			print("\033[48;2");
			for ( uint8_t ch = 0; ch < elem->channels; ch++ )
			{
				print(";");
				uint16_t ch_pos = elem->indices[ch];
				PixelBuf *pixbuf = Pixel_bufferMap( ch_pos );
				printInt8( PixelBuf16( pixbuf, ch_pos ) );
			}
			print("m");
			print(" ");
			print("\033[0m");
		}

		// Determine what to increment next
		if ( col >= Pixel_DisplayMapping_Cols_KLL - 1 )
		{
			col = 0;
			row++;
			print(" ");
			print(NL);
		}
		else
		{
			col++;
		}
	}
}

void cliFunc_rectDisp( char* args )
{
	print( NL ); // No \r\n by default after the command is entered

	// TODO move to own function, use this func to control startup/args
	Pixel_dispBuffer();
}


#if Storage_Enable_define == 1
void Pixel_loadConfig()
{
	// Animation setup
	Pixel_initializeStartAnimations();

	// Fade periods
	Pixel_SecondaryProcessing_setup();
}

void Pixel_saveConfig() {
	// Animations
	for ( uint8_t pos = 0; pos < Pixel_AnimationStackSize; pos++ )
	{
		if (pos < Pixel_AnimationStack.size) {
			AnimationStackElement *elem = Pixel_AnimationStack.stack[pos];
			settings.animations[pos].index = elem->index;

			// Save position, only if paused
			if ( elem->state == AnimationPlayState_Pause )
			{
				settings.animations[pos].pos = elem->pos - 1;
			}
			else
			{
				settings.animations[pos].pos = 0;
			}
		} else {
			settings.animations[pos].index = 255;
			settings.animations[pos].pos = 0;
		}
	}

	// Fade periods
	for (uint8_t profile=0; profile<4; profile++)
	{
		for (uint8_t config=0; config<4; config++)
		{
			// XXX TODO: Needs a real lookup
			const PixelPeriodConfig period_config = Pixel_pixel_fade_profile_entries[profile].conf[config];
			settings.fade_periods[profile][config] = period_config;
		}
		settings.fade_brightness[profile] = Pixel_pixel_fade_profile_entries[profile].brightness;
	}
}

void Pixel_printConfig() {
	// Animations
	print(" \033[35mAnimations\033[0m" NL);
	for ( uint8_t pos = 0; pos < Pixel_AnimationStackSize; pos++ )
	{
		uint8_t index = settings.animations[pos].index;
		uint8_t fpos = settings.animations[pos].pos;
		if (index != 255) {
			print("AnimationStack.stack[");
			printInt8(pos);
			print("]->index = ");
			printInt8(index);
			print("; pos = ");
			printInt8(fpos);
			print(NL);
		}
	}

	// Fade periods
	print(NL " \033[35mFades\033[0m" NL);
	for (uint8_t profile=0; profile<4; profile++)
	{
		for (uint8_t config=0; config<4; config++)
		{
			PixelPeriodConfig period = settings.fade_periods[profile][config];
			print("FadeConfig[");
			printInt8(profile);
			print("][");
			printInt8(config);
			print("] = {");
			printInt8(period.start);
			print(", ");
			printInt8(period.end);
			print("}");
			print(NL);
		}
	}

	// Profile brightness
	print(NL " \033[35mProfile Brightnesses\033[0m" NL);
	for (uint8_t profile=0; profile<4; profile++)
	{
		printInt8(profile);
		print(" ");
		printInt8(settings.fade_brightness[profile]);
		print(NL);
	}
}
#endif
