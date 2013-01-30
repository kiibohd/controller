###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2012 for the Kiibohd Controller
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
add_definitions( -I${HEAD_DIR}/Keymap )
add_definitions(
	-I${HEAD_DIR}/Scan/matrix
)	

#| Keymap Settings
add_definitions(
	-DMODIFIER_MASK=ibmconv_ModifierMask
	#-DKEYINDEX_MASK=ibmconv_DefaultMap
	-DKEYINDEX_MASK=ibmconv_ColemakMap
)


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	avr
)

