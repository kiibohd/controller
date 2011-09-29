###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2011 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Module C files
#

set( SCAN_SRCS
	scan_loop.c
)


###
# Module Specific Options TODO Fixme!! (../)
#
add_definitions( -I../Keymap )

