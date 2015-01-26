###| CMake Kiibohd Controller Muxed UART and USB Output Module |###
#
# Written by Jacob Alexander in 2014-2015 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Submodules
#

AddModule ( Output pjrcUSB )
AddModule ( Output uartOut )


###
# Module C files
#

set( Module_SRCS
	output_com.c
)


###
# Compiler Family Compatibility
#
set( ModuleCompatibility
	arm
#	avr # TODO
)

