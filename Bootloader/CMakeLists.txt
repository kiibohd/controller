###| CMAKE Kiibohd Controller Bootloader |###
#
# Jacob Alexander 2011-2018
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
#	"sam4s8c"          # Sam
	CACHE STRING "Chip"
)

if ( "${CHIP}" MATCHES "^mk2.*$" )
	set( FAMILY "kinetis" )
elseif ( "${CHIP}" MATCHES "^sam.*$" )
	set( FAMILY "sam" )
else ( )
	message( FATAL_ERROR "Unknown chip ${CHIP}" )
endif ( )


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
# Debug
#

# Use a JLink programmer (no bootloader required)
set( JLINK
	1
	CACHE STRING "Enable JLINK Programming/Debugging"
)

#| swd or jtag
set( JLINK_INTERFACE
	"swd"
	CACHE STRING "JLink Interface"
)

#| Speed in kHz or auto
set( JLINK_SPEED
	"auto"
	CACHE STRING "JLink Speed"
)


###
# Compiler Intialization
#
set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/../Lib/CMake )
include ( initialize )


##
# Source Defines
#
if ( FAMILY STREQUAL "kinetis" )
set ( SRCS
	main.c
	debug.c
	dfu.c
	dfu.desc.c
	flash.c
	usb.c
	${FAMILY}.c
	Devices/${CHIP}.c
)

elseif ( FAMILY STREQUAL "sam" )
set ( SRCS
	../Lib/ASF/common/services/clock/sam4s/sysclk.c
	../Lib/ASF/common/services/isp/flip/sam/isp.c
	../Lib/ASF/common/services/usb/class/dfu_flip/device/udi_dfu_atmel.c
	../Lib/ASF/common/services/usb/class/dfu_flip/device/udi_dfu_atmel_desc.c
	../Lib/ASF/common/services/usb/udc/udc.c
	../Lib/ASF/common/utils/interrupt/interrupt_sam_nvic.c
	../Lib/ASF/sam/drivers/efc/efc.c
	../Lib/ASF/sam/drivers/matrix/matrix.c
	../Lib/ASF/sam/drivers/pmc/pmc.c
	../Lib/ASF/sam/drivers/udp/udp_device.c
	../Lib/ASF/sam/drivers/wdt/wdt.c
	../Lib/ASF/sam/services/flash_efc/flash_efc.c
	../Lib/ASF/sam/utils/cmsis/sam4s/source/templates/system_sam4s.c

	swd/DAP.c
	swd/SW_DP.c
	swd/swd_host.c
	swd/target_reset_nrf52.c

	main.c
	debug.c
	dfu.c
	dfu.desc.c
	flash.c
	usb.c
	Devices/${CHIP}.c
)

add_definitions(
	-D__SAM4S8C__
	-DBOARD=SAM4S_XPLAINED_PRO
	-DDFU_EXTRA_BLE_SWD_SUPPORT=${DFU_EXTRA_BLE_SWD_SUPPORT}
)

else ( )
	message( FATAL_ERROR "Unsupported family ${FAMILY}" )

endif ( )


# Add device file, if it exists
string ( REPLACE " " "_" BOOT_PRODUCT_STR_CLEAN ${BOOT_PRODUCT_STR} )
if ( EXISTS ${CMAKE_SOURCE_DIR}/Devices/${BOOT_PRODUCT_STR_CLEAN}.c )
	set ( SRCS ${SRCS}
		Devices/${BOOT_PRODUCT_STR_CLEAN}.c
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

	"../Lib/ASF"
	"../Lib/ASF/config"
	"../Lib/ASF/common/boards"
	"../Lib/ASF/common/services/isp/flip"
	"../Lib/ASF/common/services/isp/flip/module_config"
	"../Lib/ASF/common/services/sleepmgr"
	"../Lib/ASF/common/services/usb/class/dfu_flip"
	"../Lib/ASF/common/services/usb/class/dfu_flip/device"
	"../Lib/ASF/sam/boards"
	"../Lib/ASF/sam/utils"
	"../Lib/ASF/sam/utils/cmsis/sam4s/include"
	"../Lib/ASF/sam/utils/header_files"
	"../Lib/ASF/sam/utils/preprocessor"
	"../Lib/CMSIS/Include"
	"../Lib/CMSIS/Lib/GCC"
)



###
# Project Description
#

#| Project
project ( kiibohd_bootloader C )

#| Target Name (output name)
set ( TARGET kiibohd_bootloader )

#| General Settings
cmake_minimum_required ( VERSION 3.0 )

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
	execute_process( COMMAND ${CTAGS_EXECUTABLE} ${SRCS}
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
set ( TARGET_HEX ${TARGET}.hex )
set(
	TARGET_ADDRESS "0x0"
	CACHE STRING "Bootloader starting address"
)
add_custom_command ( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND ${CMAKE_OBJCOPY} ${BIN_FLAGS} ${TARGET_ELF} ${TARGET_BIN}
	COMMAND ${CMAKE_OBJCOPY} -O ihex ${TARGET_ELF} ${TARGET_HEX}
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


###
# Setup Loader Script and Program
#

if ( JLINK )
configure_file( ${CMAKE_SOURCE_DIR}/../LoadFile/load.jlink load NEWLINE_STYLE UNIX )
configure_file( ${CMAKE_SOURCE_DIR}/../LoadFile/dump.jlink dump NEWLINE_STYLE UNIX @ONLY )
configure_file( ${CMAKE_SOURCE_DIR}/../LoadFile/debug.jlink debug NEWLINE_STYLE UNIX )
configure_file( ${CMAKE_SOURCE_DIR}/../LoadFile/rtt.jlink rtt NEWLINE_STYLE UNIX )
configure_file( ${CMAKE_SOURCE_DIR}/../LoadFile/reset.jlink reset NEWLINE_STYLE UNIX )
endif()
