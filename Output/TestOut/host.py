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




### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'



### Variables ###

debug = False
control = None



### Classes ###

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
		pass

	def keyboard_send( self, args ):
		'''
		TODO
		'''
		pass

	def mouse_send( self, args ):
		'''
		TODO
		'''
		pass

	def rawio_available( self, args ):
		'''
		TODO
		'''
		pass

	def rawio_rx( self, args ):
		'''
		TODO
		'''
		pass

	def rawio_tx( self, args ):
		'''
		TODO
		'''
		pass

	def reset( self, args ):
		'''
		TODO
		'''
		pass

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

