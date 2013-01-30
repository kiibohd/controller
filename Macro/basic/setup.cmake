###| CMake Kiibohd Controller Macro Module |###
#
# Written by Jacob Alexander in 2011 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Warning, module has known speed issues on non-matrix designs
# Has not been tested in a long time
#
message( AUTHOR_WARNING
"The 'basic' macro module was originally designed for matrix scanning designs,
it was found not to be scalable with NKRO keyboard converters.
It has also not been tested in a long time, use at your own risk."
)


###
# Module C files
#

set( MACRO_SRCS
	macro.c
)


###
# Module Specific Options
#


###
# Compiler Family Compatibility
#
set( MacroModuleCompatibility
	avr
)

