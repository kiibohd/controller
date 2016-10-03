###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2016
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
	Lib/host.c
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
	# TODO
	set ( TUNING "-nostdlib -fshort-enums -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin" )

#| GCC Compiler
else()
	# TODO
	set( TUNING "-nostdlib -fshort-enums -fdata-sections -ffunction-sections -fshort-wchar -fno-builtin -nostartfiles" )
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


#| Dependency Files
#| Compiler flags to generate dependency files.
set( GENDEPFLAGS "-MD" )


#| Compiler Flags
add_definitions( -D_host_=1 ${TUNING} ${WARN} ${CSTANDARD} ${GENDEPFLAGS} )


#| Linker Flags
set( LINKER_FLAGS "${TUNING} -Wl,-Map=link.map,--cref -Wl,--gc-sections" )


#| Lss Flags
if ( "${COMPILER}" MATCHES "clang" )
	set( LSS_FLAGS -section-headers )
else ()
	set( LSS_FLAGS -h -S -z )
endif ()

