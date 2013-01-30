###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2011 for the Kiibohd Controller
#
# Released into the Public Domain
#
###

###
# Warning, that this module is not meant to be built stand-alone
#
message( FATAL_ERROR
"The 'matrix' module is not a stand-alone module, and requires further setup.
See BudKeypad module for as an example module."
)

###
# Module C files
#

set( SCAN_SRCS
	matrix_scan.c
	scan_loop.c
)


###
# Module Specific Options
#
add_definitions( -I${HEAD_DIR}/Keymap )

add_definitions(
	#-DMODIFIER_MASK=
	#-DKEYINDEX_MASK=
)


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	avr
)

