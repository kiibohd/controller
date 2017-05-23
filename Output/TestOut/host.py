#!/usr/bin/env python3
'''
Host-Side Python Commands for TestOut Output Module
'''

# Copyright (C) 2016 by Jacob Alexander
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

import os
import sys

from ctypes import POINTER, cast, c_uint8, c_uint16



### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'



### Variables ###

data = None
debug = False
control = None



### Classes ###

class USBKeyboard:
	'''
	Stores USB Keyboard packet information

	Depending on which protocol is set, the output data may look different.
	Byte Array vs. Bit Mask
	(6KRO vs. NKRO)
	'''
	def __init__( self, bitfield_size, protocol, modifiers, keys, consumer_ctrl, system_ctrl ):
		'''
		Generate a USB Keyboard event packet
		'''
		self.bitfield_size = bitfield_size
		self.protocol = protocol # Byte
		self.modifiers = modifiers # Byte
		self.consumer_ctrl = consumer_ctrl # Short
		self.system_ctrl = system_ctrl # Byte
		self.keys = keys # List of bytes

	def codes( self ):
		'''
		Return a list of usb codes in the USB packet
		'''
		keys = []

		# Calculate modifiers
		for bit in range( 0, 8 ):
			if self.modifiers & (1<<bit):
				keys.append( 0xE0 | (1<<bit) )

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
	Container class of commands available to controll the host-side KLL implementation
	'''


class Callbacks:
	'''
	Container class of commands required byt the host-side KLL implementation
	'''

	def device_reload( self, args ):
		'''
		TODO
		'''
		print("device_reload not implemented")

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
		modifiers     = cast( control.kiibohd.USBKeys_Modifiers,    POINTER( c_uint8 ) )[0]
		keys          = cast( control.kiibohd.USBKeys_Keys,         POINTER( c_uint8 * bitfield_size ) )[0]
		protocol      = cast( control.kiibohd.USBKeys_Protocol,     POINTER( c_uint8 ) )[0]
		consumer_ctrl = cast( control.kiibohd.USBKeys_ConsCtrl,     POINTER( c_uint16 ) )[0]
		system_ctrl   = cast( control.kiibohd.USBKeys_SysCtrl,      POINTER( c_uint8 ) )[0]

		# keys array format
		# Bits   0 -  45 (bytes  0 -  5) correspond to USB Codes   4 -  49 (Main)
		# Bits  48 - 161 (bytes  6 - 20) correspond to USB Codes  51 - 164 (Secondary)
		# Bits 168 - 213 (bytes 21 - 26) correspond to USB Codes 176 - 221 (Tertiary)
		# Bits 214 - 216                 unused
		key_list = []
		for index in range( 0, bitfield_size ):
			key_list.append( keys[ index ] )

		data.usb_keyboard_data = USBKeyboard(
			bitfield_size,
			protocol,
			modifiers,
			key_list,
			consumer_ctrl,
			system_ctrl,
		)

		# Indicate we are done with the buffer
		cast( control.kiibohd.USBKeys_Changed, POINTER( c_uint8 ) )[0] = 0

	def mouse_send( self, args ):
		'''
		TODO
		'''
		print("mouse_send not implemented")

	def rawio_available( self, args ):
		'''
		TODO
		'''
		print("rawio_available not implemented")

	def rawio_rx( self, args ):
		'''
		TODO
		'''
		print("rawio_tx not implemented")

	def rawio_tx( self, args ):
		'''
		TODO
		'''
		print("rawio_tx not implemented")

	def restart( self, args ):
		'''
		TODO
		'''
		print("restart not implemented")

	def serial_available( self, args ):
		'''
		Return number of characters available to read from the serial buffer
		'''
		total = len( control.serial_buf )
		if debug:
			print("serial_available:", total )
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
			print("serial_read:", character, conv_char )
		return conv_char

	def serial_write( self, output ):
		'''
		Output to screen and to virtual serial interface if it exists
		'''
		print( output, end='' )

		# If serial is enabled, duplicate output to stdout and serial interface
		# Must re-encode back to bytes from utf-8
		if control.serial is not None and len( output ) > 0:
			os.write( control.serial, output.encode("ascii", "ignore") )



### Main Entry Point ###

if __name__ == '__main__':
	print( "{0} Do not call directly.".format( ERROR ) )
	sys.exit( 1 )

