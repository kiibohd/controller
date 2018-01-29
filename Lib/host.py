'''
Host-Side Setup Routines for KLL
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

import argparse
import builtins
import inspect
import json
import logging
import os
import pty
import sys
import termios

from collections import namedtuple
from ctypes import (
    c_char_p,
    c_int,
    c_uint8,
    c_uint16,
    c_uint32,
    c_void_p,
    cast,
    CDLL,
    CFUNCTYPE,
    POINTER,
    Structure,
)

import kiilogger



### Logger ###

logger = kiilogger.get_logger('Lib/host.py')



### Decorators ###

## Python Text Formatting Fixer...
##  Because the creators of Python are averse to proper capitalization.
textFormatter_lookup = {
    "usage: "            : "Usage: ",
    "optional arguments" : "Optional Arguments",
}

def textFormatter_gettext( s ):
    return textFormatter_lookup.get( s, s )

argparse._ = textFormatter_gettext



### Variables ###

callback_ptrs = []



### Structs ###

class TriggerEvent( Structure ):
    '''
    TriggerEvent struct
    See kll.h in Macro/PartialMap
    '''
    _fields_ = [
        ( 'type',  c_uint8 ),
        ( 'state', c_uint8 ),
        ( 'index', c_uint8 ),
    ]

    def __repr__(self):
        val = "(type={}, state={}, index={})".format(
            self.type,
            self.state,
            self.index,
        )
        return val


class TriggerGuide( Structure ):
    '''
    TriggerGuide struct
    See kll.h in Macro/PartialMap
    '''
    _fields_ = [
        ( 'type',     c_uint8 ),
        ( 'state',    c_uint8 ),
        ( 'scanCode', c_uint8 ),
    ]

    def __repr__(self):
        val = "(type={}, state={}, scanCode={})".format(
            self.type,
            self.state,
            self.scanCode,
        )
        return val


class TriggerMacro( Structure ):
    '''
    TriggerMacro struct
    See Macro/PartialMap/kll.h
    '''
    # TODO result is a var_uint_t, needs to be dynamic
    _fields_ = [
        ( 'guide',  POINTER( c_uint8 ) ),
        ( 'result', c_uint8 ),
    ]

    def __repr__(self):
        val = "(guide={}, result={})".format(
            self.guide,
            self.result,
        )
        return val


class ResultsPendingElem( Structure ):
    '''
    ResultsPendingElem struct
    See Macro/PartialMap/kll.h
    '''
    # TODO index is a index_uint_t, needs to be dynamic
    _fields_ = [
        ( 'trigger', POINTER( TriggerMacro ) ),
        ( 'index',   c_uint16 ),
    ]

    def __repr__(self):
        val = "(trigger={}, index={})".format(
            self.trigger,
            self.index,
        )
        return val


class ResultsPending( Structure ):
    '''
    ResultsPending struct
    See Macro/PartialMap/kll.h
    '''
    # TODO size is a index_uint_t, needs to be dynamic
    _fields_ = [
        ( 'data', POINTER( ResultsPendingElem ) ),
        ( 'size', c_uint16 ),
    ]

    def __repr__(self):
        val = "(data={}, size={})".format(
            self.data,
            self.size,
        )
        return val



### Classes ###

class CapabilityHistory:
    '''
    Class that keeps track of the capability calls and maintains a history of calls.
    '''
    def __init__(self):
        self.history = []
        self.last_read = 0

    def new_callback(self, cbhistory):
        '''
        Add a callback to the history (append only).
        Expects a CallbackHistoryItem namedtuple.
        '''
        logger.debug("{}", cbhistory)
        self.history.append(cbhistory)

    def all(self):
        '''
        All callbacks in history.
        Does not adjust unread callback position.
        '''
        return self.history

    def prune(self, items=None):
        '''
        Prune read callback history.

        @param items: Max number of items to prune
        '''
        to_prune = self.last_read
        if items is not None:
            to_prune = min(items, to_prune)

        logger.debug("Pruning {}", to_prune)

        self.history = self.history[to_prune:]
        self.last_read -= to_prune

    def unread(self):
        '''
        Returns list of unread callbacks
        '''
        var = self.history[self.last_read:]
        self.last_read = len(self.history)
        return var


class LayerHistory:
    '''
    Class that keeps track of the layer state for each processing loop.
    To prevent memory leaks, only so many state transitions are kept.
    '''
    def __init__(self, history_len=10):
        '''
        @param history_len: Number of layer states to retain
        '''
        self.history_len = history_len
        self.history = []

    def add(self, layerstate):
        '''
        Adds LayerStateInstance to history, truncating oldest state if necessary

        @param layerstate: NamedTuple (LayerStateInstance) of layer state and stack information
        '''
        self.history.append(layerstate)

        # Truncate first element if necessary
        if len(self.history) > self.history_len:
            self.history = self.history[1:]

    def last(self, num=1):
        '''
        Returns an array with the last x number of LayerStateInstance objects

        @param num: Number of LayerStateInstance objects to return

        @return: Array of LayerStateInstance objects (if there are any available)
        '''
        total = min(num,len(self.history))
        start = len(self.history) - total

        return list(reversed(self.history[start:]))


class Data:
    '''
    Generic class used to hold data retrieved from libkiibohd callbacks
    '''
    def __init__( self ):
        self.usb_keyboard_data = None

        # List of capability callbacks
        self.capability_history = CapabilityHistory()

        # History of layer states - Last 10 state changes
        self.layer_history = LayerHistory(10)

        self.rawio_loopback = False
        self.rawio_incoming_buffer = []
        self.rawio_outgoing_buffer = []

    def usb_keyboard( self ):
        '''
        Returns a tuple of USB Keyboard output
        '''
        ntuple = namedtuple('USBKeyboardData', ['protocol', 'keyboardcodes', 'consumercode', 'systemcode'])
        if self.usb_keyboard_data is not None:
            return ntuple(self.usb_keyboard_data.protocol, self.usb_keyboard_data.codes(), self.usb_keyboard_data.consumer_ctrl, self.usb_keyboard_data.system_ctrl)
        return None

    def trigger_list_buffer( self ):
        '''
        Returns trigger list buffer
        '''
        size_width = control.var_uint_t
        triggers_len = cast( control.kiibohd.macroTriggerEventBufferSize, POINTER( size_width ) )[0]
        triggers = cast( control.kiibohd.macroTriggerEventBuffer, POINTER( TriggerEvent * triggers_len ) )[0]
        output = []
        for index in range( 0, triggers_len ):
            output.append( triggers[ index ] )
        return output

    def pending_trigger_list( self ):
        '''
        Returns list of pending triggers
        '''
        size_width = control.index_uint_t
        triggers_len = cast( control.kiibohd.macroTriggerMacroPendingListSize, POINTER( size_width ) )[0]
        triggers = cast( control.kiibohd.macroTriggerMacroPendingList, POINTER( size_width * triggers_len ) )[0]
        output = []
        for index in range( 0, triggers_len ):
            output.append( triggers[ index ] )
        return output

    def pending_result_list( self ):
        '''
        Returns list of pending results
        '''
        # XXX TODO - Working? Seems to be a bug with the .size
        # TODO - Add some sort of TriggerMacro guide interpretation
        results_pending = cast( control.kiibohd.macroResultMacroPendingList, POINTER( ResultsPending ) )[0]
        results_pending_list = []
        for index in range( 0, results_pending.size ):
            results_pending_list.append( [
                results_pending.data[ index ].index,
                results_pending.data[ index ].trigger[0].result,
            ] )
        return results_pending_list


class Control:
    '''
    Handles general control of the libkiibohd host setup
    '''
    def __init__( self, scan_module, output_module, libkiibohd_path, CustomLoader, json_input_path ):
        '''
        Initializes control object

        @param scan_module:   Path to ScanModule python script
        @param output_module: Path to OutputModule python script
        '''
        self.scan_module = scan_module
        self.output_module = output_module

        # Parse json file
        with open( json_input_path, 'r' ) as inputFile:
            self.json_input = json.load( inputFile )

        self.CTYPE_callback = None
        self.CTYPE_callback_ref = None
        self.serial = None
        self.serial_buf = ""

        # Provide reference to this class when running callback
        # Due to memory schemes, we have to use a standard Python function and not a method
        # or event a factory function (my experiments failed miserably on multiple calls)
        global control
        control = self
        builtins.kiibohd_control = self

        # Import libkiibohd
        global kiibohd
        try:
            kiibohd = CDLL( libkiibohd_path )
        except Exception as err:
            logger.error("Could not open -> {}\n{}", libkiibohd_path, err )
            sys.exit( 1 )
        self.kiibohd = kiibohd
        builtins.kiibohd_library = kiibohd

        # Query Dynamically Sized Types
        self.var_uint_t = self.ctype_lookup(cast(kiibohd.StateWordSize, POINTER(c_uint8)).contents.value)
        self.index_uint_t = self.ctype_lookup(cast(kiibohd.IndexWordSize, POINTER(c_uint8)).contents.value)
        self.state_uint_t = self.ctype_lookup(cast(kiibohd.ScheduleStateSize, POINTER(c_uint8)).contents.value)

        # Container for any libkiibohd callback data storage
        global data
        self.data = Data()
        builtins.kiibohd_data = self.data

        # Import Scan and Output modules
        global scan
        global output
        scan = CustomLoader( "Scan", scan_module ).load_module("Scan")
        builtins.kiibohd_scan = scan
        output = CustomLoader( "Output", output_module ).load_module("Output")
        builtins.kiibohd_output = output

        # Build command and callback dictionaries
        self.build_command_list()
        self.build_callback_list()

        # Register Callback
        self.callback_setup()

    def ctype_lookup(self, size):
        '''
        Returns associated ctype struct from a given size

        @param size: Integer width of type

        @return: ctype object
        '''
        if size == 8:
            return c_uint8
        elif size == 16:
            return c_uint16
        elif size == 32:
            return c_uint32
        else:
            logger.error("Invalid ctype width {}", size)
            return None

    def build_command_list( self ):
        '''
        Builds dictionary of commands that can be called
        '''
        # Compatible dictionary merge
        self.command_dict = get_method_dict( scan.Commands() ).copy()
        self.command_dict.update( get_method_dict( output.Commands() ) )

    def build_callback_list( self ):
        '''
        Builds dictionary of callbacks that libkiibohd.so may call
        '''
        # Compatible dictionary merge
        self.callback_dict = get_method_dict( scan.Callbacks() ).copy()
        self.callback_dict.update( get_method_dict( output.Callbacks() ) )

    def callback_setup( self ):
        '''
        Setup callback
        '''
        self.CTYPE_callback = CFUNCTYPE( c_int, c_char_p, c_void_p )
        try:
            refresh_callback()
        except Exception as err:
            logger.error("Could not register libkiibohd callback function\n{}", err)
            sys.exit( 1 )

    def process_args( self ):
        '''
        Process command line arguments
        '''
        # Setup argument processor
        parser = argparse.ArgumentParser(
            usage="%(prog)s [options..]",
            description="Kiibohd Host KLL Implementation",
            epilog="Example: {0} TODO".format( os.path.basename( sys.argv[0] ) ),
            formatter_class=argparse.RawTextHelpFormatter,
            add_help=False,
        )

        # Optional Arguments
        parser.add_argument( '-h', '--help',
            action="help",
            help="This message."
        )
        parser.add_argument( '-c', '--cli',
            action="store_true",
            help="Enables virtual serial port interface."
        )
        parser.add_argument( '-d', '--debug',
            action="store_true",
            help="Enable debug mode."
        )
        parser.add_argument( '-t', '--test',
            action="store_true",
            help="Run small test function to validate that Python callback interface is working."
        )

        # Process Arguments
        args = parser.parse_args()

        # Enable debug mode
        self.debug = args.debug
        scan.debug = args.debug
        output.debug = args.debug

        # Enable virtual serial port
        if args.cli:
            logger.info("Enabling Virtual Serial Port")
            self.virtual_serialport_setup()

        # Run test if requested, then exit
        if args.test:
            logger.info("libkiibohd.so - Callback Test")
            val = self.kiibohd.Host_callback_test()
            logger.info("Return Value:", val )
            sys.exit( 0 )

        return args

    def process( self ):
        '''
        Run main commands
        '''
        # Initialize kiibohd
        logger.info("Host_init")
        self.kiibohd.Host_init()
        print("")

        # Run cli if enabled
        self.virtual_serialport_process()

    def loop( self, number_of_loops=1 ):
        '''
        Run Host-side KLL main processing loop N number of times

        @param number_of_loops: Number of times to run main loop
        '''
        loop = 0
        while loop < number_of_loops:
            # Refresh callback interface
            refresh_callback()

            logger.debug("Host_process ({})", loop)
            self.kiibohd.Host_process()
            loop += 1

    def refresh_callback( self ):
        '''
        Convenience function for refreshing callback
        Useful when Scan or Output functions need to call directly
        '''
        return refresh_callback()

    def cmd( self, command_name ):
        '''
        Run given command from Host-side KLL

        Does a lookup of both Scan and Output module commands

        @param command_name: String of command
        @return: Function
        '''
        # Refresh callback interface
        refresh_callback()

        # Only print stack (show full calling function) info if in debug mode
        if logger.isEnabledFor(logging.DEBUG):
            parentstack_info = inspect.stack()[-1]
            logger.debug("cmd - {} {}:{}",
                parentstack_info.code_context[0][:-1],
                parentstack_info.filename,
                parentstack_info.lineno
            )

        return self.command_dict[ command_name ]

    def cli( self ):
        '''
        Setup and process cli commands
        Convenience function for test cases
        '''
        self.virtual_serialport_setup()
        self.virtual_serialport_process()

    def virtual_serialport_process( self ):
        '''
        Process virtual serial port commands
        '''
        # Run cli loop if available
        while self.serial is not None:
            value = os.read( self.serial_master, 1 ).decode('utf-8')
            self.serial_buf += value

            # Debug output
            if self.debug:
                print( value, end='' )
                sys.stdout.flush()

            # Check if any cli commands need to be processed
            ret = self.kiibohd.Host_cli_process()

            # If non-zero return, break out of processing loop
            if ret != 0:
                # Cleanup virtual serialport
                os.close( self.serial_master )
                os.close( self.serial_slave )
                break

    def virtual_serialport_setup( self ):
        '''
        Setup virtual serial port
        '''
        # Open pty device, and disable echo (to simulate microcontroller virtual serial port)
        self.serial_master, self.serial_slave = pty.openpty()
        settings = termios.tcgetattr( self.serial_master )
        settings[3] = settings[3] & ~termios.ECHO
        termios.tcsetattr( self.serial_master, termios.TCSADRAIN, settings )

        # Setup ttyname
        self.serial_name = os.ttyname( self.serial_slave )
        logger.info( self.serial_name )

        # Setup serial interface
        self.serial = self.serial_master



### Functions ###

def get_method_dict( obj ):
    '''
    Given an object return a dictionary of function name:function mappings
    '''
    output = {}
    for name, function in inspect.getmembers( obj, predicate=inspect.ismethod ):
        output[ name ] = function
    return output


def refresh_callback():
    '''
    Refresh callback pointer

    XXX (HaaTa)
    For some reason, either garbage collection, or something else, the pointer becomes stale in certain situations
    Usually when calling different library functions
    This just refreshes the pointer (shouldn't be necessary, but it works...)
    '''
    # Prevent garbage collection
    global callback_func
    callback_func = control.CTYPE_callback( callback )

    control.CTYPE_callback_ref = kiibohd.Host_register_callback( callback_func )


def callback( command, args ):
    '''
    libkiibohd callback function
    '''
    if control.debug:
        logger.debug("Callback: {} {}", command, args)

    # Lookup function in callback dictionary
    # Every function must taken a single argument
    # Must convert byte string to utf-8 first
    ret = control.callback_dict[ command.decode('utf-8') ]( args )

    # Refresh callback interface
    refresh_callback()

    # If returning None (default), change out to 1, C default
    return ret is None and 1 or ret



### Main Entry Point ###

if __name__ == '__main__':
    logger.error("Do not call directly.")
    sys.exit( 1 )

