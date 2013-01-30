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
	scan_loop.c
)


###
# Module Specific Options
#
add_definitions( -I${HEAD_DIR}/Keymap )

#| Keymap Settings
add_definitions(
	-DMODIFIER_MASK=kaypro1_ModifierMask
	#-DKEYINDEX_MASK=kaypro1_ColemakMap
	-DKEYINDEX_MASK=kaypro1_DefaultMap
)


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	avr
)

