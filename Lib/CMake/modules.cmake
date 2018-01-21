###| CMAKE Kiibohd Controller Source Configurator |###
#
# Written by Jacob Alexander in 2011-2017 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# CMake Custom Modules Path
#

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/Lib/CMake/" )



###
# Host Build Mode (Override ScanModule and OutputModules)
# Or Normal Path Setup
#
if ( HostBuild )
	set(  ScanModulePath    "Scan/TestIn"          )
	set( MacroModulePath   "Macro/${MacroModule}"  )
	set( OutputModulePath "Output/TestOut"         )
	set( DebugModulePath   "Debug/${DebugModule}"  )

#| Normal Path Setup
else ()
	set(  ScanModulePath    "Scan/${ScanModule}"   )
	set( MacroModulePath   "Macro/${MacroModule}"  )
	set( OutputModulePath "Output/${OutputModule}" )
	set( DebugModulePath   "Debug/${DebugModule}"  )
endif ()

#| Top-level directory adjustment
set( HEAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )



###
# Module Check Function
#

function ( ModuleCompatibility ModulePath )
	foreach ( mod_var ${ARGN} )
		if ( ${mod_var} STREQUAL ${COMPILER_FAMILY} )
			# Module found, no need to scan further
			return()
		endif ()
	endforeach ()

	message ( FATAL_ERROR "${ModulePath} does not support the ${COMPILER_FAMILY} family..." )
endfunction ()



###
# Module Processing
#

#| Go through lists of sources and append paths
#| Usage:
#|  PathPrepend( OutputListOfSources <Prepend Path> <InputListOfSources> )
macro ( PathPrepend Output SourcesPath )
	unset ( tmpSource )

	# Loop through items
	foreach ( item ${ARGN} )
		# If the leading character is a / treat as an absolute path
		string ( SUBSTRING "${item}" 0 1 character )
		string ( SUBSTRING "${item}" 1 2 windows_drv )
		if ( character STREQUAL "/" )
			set ( tmpSource ${tmpSource} "${item}" )

		# Check if a Windows Drive path
		elseif ( windows_drv STREQUAL ":/" )
			set ( tmpSource ${tmpSource} "${item}" )

		# Otherwise just set the path
		else ()
			set ( tmpSource ${tmpSource} "${SourcesPath}/${item}" )
		endif ()
	endforeach ()

	# Finalize by writing the new list back over the old one
	set ( ${Output} ${tmpSource} )
endmacro ()



###
# Add Module Macro
#
# Optional Arg 1: Main Module Check, set to True/1 if adding a main module

function ( AddModule ModuleType ModuleName )
	# Module path
	set ( ModulePath                 ${ModuleType}/${ModuleName} )
	set ( ModuleFullPath ${HEAD_DIR}/${ModuleType}/${ModuleName} )

	# Include setup.cmake file
	include ( ${ModuleFullPath}/setup.cmake )

	# Check if this is a main module add
	foreach ( extraArg ${ARGN} )
		# Make sure this isn't a submodule
		if ( DEFINED SubModule )
			message ( FATAL_ERROR
			"The '${ModuleName}' module is not a stand-alone module, and requires further setup."
			)
		endif ()
	endforeach ()

	# PathPrepend to give proper paths to each of the source files
	PathPrepend ( Module_SRCS ${ModulePath} ${Module_SRCS} )

	# Check the current scope to see if a sub-module added some source files
	# Append each of the sources to each type of module srcs list
	set ( ${ModuleType}_SRCS ${${ModuleType}_SRCS} ${Module_SRCS} )

	# Add .h files
	add_definitions ( -I${ModuleFullPath} )

	# Check module compatibility
	# Ignore if overriding the compiler family
	if ( NOT CompilerOverride )
		ModuleCompatibility( ${ModulePath} ${ModuleCompatibility} )
	endif ()

	# Check if this is a main module add
	foreach ( extraArg ${ARGN} )
		# Display detected source files
		if ( NOT DEFINED SubModule )
			message ( STATUS "Detected ${ModuleType} Module Source Files:" )
			message ( "${${ModuleType}_SRCS}" )
		endif ()
	endforeach ()

	# Check for any capabilities.kll files in the Module
	set ( kll_capabilities_file "${ModuleFullPath}/capabilities.kll" )
	if ( EXISTS ${kll_capabilities_file} )
		# Add the kll file and any submodule kll files to the running list
		set ( ${ModuleType}Module_KLL ${${ModuleType}Module_KLL} ${kll_capabilities_file} )
	endif ()


	# Finally, add the sources and kll files to the parent scope (i.e. return)
	set ( ${ModuleType}_SRCS ${${ModuleType}_SRCS} PARENT_SCOPE )
	set ( ${ModuleType}Module_KLL ${${ModuleType}Module_KLL} PARENT_SCOPE )
endfunction ()


#| Add main modules
if ( HostBuild )
	AddModule ( Scan   TestIn          1 )
	AddModule ( Macro  ${MacroModule}  1 )
	AddModule ( Output TestOut         1 )
	AddModule ( Debug  ${DebugModule}  1 )
else ()
	AddModule ( Scan   ${ScanModule}   1 )
	AddModule ( Macro  ${MacroModule}  1 )
	AddModule ( Output ${OutputModule} 1 )
	AddModule ( Debug  ${DebugModule}  1 )
endif ()



###
# CMake Build Env Checking
#

include( buildinfo )



#| Uses CMake variables to include as defines
#| Primarily for USB configuration
configure_file ( ${CMAKE_CURRENT_SOURCE_DIR}/Lib/_buildvars.h buildvars.h )



###
# Source Defines
#
set ( SRCS
	${MAIN_SRCS}
	${COMPILER_SRCS}
	${Scan_SRCS}
	${Macro_SRCS}
	${Output_SRCS}
	${Debug_SRCS}
)

#| Directories to include by default
include_directories ( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )



###
# ctag Generation
#

find_package ( Ctags ) # Optional

if ( CTAGS_EXECUTABLE )
	# Populate list of directories for ctags to parse
	# NOTE: Doesn't support dots in the folder names...
	foreach ( filename ${SRCS} )
		string ( REGEX REPLACE "/[a-zA-Z0-9_-]+.c$" "" pathglob ${filename} )
		file ( GLOB filenames "${pathglob}/*.c" )
		set ( CTAG_PATHS ${CTAG_PATHS} ${filenames} )
		file ( GLOB filenames "${pathglob}/*.h" )
		set ( CTAG_PATHS ${CTAG_PATHS} ${filenames} )
	endforeach ()

	# Generate the ctags
	execute_process ( COMMAND ${CTAGS_EXECUTABLE} --fields=+l ${CTAG_PATHS}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)
endif ()


###
# Create compile_commands.json (Useful for language servers as a ctags alternative)
#
set( CMAKE_EXPORT_COMPILE_COMMANDS ON)
execute_process (
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
)
