###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2016
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# Freescale ARM CMake Build Configuration
#
###


###
# Compiler Check
#

#|
#| In CMake 3.6 a new feature to configure try_compile to work with cross-compilers
#| https://cmake.org/cmake/help/v3.6/variable/CMAKE_TRY_COMPILE_TARGET_TYPE.html#variable:CMAKE_TRY_COMPILE_TARGET_TYPE
#| If we detect CMake 3.6 or higher, use the new method
#|
if ( NOT CMAKE_VERSION VERSION_LESS "3.6" )
	# Prepare for cross-compilation
	set( CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY" )
	set( CMAKE_SYSTEM_NAME "Generic" )

	message( STATUS "Compiler Selected:" )
	if ( "${COMPILER}" MATCHES "gcc" )
		set( CMAKE_C_COMPILER   arm-none-eabi-gcc )
		set( _CMAKE_TOOLCHAIN_PREFIX arm-none-eabi- )
		message( "gcc" )
	elseif ( "${COMPILER}" MATCHES "clang" )
		set( CMAKE_C_COMPILER    clang )
		set( CMAKE_C_COMPILER_ID ARMCCompiler )
		set( _CMAKE_TOOLCHAIN_PREFIX llvm- )
		message( "clang" )
	else ()
		message( AUTHOR_WARNING "COMPILER: ${COMPILER} - Unknown compiler selection" )
	endif ()

#|
#| Before CMake 3.6 the cmake_force_c_compiler command was necessary to select cross-compilers
#| and other problemmatic compilers.
#|
else ()
	# Set the Compilers (must be set first)
	include( CMakeForceCompiler )
	message( STATUS "Compiler Selected:" )
	if ( "${COMPILER}" MATCHES "gcc" )
		cmake_force_c_compiler( arm-none-eabi-gcc ARMCCompiler )
		set( _CMAKE_TOOLCHAIN_PREFIX arm-none-eabi- )
		message( "gcc" )
	elseif ( "${COMPILER}" MATCHES "clang" )
		cmake_force_c_compiler( clang ARMCCompiler )
		set( _CMAKE_TOOLCHAIN_PREFIX llvm- )
		message( "clang" )
	else ()
		message( AUTHOR_WARNING "COMPILER: ${COMPILER} - Unknown compiler selection" )
	endif ()
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


#| Chip Size and CPU Frequency Database
#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.
#| MCHCK Based / Kiibohd-dfu
if ( "${CHIP}" MATCHES "mk20dx128vlf5" )
	set( SIZE_RAM    16384 )
	set( SIZE_FLASH 126976 )
	set( F_CPU "48000000" )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 4096 )
	endif ()

#| Kiibohd-dfu
elseif ( "${CHIP}" MATCHES "mk20dx256vlh7" )
	set( SIZE_RAM    65536 )
	set( SIZE_FLASH 253952 )
	set( F_CPU "72000000" )

	# Bootloader has a lower flash restriction to fit inside protected area
	if ( BOOTLOADER )
		set( SIZE_FLASH 8192 )
	endif ()

#| Teensy 3.0
elseif ( "${CHIP}" MATCHES "mk20dx128" )
	set( SIZE_RAM    16384 )
	set( SIZE_FLASH 131072 )
	set( F_CPU "48000000" )

#| Teensy 3.1
elseif ( "${CHIP}" MATCHES "mk20dx256" )
	set( SIZE_RAM    65536 )
	set( SIZE_FLASH 262144 )
	set( F_CPU "48000000" ) # XXX Also supports 72 MHz, but may requires code changes

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
	Lib/entropy.c
	Lib/time.c
)

#| Clang needs a few more functions for linking
if ( "${COMPILER}" MATCHES "clang" )
	set( COMPILER_SRCS ${COMPILER_SRCS}
		Lib/clang.c
	)
endif ()

message( STATUS "Compiler Source Files:" )
message( "${COMPILER_SRCS}" )


#| USB Defines, this is how the loader programs detect which type of chip base is used
message( STATUS "Bootloader Type:" )
if ( "${CHIP}" MATCHES "mk20dx128vlf5" OR "${CHIP}" MATCHES "mk20dx256vlh7" )
	set( VENDOR_ID       "0x1C11" )
	set( PRODUCT_ID      "0xB04D" )
	set( BOOT_VENDOR_ID  "0x1C11" )
	set( BOOT_PRODUCT_ID "0xB007" )
	set( BOOT_DFU_ALTNAME "Kiibohd DFU" )
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
#| Bootloader Compiler Flags
if ( BOOTLOADER )

	## Clang Compiler
	if ( "${COMPILER}" MATCHES "clang" )
		# TODO Not currently working, clang doesn't support all the neccessary extensions
		message ( AUTHOR_WARNING "clang doesn't support all the needed extensions, code may need rework to use clang" )
		set ( TUNING "-D_bootloader_ -Wno-main -msoft-float -target arm-none-eabi -mtbm -nostdlib -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin -fplan9-extensions -fstrict-volatile-bitfields -flto -fno-use-linker-plugin" )

	## GCC Compiler
	else ()
		set ( TUNING "-D_bootloader_ -Wno-main -msoft-float -mthumb -fplan9-extensions -ffunction-sections -fdata-sections -fno-builtin -fstrict-volatile-bitfields -flto -fno-use-linker-plugin -nostdlib" )
		#set( TUNING "-mthumb -fdata-sections -ffunction-sections -fno-builtin -msoft-float -fstrict-volatile-bitfields -flto -fno-use-linker-plugin -fwhole-program -Wno-main -nostartfiles -fplan9-extensions -D_bootloader_" )
	endif ()

#| Firmware Compiler Flags
else ()
	## Clang Compiler
	if ( "${COMPILER}" MATCHES "clang" )
		set ( TUNING "-target arm-none-eabi -nostdlib -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin" )

	## GCC Compiler
	else ()
		set( TUNING "-mthumb -nostdlib -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin -nostartfiles" )
	endif ()
endif ()


#| Optimization level, can be [0, 1, 2, 3, s].
#|     0 = turn off optimization. s = optimize for size.
#|     (Note: 3 is not always the best optimization level.)
set( OPT "s" )


#| Compiler Flags
add_definitions( "-mcpu=${CPU} -DF_CPU=${F_CPU} -D_${CHIP}_=1 -O${OPT} ${TUNING} ${WARN} ${CSTANDARD}" )


#| Linker Flags
if ( BOOTLOADER )
	# Bootloader linker flags
	set( LINKER_FLAGS "${TUNING} -Wl,--gc-sections -fwhole-program -T${CMAKE_CURRENT_SOURCE_DIR}/../Lib/${CHIP}.bootloader.ld -nostartfiles -Wl,-Map=link.map" )
else ()
	## Clang Compiler
	if ( "${COMPILER}" MATCHES "clang" )
		set( LINKER_FLAGS "${TUNING} -Wl,-Map=link.map,--cref -Wl,--gc-sections -Wl,--no-wchar-size-warning -T${CMAKE_CURRENT_SOURCE_DIR}/Lib/${CHIP}.ld" )
	else ()
		# Normal linker flags
		set( LINKER_FLAGS "${TUNING} -Wl,-Map=link.map,--cref -Wl,--gc-sections -Wl,--no-wchar-size-warning -T${CMAKE_CURRENT_SOURCE_DIR}/Lib/${CHIP}.ld" )
	endif ()
endif ()


#| Enable color output with Ninja
if( CMAKE_GENERATOR STREQUAL "Ninja" )
	## Clang Compiler
	if ( "${COMPILER}" MATCHES "clang" )
		add_definitions( "-fcolor-diagnostics" )
	## GCC Compiler
	else ()
		add_definitions( "-fdiagnostics-color=always" )
	endif ()

	# We always use the gcc linker for arm-none-eabi
	set( LINKER_FLAGS "${LINKER_FLAGS} -fdiagnostics-color=always" )
endif ()


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

