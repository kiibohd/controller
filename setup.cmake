###| CMAKE Kiibohd Controller Source Configurator |###
#
# Written by Jacob Alexander in 2011-2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###



###
# Project Modules
#

#| Note: This is the only section you probably want to modify
#| Each module is defined by it's own folder (e.g. Scan/Matrix represents the "Matrix" module)
#| All of the modules must be specified, as they generate the sources list of files to compile
#| Any modifications to this file will cause a complete rebuild of the project

#| Please look at the {Scan,Macro,USB,Debug}/module.txt for information on the modules and how to create new ones

##| Deals with acquiring the keypress information and turning it into a key index
set(   ScanModule "DPH" )

##| Provides the mapping functions for DefaultMap and handles any macro processing before sending to the OutputModule
set(  MacroModule "PartialMap" )

##| Sends the current list of usb key codes through USB HID
set( OutputModule "pjrcUSB" )

##| Debugging source to use, each module has it's own set of defines that it sets
set(  DebugModule "full" )



###
# Keymap Configuration
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
# Module Overrides (Used in the buildall.bash script)
#
if ( ( DEFINED ScanModuleOverride ) AND ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Scan/${ScanModuleOverride} ) )
	set( ScanModule ${ScanModuleOverride} )
endif ()



###
# Path Setup
#
set(  ScanModulePath    "Scan/${ScanModule}"   )
set( MacroModulePath   "Macro/${MacroModule}"  )
set( OutputModulePath "Output/${OutputModule}" )
set( DebugModulePath   "Debug/${DebugModule}"  )

#| Top-level directory adjustment
set( HEAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )



###
# Module Check Function
#

#| Usage:
#|  PathPrepend( ModulePath <ListOfFamiliesSupported> )
#| Uses the ${COMPILER_FAMILY} variable
function( ModuleCompatibility ModulePath )
	foreach( mod_var ${ARGN} )
		if ( ${mod_var} STREQUAL ${COMPILER_FAMILY} )
			# Module found, no need to scan further
			return()
		endif ()
	endforeach()

	message( FATAL_ERROR "${ModulePath} does not support the ${COMPILER_FAMILY} family..." )
endfunction()



###
# Module Configuration
#

#| Additional options, usually define settings
add_definitions()

#| Include path for each of the modules
add_definitions(
	-I${HEAD_DIR}/${ScanModulePath}
	-I${HEAD_DIR}/${MacroModulePath}
	-I${HEAD_DIR}/${OutputModulePath}
	-I${HEAD_DIR}/${DebugModulePath}
)




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
		# Set the path
		set( tmpSource ${tmpSource} "${SourcesPath}/${item}" )
	endforeach()

	# Finalize by writing the new list back over the old one
	set( ${Output} ${tmpSource} )
endmacro()


#| Scan Module
include    (            "${ScanModulePath}/setup.cmake"  )
PathPrepend(  SCAN_SRCS  ${ScanModulePath} ${SCAN_SRCS}  )

#| Macro Module
include    (           "${MacroModulePath}/setup.cmake"  )
PathPrepend( MACRO_SRCS ${MacroModulePath} ${MACRO_SRCS} )

#| Output Module
include    (             "${OutputModulePath}/setup.cmake"   )
PathPrepend( OUTPUT_SRCS  ${OutputModulePath} ${OUTPUT_SRCS} )

#| Debugging Module
include    (           "${DebugModulePath}/setup.cmake"  )
PathPrepend( DEBUG_SRCS ${DebugModulePath} ${DEBUG_SRCS} )


#| Default Map
# TODO Add support for different defaultMaps
configure_file( "${ScanModulePath}/defaultMap.h" defaultMap.h )


#| Print list of all module sources
message( STATUS "Detected Scan Module Source Files:" )
message( "${SCAN_SRCS}" )
message( STATUS "Detected Macro Module Source Files:" )
message( "${MACRO_SRCS}" )
message( STATUS "Detected Output Module Source Files:" )
message( "${OUTPUT_SRCS}" )
message( STATUS "Detected Debug Module Source Files:" )
message( "${DEBUG_SRCS}" )



###
# Generate USB Defines
#

#| Manufacturer name
set( MANUFACTURER "Kiibohd" )


#| Serial Number
#| Attempt to call Git to get the branch, last commit date, and whether code modified since last commit

#| Modified
#| Takes a bit of work to extract the "M " using CMake, and not using it if there are no modifications
execute_process( COMMAND git status -s -uno --porcelain
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Modified_INFO
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
string( LENGTH "${Git_Modified_INFO}" Git_Modified_LENGTH )
set( Git_Modified_Status "Clean" )
if ( ${Git_Modified_LENGTH} GREATER 2 )
	string( SUBSTRING "${Git_Modified_INFO}" 1 2 Git_Modified_Flag_INFO )
	set( Git_Modified_Status "Dirty" )
endif ()

#| Branch
execute_process( COMMAND git rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Branch_INFO
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Date
execute_process( COMMAND git show -s --format=%ci
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Date_INFO
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Commit Author and Email
execute_process( COMMAND git show -s --format="%cn <%ce>"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Commit_Author
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Commit Revision
execute_process( COMMAND git show -s --format=%H
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Commit_Revision
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Origin URL
execute_process( COMMAND git config --get remote.origin.url
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Origin_URL
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Build Date
execute_process( COMMAND "date" "+%Y-%m-%d %T %z"
	OUTPUT_VARIABLE Build_Date
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Last Commit Date
set( GitLastCommitDate "${Git_Modified_Status} ${Git_Branch_INFO} - ${Git_Date_INFO}" )

#| Uses CMake variables to include as defines
#| Primarily for USB configuration
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/Lib/_buildvars.h buildvars.h )

