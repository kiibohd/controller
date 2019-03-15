###| CMAKE Kiibohd Controller KLL Configurator |###
#
# Written by Jacob Alexander in 2014-2019 for the Kiibohd Controller
#
# Released into the Public Domain
#
###



###
# Check if KLL compiler is needed
#

if ( "${MacroModule}" STREQUAL "PartialMap" OR "${MacroModule}" STREQUAL "PixelMap" )



###
# Python 3 is required for kll
# Check disabled for Win32 as it can't detect version correctly (we don't use Python directly through CMake anyways)
#

set ( PYTHON_EXECUTABLE
	python3
	CACHE STRING "Python 3 Executable Path"
)
if ( NOT "${DETECTED_BUILD_KERNEL}" MATCHES "CYGWIN" )
	# Required on systems where python is 2, not 3
	find_package ( PythonInterp 3 REQUIRED )
else ()
	message ( STATUS "Python Executable: ${PYTHON_EXECUTABLE}" )
endif ()



###
# KLL Detection
# Check for KLL in this order
# 1) If KLL_EXECUTABLE is set
# 2) If kll directory is present, check for kll/kll or kll/kll/kll

message ( STATUS "Checking for kll" )

### XXX XXX XXX - Remember to update Pipfile as well when you change the version! ###
set ( KLL_MIN_VERSION "0.5.7.10" )

# 1) Check for environment variable
if ( NOT DEFINED KLL_EXECUTABLE )
	# 2) Check for local copy of kll compiler
	if ( EXISTS "${PROJECT_SOURCE_DIR}/kll/kll/kll" )
		set ( KLL_EXECUTABLE
			${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/kll/kll/kll
		)
		set ( KLL_WORKING_DIRECTORY
			"${PROJECT_SOURCE_DIR}/kll"
		)

	# 3) Use pip kll
	else ()
		set ( KLL_EXECUTABLE
			${PYTHON_EXECUTABLE} -m kll
		)
		set ( KLL_WORKING_DIRECTORY
			"${PROJECT_SOURCE_DIR}"
		)
	endif ()
endif ()

# Determine if GITHUB_APIKEY was set, and apply it to the KLL compiler environment if it is
if ( NOT "$ENV{GITHUB_APIKEY}" STREQUAL "" )
	message ( STATUS "GITHUB_APIKEY was set." )
	set ( KLL_EXECUTABLE
		cmake -E env GITHUB_APIKEY=$ENV{GITHUB_APIKEY} ${KLL_EXECUTABLE}
	)
endif ()

# Make sure kll is a high enough version
execute_process ( COMMAND ${KLL_EXECUTABLE} --version
	OUTPUT_VARIABLE kll_version_output_display
	OUTPUT_STRIP_TRAILING_WHITESPACE
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
)
if ( kll_version_output_display MATCHES "^kll " )
	string ( REPLACE "kll " "" kll_version_output "${kll_version_output_display}" )
	string ( REGEX REPLACE "\\..* - .*#" "" kll_version_output "${kll_version_output}" )
	message ( STATUS "kll Version Detected: ${kll_version_output_display}" )

	# Check KLL_MIN_VERSION
	if ( kll_version_output VERSION_LESS KLL_MIN_VERSION )
		message ( FATAL_ERROR "kll version '${kll_version_output}' is lower than the minimum '${KLL_MIN_VERSION}'. Please update kll." )
	endif ()
else ()
	message ( FATAL_ERROR "kll compiler is not available '${KLL_EXECUTABLE}': ${kll_version_output}. You may need to set KLL_EXECUTABLE manually. Or check the Pipfile." )
endif ()
unset ( kll_version_output )
unset ( kll_version_output_display )

# Retrieve installation path for kll, needed find the default layouts
execute_process ( COMMAND ${KLL_EXECUTABLE} --path
	OUTPUT_VARIABLE kll_installation_path
	OUTPUT_STRIP_TRAILING_WHITESPACE
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
)
file ( TO_CMAKE_PATH "${kll_installation_path}" kll_installation_path )
message ( STATUS "kll Installation Path: ${kll_installation_path}" )
set ( kll_layouts_path
	"${kll_installation_path}/layouts"
)

# Retrieve cache path for layouts
execute_process ( COMMAND ${KLL_EXECUTABLE} --layout-cache-path
	OUTPUT_VARIABLE kll_cache_path
	OUTPUT_STRIP_TRAILING_WHITESPACE
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
)
file ( TO_CMAKE_PATH "${kll_cache_path}" kll_cache_path )
message ( STATUS "kll Layouts Cache Path: ${kll_cache_path}" )



###
# Prepare KLL layout arguments
#

#| KLL_DEPENDS is used to build a dependency tree for kll.py, this way when files are changed, kll.py gets re-run

#| Add each of the detected capabilities.kll
foreach ( filename ${ScanModule_KLL} ${MacroModule_KLL} ${OutputModule_KLL} ${DebugModule_KLL} ${Device_KLL})
	set ( Config_Args ${Config_Args} ${filename} )
	set ( KLL_DEPENDS ${KLL_DEPENDS} ${filename} )
endforeach ()

#| If this is a HostBuild, we're overriding the BaseMap directory to the original module
#| If set BaseMap cannot be found, use default map
if ( HostBuild )
	set ( pathname "${PROJECT_SOURCE_DIR}/Scan/${ScanModule}" )
else ()
	set ( pathname "${PROJECT_SOURCE_DIR}/${ScanModulePath}" )
endif ()

string ( REPLACE " " ";" MAP_LIST ${BaseMap} ) # Change spaces to semicolons
foreach ( MAP ${MAP_LIST} )
	# Only check the Scan Module for BaseMap .kll files, default to scancode_map.kll or defaultMap.kll
	if ( NOT EXISTS ${pathname}/${MAP}.kll )
		if ( EXISTS ${pathname}/scancode_map.kll )
			set ( BaseMap_Args ${BaseMap_Args} ${pathname}/scancode_map.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/scancode_map.kll )
		else ()
			set ( BaseMap_Args ${BaseMap_Args} ${pathname}/defaultMap.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/defaultMap.kll )
		endif ()
	elseif ( EXISTS "${pathname}/${MAP}.kll" )
		set ( BaseMap_Args ${BaseMap_Args} ${pathname}/${MAP}.kll )
		set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/${MAP}.kll )
	else ()
		message ( FATAL " Could not find '${MAP}.kll' BaseMap in Scan module directory" )
	endif ()
endforeach ()
if ( NOT "${BaseMap_Args}" STREQUAL "" )
	# Prepend --base flag if there are BaseMap args
	set ( BaseMap_Args --base ${BaseMap_Args} )
endif ()

#| Configure DefaultMap if specified
if ( NOT "${DefaultMap}" STREQUAL "" )
	string ( REPLACE " " ";" MAP_LIST ${DefaultMap} ) # Change spaces to semicolons
	foreach ( MAP ${MAP_LIST} )
		# Check if kll file is in build directory, otherwise default to layout directory
		if ( EXISTS "${PROJECT_BINARY_DIR}/${MAP}.kll" )
			set ( DefaultMap_Args ${DefaultMap_Args} ${PROJECT_BINARY_DIR}/${MAP}.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${PROJECT_BINARY_DIR}/${MAP}.kll )
		elseif ( EXISTS "${kll_layouts_path}/${MAP}.kll" )
			set ( DefaultMap_Args ${DefaultMap_Args} ${kll_layouts_path}/${MAP}.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${kll_layouts_path}/${MAP}.kll )
		elseif ( EXISTS "${pathname}/${MAP}.kll" )
			set ( DefaultMap_Args ${DefaultMap_Args} ${pathname}/${MAP}.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/${MAP}.kll )
		else ()
			message ( FATAL " Could not find '${MAP}.kll' DefaultMap" )
		endif ()
	endforeach ()
endif ()
if ( NOT "${DefaultMap_Args}" STREQUAL "" )
	# Prepend --default flag if there are DefaultMap args
	set ( DefaultMap_Args --default ${DefaultMap_Args} )
endif ()

#| Configure PartialMaps if specified
if ( NOT "${PartialMaps}" STREQUAL "" )
	# For each partial layer
	foreach ( MAP ${PartialMaps} )
		set ( PartialMap_Args ${PartialMap_Args} --partial )

		# Combine each layer
		string ( REPLACE " " ";" MAP_LIST ${MAP} ) # Change spaces to semicolons
		foreach ( MAP_PART ${MAP_LIST} )
			# Check if kll file is in build directory, otherwise default to layout directory
			if ( EXISTS "${PROJECT_BINARY_DIR}/${MAP_PART}.kll" )
				set ( PartialMap_Args ${PartialMap_Args} ${PROJECT_BINARY_DIR}/${MAP_PART}.kll )
				set ( KLL_DEPENDS ${KLL_DEPENDS} ${PROJECT_BINARY_DIR}/${MAP_PART}.kll )
			elseif ( EXISTS "${kll_layouts_path}/${MAP_PART}.kll" )
				set ( PartialMap_Args ${PartialMap_Args} ${kll_layouts_path}/${MAP_PART}.kll )
				set ( KLL_DEPENDS ${KLL_DEPENDS} ${kll_layouts_path}/${MAP_PART}.kll )
			elseif ( EXISTS "${pathname}/${MAP_PART}.kll" )
				set ( PartialMap_Args ${PartialMap_Args} ${pathname}/${MAP_PART}.kll )
				set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/${MAP_PART}.kll )
			elseif ( EXISTS "${pathname}/${MAP}.kll" )
				set ( PartialMap_Args ${PartialMap_Args} ${pathname}/${MAP}.kll )
				set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/${MAP}.kll )
			else ()
				message ( FATAL " Could not find '${MAP_PART}.kll' PartialMap" )
			endif ()
		endforeach ()
	endforeach ()
endif ()


#| Print list of layout sources used
message ( STATUS "Detected Layout Files:" )
foreach ( filename ${KLL_DEPENDS} )
	message ( "${filename}" )
endforeach ()



###
# Run KLL Compiler
#

#| KLL Options
set ( kll_emitter    kiibohd )
set ( kll_keymap     "${PROJECT_BINARY_DIR}/generatedKeymap.h" )
set ( kll_hid_lookup "${PROJECT_BINARY_DIR}/usb_hid.h" )
set ( kll_defs       "${PROJECT_BINARY_DIR}/kll_defs.h" )
set ( kll_pixelmap   "${PROJECT_BINARY_DIR}/generatedPixelmap.c" )
set ( kll_json       "${PROJECT_BINARY_DIR}/kll.json" )
set ( kll_outputname ${kll_keymap} ${kll_defs} ${kll_hid_lookup} ${kll_pixelmap} ${kll_json} )

#| KLL Configurator Options
#|
#| Applied when running a compilation using KiiConf
#|
if ( DEFINED CONFIGURATOR )
	set ( ignore ${CONFIGURATOR} ) # Needs to be here to hide warning about CONFIGURATOR
	set ( kll_configurator_options
		--preprocessor-tmp-path ${PROJECT_BINARY_DIR}/tmp_kll
	)
endif ()

#| KLL Cmd
set ( kll_cmd
	${KLL_EXECUTABLE}
	--kiibohd-debug
	--config ${Config_Args}
	${BaseMap_Args}
	${DefaultMap_Args}
	${PartialMap_Args}
	--emitter ${kll_emitter}
	--def-output ${kll_defs}
	--map-output ${kll_keymap}
	--hid-output ${kll_hid_lookup}
	--pixel-output ${kll_pixelmap}
	--json-output ${kll_json}
	${kll_configurator_options}
)

set ( kll_cmd_debug_options
	--operation-organization-debug
	--data-organization-debug
	--data-finalization-debug
	--data-analysis-debug
)

set ( kll_cmd_display_options
	--operation-organization-display
	--data-organization-display
	--data-finalization-display
	--data-analysis-display
)

set ( kll_cmd_final_display_options
	--data-analysis-display
)

add_custom_command ( OUTPUT ${kll_outputname}
	COMMAND ${kll_cmd}
	DEPENDS ${KLL_DEPENDS}
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
	COMMENT "Generating KLL Layout"
)

#| KLL Regen Convenience Target
add_custom_target ( kll_regen
	COMMAND ${kll_cmd}
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
	COMMENT "Re-generating KLL Layout"
)

#| KLL Regen Debug Target
add_custom_target ( kll_debug
	COMMAND ${kll_cmd} ${kll_cmd_debug_options}
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
	COMMENT "Re-generating KLL Layout in Debug Mode"
)

#| KLL Regen Display Target
add_custom_target ( kll_display
	COMMAND ${kll_cmd} ${kll_cmd_display_options}
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
	COMMENT "Re-generating KLL Layout in Display Mode"
)

#| KLL Regen Final Display Target
add_custom_target ( kll_final_display
	COMMAND ${kll_cmd} ${kll_cmd_final_display_options}
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
	COMMENT "Re-generating KLL Layout in Final Display Mode"
)

#| KLL Regen Token Debug
add_custom_target ( kll_token
	COMMAND ${kll_cmd} --token-debug
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
	COMMENT "Re-generating KLL Layout in Token Debug Mode"
)

#| KLL Regen Parser Debug
add_custom_target ( kll_parser
	COMMAND ${kll_cmd} --parser-debug --parser-token-debug
	WORKING_DIRECTORY ${KLL_WORKING_DIRECTORY}
	COMMENT "Re-generating KLL Layout in Parser Debug Mode"
)

#| Append generated file to required sources so it becomes a dependency in the main build
set ( SRCS ${SRCS} ${kll_outputname} )


else ()
message ( AUTHOR_WARNING "Unknown Macro module, ignoring kll generation" )
endif () # PartialMap

