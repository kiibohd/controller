###| CMake Kiibohd Controller Output Module |###
#
# Written by Jacob Alexander in 2011-2017 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#
AddModule ( Output HID-IO )



###
# Module C files
#

#| AVR Compiler
if ( ${COMPILER_FAMILY} MATCHES "avr" )

	set ( Module_SRCS
		output_com.c
		avr/usb_keyboard_serial.c
	)

#| ARM Compiler
elseif ( ${COMPILER_FAMILY} MATCHES "arm" )

	set ( Module_SRCS
		output_com.c
		arm/usb_desc.c
		arm/usb_dev.c
		arm/usb_joystick.c
		arm/usb_keyboard.c
		arm/usb_mem.c
		arm/usb_mouse.c
		arm/usb_rawio.c
		arm/usb_serial.c
	)

endif ( ${COMPILER_FAMILY} MATCHES "avr" )


###
# Compiler Family Compatibility
#
set( ModuleCompatibility
	arm
	avr
)

