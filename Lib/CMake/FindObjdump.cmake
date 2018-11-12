# The module defines the following variables:
#   OBJDUMP_EXECUTABLE - path to dfu-suffix command line client
#   OBJDUMP_FOUND - true if the command line client was found
#   OBJDUMP_VERSION_STRING - the version of dfu-suffix found (since CMake 2.8.8)
# Example usage:
#   find_package( DFUSuffix )
#   if( OBJDUMP_FOUND )
#     message("objdump found: ${OBJDUMP_EXECUTABLE}")
#   endif()
# Requires OBJDUMP be set

find_program ( OBJDUMP_EXECUTABLE
	NAMES ${CMAKE_OBJDUMP}
	DOC "objdump executable"
)
mark_as_advanced ( OBJDUMP_EXECUTABLE )

if ( OBJDUMP_EXECUTABLE )
	execute_process ( COMMAND ${OBJDUMP_EXECUTABLE} --version
		OUTPUT_VARIABLE objdump_version
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if ( objdump_version MATCHES "^GNU objdump \\(GNU Binutils\\)" )
		string ( REPLACE "\n" "" OBJDUMP_VERSION_STRING ${objdump_version} )
		string ( REPLACE "GNU objdump (GNU Binutils) " "" OBJDUMP_VERSION_STRING ${OBJDUMP_VERSION_STRING} )
		string ( REGEX REPLACE "Copyright .*$" "" OBJDUMP_VERSION_STRING ${OBJDUMP_VERSION_STRING} )
	endif ()
	unset ( objdump_version )
endif ()

# Handle the QUIETLY and REQUIRED arguments and set OBJDUMP_FOUND to TRUE if
# all listed variables are TRUE

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( OBJDUMP
	REQUIRED_VARS OBJDUMP_EXECUTABLE
	VERSION_VAR OBJDUMP_VERSION_STRING
)

