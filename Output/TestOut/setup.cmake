###| CMake Kiibohd Controller Output Module |###
#
# Written by Jacob Alexander in 2011-2017 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#
AddModule ( Output HID-IO )



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
	host
)

