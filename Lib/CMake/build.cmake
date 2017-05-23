###| CMAKE Kiibohd Controller Source Configurator |###
#
# Written by Jacob Alexander in 2011-2015 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Disable -Wl,-search_paths_first for OSX (not supported by avr-gcc or arm-none-eabi-gcc)
#

if ( APPLE )
	string ( REPLACE "-Wl,-search_paths_first" "" CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS} )
	string ( REPLACE "-Wl,-search_paths_first" "" CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} )
endif ()



###
# Build Targets
#

#| Create the .ELF file
if ( NOT HOST )
	set ( TARGET_OUT ${TARGET}.elf )

	# .ELF Binary
	add_executable ( ${TARGET} ${SRCS} generatedKeymap.h )

	# .ELF Properties
	set_target_properties ( ${TARGET} PROPERTIES
		LINK_FLAGS ${LINKER_FLAGS}
		SUFFIX ".elf"
	)

#| Create host library
else ()
	set ( TARGET_OUT ${CMAKE_SHARED_LIBRARY_PREFIX}${TARGET}${CMAKE_SHARED_LIBRARY_SUFFIX} )

	# Shared Library
	add_library ( ${TARGET} SHARED ${SRCS} generatedKeymap.h )

	# Shared Library Properties
	set_target_properties ( ${TARGET} PROPERTIES
		LINK_FLAGS ${LINKER_FLAGS}
	)
endif ()

#| llvm-clang does not have an objcopy equivalent
if ( "${COMPILER}" MATCHES "clang" )
	if ( "${COMPILER_FAMILY}" MATCHES "arm" )
		set ( OBJ_COPY arm-none-eabi-objcopy )
	elseif ( "${COMPILER_FAMILY}" MATCHES "arm" )
		set ( OBJ_COPY avr-objcopy )
	endif ()
else ()
	set ( OBJ_COPY ${CMAKE_OBJCOPY} )
endif ()


#| Convert the .ELF into a .bin to load onto the McHCK
#| Then sign using dfu-suffix (requries dfu-util)
if ( DEFINED DFU )
	# dfu-suffix is required to sign the dfu binary
	find_package ( DFUSuffix )

	set( TARGET_BIN ${TARGET}.dfu.bin )
	if ( DFU_SUFFIX_FOUND )
		add_custom_command( TARGET ${TARGET} POST_BUILD
			COMMAND ${OBJ_COPY} ${BIN_FLAGS} ${TARGET_OUT} ${TARGET_BIN}
			COMMAND ${DFU_SUFFIX_EXECUTABLE} --add ${TARGET_BIN} --vid ${BOOT_VENDOR_ID} --pid ${BOOT_PRODUCT_ID} 1> /dev/null
			COMMENT "Create and sign dfu bin file:  ${TARGET_BIN}"
		)
	else ()
		message ( WARNING "DFU Binary has not been signed, requires dfu-suffix..." )
		add_custom_command( TARGET ${TARGET} POST_BUILD
			COMMAND ${OBJ_COPY} ${BIN_FLAGS} ${TARGET_OUT} ${TARGET_BIN}
			COMMENT "Creating dfu binary file:      ${TARGET_BIN}"
		)
	endif ()
endif ()


#| Convert the .ELF into a .HEX to load onto the Teensy
if ( DEFINED TEENSY )
	set( TARGET_HEX ${TARGET}.teensy.hex )
	add_custom_command( TARGET ${TARGET} POST_BUILD
		COMMAND ${OBJ_COPY} ${HEX_FLAGS} ${TARGET_OUT} ${TARGET_HEX}
		COMMENT "Creating iHex file to load:    ${TARGET_HEX}"
	)
endif()


#| Generate the Extended .LSS
set( TARGET_LSS ${TARGET}.lss )
add_custom_command( TARGET ${TARGET} POST_BUILD
	COMMAND ${CMAKE_OBJDUMP} ${LSS_FLAGS} ${TARGET_OUT} > ${TARGET_LSS}
	COMMENT "Creating Extended Listing:     ${TARGET_LSS}"
)


#| Generate the Symbol Table .SYM
set( TARGET_SYM ${TARGET}.sym )
add_custom_command( TARGET ${TARGET} POST_BUILD
	COMMAND ${CMAKE_NM} -n ${TARGET_OUT} > ${TARGET_SYM}
	COMMENT "Creating Symbol Table:         ${TARGET_SYM}"
)


#| Compiler Selection Record
add_custom_command( TARGET ${TARGET} POST_BUILD
	COMMAND ${CMAKE_SOURCE_DIR}/Lib/CMake/writer compiler ${COMPILER_FAMILY}
)



###
# Size Information
#

if ( NOT DEFINED HOST )
	#| After Changes Size Information
	add_custom_target( SizeAfter ALL
		COMMAND ${CMAKE_SOURCE_DIR}/Lib/CMake/sizeCalculator ${CMAKE_SIZE} ram   ${TARGET_OUT} ${SIZE_RAM}   " SRAM"
		COMMAND ${CMAKE_SOURCE_DIR}/Lib/CMake/sizeCalculator ${CMAKE_SIZE} flash ${TARGET_OUT} ${SIZE_FLASH} "Flash"
		DEPENDS ${TARGET}
		COMMENT "Chip usage for ${CHIP}"
	)
endif ()



###
# Setup Loader Script and Program
#

#| First check for DFU based controllers
if( DEFINED DFU )
	configure_file( LoadFile/load.dfu load NEWLINE_STYLE UNIX )

#| Next check for Teensy based
elseif ( DEFINED TEENSY )
	# Provides the user with the correct teensy-loader-cli command for the built .HEX file
	# Windows
	if( CMAKE_SYSTEM_NAME MATCHES "Windows" )
		configure_file( LoadFile/winload.teensy load NEWLINE_STYLE UNIX )
	# Default
	else()
		configure_file( LoadFile/load.teensy load NEWLINE_STYLE UNIX )
	endif()
endif()



###
# Compiler Command Generation
#

#| Generate list of compiler commands for clang-tidy usage
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

#| Make sure symlink exists (for convenience)
if ( UNIX AND NOT DEFINED CONFIGURATOR )
	# Make sure symlink is created immediately
	execute_process ( COMMAND ln -sfn ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/. )

	# Also update before each build
	add_custom_command( TARGET ${TARGET} POST_BUILD
		COMMAND ln -sfn ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/.
	)
endif ()

