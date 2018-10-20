'''
Host-Side Python Commands for TestIn Scan Module
'''

# Copyright (C) 2016-2018 by Jacob Alexander
#
# This file is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file.  If not, see <http://www.gnu.org/licenses/>.

### Imports ###

import builtins
import copy
import sys
import time

from collections import namedtuple
from ctypes import (
    byref,
    c_char_p,
    c_uint8,
    c_uint16,
    c_uint32,
    c_void_p,
    cast,
    create_string_buffer,
    CFUNCTYPE,
    POINTER,
    Structure,
)

import kiilogger



### Logger ###

logger = kiilogger.get_logger('Scan/TestIn/host.py')



### Variables ###

data = builtins.kiibohd_data
debug = False
control = builtins.kiibohd_control



### Enums ###

class ScheduleState:
    '''
    See kll.h ScheduleState
    '''
    P      = 0x01 # Press
    H      = 0x02 # Hold
    R      = 0x03 # Release
    O      = 0x00 # Off
    UP     = 0x04 # Unique Press
    UR     = 0x05 # Unique Release

    A      = 0x01 # Activate
    On     = 0x02 # On
    D      = 0x03 # Deactivate
    Off    = 0x00 # Off

    Done   = 0x06 # Done
    Repeat = 0x07 # Repeat

    Shift  = 0x10 # Shift
    Latch  = 0x20 # Latch
    Lock   = 0x40 # Lock

    Debug  = 0xFF # Print capability name


class TriggerType:
    '''
    See kll.h TriggerType
    '''
    Switch1    = 0x00
    Switch2    = 0x01
    Switch3    = 0x02
    Switch4    = 0x03
    LED1       = 0x04
    Analog1    = 0x05
    Analog2    = 0x06
    Analog3    = 0x07
    Analog4    = 0x08
    Layer1     = 0x09
    Layer2     = 0x0A
    Layer3     = 0x0B
    Layer4     = 0x0C
    Animation1 = 0x0D
    Animation2 = 0x0E
    Animation3 = 0x0F
    Animation4 = 0x10
    Sleep1     = 0x11
    Resume1    = 0x12
    Inactive1  = 0x13
    Active1    = 0x14
    Rotation1  = 0x15

    Debug      = 0xFF


class LayerStateType:
    '''
    See kll.h LayerStateType
    '''
    Off   = 0x00
    Shift = 0x01
    Latch = 0x02
    Lock  = 0x04



### Structures ###

class TriggerMacro( Structure ):
    '''
    C-Struct for TriggerMacro
    See Macro/PartialMap/kll.h
    '''
    _fields_ = [
        ( "guide",  POINTER( control.var_uint_t ) ),
        ( "result", c_uint8 ),
    ]

    def __repr__(self):
        val = "(guide={}, result={})".format(
            self.guide,
            self.result,
        )
        return val


class ResultCapabilityStackItem( Structure ):
    '''
    C-Struct for ResultCapabilityStackItem
    See Macro/PartialMap/result.c
    '''
    _fields_ = [
        ( "trigger",         POINTER( TriggerMacro ) ),
        ( "state",           c_uint8 ),
        ( "stateType",       c_uint8 ),
        ( "capabilityIndex", c_uint8 ),
        ( "args",            POINTER( c_uint8 ) ),
    ]

    def copy(self):
        '''
        @returns: Copy of object
        '''
        val = ResultCapabilityStackItem()
        val.trigger = self.trigger
        val.state = copy.copy(self.state)
        val.stateType = copy.copy(self.stateType)
        val.capabilityIndex = copy.copy(self.capabilityIndex)
        val.args = self.args
        return val

    def read_capability(self):
        '''
        Using kll.json, determine capability

        @returns: Name of capability, Arguments
        '''
        capability_dict = control.json_input['Capabilities']

        # Retrieve capability information using index
        arg_definitions = None
        name = None
        for key, cap in capability_dict.items():
            if cap['index'] == self.capabilityIndex:
                name = key
                arg_definitions = cap['args']
                break

        assert name is not None, "Could not find capability index {}".format(self.capabilityIndex)

        return name, arg_definitions

    def read_args(self):
        '''
        Using kll.json, read the arguments from the args pointer.

        @returns: List of arguments
        '''
        name, arg_definitions = self.read_capability()

        # Build of args for namedtuple
        arg_names = [arg['name'] for arg in arg_definitions]
        ntuple = namedtuple(name, arg_names)

        # Build list of extracted arguments
        arg_values = []
        position = 0
        for arg in arg_definitions:
            value = None

            # Cast based on the width
            if arg['width'] == 1:
                value = cast(byref(self.args.contents, position), POINTER(c_uint8)).contents
                position += 1
            elif arg['width'] == 2:
                value = cast(byref(self.args.contents, position), POINTER(c_uint16)).contents
                position += 2
            elif arg['width'] == 4:
                value = cast(byref(self.args.contents, position), POINTER(c_uint32)).contents
                position += 4
            else:
                logger.error("Unknown width: {}", arg['width'])

            arg_values.append(value)

        # Build namedtuple with read values
        arg_values = tuple(arg_values)
        return ntuple(*arg_values)

    def __repr__(self):
        '''
        Representation of ResultCapabilityStackItem
        '''
        val = "(trigger={}, state={}, stateType={}, capabilityIndex={}, capabilityName={}, args={})".format(
            self.trigger.contents,
            self.state,
            self.stateType,
            self.capabilityIndex,
            self.read_capability()[0],
            self.read_args(),
        )
        return val


class AnimationStackElement( Structure ):
    '''
    C-Struct for AnimationStackElement
    See Macro/PixelMap/pixel.h
    '''
    _fields_ = [
        ( "trigger",     POINTER( TriggerMacro ) ),
        ( "index",       c_uint16 ),
        ( "pos",         c_uint16 ),
        ( "subpos",      c_uint8 ),
        ( "loops",       c_uint8 ),
        ( "framedelay",  c_uint8 ),
        ( "frameoption", c_uint8 ),
        ( "ffunc",       c_uint8 ),
        ( "pfunc",       c_uint8 ),
        ( "replace",     c_uint8 ),
        ( "state",       c_uint8 ),
    ]

    def frameoption_lookup(self):
        '''
        Do lookup on frameoption

        Options are set using each bit

        @returns: String name of frameoption index
        '''
        output = []
        if self.frameoption & 1:
            output.append('framestretch')

        return output

    def ffunc_lookup(self):
        '''
        Do lookup on ffunc (Frame Function)

        @returns: String name of ffunc index
        '''
        lookup = {
            0: 'off',
        }
        return lookup[self.ffunc]

    def pfunc_lookup(self):
        '''
        Do lookup on pfunc (Pixel Function)

        @returns: String name of pfunc index
        '''
        lookup = {
            0: 'off',
            1: 'interp',
        }
        return lookup[self.pfunc]

    def __repr__(self):
        val = "(trigger={}, index={}, pos={}, subpos={}, loops={}, framedelay={}, frameoption={}, ffunc={}, pfunc={})".format(
            self.trigger.contents,
            self.index,
            self.pos,
            self.subpos,
            self.loops,
            self.framedelay,
            self.frameoption_lookup(),
            self.ffunc_lookup(),
            self.pfunc_lookup(),
        )
        return val


class PixelBuf( Structure ):
    '''
    C-Struct for PixelBuf
    See Macro/PixelMap/pixel.h
    '''
    _fields_ = [
        ( "size",   c_uint8 ),
        ( "width",  c_uint8 ),
        ( "offset", c_uint16 ),
        ( "data",   c_void_p ),
    ]

    def __repr__(self):
        val = "(size={}, width={}, offset={}, data={})".format(
            self.size,
            self.width,
            self.offset,
            self.data,
        )
        return val


class Capability(Structure):
    '''
    C-Struct for Capability
    See Macro/PartialMap/kll.h
    '''
    _fields_ = [
        ("func",     c_void_p),
        ("argCount", c_uint8),
        ("features", c_uint8),
    ]

    def __repr__(self):
        val = "(func={}, argCount={} features={})".format(
            self.func,
            self.argCount,
            self.features,
        )
        return val



### Classes ###

class Commands:
    '''
    Container class of commands available to controll the host-side KLL implementation
    '''

    def addScanCode( self, index, index_type=TriggerType.Switch1 ):
        '''
        Adds a Scan Code to the internal KLL buffer

        Returns 1 if added, 0 if the ScanCode is already in the buffer
        Returns 2 if there's an error
        Generally 1 will be the return
        '''
        return control.kiibohd.Scan_addScanCode( int( index ), int( index_type ) )

    def removeScanCode( self, index, index_type=TriggerType.Switch1 ):
        '''
        Removes a Scan Code from the internal KLL buffer
        Ignored if the Scan Code was not in the buffer

        Returns 1 if added, 0 if the ScanCode is already in the buffer
        Returns 2 if there's an error
        Generally 0 will be the return
        '''
        return control.kiibohd.Scan_removeScanCode( int( index ), int( index_type ) )

    def setMacroDebugMode( self, debugmode ):
        '''
        Sets macroDebugMode

        0 - Disable (default)
        1 - Disable USB Output, show debug
        2 - Enabled USB Output, show debug
        3 - Disable USB Output
        '''
        cast( control.kiibohd.macroDebugMode, POINTER( c_uint8 ) )[0] = debugmode

    def setVoteDebugMode( self, debugmode ):
        '''
        Sets voteDebugMode

        0 - Disable (default)
        1 - Show result of each vote
        '''
        cast( control.kiibohd.voteDebugMode, POINTER( c_uint8 ) )[0] = debugmode

    def setLayerDebugMode( self, debugmode ):
        '''
        Sets layerDebugMode

        0 - Disable (default)
        1 - Show result of layer change
        '''
        cast( control.kiibohd.layerDebugMode, POINTER( c_uint8 ) )[0] = debugmode

    def setTriggerPendingDebugMode( self, debugmode ):
        '''
        Sets triggerPendingDebugMode

        0 - Disable (default)
        1 - Show pending triggers before evaluating
        '''
        cast( control.kiibohd.triggerPendingDebugMode, POINTER( c_uint8 ) )[0] = debugmode

    def applyLayer( self, state, layer, layer_state ):
        '''
        Applies a given layer with a layer_state
        If state is already applied, it will be unset.

        @param state: Input state used to apply layer (e.g. Press/Release)
        @param layer: Layer index
        @param layer_state: LayerStateType value
        '''
        trigger = 0
        state_type = TriggerType.Switch1
        control.kiibohd.Layer_layerStateSet(int(trigger), int(state), int(state_type), int(layer), int(layer_state))
        self.recordLayerState()

    def lockLayer( self, layer ):
        '''
        Lock specified layer

        @param layer: Layer index to lock
        '''
        trigger = 0
        state = ScheduleState.P
        state_type = TriggerType.Switch1
        layer_state = LayerStateType.Lock
        control.kiibohd.Layer_layerStateSet(int(trigger), int(state), int(state_type), int(layer), int(layer_state))
        self.recordLayerState()

    def clearLayers( self ):
        '''
        Clears all layer state
        i.e. Sets to default layer state
        '''
        control.kiibohd.Layer_clearLayers()
        self.recordLayerState()

    def getLayerState( self ):
        '''
        Retrieves current layer state

        @return: LayerStateInstance namedtuple of state and stack
        '''
        # Gather layer state data
        layerStateData = []
        layerNum = cast(control.kiibohd.LayerNum_host, POINTER(c_uint32)).contents.value
        layerState = cast(control.kiibohd.LayerState, POINTER(c_uint8 * layerNum)).contents
        for layer in layerState:
            layerStateData.append(layer)

        # Gather layer stack data
        layerStackData = []
        layerStackSize = cast(control.kiibohd.macroLayerIndexStackSize, POINTER(control.index_uint_t)).contents.value
        layerStack = cast(control.kiibohd.macroLayerIndexStack, POINTER(control.index_uint_t * layerStackSize)).contents
        for layer in layerStack:
            layerStackData.append(layer)

        ntuple = namedtuple('LayerStateInstance', ['state', 'stack', 'time'])
        value = ntuple(layerStateData, layerStackData, time.time())
        return value

    def recordLayerState( self ):
        '''
        Used to record layer state when using Python API to modify the layers
        Necessary to maintain proper layer history
        '''
        data.layer_history.add(self.getLayerState())

    def clearMacroTriggerEventBuffer( self ):
        '''
        Clears the macroTriggerEventBuffer to make sure no old events are processed.
        '''
        cast( control.kiibohd.macroTriggerEventBufferSize, POINTER( control.var_uint_t ) )[0] = 0

    def addAnimation( self, name=None, index=0, pos=0, loops=1, divmask=0x0, divshift=0x0, ffunc=0, pfunc=0 ):
        '''
        Adds a given animation (by index) to the processing loop

        @param index:    Animation index
        @param pos:      Starting frame position
        @param loops:    Number of loops, 0 is infinite
        @param divmask:  Sub-frame position mask (frame vs. subframe)
        @param divshift: Frame position shift
        @param ffunc:    Frame tweening function
        @param pfunc:    Pixel tweening function

        Returns 1 if added, 0 if it failed to allocate memory
        Generally 0 will be the return value
        '''
        # Read AnimationStackElement size
        size = cast( control.kiibohd.Pixel_AnimationStackElement_HostSize, POINTER( c_uint8 ) )[0]
        control.refresh_callback()

        # If a name is given, do lookup using json
        if name is not None:
            lookup = control.json_input['AnimationIds']
            if name not in lookup.keys():
                logger.error("'{}' is an invalid animation id.", name)
            else:
                index = lookup[ name ]

        # Allocate memory for AnimationStackElement struct
        elem = cast( create_string_buffer( size ), POINTER( AnimationStackElement ) )
        elem[0].index = index
        elem[0].pos = pos
        elem[0].loops = loops
        elem[0].divmask = divmask
        elem[0].divshift = divshift
        elem[0].ffunc = ffunc
        elem[0].pfunc = pfunc

        return control.kiibohd.Pixel_addAnimation( elem )

    def animationStackInfo( self ):
        '''
        Returns the current animation stack and size
        '''
        # Read in size (needed to read stack array)
        size = cast( control.kiibohd.Pixel_AnimationStack_HostSize, POINTER( c_uint16 ) )[0]

        # Update struct to use read size in order to determine size of stack
        class AnimationStack( Structure ):
            '''
            C-Struct for AnimationStack
            See Macro/PixelMap/pixel.h
            '''

            def __repr__(self):
                val = "(size={}, stack={})".format(
                    self.size,
                    self.stack,
                )
                return val

        # Dynamic size for stack
        AnimationStack._fields_ = [
            ( "size",  c_uint16 ),
            ( "stack", POINTER( AnimationStackElement * size ) ),
        ]

        # Cast animation stack
        stack = cast( control.kiibohd.Pixel_AnimationStack, POINTER( AnimationStack ) )[0]

        return stack

    def setAnimationControl( self, state ):
        '''
        Set the PixelMap AnimationState
        NOTE: You must do one processing cycle in order for this setting to take hold.

        0 - AnimationControl_Forward (Default state)
        1 - AnimationControl_ForwardOne
        2 - AnimationControl_Pause
        3 - AnimationControl_Stop (Very useful in cleaning up the current PixelMap state)
        4 - AnimationControl_Reset
        5 - AnimationControl_WipePause
        '''
        cast( control.kiibohd.Pixel_animationControl, POINTER( c_uint8 ) )[0] = state

    def animationDisplayBuffers( self ):
        '''
        Returns a list of display buffers currently allocated
        '''
        # Query number of buffers
        num_buffers = cast( control.kiibohd.Pixel_Buffers_HostLen, POINTER( c_uint8 ) )[0]

        # Read in array of PixelBufs
        pixelbufs = cast( control.kiibohd.Pixel_Buffers, POINTER( PixelBuf * num_buffers ) )[0]

        # Build list of list of buffer elements
        outputbufs = []
        for buf in pixelbufs:
            if buf.width == 8:
                data = cast( buf.data, POINTER( c_uint8 * buf.size ) )[0]
            elif buf.width == 16:
                data = cast( buf.data, POINTER( c_uint16 * buf.size ) )[0]
            elif buf.width == 32:
                data = cast( buf.data, POINTER( c_uint32 * buf.size ) )[0]
            outputbufs.append( ( [ elem for elem in data ], buf.offset, buf.size ) )

        return outputbufs

    def setFrameState( self, state ):
        '''
        Set the PixelMap FrameState
        0 - FrameState_Ready
        1 - FrameState_Sending
        2 - FrameState_Update

        For host-side KLL, this needs to be set to FrameState_Update on each loop iteration.
        It is used to delay updating the buffer until the target device has copied out the data
        '''
        cast( control.kiibohd.Pixel_FrameState, POINTER( c_uint8 ) )[0] = state

    def readPixel( self, index ):
        '''
        Reads pixel at index
        May return N number of channels depending on the pixel definition
        '''
        # Determine indices width
        indices_width = cast( control.kiibohd.Pixel_MaxChannelPerPixel_Host, POINTER( c_uint8 ) )[0]

        # Update struct according to indices_width
        class PixelElement( Structure ): pass
        PixelElement._fields_ = [
            ( "width",    c_uint8 ),
            ( "channels", c_uint8 ),
            ( "indices",  c_uint16 * indices_width ),
        ]

        # Determine Pixel_Mapping length
        mapping_len = cast( control.kiibohd.Pixel_Mapping_HostLen, POINTER( c_uint16 ) )[0]

        # Lookup pixel
        pixels = cast( control.kiibohd.Pixel_Mapping, POINTER( PixelElement * mapping_len ) )[0]
        pixel = pixels[index - 1] # 0 indexed list

        # Lookup channels
        output_ch = []
        for ch in range( pixel.channels ):
            output_ch.append( pixel.indices[ch] )

        # Get current buffers and offsets
        bufs = self.animationDisplayBuffers()

        # Read values of channels
        read_value = []
        for ch in output_ch:
            for buf in bufs:
                # Determine if this buffer containes the channel
                if ch < buf[1] + buf[2]:
                    read_value.append( buf[0][ch - buf[1]] )
                    break

        return tuple( output_ch ), tuple( read_value )

    def rectDisp( self ):
        '''
        Show current MCU pixel buffer
        '''
        control.kiibohd.Pixel_dispBuffer()

    def setPeriodic(self, value):
        '''
        Set interval period

        @param value: Clock cycles between scans
        '''
        control.kiibohd.Periodic_init(int(value))

    def getPeriodic(self):
        '''
        Retrieve currently set interval period

        @return: Clock cycles between scans
        '''
        return int(control.kiibohd.Periodic_cycles())

    def capability(self, name, trigger, state, state_type, args):
        '''
        Call KLL capability

        @param name:       Name of capability
        @param trigger:    TriggerMacro* trigger
        @param state:      (uint8_t) ScheduleState
        @param state_type: (uint8_t) TriggerType
        @param args:       List of arguments
                           Uses KLL json to determine type of each arg (and it's width).
        '''
        klljson = control.json_input['Capabilities']

        # Do a lookup on the capability name
        # Make sure it exists
        if name not in klljson.keys():
            logger.error("Could not find capability '{}:{}'", name, args)
            return
        elem = klljson[name]

        # Make sure there are enough arguments
        if len(args) != elem['args_count']:
            logger.error("Arg count does not match '{}:{}' {} -> {}", name, args, len(args), elem['args_count'])
            return

        # Construct args pointer
        # Each argument must be split into byte sized elements
        arg_list = []
        for arg, info in zip(args, elem['args']):
            byte_form = arg.to_bytes(
                info['width'],
                byteorder='little',
            )
            byte_vals = [int(byte) for byte in byte_form]
            arg_list.extend(byte_vals)

        # Next convert the list of bytes to a pointer
        arg_ptr = (c_uint8 * len(arg_list))(*arg_list)

        # Lookup capability
        caplist = cast(control.kiibohd.CapabilitiesList, POINTER(Capability * len(klljson.keys()))).contents
        capfunc = cast(
            caplist[elem['index']].func,
            CFUNCTYPE(
                None,
                POINTER(TriggerMacro), c_uint8, c_uint8, POINTER(c_uint8)
            )
        )

        # Call Capability Function
        capfunc(trigger, state, state_type, arg_ptr)
        return


class Callbacks:
    '''
    Container class of commands required by the host-side KLL implementation
    '''
    def capabilityCallback( self, args ):
        '''
        Called whenever a capability is called.
        Argument defines if it's an immediate capability or delayed.

        Capability is executed after this method returns.

        This callback indicates that the resultCapabilityCallbackData variable is ready.
        '''
        arg = cast( args, c_char_p ).value
        item = cast( control.kiibohd.resultCapabilityCallbackData, POINTER( ResultCapabilityStackItem ) ).contents
        cbhistory = namedtuple('CallbackHistoryItem', ['callbackdata', 'type', 'time'])
        data.capability_history.new_callback(cbhistory(item.copy(), arg, time.time()))

    def layerState(self, args):
        '''
        Called on each processing loop to record the current layer state

        Used to determine what the previous layer state was
        Argument is unused
        '''
        control.cmd('recordLayerState')()



### Main Entry Point ###

if __name__ == '__main__':
    logger.error("Do not call directly.")
    sys.exit( 1 )

