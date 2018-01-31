###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2016-2017
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# Host Compiler CMake Build Configuration
#
###



#| Set the Compilers (must be set first)
message( STATUS "Compiler Selected:" )
if ( "${COMPILER}" MATCHES "gcc" )
	set( CMAKE_C_COMPILER gcc )
	message( "gcc" )
elseif ( "${COMPILER}" MATCHES "clang" )
	set( CMAKE_C_COMPILER clang )
	set( _CMAKE_TOOLCHAIN_PREFIX llvm- )
	message( "clang" )
else ()
	message( AUTHOR_WARNING "COMPILER: ${COMPILER} - Unknown compiler selection" )
endif ()

# If Apple and gcc, then this is actually clang
if ( APPLE AND "${COMPILER}" MATCHES "gcc" )
	set( COMPILER "clang" )
endif ()



###
# OS Specific Configuration
#
set( CMAKE_MACOSX_RPATH 1 )



###
# System Compiler Defines and Linker Options
#

#| Indicate that we are building a host binary
set( HOST 1 )

#| Extra Compiler Sources
#| Mostly for convenience functions like interrupt handlers
set( COMPILER_SRCS
	Lib/entropy.c
	Lib/host.c
	Lib/periodic.c
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


#| Compiler flag to set the C Standard level.
#|     c89   = "ANSI" C
#|     gnu89 = c89 plus GCC extensions
#|     c99   = ISO C99 standard (not yet fully implemented)
#|     gnu99 = c99 plus GCC extensions
#|     gnu11 = c11 plus GCC extensions
set( CSTANDARD "-std=gnu99" ) # Staying with gnu99 for compatibility with older compilers


#| Warning Options
#|  -Wall...:     warning level
set( WARN "-Wall -ggdb3" )


#| Tuning Options
#|  -f...:        tuning, see GCC manual
#| NOTE: -fshort-wchar is specified to allow USB strings be passed conveniently
#| Clang Compiler
if ( "${COMPILER}" MATCHES "clang" )
	if ( APPLE )
		set( TUNING "-fshort-enums -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin" )
	else ()
		set( TUNING "-nostdlib -fshort-enums -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin" )
	endif ()

	# Set compiler utilities for clang on macOS
	# Requires homebrew binutils
	if ( APPLE )
		set ( CMAKE_OBJDUMP gobjdump )
		set ( CMAKE_NM gnm )
	endif ()

#| GCC Compiler
else()
	if ( "${DETECTED_BUILD_KERNEL}" MATCHES "CYGWIN" )
		set( TUNING "-fshort-enums -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin -nostartfiles" )
	else ()
		set( TUNING "-nostdlib -fshort-enums -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin -nostartfiles" )
	endif ()
endif()


#| Enable color output with Ninja
if( CMAKE_GENERATOR STREQUAL "Ninja" )
	## Clang Compiler
	if ( "${COMPILER}" MATCHES "clang" )
		set( TUNING "${TUNING} -fcolor-diagnostics" )
	## GCC Compiler
	else ()
		set( TUNING "${TUNING} -fdiagnostics-color=always" )
	endif ()
endif ()


#| OS Specific defines
if ( APPLE )
	set( OS_TUNING -D_APPLE_=1 -fno-stack-protector )
endif ()


#| Compiler Flags
add_definitions( -D_host_=1 ${OS_TUNING} ${TUNING} ${WARN} ${CSTANDARD} )


#| Linker Flags
if ( APPLE )
	set( LINKER_FLAGS "${TUNING}" )
else ()
	set( LINKER_FLAGS "${TUNING} -Wl,-Map=link.map,--cref -Wl,--gc-sections" )
endif ()


#| Lss Flags
if ( "${COMPILER}" MATCHES "clang" AND NOT APPLE )
	set( LSS_FLAGS -section-headers )
else ()
	set( LSS_FLAGS -h -S -z )
endif ()

