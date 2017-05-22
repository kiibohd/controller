#!/usr/bin/env python3
'''
Host-Side Python Commands for TestIn Scan Module
'''

# Copyright (C) 2016-2017 by Jacob Alexander
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

import sys

from ctypes import (Structure, POINTER, cast, c_uint32, c_uint16, c_uint8, c_void_p, create_string_buffer)



### Decorators ###

## Print Decorator Variables
ERROR = '\033[5;1;31mERROR\033[0m:'
WARNING = '\033[5;1;33mWARNING\033[0m:'



### Variables ###

data = None
debug = False
control = None



### Structures ###

class TriggerMacro( Structure ):
	'''
	C-Struct for TriggerMacro
	See Macro/PartialMap/kll.h
	'''
	# TODO result is var_uint_t (dynamically sized)
	_fields_ = [
		( "guide",  POINTER( c_uint8 ) ),
		( "result", c_uint8 ),
	]


class AnimationStackElement( Structure ):
	'''
	C-Struct for AnimationStackElement
	See Macro/PixelMap/pixel.h
	'''
	_fields_ = [
		( "trigger",  POINTER( TriggerMacro ) ),
		( "index",    c_uint16 ),
		( "pos",      c_uint16 ),
		( "loops",    c_uint8 ),
		( "divmask",  c_uint8 ),
		( "divshift", c_uint8 ),
		( "ffunc",    c_uint8 ),
		( "pfunc",    c_uint8 ),
	]

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



### Classes ###

class Commands:
	'''
	Container class of commands available to controll the host-side KLL implementation
	'''

	def addScanCode( self, scan_code ):
		'''
		Adds a Scan Code to the internal KLL buffer

		Returns 1 if added, 0 if the ScanCode is already in the buffer
		Returns 2 if there's an error
		Generally 1 will be the return
		'''
		return control.kiibohd.Scan_addScanCode( int( scan_code ) )

	def removeScanCode( self, scan_code ):
		'''
		Removes a Scan Code from the internal KLL buffer
		Ignored if the Scan Code was not in the buffer

		Returns 1 if added, 0 if the ScanCode is already in the buffer
		Returns 2 if there's an error
		Generally 0 will be the return
		'''
		return control.kiibohd.Scan_removeScanCode( int( scan_code ) )

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
				print( "{0} '{1}' is an invalid animation id.".format( ERROR, name ) )
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
		class AnimationStack( Structure ): pass
		AnimationStack._fields_ = [
			( "size",  c_uint16 ),
			( "stack", POINTER( AnimationStackElement * size ) ),
		]

		# Cast animation stack
		stack = cast( control.kiibohd.Pixel_AnimationStack, POINTER( AnimationStack ) )[0]

		return stack

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


class Callbacks:
	'''
	Container class of commands required byt the host-side KLL implementation
	'''



### Main Entry Point ###

if __name__ == '__main__':
	print( "{0} Do not call directly.".format( ERROR ) )
	sys.exit( 1 )

