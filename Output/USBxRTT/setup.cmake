###| CMake Kiibohd Controller Muxed UART and USB Output Module |###
#
# Written by Rowan decker in 2018 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Submodules
#

AddModule ( Output USB )
AddModule ( Output SeggerRTT )


###
# Module C files
#

set ( Module_SRCS
	output_com.c
)

# Remove duplicate output_com.c files from USB and UARTOut
list ( REMOVE_ITEM Output_SRCS
	Output/USB/output_com.c
	Output/SeggerRTT/output_com.c
)


###
# Compiler Family Compatibility
#

set ( ModuleCompatibility
	arm
)

