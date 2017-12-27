###| CMake Kiibohd Controller UART Output Module |###
#
# Written by Jacob Alexander in 2014-2017 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#

AddModule ( Output Interface )


###
# Module C files
#

set ( Module_SRCS
	output_com.c
)


###
# Compiler Family Compatibility
#

set( ModuleCompatibility
	arm
	avr
)

