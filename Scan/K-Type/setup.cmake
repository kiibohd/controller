###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2015 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#
AddModule ( Scan ISSILed )
AddModule ( Scan MatrixARM )
AddModule ( Scan PortSwap )
AddModule ( Scan UARTConnect )


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

