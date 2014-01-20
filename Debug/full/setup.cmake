###| CMake Kiibohd Controller Debug Module |###
#
# Written by Jacob Alexander in 2011-2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Module C files
#

#| XXX Requires the ../ due to how the paths are constructed
set( DEBUG_SRCS
	../cli/cli.c
	../led/led.c
	../print/print.c
)


###
# Setup File Dependencies
#
add_file_dependencies( ../led/led.c ../led/led.h )
add_file_dependencies( ../print/print.c ../print/print.h )


###
# Module Specific Options
#
add_definitions(
	-I${HEAD_DIR}/Debug/cli
	-I${HEAD_DIR}/Debug/led
	-I${HEAD_DIR}/Debug/print
)


###
# Compiler Family Compatibility
#
set( DebugModuleCompatibility
	arm
	avr
)

