###| CMAKE Kiibohd Controller Initialization |###
#
# Written by Jacob Alexander in 2011-2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


#| Windows / Cygwin Compatibility options
set( CMAKE_LEGACY_CYGWIN_WIN32 0 )
set( CMAKE_USE_RELATIVE_PATHS  1 )


#| Prevent In-Source Builds
set( CMAKE_DISABLE_SOURCE_CHANGES  ON )
set( CMAKE_DISABLE_IN_SOURCE_BUILD ON )


###
# Compiler Lookup
#

#| avr match
if ( "${CHIP}" MATCHES "^at90usb.*$" OR "${CHIP}" MATCHES "^atmega.*$" )
	set( COMPILER_FAMILY "avr" )

#| arm match
elseif ( "${CHIP}" MATCHES "^mk20dx.*$" )
	set( COMPILER_FAMILY "arm" )

#| Invalid CHIP
else ()
	message( FATAL_ERROR "CHIP: ${CHIP} - Unknown chip, could not choose compiler..." )
endif ()

#| Results of Compiler Lookup
message( STATUS "Compiler Family:" )
message( "${COMPILER_FAMILY}" )

#| Compiler Selection Record
#|  This is used to check if the chip target has changed (a complete cmake reset is needed)
if ( EXISTS compiler )
	file( READ ${CMAKE_BINARY_DIR}/compiler COMPILER_RECORD )

	# Detect case if a full cmake reset is required
	if ( NOT COMPILER_FAMILY STREQUAL COMPILER_RECORD )
		message( FATAL_ERROR "Changing compilers requires a cmake reset\ne.g. rm -rf *; cmake .." )
	endif()
endif ()

#| Load the compiler family specific configurations
include( ${COMPILER_FAMILY} )

#| Binutils not set by CMake
set( CMAKE_SIZE "${_CMAKE_TOOLCHAIN_PREFIX}size" )

