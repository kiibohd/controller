###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014-2019 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Sub-module flag, cannot be included stand-alone
#
set ( SubModule 1 )


###
# Required Submodules
#
AddModule ( Scan Devices/SPI )


###
# Module C files
#
set ( Module_SRCS
	led_scan.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
)

