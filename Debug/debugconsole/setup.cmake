###| CMake Kiibohd Controller Debug Module |###
#
# Written by Jacob Alexander in 2011-2015 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Submodules
#

AddModule ( Debug cli )
AddModule ( Debug led )
AddModule ( Debug print )

add_definitions(-DDEBUG)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
	avr
)

