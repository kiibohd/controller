###| CMake Kiibohd Controller Scan Module |###
#
# Written by Rowan Decker in 2018 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Sub-module flag, cannot be included stand-alone
#
set ( SubModule 1 )


###
# Module C files
#
set ( Module_SRCS
	storage.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
)

