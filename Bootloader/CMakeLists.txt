###| CMAKE Kiibohd Controller Bootloader |###
#
# Jacob Alexander 2011-2017
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# This bootloader is based upon the MCHCK dfu-usb bootloader.
# DO NOT USE with Teensy based microcontrollers.
#
###



###
# Chip Selection
#

#| You _MUST_ set this to match the microcontroller you are trying to compile for
#| You _MUST_ clean the build directory if you change this value
#|
set ( CHIP
	"mk20dx128vlf5"    # McHCK       mk20dx128vlf5
#	"mk20dx256vlh7"    # Kiibohd-dfu mk20dx256vlh7
	CACHE STRING "Chip"
)



###
# Compiler Selection
#

#| *** EXPERIMENTAL ***
#| Stick with gcc unless you know what you're doing
#| Currently only arm is supported with clang
set ( COMPILER
	"gcc"   # arm-none-eabi-gcc / avr-gcc - Default
#       "clang" # arm-none-eabi
	CACHE STRING "Compiler Type"
)



###
# Product String
#
set ( BOOT_PRODUCT_STR
	"Kiibohd DFU Bootloader"
	CACHE STRING "Product String"
)



###
# Flashing Station ID
#
set ( FLASHING_STATION_ID
	"xXXx"
	CACHE STRING "Placeholder string for flashing station"
)



###
# Bootloader Configuration
#
set ( BOOTLOADER 1 )



###
# Compiler Intialization
#
set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/../Lib/CMake )
include ( initialize )



##
# Source Defines
#
set ( SRCS
	main.c
	dfu.c
	dfu.desc.c
	flash.c
	kinetis.c
	usb.c
)

# Only compile in if necessary
if ( CHIP STREQUAL "mk20dx256vlh7" )
	set( SRCS ${SRCS}
		debug.c
	)
endif ()


message ( STATUS "Bootloader Source Files:" )
message ( "${SRCS}" )

#| Add Lib sources to main list
foreach ( SRC_FILE ${COMPILER_SRCS} )
	set ( SRCS ${SRCS} ${CMAKE_SOURCE_DIR}/../${SRC_FILE} )
endforeach ()




###
# Directory Includes
#
include_directories (
	${CMAKE_SOURCE_DIR}/../Lib
	${CMAKE_SOURCE_DIR}/..
	${CMAKE_SOURCE_DIR}
)



###
# Project Description
#

#| Project
project ( kiibohd_bootloader C )

#| Target Name (output name)
set ( TARGET kiibohd_bootloader )

#| General Settings
cmake_minimum_required ( VERSION 2.8 )

#| Compiler Version (only available after project() command)
message( STATUS "Full Compiler:" )
message( "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_VERSION}" )



###
# Minimum Compiler Version Checks
#

# Due to optimization requirements, we have to impose a minimum GCC version on the bootloader
set ( BOOTLOADER_MIN_GCC_VERSION "4.9" )
if ( "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" )
	if ( CMAKE_C_COMPILER_VERSION VERSION_LESS "${BOOTLOADER_MIN_GCC_VERSION}" )
		message( FATAL_ERROR "arm-none-eabi-gcc ${BOOTLOADER_MIN_GCC_VERSION} or higher is required for bootloader" )
	endif ()
endif ()



###
# CMake Build Env
#

include ( buildinfo )



###
# Generate Header Files
#

configure_file ( _buildvars.h buildvars.h )
include_directories ( ${CMAKE_BINARY_DIR} )



###
# ctag Generation
#

find_package ( Ctags ) # Optional

if ( CTAGS_EXECUTABLE )
	# Generate the ctags
	execute_process( COMMAND ctags ${SRCS}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
endif ()



###
# Disable -Wl,-search_paths_first for OSX (not supported by arm-none-eabi-gcc)
#

if ( APPLE )
	string ( REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS} )
	string ( REPLACE "-Wl,-search_paths_first" "" CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} )

	message ( AUTHOR_WARNING "Bootloader binary may not fit on device (must be less than 4096 bytes). Macports arm-none-eabi-gcc 4.7.3 doesn't seem to work properly with -flto. However, even disabling it doesn't shrink the binary enough... 4.9.1 is known to work on Arch Linux." )
endif ()



###
# Build Targets
#

#| Create the .ELF file
set ( TARGET_ELF ${TARGET}.elf )
add_executable ( ${TARGET_ELF} ${SRCS} )


#| .ELF Properties
set_target_properties ( ${TARGET_ELF} PROPERTIES
	LINK_FLAGS ${LINKER_FLAGS}
	SUFFIX ""                               # XXX Force Windows to keep the .exe off
)


#| Convert the .ELF into a .bin to load onto the McHCK
set ( TARGET_BIN ${TARGET}.bin )
add_custom_command ( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} ${BIN_FLAGS} ${TARGET_ELF} ${TARGET_BIN}
	COMMENT "Creating binary file to load:  ${TARGET_BIN}"
)


#| Generate the Extended .LSS
set ( TARGET_LSS ${TARGET}.lss )
add_custom_command ( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND ${CMAKE_OBJDUMP} ${LSS_FLAGS} ${TARGET_ELF} > ${TARGET_LSS}
	COMMENT "Creating Extended Listing:     ${TARGET_LSS}"
)


#| Generate the Symbol Table .SYM
set ( TARGET_SYM ${TARGET}.sym )
add_custom_command ( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND ${CMAKE_NM} -n ${TARGET_ELF} > ${TARGET_SYM}
	COMMENT "Creating Symbol Table:         ${TARGET_SYM}"
)


#| Compiler Selection Record
add_custom_command ( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND ${CMAKE_SOURCE_DIR}/../Lib/CMake/writer compiler ${COMPILER_FAMILY}
)



###
# Size Information
#

#| After Changes Size Information
add_custom_target ( SizeAfter ALL
	COMMAND ${CMAKE_SOURCE_DIR}/../Lib/CMake/sizeCalculator ${CMAKE_SIZE} ram   ${TARGET_ELF} ${SIZE_RAM}   " SRAM"
	COMMAND ${CMAKE_SOURCE_DIR}/../Lib/CMake/sizeCalculator ${CMAKE_SIZE} flash ${TARGET_ELF} ${SIZE_FLASH} "Flash"
	DEPENDS ${TARGET_ELF}
	COMMENT "Chip usage for ${CHIP}"
)

