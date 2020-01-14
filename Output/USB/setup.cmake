###| CMake Kiibohd Controller Output Module |###
#
# Written by Jacob Alexander in 2011-2019 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Required Sub-modules
#

AddModule ( Output HID-IO )
AddModule ( Output Interface )


###
# Module C files
#

#| AVR Compiler
if ( ${COMPILER_FAMILY} MATCHES "avr" )

	set ( Module_SRCS
		output_com.c
		output_usb.c
		avr/usb_keyboard_serial.c
	)

#| ARM Compiler
elseif ( ${COMPILER_FAMILY} MATCHES "arm" )

	set ( Module_SRCS
		output_com.c
		output_usb.c
		arm/usb_desc.c
		arm/usb_dev.c
		arm/usb_keyboard.c
		arm/usb_mem.c
		arm/usb_mouse.c
		arm/usb_rawio.c
	)


	if ( "${CHIP}" MATCHES "^sam.*$" )
	list(APPEND Module_SRCS
		../../Lib/ASF/sam/drivers/udp/udp_device.c
		../../Lib/ASF/common/services/usb/udc/udc.c
		../../Lib/ASF/common/services/usb/class/hid/device/generic/udi_hid_generic.c
	)
	endif ( )

#| Host Mode
elseif ( ${COMPILER_FAMILY} MATCHES "host" )

	set ( Module_SRCS
		output_com.c
		output_usb.c
	)

endif ()


###
# Compiler Family Compatibility
#

# XXX (HaaTa) USB module only supports host family used as a TestOut submodule
set ( ModuleCompatibility
	arm
	avr
	host
)

