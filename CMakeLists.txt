###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2014
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
#	"at90usb162"       # Teensy   1.0 (avr)
#	"atmega32u4"       # Teensy   2.0 (avr)
#	"at90usb646"       # Teensy++ 1.0 (avr)
	"at90usb1286"      # Teensy++ 2.0 (avr)
#	"mk20dx128"        # Teensy   3.0 (arm)
#	"mk20dx256"        # Teensy   3.1 (arm)
)



###
# Compiler Intialization
#
include( Lib/CMake/initialize.cmake )



###
# Project Modules
#

#| Note: This is the only section you probably want to modify
#| Each module is defined by it's own folder (e.g. Scan/Matrix represents the "Matrix" module)
#| All of the modules must be specified, as they generate the sources list of files to compile
#| Any modifications to this file will cause a complete rebuild of the project

#| Please look at the {Scan,Macro,Output,Debug} for information on the modules and how to create new ones

##| Deals with acquiring the keypress information and turning it into a key index
set(   ScanModule "DPH" )

##| Provides the mapping functions for DefaultMap and handles any macro processing before sending to the OutputModule
set(  MacroModule "PartialMap" )

##| Sends the current list of usb key codes through USB HID
set( OutputModule "pjrcUSB" )

##| Debugging source to use, each module has it's own set of defines that it sets
set(  DebugModule "full" )



###
# Keymap Configuration (XXX - Not worky yet, currently ignored)
#

##| If there are multiple DefaultMaps, it is defined here. If, the specified DefaultMap is not found, defaultMap.h is used.
set(   DefaultMap "kishsaver" )

##| PartialMap combined keymap layering. The first keymap has the "least" precedence.
set(  CombinedMap colemak capslock2ctrl )

##| ParitalMaps available on top of the CombinedMap. If there are input conflicts, the last PartialMap takes precedence.
set(  PartialMaps hhkbnav kbdctrl )

##| MacroSets define extra capabilities that are not provided by the Scan or Output modules. Last MacroSet takes precedence.
set(    MacroSets retype )



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
project( kiibohd_controller )

#| Target Name (output name)
set( TARGET kiibohd )

#| General Settings
cmake_minimum_required( VERSION 2.8 )



###
# Module Initialization / Compilation / Targets
#
include( Lib/CMake/modules.cmake )

