'''
Host-Side Python Commands for TestOut Output Module
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
import os
import sys

from ctypes import POINTER, cast, c_char_p, c_uint8, c_uint16, Structure

import kiilogger



### Logger ###

logger = kiilogger.get_logger('Output/TestOut/host.py')



### Variables ###

data = builtins.kiibohd_data
debug = False
control = builtins.kiibohd_control



### Structs ###

class HIDIO_Packet( Structure ):
    '''
    HIDIO_Packet struct
    See hidio_com.h in Output/HID-IO
    '''
    _fields_ = [
        ( 'type',      c_uint8, 3 ),
        ( 'cont',      c_uint8, 1 ),
        ( 'id_width',  c_uint8, 1 ),
        ( 'reserved',  c_uint8, 1 ),
        ( 'upper_len', c_uint8, 2 ),
        ( 'len',       c_uint8 ),
    ]

    def copy( self ):
        '''
        Makes a copy of the structure values
        '''
        val = HIDIO_Packet()
        val.upper_len = copy.copy( self.upper_len )
        val.width = copy.copy( self.id_width )
        val.cont = copy.copy( self.cont )
        val.type = copy.copy( self.type )
        val.len = copy.copy( self.len )
        return val

    def id_width_bytes( self ):
        '''
        Calculate number of bytes the Id is
        '''
        # 16 bit Id
        if self.id_width == 0:
            return 2

        # 32 bit Id
        else:
            return 4

    def full_len( self ):
        '''
        Calculate full length
        '''
        return self.len + (self.upper_len << 8)

    def __repr__( self ):
        val = "(len={0}, width={1}, cont={2}, type={3})".format(
            self.full_len(),
            self.width,
            self.cont,
            self.type,
        )
        return val


class USBKeys( Structure ):
    '''
    USBKeys struct
    See Output/USB/output_usb.h
    '''

    _fields_ = [
        ( 'modifiers', c_uint8 ),
        ( 'keys',      c_uint8 * 27 ), # XXX (HaaTa) There should be a way to make this dynamic
                                       # XXX (HaaTa) Use builtins to parse this value early from the libray
        ( 'sys_ctrl',  c_uint8 ),
        ( 'cons_ctrl', c_uint16 ),
        ( 'changed',   c_uint8 ),
    ]

    def __repr__( self ):
        val = "(modifiers={}, keys={}, sys_ctrl={}, cons_ctrl={}, changed={})".format(
            self.modifiers,
            self.keys,
            self.sys_ctrl,
            self.cons_ctrl,
            self.changed,
        )
        return val



### Classes ###

class USBKeyboard:
    '''
    Stores USB Keyboard packet information

    Depending on which protocol is set, the output data may look different.
    Byte Array vs. Bit Mask
    (6KRO vs. NKRO)
    '''
    def __init__( self, bitfield_size, protocol, usb_keys ):
        '''
        Generate a USB Keyboard event packet
        '''
        self.bitfield_size = bitfield_size
        self.protocol = protocol # Byte
        self.usb_keys = usb_keys # USBKeys struct
        self.modifiers = usb_keys.modifiers # Byte
        self.consumer_ctrl = usb_keys.cons_ctrl # Short
        self.system_ctrl = usb_keys.sys_ctrl # Byte
        self.keys = usb_keys.keys # List of bytes

    def codes( self ):
        '''
        Return a list of usb codes in the USB packet
        '''
        # keys array format
        # Bits   0 -  45 (bytes  0 -  5) correspond to USB Codes   4 -  49 (Main)
        # Bits  48 - 161 (bytes  6 - 20) correspond to USB Codes  51 - 164 (Secondary)
        # Bits 168 - 213 (bytes 21 - 26) correspond to USB Codes 176 - 221 (Tertiary)
        # Bits 214 - 216                 unused
        keys = []

        # Calculate modifiers
        for bit in range( 0, 8 ):
            if self.modifiers & (1<<bit):
                keys.append( 0xE0 + bit )

        # 6 keys for boot mode
        if self.protocol == 0:
            for index in range( 0, 6 ):
                if self.keys[ index ] != 0x00:
                    keys.append( self.keys[ index ] )
        # NKRO key extraction
        elif self.protocol == 1:
            # See output_com.c Output_usbCodeSend_capability for details on start/pos
            start = 0
            for byte in self.keys:
                for pos, bit in zip( range( start, start + 8 ), range( 0, 8 ) ):
                    # Check if bit is set
                    if byte & (1<<bit):
                        if pos <= 45:
                            keys.append( pos + 4 )
                        elif pos <= 47:
                            pass
                        elif pos <= 161:
                            keys.append( pos + 3 )
                        elif pos <= 167:
                            pass
                        elif pos <= 213:
                            keys.append( pos + 8 )
                        elif pos <= 216:
                            pass
                start += 8

        return keys

    def __repr__( self ):
        '''
        Visual version of the USB packet
        '''
        output = "{0} {1:02b} {2:04X} {3:02X} ".format(
            self.protocol,
            self.modifiers,
            self.consumer_ctrl,
            self.system_ctrl,
        )

        # Boot mode
        if self.protocol == 0:
            for index in range( 0, 6 ):
                output += "{0:02X}".format( self.keys[ index ] )

        # NKRO mode
        elif self.protocol == 1:
            for index in range( 0, self.bitfield_size ):
                output += "{0:08b}".format( self.keys[ index ] )

        return output


class Commands:
    '''
    Container class of commands available to control the host-side KLL implementation
    '''

    def setOutputDebugMode(self, debug):
        '''
        Set Output Module debug mode

        0 - Disable (default)
        1 - Show output packet
        2 - Extra debug output
        '''
        cast(control.kiibohd.Output_DebugMode, POINTER(c_uint8))[0] = debug

    def setRawIOLoopback( self, enable=True ):
        '''
        Enable/Disable RawIO loopback
        Used to internally test the kiibohd RawIO mechanism.

        NOTE: May not work well with all packet types as it's Device-to-Device instead of Host-to-Device.
        '''
        data.rawio_loopback = enable

    def setRawIOPacketSize( self, packet_size=8 ):
        '''
        Set RawIO packet size
        Should not be set lower than 8 bytes

        Typical value is 64 bytes.
        '''
        control.kiibohd.HIDIO_packet_size.argtypes = [ c_uint16 ]
        return control.kiibohd.HIDIO_packet_size( packet_size )

    def HIDIO_test_2_request( self, payload_len, payload_value ):
        '''
        HIDIO_test_2_request wrapper
        '''
        control.kiibohd.HIDIO_test_2_request.argtypes = [ c_uint16, c_uint16 ]
        return control.kiibohd.HIDIO_test_2_request( payload_len, payload_value )

    def HIDIO_invalid_65535_request( self ):
        '''
        HIDIO_invalid_65535_request wrapper
        '''
        control.kiibohd.HIDIO_invalid_65535_request.argtypes = []
        return control.kiibohd.HIDIO_invalid_65535_request()



class Callbacks:
    '''
    Container class of commands required byt the host-side KLL implementation
    '''

    def device_reload( self, args ):
        '''
        TODO
        '''
        logger.warning("device_reload not implemented")

    def keyboard_send( self, args ):
        '''
        Callback received when Host-side KLL is ready to send USB keyboard codes
        When this command is received, we must do a few things
        1) Read the Bitfield size
        2) Read in USBKeys_Keys data array and USBKeys_Keys modifier byte
        3) Read in USBKeys_Protocol to determine format of USBKeys_Keys data
        4) Convert USBKeys_Keys into an array of USB Keyboard Codes
        5) Set USBKeys_Changed to 0x00 (USBKeysChangeState_None)
        '''
        # Gather data/pointers
        bitfield_size = cast( control.kiibohd.USBKeys_BitfieldSize, POINTER( c_uint8 ) )[0]
        protocol      = cast( control.kiibohd.USBKeys_Protocol,     POINTER( c_uint8 ) )[0]
        usb_keys      = cast( control.kiibohd.USBKeys_primary,      POINTER( USBKeys ) )[0]

        # Map into a more friendly datastructure
        data.usb_keyboard_data = USBKeyboard(
            bitfield_size,
            protocol,
            usb_keys,
        )

        # Indicate we are done with the buffer
        usb_keys.changed = 0

    def mouse_send( self, args ):
        '''
        TODO
        '''
        logger.warning("mouse_send not implemented")

    def rawio_available( self, args ):
        '''
        Returns the size of rawio_outgoing_buffer
        '''
        return len( data.rawio_outgoing_buffer )

    def rawio_rx( self, args ):
        '''
        Copy 64 byte buffer to received pointer
        '''
        # TODO (HaaTa): Support packet sizes other than 64 bytes

        # Prepare buffer
        buf = cast( args, POINTER( c_uint8 * 64 ) )[0]

        # Check if there are packets
        if len( data.rawio_outgoing_buffer ) == 0:
            return 0

        # Pop entry from outgoing buffer
        dataelem = data.rawio_outgoing_buffer.pop(0)

        # Copy payload
        for idx in range( 0, dataelem[0].full_len() + 2 ):
            buf[idx] = dataelem[3][idx]

        return 1

    def rawio_tx( self, args ):
        '''
        Add 64 byte buffer to rawio_incoming_buffer
        '''
        # TODO (HaaTa): Support packet sizes other than 64 bytes

        # Get buffer and packet hdr
        buf = cast( args, POINTER( c_uint8 * 64 ) )[0]
        header_pkt = cast( args, POINTER( HIDIO_Packet ) )[0]
        header = header_pkt.copy()

        # Get id, and convert to an int
        width = header.id_width_bytes() + 2
        id_bytes = buf[2:width]
        idval = int.from_bytes( id_bytes, byteorder='little', signed=False )

        # Determine payload length
        length = header.full_len()

        # Get payload
        payload = buf[width:length + 2]

        # Prepare tuple
        # TODO (HaaTa): Copy buf contents so they don't disappear on us
        for_buf = (
            header,
            idval,
            payload,
            buf[:],
        )

        # Store header, id, payload and data (data so that we can redirect the raw packet as necessary)
        if not data.rawio_loopback:
            data.rawio_incoming_buffer.append( for_buf )
        else:
            data.rawio_outgoing_buffer.append( for_buf )

        return 1

    def restart( self, args ):
        '''
        TODO
        '''
        logger.warning("restart not implemented")

    def serial_available( self, args ):
        '''
        Return number of characters available to read from the serial buffer
        '''
        total = len( control.serial_buf )
        if debug:
            logger.debug("serial_available: {}", total)
        return total

    def serial_read( self, args ):
        '''
        Query virtual serial interface for characters

        Only returns a single character.
        Yes, this isn't efficient, however it's necessary to copy the microcontroller behaviour
        (memory and buffer size constraints)
        '''
        character = control.serial_buf[0].encode("ascii", "ignore")
        control.serial_buf = control.serial_buf[1:]
        conv_char = ord( character )
        if debug:
            logger.debug("serial_read: {} {}", character, conv_char)
        return conv_char

    def serial_write( self, args ):
        '''
        Output to screen and to virtual serial interface if it exists
        '''
        output = cast( args, c_char_p ).value
        try:
            print( output.decode("utf-8"), end='' )
        except UnicodeDecodeError:
            print( output, end='' )

        # If serial is enabled, duplicate output to stdout and serial interface
        # Must re-encode back to bytes from utf-8
        if control.serial is not None and len( output ) > 0:
            os.write( control.serial, output.encode("ascii", "ignore") )



### Main Entry Point ###

if __name__ == '__main__':
    logger.error("Do not call directly.")
    sys.exit( 1 )

