#!/usr/bin/env python3

# Copyright (C) 2015 by Jacob Alexander
# Copyright (C) 2016 by Cui Yuting
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

class Bitmap_Size:
        def __init__( self, height=0, width=0, size=0, x_ppem=0, y_ppem=0):
                self.height = height
                self.width = width
                self.size = size
                self.x_ppem = x_ppem
                self.y_ppem = y_ppem

# Convenience class to deal with converting images to a C array
class STLcdFont:
        # Some constants for the LCD Driver
        array('B')

        def __init__( self, glyph_width, glyph_height, num_glyphs ):
                self.glyph_height = glyph_height
                self.glyph_width  = glyph_width
                self.glyph_size = (glyph_height + 7) // 8 * glyph_width
                self.num_fixed_sizes = 1
                self.availible_sizes = [ Bitmap_Size( self.glyph_height, self.glyph_width ) ]
                self.num_glyphs = num_glyphs
                self.glyph_data = []
                for index in range( 0, self.num_glyphs ):
                        self.glyph_data.append( array( 'B', [0] * self.glyph_size ) )
                        
        def setpixel( self, index, x, y ):
                newy = self.glyph_height - y - 1
                # Calculate which byte
                byte = newy // 8 * glyph_width + x

                # Calculate which bit
                bit = newy % 8

                # Set pixel bit
                self.glyph_data[ index ][ byte ] |= (1 << bit)

        def renderglyph( self, index, flags=0 ):
                return self.glyph_data[ index ]

        def getarray( self ):
                struct = "{\n"

                for glyph in self.glyph_data:
                        for elem in glyph:
                                struct += "0x{0:02x}, ".format( elem )
                struct += "\n}"

                return struct

filename = sys.argv[1]
glyph_width = int( sys.argv[2] )
glyph_height = int( sys.argv[3] )
bitmap_spacing = 0
bitmap_linespacing = 0
if ( len( sys.argv ) >= 6 ):
        bitmap_spacing = int( sys.argv[4] )
        bitmap_linespacing = int( sys.argv[5] )
num_glyphs = 128 # ASCII
if filename is None:
        print( "You must specify a bitmap filename. Try './bitmapfont2Struct.py font.bmp 4 6 0 0'" )
        sys.exit( 1 )
output_image = STLcdFont( glyph_width, glyph_height, num_glyphs )



# Load the input filename and convert to black & white
try:
        input_image = Image.open( filename ).convert('1')
except:
        print( "Unable to load image '{0}'".format( filename ) )

input_width, input_height = input_image.size
columns = ( input_width - glyph_width ) // ( glyph_width + bitmap_spacing ) + 1
rows = ( input_width - glyph_height ) // ( glyph_height + bitmap_linespacing ) + 1

# Iterate over all of the pixels
# Also prepare the debug view of the image (disp_test)
disp_test = "+" + "-" * input_width + "+\n"
glyph_index = 0
for y in range( 0, input_height ):
        row = y // ( glyph_height + bitmap_linespacing )
        if row >= rows:
                break
        if y % ( glyph_height + bitmap_linespacing ) >= glyph_height:
                # empty
                continue
        disp_test += "|"
        for x in range( 0, input_width ):
                column = x // ( glyph_width + bitmap_spacing )
                if x % ( glyph_width + bitmap_spacing ) >= glyph_width:
                        continue
                glyph_index = row * columns + column
                if column >= columns or glyph_index >= num_glyphs:
                        disp_test += " "
                else:
                        # Use image value to determine pixel
                        try:
                                if input_image.getpixel( (x, y) ) == 0:
                                        disp_test += "*"
                                        output_image.setpixel( glyph_index,
                                                               x % ( glyph_width + bitmap_spacing ),
                                                               y % ( glyph_height + bitmap_linespacing ) ) 
                                else:
                                        disp_test += " "
                        except IndexError:
                                print( (x, y) )
                                pass

        disp_test += "|\n"

disp_test += "+"
for pixel in range( 0, input_width ):
        disp_test += "-"
disp_test += "+\n"

# BMP Conversion preview
print( disp_test )
#print ()
print( "uint8_t array[] = {0};".format( output_image.getarray() ) )
print( "uint8_t font_glyphs_width = {0};".format( output_image.glyph_width ) )
print( "uint8_t font_glyphs_height = {0};".format( output_image.glyph_height ) )
print( "uint8_t font_glyphs_size = {0};".format( output_image.glyph_size ) )
print( "uint8_t font_num_glyphs = {0};".format( output_image.num_glyphs ) )
