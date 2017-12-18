###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2017
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# Kinetis ARM CMake Build Configuration
#
###

#| Chip Name (Linker)
#|
#| "mk20dx128vlf5"    # McHCK / Kiibohd-dfu
#| "mk20dx128vlh7"    # Kiibohd-dfu
#| "mk20dx256vlh7"    # Kiibohd-dfu
#| "mk22fx512avlh12"  # Kiibohd-dfu
#|
#| "mk20dx128"        # Teensy   3.0
#| "mk20dx256"        # Teensy   3.1/3.2
#| "mk64fx512"        # Teensy   3.5
#| "mk66fx1m0"        # Teensy   3.6


#| Chip Size and CPU Frequency Database
#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.
#| MCHCK Based / Kiibohd-dfu
if ( "${CHIP}" MATCHES "mk20dx128vlf5" )
	set( SIZE_RAM    16384 )
	#set( SIZE_FLASH 126976 ) # XXX (HaaTa) Old size, still valid for nearly all keyboards
	set( SIZE_FLASH 122880 ) # For extended bootloader (8kB)
	set( F_CPU "48000000" )
	set( CHIP_SHORT "mk20dx128" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		#set( SIZE_FLASH 4096 ) # XXX (HaaTa) Old size
		set( SIZE_FLASH 8192 ) # XXX (HaaTa) Old size
	endif ()

#| Kiibohd-dfu
elseif ( "${CHIP}" MATCHES "mk20dx128vlh7" )
	set( SIZE_RAM    32768 )
	set( SIZE_FLASH 122880 ) # For extended bootloader (8kB)
	set( F_CPU "72000000" )
	set( CHIP_SHORT "mk20dx128" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Kiibohd-dfu
elseif ( "${CHIP}" MATCHES "mk20dx256vlh7" OR "${CHIP}" MATCHES "mk20dx256vmc7" )
	set( SIZE_RAM    65536 )
	set( SIZE_FLASH 253952 )
	set( F_CPU "72000000" )
	set( CHIP_SHORT "mk20dx256" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Kiibohd-dfu
# https://www.nxp.com/part/MK22FX512AVLH12
elseif ( "${CHIP}" MATCHES "mk22fx512avlh12" )
	set( SIZE_RAM   131072 )
	set( SIZE_FLASH 507904 )
	set( F_CPU "72000000" ) # XXX (HaaTa) purposefully slow for now
	set( CHIP_SHORT "mk22fx512a" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 16384 )
	endif ()

#| Teensy 3.0
elseif ( "${CHIP}" MATCHES "mk20dx128" )
	set( SIZE_RAM    16384 )
	set( SIZE_FLASH 131072 )
	set( F_CPU "48000000" )
	set( CHIP_SHORT "mk20dx128" )
	set( TEENSY 1 )

#| Teensy 3.1 / 3.2
elseif ( "${CHIP}" MATCHES "mk20dx256" )
	set( SIZE_RAM    65536 )
	set( SIZE_FLASH 262144 )
	set( F_CPU "48000000" ) # XXX Also supports 72 MHz, but may requires code changes
	set( CHIP_SHORT "mk20dx256" )
	set( TEENSY 1 )

#| Teensy 3.5
elseif ( "${CHIP}" MATCHES "mk64fx512" )
	set( SIZE_RAM   196608 )
	set( SIZE_FLASH 524288 )
	set( F_CPU "48000000" ) # XXX (HaaTa) purposefully slow for now
	set( CHIP_SHORT "mk64fx512" )
	set( TEENSY 1 )

#| Teensy 3.6
elseif ( "${CHIP}" MATCHES "mk66fx1m0" )
	set( SIZE_RAM    262144 )
	set( SIZE_FLASH 1048576 )
	set( F_CPU "48000000" ) # XXX (HaaTa) purposefully slow for now
	set( CHIP_SHORT "mk66fx1m0" )
	set( TEENSY 1 )

#| Unknown ARM
else ()
	message( AUTHOR_WARNING "CHIP: ${CHIP} - Unknown Kinetis ARM microcontroller" )
endif ()


#| Chip Base Type
#| Automatically chosed based on the chip name.
if ( "${CHIP}" MATCHES "^mk2.*$" )
	set( CHIP_FAMILY "mk2x" )
	set( CHIP_SUPPORT "kinetis" )
elseif ( "${CHIP}" MATCHES "^mk6.*$" )
	set( CHIP_FAMILY "mk6x" )
	set( CHIP_SUPPORT "kinetis" )
else ()
	message( FATAL_ERROR "Unknown chip family: ${CHIP}" )
endif ()


#| CPU Type
#| You _MUST_ set this to match the board you are using
#| type "make clean" after changing this, so all files will be rebuilt
#|
#| "cortex-m4"        # Teensy   3.0, 3.1, McHCK
set( CPU "cortex-m4" )

