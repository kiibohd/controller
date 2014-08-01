###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###

###
# Warning, that this module is not meant to be built stand-alone
#
message( FATAL_ERROR
"The 'MatrixARM' module is not a stand-alone module, and requires further setup."
)

###
# Module C files
#

set( SCAN_SRCS
	matrix_scan.c
)


###
# Module Specific Options
#


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	arm
)

