###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Module C files
#

set( SCAN_SRCS
	scan_loop.c
	../MatrixARM/matrix_scan.c
)


###
# Module Specific Options
#
add_definitions(
	-I${HEAD_DIR}/Scan/MatrixARM
)


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	arm
)

