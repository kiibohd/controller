# The module defines the following variables:
#   DFU_SUFFIX_EXECUTABLE - path to dfu-suffix command line client
#   DFU_SUFFIX_FOUND - true if the command line client was found
#   DFU_SUFFIX_VERSION_STRING - the version of dfu-suffix found (since CMake 2.8.8)
# Example usage:
#   find_package( DFUSuffix )
#   if( DFU_SUFFIX_FOUND )
#     message("dfu-suffix found: ${DFU_SUFFIX_EXECUTABLE}")
#   endif()

find_program ( DFU_SUFFIX_EXECUTABLE
	NAMES dfu-suffix
	DOC "dfu-suffix executable"
)
mark_as_advanced ( DFU_SUFFIX_EXECUTABLE )

if ( DFU_SUFFIX_EXECUTABLE )
	execute_process ( COMMAND ${DFU_SUFFIX_EXECUTABLE} --version
		OUTPUT_VARIABLE dfu_suffix_version
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	if ( dfu_suffix_version MATCHES "^dfu-suffix \\(dfu-util\\)" )
		string ( REPLACE "\n" "" DFU_SUFFIX_VERSION_STRING ${dfu_suffix_version} )
		string ( REPLACE "dfu-suffix (dfu-util) " "" DFU_SUFFIX_VERSION_STRING ${DFU_SUFFIX_VERSION_STRING} )
		string ( REGEX REPLACE "Copyright .*$" "" DFU_SUFFIX_VERSION_STRING ${DFU_SUFFIX_VERSION_STRING} )
	endif ()
	unset ( dfu_suffix_version )
endif ()

# Handle the QUIETLY and REQUIRED arguments and set DFU_SUFFIX_FOUND to TRUE if
# all listed variables are TRUE

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( DFU_SUFFIX
	REQUIRED_VARS DFU_SUFFIX_EXECUTABLE
	VERSION_VAR DFU_SUFFIX_VERSION_STRING
)

