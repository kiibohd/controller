###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2015-2018 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Overrides
#
set ( MANUFACTURER "Input Club" PARENT_SCOPE )


###
# Required Sub-modules
#
AddModule ( Scan Devices/ISSILed )
AddModule ( Scan Devices/MatrixARMPeriodic )
AddModule ( Scan Devices/Storage )


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

