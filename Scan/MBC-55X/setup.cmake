###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2013,2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
# For the Sanyo MBC-55X Series of keyboards
#
###


###
# Module C files
#

set( SCAN_SRCS
	scan_loop.c
)


###
# Module Specific Options
#


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	arm
	avr
)

