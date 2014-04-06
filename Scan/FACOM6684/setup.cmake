###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2013,2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
# For the Fujitsu FACOM 6684KC3 Supercomputer Terminal Keyboard (M-780, M-1800, etc.)
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
	avr
)

