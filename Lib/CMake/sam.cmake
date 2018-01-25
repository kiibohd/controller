###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2018
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# SAM ARM CMake Build Configuration
#
###

#| Chip Name (Linker)
#|
#| "sam4s2a"   #  48-pin Kiibohd-dfu
#| "sam4s2b"   #  64-pin Kiibohd-dfu
#| "sam4s2c"   # 100-pin Kiibohd-dfu
#| "sam4s4a"   #  48-pin Kiibohd-dfu
#| "sam4s4b"   #  64-pin Kiibohd-dfu
#| "sam4s4c"   # 100-pin Kiibohd-dfu
#| "sam4s8b"   #  64-pin Kiibohd-dfu
#| "sam4s8c"   # 100-pin Kiibohd-dfu
#| "sam4s16b"  #  64-pin Kiibohd-dfu
#| "sam4s16c"  # 100-pin Kiibohd-dfu
#| "sam4sa16b" #  64-pin Kiibohd-dfu
#| "sam4sa16c" # 100-pin Kiibohd-dfu
#| "sam4sd16b" #  64-pin Kiibohd-dfu
#| "sam4sd16c" # 100-pin Kiibohd-dfu
#| "sam4sd32b" #  64-pin Kiibohd-dfu
#| "sam4sd32c" # 100-pin Kiibohd-dfu


#| Chip Size and CPU Frequency Database
#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.

#| Kiibohd-dfu
# http://www.microchip.com/wwwproducts/en/ATSAM4S2A
# http://www.microchip.com/wwwproducts/en/ATSAM4S2B
# http://www.microchip.com/wwwproducts/en/ATSAM4S2C
if ( "${CHIP}" MATCHES "sam4s2a" OR "${CHIP}" MATCHES "sam4s2b" OR "${CHIP}" MATCHES "sam4s2c" )
	set( SIZE_RAM     65536 )
	set( SIZE_FLASH  122880 ) # 8kB bootloader (131072 bytes)
	set( F_CPU    120000000 )
	set( CHIP_SHORT "sam4s2" )
	set( CHIP_FAMILY "sam4s" )
	set( CHIP_SUPPORT "sam" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Kiibohd-dfu
# http://www.microchip.com/wwwproducts/en/ATSAM4S4A
# http://www.microchip.com/wwwproducts/en/ATSAM4S4B
# http://www.microchip.com/wwwproducts/en/ATSAM4S4C
elseif ( "${CHIP}" MATCHES "sam4s4a" OR "${CHIP}" MATCHES "sam4s4b" OR "${CHIP}" MATCHES "sam4s4c" )
	set( SIZE_RAM     65536 )
	set( SIZE_FLASH  253952 ) # 8kB bootloader (262144 bytes)
	set( F_CPU    120000000 )
	set( CHIP_SHORT "sam4s4" )
	set( CHIP_FAMILY "sam4s" )
	set( CHIP_SUPPORT "sam" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()


#| Kiibohd-dfu
# http://www.microchip.com/wwwproducts/en/ATSAM4S8B
# http://www.microchip.com/wwwproducts/en/ATSAM4S8C
elseif ( "${CHIP}" MATCHES "sam4s8b" OR "${CHIP}" MATCHES "sam4s8c" )
	set( SIZE_RAM    131072 )
	set( SIZE_FLASH  516096 ) # 8kB bootloader (524288 bytes)
	set( F_CPU    120000000 )
	set( CHIP_SHORT "sam4s8" )
	set( CHIP_FAMILY "sam4s" )
	set( CHIP_SUPPORT "sam" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Kiibohd-dfu
# http://www.microchip.com/wwwproducts/en/ATSAM4S16B
# http://www.microchip.com/wwwproducts/en/ATSAM4S16C
elseif ( "${CHIP}" MATCHES "sam4s16b" OR "${CHIP}" MATCHES "sam4s16c" )
	set( SIZE_RAM    131072 )
	set( SIZE_FLASH 1040384 ) # 8kB bootloader (1048576 bytes)
	set( F_CPU    120000000 )
	set( CHIP_SHORT "sam4s16" )
	set( CHIP_FAMILY "sam4s" )
	set( CHIP_SUPPORT "sam" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Kiibohd-dfu
# http://www.microchip.com/wwwproducts/en/ATSAM4SA16B
# http://www.microchip.com/wwwproducts/en/ATSAM4SA16C
elseif ( "${CHIP}" MATCHES "sam4sa16b" OR "${CHIP}" MATCHES "sam4sa16c" )
	set( SIZE_RAM    163840 )
	set( SIZE_FLASH 1040384 ) # 8kB bootloader (1048576 bytes)
	set( F_CPU    120000000 )
	set( CHIP_SHORT "sam4sa16" )
	set( CHIP_FAMILY "sam4s" )
	set( CHIP_SUPPORT "sam" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Kiibohd-dfu
# http://www.microchip.com/wwwproducts/en/ATSAM4SD16B
# http://www.microchip.com/wwwproducts/en/ATSAM4SD16C
elseif ( "${CHIP}" MATCHES "sam4sd16b" OR "${CHIP}" MATCHES "sam4sd16c" )
	set( SIZE_RAM    163840 )
	set( SIZE_FLASH 1040384 ) # 8kB bootloader (1048576 bytes)
	set( F_CPU    120000000 )
	set( CHIP_SHORT "sam4sd16" )
	set( CHIP_FAMILY "sam4s" )
	set( CHIP_SUPPORT "sam" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Kiibohd-dfu
# http://www.microchip.com/wwwproducts/en/ATSAM4SD32B
# http://www.microchip.com/wwwproducts/en/ATSAM4SD32C
elseif ( "${CHIP}" MATCHES "sam4sd32b" OR "${CHIP}" MATCHES "sam4sd32c" )
	set( SIZE_RAM    163840 )
	set( SIZE_FLASH 1040384 ) # 8kB bootloader (2097152 bytes)
	set( F_CPU    120000000 )
	set( CHIP_SHORT "sam4sd32" )
	set( CHIP_FAMILY "sam4s" )
	set( CHIP_SUPPORT "sam" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Unknown ARM
else ()
	message( AUTHOR_WARNING "CHIP: ${CHIP} - Unknown SAM ARM microcontroller" )
endif ()


#| CPU Type
#| You _MUST_ set this to match the board you are using
#| type "make clean" after changing this, so all files will be rebuilt
#|
set( CPU "cortex-m4" )

