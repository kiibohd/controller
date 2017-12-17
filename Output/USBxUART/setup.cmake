###| CMake Kiibohd Controller Muxed UART and USB Output Module |###
#
# Written by Jacob Alexander in 2014-2017 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Submodules
#

AddModule ( Output USB )
AddModule ( Output UARTOut )


###
# Module C files
#

set( Module_SRCS
	output_com.c
)

# Remove duplicate output_com.c files from pjrcUSB and uartOut
list ( REMOVE_ITEM Output_SRCS
	Output/USB/output_com.c
	Output/UARTOut/output_com.c
)


###
# Compiler Family Compatibility
#
set( ModuleCompatibility
	arm
#       avr # TODO
)

