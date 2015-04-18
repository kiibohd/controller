#!/usr/bin/env python3

# Copyright (C) 2015 by Jacob Alexander
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

# Imports
import sys

from array import *
from PIL import Image


# Convenience class to deal with converting images to a C array
class STLcdGraphic:
	# Some constants for the LCD Driver
	page_width = 8
	page_max_length = 132

	array('B')

	def __init__( self, height, width ):
		self.height = height
		self.width  = width

		# Calculate number of pages
		self.page_count  = int( self.height / self.page_width )
		self.page_length = self.width

		# Initialize pages to 0's
		self.page_data = []
		for page in range( 0, self.page_count ):
			self.page_data.append( array( 'B', [0] * self.page_length ) )

	def setpixel( self, x, y ):
		# Calculate which page
		page = int( ( self.height - y ) / self.page_width )

		if page == 4:
			print("YAR", (x,y))

		# Calculate which byte
		byte = x

		# Calculate which bit
		bit = int( ( self.height - y ) % self.page_width )

		# Set pixel bit
		self.page_data[ page ][ byte ] |= (1 << bit)

	def getpage( self, page ):
		return self.page_data[ page ]

	def getarray( self ):
		struct = "{\n"

		for page in range( 0, self.page_count ):
			for elem in self.page_data[ page ]:
				struct += "0x{0:02x}, ".format( elem )

			if page != self.page_count - 1:
				struct += "\n"

		struct += "\n}"

		return struct

	# Prints out what the image will look like on the display
	def preview( self ):
		# Top border first
		display = "+"
		for pixel in range( 0, self.width ):
			display += "-"
		display += "+\n"

		# Each Page
		for page in range( self.page_count - 1, -1, -1 ):
			# Each Bit (Line)
			for line in range( 7, -1, -1 ):
				# Border
				display += "|"

				# Each Byte (Column/Pixel)
				for byte in range( 0, self.width ):
					if self.page_data[ page ][ byte ] & (1 << line):
						display += "*"
					else:
						display += " "

				# Border
				display += "|\n"

		# Bottom border
		display += "+"
		for pixel in range( 0, self.width ):
			display += "-"
		display += "+\n"

		return display


filename = "ic_logo_lcd.bmp"
max_height = 32
max_width = 128
x_offset = 0
y_offset = 0
output_image = STLcdGraphic( max_height, max_width )



# Load the input filename and convert to black & white
try:
	input_image = Image.open( filename ).convert('1')
except:
	print( "Unable to load image '{0}'".format( filename ) )

# Check the image size to see if within the bounds of the display
if input_image.size[0] > max_width or input_image.size[1] > max_height:
	print( "ERROR: '{0}:{1}' is too large, must be no larger than {2}x{3}".format(
		filename,
		( input_image.format, input_image.size, input_image.mode ),
		max_width,
		max_height )
	)
	sys.exit( 1 )

# Center the image
height_start = int( ( max_height - input_image.size[1] ) / 2 )
height_end   = int( height_start + input_image.size[1] )
width_start  = int( ( max_width - input_image.size[0] ) / 2 )
width_end    = int( width_start + input_image.size[0] )

#print( height_start )
#print( height_end )
#print( width_start )
#print( width_end )

# Iterate over all of the pixels
# Also prepare the debug view of the image (disp_test)
disp_test = "+"
for pixel in range( 0, max_width ):
	disp_test += "-"
disp_test += "+\n"

for y in range( 0, max_height ):
	disp_test += "|"

	# Check if within height range
	if not ( y >= height_start and y < height_end ):
		disp_test += " " * max_width + "|\n|"
		continue

	for x in range( 0, max_width ):
		# Check if within width range
		if not ( x >= width_start and x < width_end ):
			disp_test += " "
			continue

		# Use image value to determine pixel
		try:
			if input_image.getpixel( (x - width_start, y - height_start) ) == 0:
				disp_test += "*"
				output_image.setpixel( x, y + 1 ) # +1 is due to page boundary
			else:
				disp_test += " "
		except IndexError:
			print( (x - width_start,y - height_start) )
			pass

	disp_test += "|\n"

disp_test += "+"
for pixel in range( 0, max_width ):
	disp_test += "-"
disp_test += "+\n"

# BMP Conversion preview
print( disp_test )


print ( output_image.preview() )
#print ()
print( "uint8_t array[] = {0};".format( output_image.getarray() ) )

