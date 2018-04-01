###| CMake Kiibohd Controller UART Output Module |###
#
# Written by Rowan Decker in 2018 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#

AddModule ( Output Interface )


###
# Module C files
#

#| ARM Compiler
if ( ${COMPILER_FAMILY} MATCHES "arm" )
	set ( Module_SRCS
		output_com.c
		output_rtt.c
		SEGGER_RTT.c
		string.c
	)
endif ()


###
# Compiler Family Compatibility
#
set( ModuleCompatibility
	arm
)

