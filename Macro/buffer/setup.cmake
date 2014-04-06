###| CMake Kiibohd Controller Macro Module |###
#
# Written by Jacob Alexander in 2011,2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Warning, this module has been deprecated
#
message( AUTHOR_WARNING
"The 'buffer' macro module has been deprecated in favour of 'Partial Map'.
This module may or may not compile/function properly.
It has been kept for historical purposes."
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
	arm
	avr
)

