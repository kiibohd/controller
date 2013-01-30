###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2012 for the Kiibohd Controller
#
# Released into the Public Domain
#
# For the Sony Word Processor OA-S3400 keyboard
#
###


###
# Module C files
#

set( SCAN_SRCS
	scan_loop.c
)


###
# Module H files
#
set( SCAN_HDRS
	scan_loop.h
)


###
# File Dependency Setup
#
ADD_FILE_DEPENDENCIES( scan_loop.c ${SCAN_HDRS} )
#add_file_dependencies( scan_loop.c ${SCAN_HDRS} )


###
# Module Specific Options
#
add_definitions( -I${HEAD_DIR}/Keymap )

#| Keymap Settings
add_definitions(
	-DMODIFIER_MASK=sonyoas3400_ModifierMask
	-DKEYINDEX_MASK=sonyoas3400_ColemakMap
	#-DKEYINDEX_MASK=sonyoas3400_DefaultMap
)


###
# Compiler Family Compatibility
#
set( ScanModuleCompatibility
	avr
)

