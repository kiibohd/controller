###| CMake Kiibohd Controller PartialMap Macro Module |###
#
# Written by Jacob Alexander in 2014-2015 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#
AddModule ( Macro Common )


###
# Module C files
#

set ( Module_SRCS
	macro.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
	avr
)

