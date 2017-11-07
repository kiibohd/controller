###| CMake Kiibohd Controller Debug Module |###
#
# Written by Jacob Alexander in 2011-2017 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Submodules
#

AddModule ( Debug cli )
AddModule ( Debug latency )
AddModule ( Debug led )
AddModule ( Debug print )


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
	avr
	host
)

