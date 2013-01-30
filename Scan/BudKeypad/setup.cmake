###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2011 for the Kiibohd Controller
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
	-DMODIFIER_MASK=budkeypad_ModifierMask
	#-DKEYINDEX_MASK=budkeypad_TheProfosistMap
	-DKEYINDEX_MASK=budkeypad_DefaultMap
)


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	avr
)

