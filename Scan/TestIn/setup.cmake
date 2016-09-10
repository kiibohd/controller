###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2016 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Submodules
#


###
# Module C files
#

set ( Module_SRCS
	scan_loop.c
)


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	host
)


###
# Configure host side Python scripts
#
configure_file ( Scan/TestIn/interface.py interface NEWLINE_STYLE UNIX )

