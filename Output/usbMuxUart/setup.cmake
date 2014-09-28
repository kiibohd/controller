###| CMake Kiibohd Controller Muxed UART and USB Output Module |###
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
	)

#| ARM Compiler
elseif ( ${COMPILER_FAMILY} MATCHES "arm" )

	set( OUTPUT_SRCS
		output_com.c
		../pjrcUSB/arm/usb_desc.c
		../pjrcUSB/arm/usb_dev.c
		../pjrcUSB/arm/usb_keyboard.c
		../pjrcUSB/arm/usb_mem.c
		../pjrcUSB/arm/usb_serial.c
		../uartOut/arm/uart_serial.c
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

