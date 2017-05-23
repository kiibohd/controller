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
from PIL import Image # Use pillow instead of PIL, it works with Python 3
import argparse


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

	def getarray( self , string=False):
            if string is not False:
                struct = "{\n"

                for page in range( 0, self.page_count ):
                        for elem in self.page_data[ page ]:
                                struct += "0x{0:02x}, ".format( elem )

                        if page != self.page_count - 1:
                                struct += "\n"

                struct += "\n}"
                return struct
            return self.page_data

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


class ImageToStruct(object):

    def __init__(self, filename):
        self.max_height = 32
        self.max_width = 128
        self.x_offset = 0
        self.y_offset = 0
        self.filename = filename
        self.output_image = STLcdGraphic( self.max_height, self.max_width )
        self.disp_test = self.prep()

    def create_and_convert_image(self):
        # Load the input filename and convert to black & white
        try:
            input_image = Image.open( self.filename ).convert('1')
        except:
            StandardError( "Unable to load image '{0}'".format( self.filename ) )
        return input_image

    def check_boundries(self, input_image):
        # Check the image size to see if within the bounds of the display
        if input_image.size[0] > self.max_width or input_image.size[1] > self.max_height:
            raise StandardError( "ERROR: '{0}:{1}' is too large, must be no larger than {2}x{3}".format(
                filename,
                ( input_image.format, input_image.size, input_image.mode ),
                self.max_width,
                self.max_height )
                )

    def center_image(self, input_image):
        # Center the image
        height_start = int( ( self.max_height - input_image.size[1] ) / 2 )
        height_end   = int( height_start + input_image.size[1] )
        width_start  = int( ( self.max_width - input_image.size[0] ) / 2 )
        width_end    = int( width_start + input_image.size[0] )
        return height_start, height_end, width_start, width_end

    def prepare_view(self, input_image, height_start, height_end, width_start, width_end):
        # Iterate over all of the pixels
        # Also prepare the debug view of the image (disp_test)
        disp_test = "+"
        for pixel in range( 0, self.max_width ):
                disp_test += "-"
        disp_test += "+\n"

        for y in range( 0, self.max_height ):
                disp_test += "|"

                # Check if within height range
                if not ( y >= height_start and y < height_end ):
                        disp_test += " " * self.max_width + "|\n|"
                        continue

                for x in range( 0, self.max_width ):
                        # Check if within width range
                        if not ( x >= width_start and x < width_end ):
                                disp_test += " "
                                continue

                        # Use image value to determine pixel
                        try:
                                if input_image.getpixel( (x - width_start, y - height_start) ) == 0:
                                        disp_test += "*"
                                        self.output_image.setpixel( x, y + 1 ) # +1 is due to page boundary
                                else:
                                        disp_test += " "
                        except IndexError:
                                print( (x - width_start,y - height_start) )
                                pass

                disp_test += "|\n"

        disp_test += "+"
        for pixel in range( 0, self.max_width ):
                disp_test += "-"
        disp_test += "+\n"
        return disp_test

    def prep(self):
        image = self.create_and_convert_image()
        self.check_boundries(image)
        height_start, height_end, width_start, width_end = self.center_image(image)
        disp_test = self.prepare_view(image, 
                                      height_start, 
                                      height_end, 
                                      width_start, 
                                      width_end)
        return disp_test

    def output_image_fn(self): 
        print(self.output_image.preview())

    def preview(self):
        print(self.disp_test)

    def getarray(self, string=False):
        return self.output_image.getarray(string=string)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--filename", 
                        type=str, 
                        required=True,
                        help="provide .bmp file")
    args = parser.parse_args()
    filename = args.filename
    prep = ImageToStruct(filename)
    prep.preview()
    prep.output_image_fn()
    print("uint8_t array[] = {}".format(prep.getarray(string=True)))
