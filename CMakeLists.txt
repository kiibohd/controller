###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2017
# Due to this file's usefulness:
#
# Released into the Public Domain
#
###



###
# Chip Selection
#

#| You _MUST_ set this to match the microcontroller you are trying to compile for
#| You _MUST_ clean the build directory if you change this value
#|
set( CHIP
#       "at90usb162"       # Teensy   1.0 (avr)
#       "atmega32u4"       # Teensy   2.0 (avr)
#       "at90usb646"       # Teensy++ 1.0 (avr)
#       "at90usb1286"      # Teensy++ 2.0 (avr)
#       "mk20dx128"        # Teensy   3.0 (arm)
	"mk20dx128vlf5"    # McHCK       mk20dx128vlf5
#       "mk20dx256"        # Teensy   3.1,3.2 (arm)
#       "mk20dx256vlh7"    # Kiibohd-dfu mk20dx256vlh7
#       "host"             # System compiler test environment
	CACHE STRING "Microcontroller Chip"
)



###
# Compiler Selection
#

#| gcc has been tested much more (and will likely give smaller binaries)
#| clang does work though
#| Currently only arm is supported with clang
set( COMPILER
	"gcc"   # arm-none-eabi-gcc / avr-gcc / host-gcc - Default
#       "clang" # arm-none-eabi / host
	CACHE STRING "Compiler Type"
)



###
# Compiler Intialization
#
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/Lib/CMake )
include( initialize )



###
# Project Modules
#

#| Note: This is the only section you probably want to modify
#| Each module is defined by it's own folder (e.g. Scan/Matrix represents the "Matrix" module)
#| All of the modules must be specified, as they generate the sources list of files to compile
#| Any modifications to this file will cause a complete rebuild of the project

#| Please look at the {Scan,Macro,Output,Debug} for information on the modules and how to create new ones

##| Deals with acquiring the keypress information and turning it into a key index
set(   ScanModule "MD1"
	CACHE STRING "Scan Module" )

##| Provides the mapping functions for DefaultMap and handles any macro processing before sending to the OutputModule
set(  MacroModule "PartialMap"
	CACHE STRING "Macro Module" )

##| Sends the current list of usb key codes through USB HID
set( OutputModule "pjrcUSB"
	CACHE STRING "Output Module"
)

##| Debugging source to use, each module has it's own set of defines that it sets
set(  DebugModule "full"
	CACHE STRING "Debug Module"
)



###
# Keymap Configuration (do not include the .kll extension)
#

#| Do not include the .kll extension
#| * BaseMap maps the native keyboard scan codes to USB Codes so the layout is compatible with all other layouts
#| * DefaultMap allows the default keymap to be modified from the BaseMap
#| * PartialMaps is a set of dynamically set layers (there is no limit, but too many may use up too much RAM...)
#| BaseMap generally does not need to be changed from "defaultMap"
#|
#| Syntax:
#|  myMap
#|    * defines a single .kll layout file, double-quotes are needed to distinguish between layers
#|  "myMap specialLayer"
#|    * defines myMap to be the main layout, then replace specialLayers on top of it
#|
#| - Only for PartialMaps -
#|  "myMap specialLayer" "myMap colemak" dvorak
#|    * As before, but also generates a second layer at index 2 and third at index 3
#|
#| NOTE:  Remember to add key(s) to enable each Partial Layer
#| NOTE2: Layers are always based up the BaseMap (which should be an ANSI-like mapping)
#| NOTE3: Compiler looks in kll/layouts and the build directory for layout files (precedence on build directory)

##| Set the base keyboard .kll map, defaults to "defaultMap" if not found
##| Looks in Scan/<Module Name> for the available BaseMaps
set(     BaseMap "defaultMap"
	CACHE STRING "KLL BaseMap/Scancode Keymapping" )

##| Layer additonal .kll maps on the BaseMap, layers are in order from 1st to nth
##| Can be set to ""
set(  DefaultMap "md1Overlay stdFuncMap"
	CACHE STRING "KLL DefaultMap" )

##| ParitalMaps available on top of the BaseMap. See above for syntax on specifying multiple layers vs. layering
##| Can be set to ""
set( PartialMaps "hhkbpro2"
	CACHE STRING "KLL PartialMaps/Layer Definitions" )



###
# Source Defines (in addition to the selected Modules)
#
set( MAIN_SRCS
	main.c
)



###
# Project Description
#

#| Project
project( kiibohd_controller C )

#| Target Name (output name)
set( TARGET kiibohd )

#| Manufacturer name
#| - This can be overridden by a Scan Module setup.cmake -
#| USB Descriptor iManufacturer string
set( MANUFACTURER "Kiibohd" )

#| Flashing Station name
#| This is a placeholder value, the flashing process will overwrite this value for you
set( FLASHING_STATION_ID "xXXx" )

#| General Settings
cmake_minimum_required( VERSION 2.8 )

#| Compiler Version (only available after project() command)
message( STATUS "Full Compiler:" )
message( "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_VERSION}" )



###
# Module Initialization / Compilation / Targets
#
include( modules )
include( kll ) # Generate kll layouts if necessary
include( build )

