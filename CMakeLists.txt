###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2014
# Due to this file's usefulness:
#
# Released into the Public Domain
#
###

#| Windows / Cygwin Compatibility options
set( CMAKE_LEGACY_CYGWIN_WIN32 0 )
set( CMAKE_USE_RELATIVE_PATHS  1 )

#| Add Dependency Macro
include( AddFileDependencies )



###
# Compiler Family
#

#| Specify the compiler family to use
#| Currently only supports AVR and ARM
#| "avr"       # Teensy   1.0
#| "avr"       # Teensy   2.0
#| "avr"       # Teensy++ 1.0
#| "avr"       # Teensy++ 2.0
#| "arm"       # Teensy   3.0
#| "arm"       # Teensy   3.1
set( COMPILER_FAMILY "arm" )
#set( COMPILER_FAMILY "avr" )

message( STATUS "Compiler Family:" )
message( "${COMPILER_FAMILY}" )



#| Load the compiler family specific configurations
include( ${COMPILER_FAMILY}.cmake )



###
# Project Description
#

#| Project
project( kiibohd_controller )

#| Target Name (output name)
set( TARGET kiibohd )

#| General Settings
cmake_minimum_required( VERSION 2.8 )



###
# Source Defines
#

#| Sources (see setup.h for configuring in/away code blocks or other complete modules)
#| XXX Not set here in this project, see setup.cmake
#set( SRCS ./main.c )

#| Instead, include the module source selector
include( setup.cmake )
set( SRCS
	main.c
	${COMPILER_SRCS}
	${SCAN_SRCS}
	${MACRO_SRCS}
	${OUTPUT_SRCS}
	${DEBUG_SRCS}
)

#| Directories to include by default
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )



###
# Module Compatibility Check
#

#| Check for whether the set modules are compatible with the specified compiler family
ModuleCompatibility( ${ScanModulePath}   ${ScanModuleCompatibility}   )
ModuleCompatibility( ${MacroModulePath}  ${MacroModuleCompatibility}  )
ModuleCompatibility( ${OutputModulePath} ${OutputModuleCompatibility} )
ModuleCompatibility( ${DebugModulePath}  ${DebugModuleCompatibility}  )



###
# Build Targets
#

#| Create the .ELF file
set( TARGET_ELF ${TARGET}.elf )
add_executable( ${TARGET_ELF} ${SRCS} )


#| .ELF Properties
set_target_properties( ${TARGET_ELF} PROPERTIES
	LINK_FLAGS ${LINKER_FLAGS}
	SUFFIX ""                               # XXX Force Windows to keep the .exe off
)


#| Convert the .ELF into a .HEX to load onto the Teensy
set( TARGET_HEX ${TARGET}.hex )
add_custom_command( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND ${OBJCOPY} ${HEX_FLAGS} ${TARGET_ELF} ${TARGET_HEX}
	COMMENT "Creating load file for Flash:  ${TARGET_HEX}"
)


#| Generate the Extended .LSS
set( TARGET_LSS ${TARGET}.lss )
add_custom_command( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND ${OBJDUMP} ${LSS_FLAGS} ${TARGET_ELF} > ${TARGET_LSS}
	COMMENT "Creating Extended Listing:     ${TARGET_LSS}"
)


#| Generate the Symbol Table .SYM
set( TARGET_SYM ${TARGET}.sym )
add_custom_command( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND ${NM} -n ${TARGET_ELF} > ${TARGET_SYM}
	COMMENT "Creating Symbol Table:         ${TARGET_SYM}"
)



###
# Size Information
#

#| After Changes Size Information
add_custom_target( SizeAfter ALL ${SIZE} --target=${FORMAT} ${TARGET_HEX} ${TARGET_ELF}
	DEPENDS ${TARGET_ELF}
	COMMENT "Size after generation:"
)



###
# Setup Loader Script
#

#| Provides the user with the correct teensy-loader-cli command for the built .HEX file
#| teensy-loader-cli must be in the user's path
if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
	configure_file( LoadFile/bash load )
endif()

#| TODO Windows
if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
	configure_file( LoadFile/bash load )
endif()

