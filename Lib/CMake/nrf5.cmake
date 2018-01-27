###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2018
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# nRF BLE ARM CMake Build Configuration
#
###

#| Chip Name (Linker)
#|
#| "nrf52810_QCAA" # 32-pin Kiibohd-dfu
#| "nrf52810_QFAA" # 48-pin Kiibohd-dfu
#| "nrf52832_QFAA" # 48-pin Kiibohd-dfu
#| "nrf52832_QFAB" # 48-pin Kiibohd-dfu
#| "nrf52832_CIAA" # 50-pin Kiibohd-dfu
#| "nrf52840_QIAA" # 73-pin Kiibohd-dfu


#| Chip Size and CPU Frequency Database
#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.

#| Kiibohd-dfu
#| https://www.nordicsemi.com/eng/Products/nRF52810
if ( "${CHIP}" MATCHES "nrf52810_qcaa" OR "${CHIP}" MATCHES "nrf52810_qfaa" )
	set( SIZE_RAM     24576 )
	set( SIZE_FLASH  196608 )
	set( F_CPU     64000000 )
	set( CHIP_SHORT "nrf52810" )
	set( CHIP_FAMILY "nrf52" )
	set( CHIP_SUPPORT "nrf5" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		#set( SIZE_FLASH 8192 ) # XXX (HaaTa) Size must be decided
	endif ()

#| Kiibohd-dfu
#| https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52832
elseif ( "${CHIP}" MATCHES "nrf52832_qfaa" OR "${CHIP}" MATCHES "nrf52832_ciaa" )
	set( SIZE_RAM     65536 )
	set( SIZE_FLASH  524288 )
	set( F_CPU     64000000 )
	set( CHIP_SHORT "nrf52832" )
	set( CHIP_FAMILY "nrf52" )
	set( CHIP_SUPPORT "nrf5" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		#set( SIZE_FLASH 8192 ) # XXX (HaaTa) Size must be decided
	endif ()

#| Kiibohd-dfu
#| https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52832
elseif ( "${CHIP}" MATCHES "nrf52832_qfab" )
	set( SIZE_RAM     32768 )
	set( SIZE_FLASH  262144 )
	set( F_CPU     64000000 )
	set( CHIP_SHORT "nRF52832" )
	set( CHIP_FAMILY "nrf52" )
	set( CHIP_SUPPORT "nrf5" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		#set( SIZE_FLASH 8192 ) # XXX (HaaTa) Size must be decided
	endif ()

#| Kiibohd-dfu
#| https://www.nordicsemi.com/eng/Products/nRF52840
elseif ( "${CHIP}" MATCHES "nrf52840_qiaa" )
	set( SIZE_RAM    262144 )
	set( SIZE_FLASH 1048576 )
	set( F_CPU     64000000 )
	set( CHIP_SHORT "nrf52840" )
	set( CHIP_FAMILY "nrf52" )
	set( CHIP_SUPPORT "nrf5" )
	set( DFU 1 )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		#set( SIZE_FLASH 8192 ) # XXX (HaaTa) Size must be decided
	endif ()

#| Unknown ARM
else ()
	message( AUTHOR_WARNING "CHIP: ${CHIP} - Unknown nRF BLE ARM microcontroller" )
endif ()


#| CPU Type
#| You _MUST_ set this to match the board you are using
#| type "make clean" after changing this, so all files will be rebuilt
#|
set( CPU "cortex-m4" )

