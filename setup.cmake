###| CMAKE Kiibohd Controller Source Configurator |###
#
# Written by Jacob Alexander in 2011 for the Kiibohd Controller
#
# Released into the Public Domain
#
###



###
# Project Modules
#

#| Each module is defined by it's own folder (e.g. Scan/Matrix represents the "Matrix" module)
#| All of the modules must be specified, as they generate the sources list of files to compile
#| Any modifications to this file will cause a complete rebuild of the project

#| Please the {Scan,Macro,USB,Debug}/module.txt for information on the modules and how to create new ones

##| Deals with acquiring the keypress information and turning it into a key index
set(  ScanModule  "matrix" )

##| Uses the key index and potentially applies special conditions to it, mapping it to a usb key code
set( MacroModule  "basic"  )

##| Sends the current list of usb key codes through USB HID
set(   USBModule  "pjrc"   )

##| Debugging source to use, each module has it's own set of defines that it sets
set( DebugModule  "basic"  )



###
# Path Setup
# 
set(  ScanModulePath  "Scan/${ScanModule}"  )
set( MacroModulePath "Macro/${MacroModule}" )
set(   USBModulePath   "USB/${USBModule}"   )
set( DebugModulePath "Debug/${DebugModule}" )




###
# Module Configuration
#

#| Additional options, usually define settings
add_definitions()

#| Include path for each of the modules TODO Fixme!! (../)
add_definitions("
	-I../${ScanModulePath}
	-I../${MacroModulePath}
	-I../${USBModulePath}
	-I../${DebugModulePath}
")



###
# Module Processing
#

#| Go through lists of sources and append paths
#| Usage:
#|  PathPrepend( OutputListOfSources <Prepend Path> <InputListOfSources> )
macro( PathPrepend Output SourcesPath )
	unset( tmpSource )

	# Loop through items
	foreach( item ${ARGN} )
		set( tmpSource ${tmpSource} "${SourcesPath}/${item}" )
	endforeach( item )

	# Finalize by writing the new list back over the old one
	set( ${Output} ${tmpSource} )
endmacro( PathPrepend )


#| Scan Module
include(  "${ScanModulePath}/setup.cmake" )
PathPrepend( SCAN_SRCS ${ScanModulePath} ${SCAN_SRCS} )

#| Macro Module
include( "${MacroModulePath}/setup.cmake" )
PathPrepend( MACRO_SRCS ${MacroModulePath} ${MACRO_SRCS} )

#| USB Module
include(   "${USBModulePath}/setup.cmake" )
PathPrepend( USB_SRCS ${USBModulePath} ${USB_SRCS} )

#| Debugging Module
include( "${DebugModulePath}/setup.cmake" )
PathPrepend( DEBUG_SRCS ${DebugModulePath} ${DEBUG_SRCS} )


#| Print list of all module sources
message( STATUS "Detected Scan Module Source Files:
${SCAN_SRCS}")
message( STATUS "Detected Macro Module Source Files:
${MACRO_SRCS}")
message( STATUS "Detected USB Module Source Files:
${USB_SRCS}")
message( STATUS "Detected Debug Module Source Files:
${DEBUG_SRCS}")

