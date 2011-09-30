###| CMake Kiibohd Controller Debug Module |###
#
# Written by Jacob Alexander in 2011 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Module C files
#

set( DEBUG_SRCS
	led.c
)


###
# Module Specific Options
#


###
# Just in case, you only want this module and are using others as well
#
add_definitions( -I${HEAD_DIR}/Debug/off )

