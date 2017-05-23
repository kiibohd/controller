###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2016-2017 for the Kiibohd Controller
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
configure_file ( Scan/TestIn/interface.py Tests/interface.py NEWLINE_STYLE UNIX )
configure_file ( Scan/TestIn/gdb          Tests/gdb          COPYONLY )


###
# Test cases
#
configure_file ( Scan/TestIn/Tests/common.py     Tests/common.py     COPYONLY )

configure_file ( Scan/TestIn/Tests/test.py       Tests/test.py       COPYONLY )
configure_file ( Scan/TestIn/Tests/animation.py  Tests/animation.py  COPYONLY )
configure_file ( Scan/TestIn/Tests/animation2.py Tests/animation2.py COPYONLY )

