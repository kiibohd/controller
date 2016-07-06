###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2011,2014,2016 for the Kiibohd Controller
#
# Released into the Public Domain
#
# For the Burroughs/Kokusai BETKB (Burroughs Ergonomic Terminal Keyboard)
#
###


###
# Module C files
#

set( Module_SRCS
	scan_loop.c
)


###
# Module Specific Options
#


###
# Compiler Family Compatibility
#
set( ModuleCompatibility
	arm
	avr
)

