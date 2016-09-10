#!/usr/bin/env python3
'''
Host-Side Setup Routines for KLL
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

import argparse
import ctypes
import importlib
import os
import sys

from ctypes import CFUNCTYPE, c_int, c_char_p




### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'


## Python Text Formatting Fixer...
##  Because the creators of Python are averse to proper capitalization.
textFormatter_lookup = {
	"usage: "            : "Usage: ",
	"optional arguments" : "Optional Arguments",
}

def textFormatter_gettext( s ):
	return textFormatter_lookup.get( s, s )

argparse._ = textFormatter_gettext



### Classes ###

class Control:
	'''
	Handles general control of the libkiibohd host setup
	'''
	def __init__( self, scan_module, output_module, libkiibohd_path ):
		'''
		Initializes control object

		@param scan_module:   Path to ScanModule python script
		@param output_module: Path to OutputModule python script
		'''
		self.scan_module = scan_module
		self.output_module = output_module

		# Import Scan and Output modules
		global scan
		global output
		spec = importlib.util.spec_from_file_location( "Scan", self.scan_module )
		scan = importlib.util.module_from_spec( spec )
		spec.loader.exec_module( scan )

		spec = importlib.util.spec_from_file_location( "Output", self.output_module )
		output = importlib.util.module_from_spec( spec )
		spec.loader.exec_module( output )

		# Import libkiibohd
		global kiibohd
		try:
			kiibohd = ctypes.CDLL( libkiibohd_path )
		except Exception as err:
			print( "{0} Could not open -> {1}".format( ERROR, libkiibohd_path ) )
			print( err )
			sys.exit( 1 )

		# Register Callback
		self.callback_ref = self.callback_factory()

		# TODO Removeme
		val = kiibohd.Host_callback_test()
		print (val )

		print( self.callback_ref )


	def callback( self, command, args ):
		'''
		libkiibohd callback function
		'''
		print( command, args )
		return 2

	def callback_factory( self ):
		'''
		libkiibohd callback factory function
		'''
		def callbacks( command, args ):
			'''
			libkiibohd callback function
			'''
			print( command, args )
			return 2

		self.CTYPE_callback = CFUNCTYPE( c_int, c_char_p, c_char_p )
		try:
			#kiibohd.Host_register_callback( self.CTYPE_callback( self.callback ) )
			kiibohd.Host_register_callback( self.CTYPE_callback( callbacks ) )
		except Exception as err:
			print( "{0} Could register libkiibohd callback function".format( ERROR ) )
			print( err )
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
		parser.add_argument( '-h', '--help', action="help",
			help="This message." )

		# Process Arguments
		args = parser.parse_args()

		return args

	def process( self ):
		'''
		Run main commands
		'''
		pass

	def virtual_serialport_setup( self ):
		'''
		Setup virtual serial port

		Uses socat
		'''
		#socat -d -d pty,raw,echo=0,b115200 pty,raw,echo=0,b115200
		pass




### Functions ###



### Main Entry Point ###

if __name__ == '__main__':
	print( "{0} Do not call directly.".format( ERROR ) )
	sys.exit( 1 )

