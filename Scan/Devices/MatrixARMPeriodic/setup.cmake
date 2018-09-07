###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014-2015 for the Kiibohd Controller
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
	matrix_scan.c
)


###
# Header file dependency tracking
#
set_property (
	SOURCE ${CMAKE_CURRENT_LIST_DIR}/matrix_scan.c
	APPEND PROPERTY OBJECT_DEPENDS ${MatrixARM_Path}/matrix.h
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
)

