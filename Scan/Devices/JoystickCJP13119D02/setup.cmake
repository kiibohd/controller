###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014-2020 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Sub-module flag, cannot be included stand-alone
#
set ( SubModule 1 )


###
# Module C files
#
set ( Module_SRCS
	joystick_scan.c
)

if ( "${CHIP}" MATCHES "^sam.*$" )
list(APPEND Module_SRCS
	../../../Lib/ASF/sam/drivers/adc/adc.c
)
endif ( )


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
)

