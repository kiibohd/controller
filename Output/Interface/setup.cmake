###| CMake Kiibohd Controller Output Module |###
#
# Written by Jacob Alexander in 2017 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Sub-module flag, cannot be included stand-alone
#
set ( SubModule 1 )


###
# Module C files
#

set ( Module_SRCS
	output_gen.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
	avr
	host
)

