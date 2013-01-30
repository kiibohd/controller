###| CMake Kiibohd Controller USB Module |###
#
# Written by Jacob Alexander in 2011-2013 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Module C files
#


#| AVR Compiler
if ( ${COMPILER_FAMILY} MATCHES "avr" )

	set( USB_SRCS
		usb_com.c
		avr/usb_keyboard_debug.c
	)

#| ARM Compiler
elseif ( ${COMPILER_FAMILY} MATCHES "arm" )

	set( USB_SRCS
		usb_com.c
		arm/usb_desc.c
		arm/usb_dev.c
		arm/usb_keyboard.c
		arm/usb_mem.c
		arm/usb_serial.c
	)

endif ( ${COMPILER_FAMILY} MATCHES "avr" )


###
# Module Specific Options
#

###
# Compiler Family Compatibility
#
set( USBModuleCompatibility
	arm
	avr
)

