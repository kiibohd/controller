###| CMake Kiibohd Controller UART Output Module |###
#
# Written by Jacob Alexander in 2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Module C files
#


#| AVR Compiler
if ( ${COMPILER_FAMILY} MATCHES "avr" )

	set( OUTPUT_SRCS
		output_com.c
		avr/uart_serial.c
	)

#| ARM Compiler
elseif ( ${COMPILER_FAMILY} MATCHES "arm" )

	set( OUTPUT_SRCS
		output_com.c
		arm/uart_serial.c
	)

endif ()


###
# Module Specific Options
#

###
# Compiler Family Compatibility
#
set( OutputModuleCompatibility
	arm
#	avr # TODO
)

