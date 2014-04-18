###| CMAKE teensy-loader-cli |###
#
# Jacob Alexander 2014
# Written to replace the pjrc's kludey Makefiles
#  (that require hand edits for different platforms)
#
# Released into the Public Domain
#
###

#| Windows / Cygwin Compatibility options
set( CMAKE_LEGACY_CYGWIN_WIN32 0 )
set( CMAKE_USE_RELATIVE_PATHS  1 )



###
# Project Description
#

#| Project
project( teensy-loader-cli )

#| Target Name (output name)
set( TARGET teensy-loader-cli )

#| General Settings
cmake_minimum_required( VERSION 2.8 )



###
# Source Defines
#

#| Sources
set( SRCS
	teensy_loader_cli.c
)



###
# Platform Setup
#
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ) # Use local find scripts

#| Linux - libusb
if( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	# Find libusb (not 1.0)
	find_package( LibUSB REQUIRED )

	# Defines
	set( DEFINES -s -DUSE_LIBUSB )

	# Include directories
	set( INCLUDE_DIRS ${LIBUSB_INCLUDE_DIRS} )

	# Libraries
	set( LIBS ${LIBUSB_LIBRARIES} )

#| Windows
elseif( CMAKE_SYSTEM_NAME MATCHES "CYGWIN" )
	message( AUTHOR_WARNING "Not Tested...")

	# Defines
	set( DEFINES -s -DUSE_WIN32 )

	# Libraries
	set( LIBS hid setupapi )

#| Mac OS X
elseif( CMAKE_SYSTEM_NAME MATCHES "Darwin" )
	message( AUTHOR_WARNING "Not Tested...")

	# Defines - XXX What is SDK?
	set( DEFINES -DUSE_APPLE_IOKIT -isysroot ${SDK} -Wl,-syslibroot,${SDK} -framework IOKit -framework CoreFoundation )

#| BSD - NetBSD and OpenBSD
elseif( CMAKE_SYSTEM_NAME MATCHES "BSD" )
	message( AUTHOR_WARNING "Not Tested...")

	# Defines
	set( DEFINES -s -DUSE_UHID )
#| Unregonized OS
else()
	message( FATAL_ERROR "${CMAKE_SYSTEM_NAME}: OS Not Recognized..." )
endif()



###
# Defines
#

#| Default CFLAGS
set( CFLAGS -O2 -Wall )

add_definitions( ${CFLAGS} ${DEFINES} )



###
# Includes
#

#| Linux
include_directories( ${INCLUDE_DIRS} )



###
# Build Targets
#

#| Create the executable
add_executable( ${TARGET} ${SRCS} )

#| Link executable
target_link_libraries( ${TARGET} ${LIBS} )

