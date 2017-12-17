###| CMake Kiibohd Controller UART Output Module |###
#
# Written by Jacob Alexander in 2014-2017 for the Kiibohd Controller
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

#| AVR Compiler
if ( ${COMPILER_FAMILY} MATCHES "avr" )

	set ( Module_SRCS
		output_com.c
		output_uart.c
		avr/uart_serial.c
	)

#| ARM Compiler
elseif ( ${COMPILER_FAMILY} MATCHES "arm" )

	set ( Module_SRCS
		output_com.c
		output_uart.c
		arm/uart_serial.c
	)

endif ()


###
# Compiler Family Compatibility
#
set( ModuleCompatibility
	arm
#	avr # TODO
)

