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

#| XXX Requires the ../ due to how the paths are constructed
set( DEBUG_SRCS
	../led/led.c
	../print/print.c
)


###
# Module Specific Options
#
add_definitions(
	-I${HEAD_DIR}/Debug/led
	-I${HEAD_DIR}/Debug/print
)

