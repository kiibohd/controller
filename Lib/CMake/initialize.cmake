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

#| Load the compiler family specific configurations
include( Lib/CMake/${COMPILER_FAMILY}.cmake )

#| Binutils not set by CMake
set( CMAKE_SIZE "${_CMAKE_TOOLCHAIN_PREFIX}size" )

