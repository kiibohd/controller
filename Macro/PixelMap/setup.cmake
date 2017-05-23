###| CMake Kiibohd Controller Macro Module |###
#
# Written by Jacob Alexander in 2016 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#
AddModule ( Macro PartialMap )



###
# Module C files
#
set ( Module_SRCS
	pixel.c
	${CMAKE_BINARY_DIR}/generatedPixelmap.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
	host
)

