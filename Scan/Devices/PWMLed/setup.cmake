###| CMake Kiibohd Controller Scan Module |###
#
# Written by Jacob Alexander in 2014-2019 for the Kiibohd Controller
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
	pwmled.c
)

if ( "${CHIP}" MATCHES "^sam.*$" )
list(APPEND Module_SRCS
	../../../Lib/ASF/sam/drivers/pdc/pdc.c
	../../../Lib/ASF/sam/drivers/pwm/pwm.c
)
endif ( )


###
# Compiler Family Compatibility
#
set ( ModuleCompatibility
	arm
)

