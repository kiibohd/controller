###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2012,2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Module C files
#

#| XXX Requires the ../ due to how the paths are constructed
set( SCAN_SRCS
	../matrix/matrix_scan.c
	../matrix/scan_loop.c
)


###
# Module Specific Options
#
add_definitions(
	-I${HEAD_DIR}/Scan/matrix
)


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	arm
	avr
)

