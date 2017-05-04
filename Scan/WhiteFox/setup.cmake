###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014-2015 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Overrides
#
set ( MANUFACTURER "Input Club" PARENT_SCOPE )


###
# Path to this module
#
set ( MatrixARM_Path ${CMAKE_CURRENT_LIST_DIR} )


###
# Required Sub-modules
#
AddModule ( Scan Devices/ISSILed )
AddModule ( Scan Devices/MatrixARM )


###
# Module C files
#
set ( Module_SRCS
	scan_loop.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
)

