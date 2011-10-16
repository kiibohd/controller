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

set( SCAN_SRCS
	matrix_scan.c
	scan_loop.c
)


###
# Module Specific Options
#
add_definitions( -I${HEAD_DIR}/Keymap )

#| Keymap Settings
add_definitions(
	-DMODIFIER_MASK=budkeypad_ModifierMask
	#-DKEYINDEX_MASK=budkeypad_TheProfosistMap
	-DKEYINDEX_MASK=budkeypad_DefaultMap
)

