###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2015 for the Kiibohd Controller
#
# Released into the Public Domain
#
###

###
# Module C files
#
set ( Module_SRCS
	scan_loop.c
	PS2Keyboard.c
)

add_definitions(
	-I${HEAD_DIR}/Lib/
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
)

