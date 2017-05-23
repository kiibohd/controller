###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2014
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# avr-gcc CMake Build Configuration
#
###



#| Set the Compilers (must be set first)
include( CMakeForceCompiler )
cmake_force_c_compiler  ( avr-gcc AVRCCompiler )
cmake_force_cxx_compiler( avr-g++ AVRCxxCompiler )
set( _CMAKE_TOOLCHAIN_PREFIX avr- )



###
# Atmel Defines and Linker Options
#

#| MCU Name
#|
#| "at90usb162"       # Teensy   1.0
#| "atmega32u4"       # Teensy   2.0
#| "at90usb646"       # Teensy++ 1.0
#| "at90usb1286"      # Teensy++ 2.0

set( MCU "${CHIP}" )

message( STATUS "MCU Selected:" )
message( "${MCU}" )


#| Chip Size Database
#| Teensy 1.0
if ( "${CHIP}" MATCHES "at90usb162" )
	set( SIZE_RAM      512 )
	set( SIZE_FLASH  15872 )

#| Teensy 2.0
elseif ( "${CHIP}" MATCHES "atmega32u4" )
	set( SIZE_RAM     2560 )
	set( SIZE_FLASH  32256 )

#| Teensy++ 1.0
elseif ( "${CHIP}" MATCHES "at90usb646" )
	set( SIZE_RAM     4096 )
	set( SIZE_FLASH  64512 )

#| Teensy++ 2.0
elseif ( "${CHIP}" MATCHES "at90usb1286" )
	set( SIZE_RAM     8192 )
	set( SIZE_FLASH 130048 )

#| Unknown AVR
else ()
	message( AUTHOR_WARNING "CHIP: ${CHIP} - Unknown AVR microcontroller" )
endif ()


#| Extra Compiler Sources
#| Mostly for convenience functions like interrupt handlers
set( COMPILER_SRCS
	# XXX Not needed for avr-gcc
)


#| CPU Type
#| This is only informational for AVR microcontrollers
#| The field can be determined by the microcontroller chip, but currently only one CPU type is used atm
set( CPU "megaAVR" )

message( STATUS "CPU Selected:" )
message( "${CPU}" )


#| USB Defines
set( VENDOR_ID       "0x1C11" )
set( PRODUCT_ID      "0xB04D" )
set( BOOT_VENDOR_ID  "0x16C0" ) # TODO Double check, this is likely incorrect
set( BOOT_PRODUCT_ID "0x047D" )


#| Only Teensy based AVRs supported
set ( TEENSY 1 )
message( STATUS "Bootloader Type:" )
message( "Teensy" )


#| Compiler flag to set the C Standard level.
#|     c89   = "ANSI" C
#|     gnu89 = c89 plus GCC extensions
#|     c99   = ISO C99 standard (not yet fully implemented)
#|     gnu99 = c99 plus GCC extensions
set( CSTANDARD "-std=gnu99" )


#| Warning Options
#|  -Wall...:     warning level
set( WARN "-Wall" )


#| Tuning Options
#|  -f...:        tuning, see GCC manual and avr-libc documentation
#| NOTE: -fshort-wchar is specified to allow USB strings be passed conveniently
set( TUNING "-funsigned-char -funsigned-bitfields -ffunction-sections -fpack-struct -fshort-enums" )


#| Optimization level, can be [0, 1, 2, 3, s].
#|     0 = turn off optimization. s = optimize for size.
#|     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
set( OPT "s" )


#| Output Format
#| srec, ihex, binary
set( FORMAT "ihex" )


#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.
set( F_CPU "16000000" )


#| Compiler Flags
add_definitions( "-mmcu=${MCU} -DF_CPU=${F_CPU} -D_${MCU}_=1 -O${OPT} ${TUNING} ${WARN} ${CSTANDARD}" )


#| Linker Flags
set( LINKER_FLAGS "-mmcu=${MCU} -Wl,-Map=link.map,--cref -Wl,--relax -Wl,--gc-sections" )


#| Hex Flags (XXX, CMake seems to have issues if you quote the arguments for the custom commands...)
set( HEX_FLAGS -O ${FORMAT} -R .eeprom -R .fuse -R .lock -R .signature )


#| Eep Flags (XXX, I've removed this target from the builds, but keeping the set line as a note)
set( EEP_FLAGS -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ${FORMAT} )


#| Lss Flags
set( LSS_FLAGS -h -S -z )

