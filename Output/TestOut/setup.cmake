###| CMake Kiibohd Controller Output Module |###
#
# Written by Jacob Alexander in 2011-2018 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#

AddModule ( Output USB )


###
# Module C files
#

set ( Module_SRCS
	output_com.c
	output_testout.c
)

# Remove duplicate output_com.c files from USB and UARTOut
list ( REMOVE_ITEM Output_SRCS
	Output/USB/output_com.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	host
)

