###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
# ADC/DAC example. DAC only works on microcontrollers that support it.
#
###


###
# Module C files
#

set( SCAN_SRCS
	scan_loop.c
	analog.c
)


###
# Module Specific Options
#


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	arm
)

