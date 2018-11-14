###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014-2018 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Overrides
#
set ( MANUFACTURER "Input Club" PARENT_SCOPE )


###
# Required Submodules
#

AddModule ( Scan Devices/ISSILed )
AddModule ( Scan Devices/MatrixARMPeriodic )


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

