###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2014
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# Freescale ARM CMake Build Configuration
#
###



#| Set the Compilers (must be set first)
include( CMakeForceCompiler )
message( STATUS "Compiler Selected:" )
if ( "${COMPILER}" MATCHES "gcc" )
	cmake_force_c_compiler  ( arm-none-eabi-gcc ARMCCompiler )
	cmake_force_cxx_compiler( arm-none-eabi-g++ ARMCxxCompiler )
	set( _CMAKE_TOOLCHAIN_PREFIX arm-none-eabi- )
	message( "gcc" )
elseif ( "${COMPILER}" MATCHES "clang" )
	cmake_force_c_compiler  ( clang   ARMCCompiler )
	cmake_force_cxx_compiler( clang++ ARMCxxCompiler )
	set( _CMAKE_TOOLCHAIN_PREFIX llvm- )
	message( "clang" )
else ()
	message( AUTHOR_WARNING "COMPILER: ${COMPILER} - Unknown compiler selection" )
endif ()



###
# ARM Defines and Linker Options
#

#| Chip Name (Linker)
#|
#| "mk20dx128vlf5"    # McHCK / Kiibohd-dfu
#| "mk20dx256vlh7"    # Kiibohd-dfu
#| "mk20dx128"        # Teensy   3.0
#| "mk20dx256"        # Teensy   3.1

message( STATUS "Chip Selected:" )
message( "${CHIP}" )
set( MCU "${CHIP}" ) # For loading script compatibility


#| Chip Size Database
#| MCHCK Based / Kiibohd-dfu
if ( "${CHIP}" MATCHES "mk20dx128vlf5" )
	set( SIZE_RAM    16384 )
	set( SIZE_FLASH 126976 )

#| Kiibohd-dfu
elseif ( "${CHIP}" MATCHES "mk20dx256vlh7" )
	set( SIZE_RAM    65536 )
	set( SIZE_FLASH 253952 )

#| Teensy 3.0
elseif ( "${CHIP}" MATCHES "mk20dx128" )
	set( SIZE_RAM    16384 )
	set( SIZE_FLASH 131072 )

#| Teensy 3.1
elseif ( "${CHIP}" MATCHES "mk20dx256" )
	set( SIZE_RAM    65536 )
	set( SIZE_FLASH 262144 )

#| Unknown ARM
else ()
	message( AUTHOR_WARNING "CHIP: ${CHIP} - Unknown ARM microcontroller" )
endif ()


#| Chip Base Type
#| Automatically chosed based on the chip name.
if ( "${CHIP}" MATCHES "^mk20dx.*$" )
	set( CHIP_FAMILY "mk20dx" )
	message( STATUS "Chip Family:" )
	message( "${CHIP_FAMILY}" )
else ()
	message( FATAL_ERROR "Unknown chip family: ${CHIP}" )
endif ()


#| CPU Type
#| You _MUST_ set this to match the board you are using
#| type "make clean" after changing this, so all files will be rebuilt
#|
#| "cortex-m4"        # Teensy   3.0, 3.1, McHCK
set( CPU "cortex-m4" )

message( STATUS "CPU Selected:" )
message( "${CPU}" )


#| Extra Compiler Sources
#| Mostly for convenience functions like interrupt handlers
set( COMPILER_SRCS
	Lib/${CHIP_FAMILY}.c
	Lib/delay.c
)

message( STATUS "Compiler Source Files:" )
message( "${COMPILER_SRCS}" )


#| USB Defines, this is how the loader programs detect which type of chip base is used
message( STATUS "Bootloader Type:" )
if ( "${CHIP}" MATCHES "mk20dx128vlf5" OR "${CHIP}" MATCHES "mk20dx256vlh7" )
	set( VENDOR_ID       "0x1C11" )
	set( PRODUCT_ID      "0xB04D" )
	set( BOOT_VENDOR_ID  "0x1C11" )
	set( BOOT_PRODUCT_ID "0xB007" )
	set( DFU 1 )
	message( "dfu" )
elseif ( "${CHIP}" MATCHES "mk20dx128" OR "${CHIP}" MATCHES "mk20dx256" )
	set( VENDOR_ID       "0x1C11" )
	set( PRODUCT_ID      "0xB04D" )
	set( BOOT_VENDOR_ID  "0x16c0" ) # TODO Double check, this is likely incorrect
	set( BOOT_PRODUCT_ID "0x0487" )
	set( TEENSY 1 )
	message( "Teensy" )
endif ()


#| Compiler flag to set the C Standard level.
#|     c89   = "ANSI" C
#|     gnu89 = c89 plus GCC extensions
#|     c99   = ISO C99 standard (not yet fully implemented)
#|     gnu99 = c99 plus GCC extensions
#|     gnu11 = c11 plus GCC extensions
set( CSTANDARD "-std=gnu11" )


#| Warning Options
#|  -Wall...:     warning level
set( WARN "-Wall -ggdb3" )


#| Tuning Options
#|  -f...:        tuning, see GCC manual
#| NOTE: -fshort-wchar is specified to allow USB strings be passed conveniently
if( BOOTLOADER )
	set( TUNING "-D_bootloader_ -Wno-main -msoft-float -mthumb -fplan9-extensions -ffunction-sections -fdata-sections -fno-builtin -fstrict-volatile-bitfields -flto -fno-use-linker-plugin -nostdlib" )
	#set( TUNING "-mthumb -fdata-sections -ffunction-sections -fno-builtin -msoft-float -fstrict-volatile-bitfields -flto -fno-use-linker-plugin -fwhole-program -Wno-main -nostartfiles -fplan9-extensions -D_bootloader_" )
elseif ( "${COMPILER}" MATCHES "clang" )
	set( TUNING "-target arm-none-eabi -mthumb -nostdlib -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin" )
else()
	set( TUNING "-mthumb -nostdlib -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin -nostartfiles" )
endif()


#| Optimization level, can be [0, 1, 2, 3, s].
#|     0 = turn off optimization. s = optimize for size.
#|     (Note: 3 is not always the best optimization level.)
set( OPT "s" )


#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.
set( F_CPU "48000000" )


#| Dependency Files
#| Compiler flags to generate dependency files.
set( GENDEPFLAGS "-MMD" )


#| Compiler Flags
add_definitions( "-mcpu=${CPU} -DF_CPU=${F_CPU} -D_${CHIP}_=1 -O${OPT} ${TUNING} ${WARN} ${CSTANDARD} ${GENDEPFLAGS}" )


#| Linker Flags
if( BOOTLOADER )
	# Bootloader linker flags
	set( LINKER_FLAGS "${TUNING} -Wl,--gc-sections -fwhole-program -T${CMAKE_CURRENT_SOURCE_DIR}/../Lib/${CHIP}.bootloader.ld -nostartfiles -Wl,-Map=link.map" )
else()
	# Normal linker flags
	set( LINKER_FLAGS "${TUNING} -Wl,-Map=link.map,--cref -Wl,--gc-sections -Wl,--no-wchar-size-warning -T${CMAKE_CURRENT_SOURCE_DIR}/Lib/${CHIP}.ld" )
endif()


#| Hex Flags (XXX, CMake seems to have issues if you quote the arguments for the custom commands...)
set( HEX_FLAGS -O ihex -R .eeprom )


#| Binary Flags
set( BIN_FLAGS -O binary )


#| Lss Flags
if ( "${COMPILER}" MATCHES "clang" )
	set( LSS_FLAGS -section-headers -triple=arm-none-eabi )
else ()
	set( LSS_FLAGS -h -S -z )
endif ()

