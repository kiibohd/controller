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

# Remove duplicate output_com.c files from pjrcUSB and uartOut
list ( REMOVE_ITEM Output_SRCS
	Output/pjrcUSB/output_com.c
	Output/uartOut/output_com.c
)


###
# Compiler Family Compatibility
#
set( ModuleCompatibility
	arm
#       avr # TODO
)

