###| CMake Kiibohd Controller Debug Module |###
#
# Written by Jacob Alexander in 2014-2016 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Module C files
#

set ( Module_SRCS
	cli.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
	avr
	host
)

