###| CMAKE Kiibohd Controller KLL Configurator |###
#
# Written by Jacob Alexander in 2014-2017 for the Kiibohd Controller
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

if ( NOT CMAKE_HOST_WIN32 )
	# Required on systems where python is 2, not 3
	set ( PYTHON_EXECUTABLE
		python3
		CACHE STRING "Python 3 Executable Path"
	)
	find_package ( PythonInterp 3 REQUIRED )
endif ()


###
# KLL Installation (Make sure repo has been cloned)
#

if ( NOT EXISTS "${PROJECT_SOURCE_DIR}/kll/kll" )
	message ( STATUS "Downloading latest kll version:" )

	# Make sure git is available
	find_package ( Git REQUIRED )

	# Clone kll git repo
	execute_process ( COMMAND ${GIT_EXECUTABLE} clone https://github.com/kiibohd/kll.git
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	)
elseif ( REFRESH_KLL ) # Otherwise attempt to update the repo
	message ( STATUS "Checking for latest kll version:" )

	# Clone kll git repo
	execute_process ( COMMAND ${GIT_EXECUTABLE} pull --rebase
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/kll
	)
endif () # kll/kll.py exists



###
# Prepare KLL layout arguments
#

#| KLL_DEPENDS is used to build a dependency tree for kll.py, this way when files are changed, kll.py gets re-run

#| Add each of the detected capabilities.kll
foreach ( filename ${ScanModule_KLL} ${MacroModule_KLL} ${OutputModule_KLL} ${DebugModule_KLL} )
	set ( Config_Args ${Config_Args} ${filename} )
	set ( KLL_DEPENDS ${KLL_DEPENDS} ${filename} )
endforeach ()

#| If set BaseMap cannot be found, use default map
set ( pathname "${PROJECT_SOURCE_DIR}/${ScanModulePath}" )

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

#| Configure DefaultMap if specified
if ( NOT "${DefaultMap}" STREQUAL "" )
	string ( REPLACE " " ";" MAP_LIST ${DefaultMap} ) # Change spaces to semicolons
	foreach ( MAP ${MAP_LIST} )
		# Check if kll file is in build directory, otherwise default to layout directory
		if ( EXISTS "${PROJECT_BINARY_DIR}/${MAP}.kll" )
			set ( DefaultMap_Args ${DefaultMap_Args} ${MAP}.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${PROJECT_BINARY_DIR}/${MAP}.kll )
		elseif ( EXISTS "${PROJECT_SOURCE_DIR}/kll/layouts/${MAP}.kll" )
			set ( DefaultMap_Args ${DefaultMap_Args} ${PROJECT_SOURCE_DIR}/kll/layouts/${MAP}.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${PROJECT_SOURCE_DIR}/kll/layouts/${MAP}.kll )
		elseif ( EXISTS "${pathname}/${MAP}.kll" )
			set ( DefaultMap_Args ${DefaultMap_Args} ${pathname}/${MAP}.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/${MAP}.kll )
		else ()
			message ( FATAL " Could not find '${MAP}.kll' DefaultMap" )
		endif ()
	endforeach ()
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
				set ( PartialMap_Args ${PartialMap_Args} ${MAP_PART}.kll )
				set ( KLL_DEPENDS ${KLL_DEPENDS} ${PROJECT_BINARY_DIR}/${MAP_PART}.kll )
			elseif ( EXISTS "${PROJECT_SOURCE_DIR}/kll/layouts/${MAP_PART}.kll" )
				set ( PartialMap_Args ${PartialMap_Args} ${PROJECT_SOURCE_DIR}/kll/layouts/${MAP_PART}.kll )
				set ( KLL_DEPENDS ${KLL_DEPENDS} ${PROJECT_SOURCE_DIR}/kll/layouts/${MAP_PART}.kll )
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
set ( kll_keymap     generatedKeymap.h )
set ( kll_defs       kll_defs.h )
set ( kll_pixelmap   generatedPixelmap.c )
set ( kll_outputname ${kll_keymap} ${kll_defs} ${kll_pixelmap} )

#| KLL Version
set ( kll_version_cmd
	${PROJECT_SOURCE_DIR}/kll/kll
	--version
)

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
	${PROJECT_SOURCE_DIR}/kll/kll
	--config ${Config_Args}
	--base ${BaseMap_Args}
	--default ${DefaultMap_Args}
	${PartialMap_Args}
	--emitter ${kll_emitter}
	--def-template ${PROJECT_SOURCE_DIR}/kll/templates/kiibohdDefs.h
	--map-template ${PROJECT_SOURCE_DIR}/kll/templates/kiibohdKeymap.h
	--pixel-template ${PROJECT_SOURCE_DIR}/kll/templates/kiibohdPixelmap.c
	--def-output ${kll_defs}
	--map-output ${kll_keymap}
	--pixel-output ${kll_pixelmap}
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
	COMMAND ${kll_version_cmd}
	COMMAND ${kll_cmd}
	DEPENDS ${KLL_DEPENDS}
	COMMENT "Generating KLL Layout"
)

#| KLL Regen Convenience Target
add_custom_target ( kll_regen
	COMMAND ${kll_version_cmd}
	COMMAND ${kll_cmd}
	COMMENT "Re-generating KLL Layout"
)

#| KLL Regen Debug Target
add_custom_target ( kll_debug
	COMMAND ${kll_version_cmd}
	COMMAND ${kll_cmd} ${kll_cmd_debug_options}
	COMMENT "Re-generating KLL Layout in Debug Mode"
)

#| KLL Regen Display Target
add_custom_target ( kll_display
	COMMAND ${kll_version_cmd}
	COMMAND ${kll_cmd} ${kll_cmd_display_options}
	COMMENT "Re-generating KLL Layout in Display Mode"
)

#| KLL Regen Final Display Target
add_custom_target ( kll_final_display
	COMMAND ${kll_version_cmd}
	COMMAND ${kll_cmd} ${kll_cmd_final_display_options}
	COMMENT "Re-generating KLL Layout in Final Display Mode"
)

#| KLL Regen Token Debug
add_custom_target ( kll_token
	COMMAND ${kll_version_cmd}
	COMMAND ${kll_cmd} --token-debug
	COMMENT "Re-generating KLL Layout in Token Debug Mode"
)

#| KLL Regen Parser Debug
add_custom_target ( kll_parser
	COMMAND ${kll_version_cmd}
	COMMAND ${kll_cmd} --parser-debug --parser-token-debug
	COMMENT "Re-generating KLL Layout in Parser Debug Mode"
)

#| Append generated file to required sources so it becomes a dependency in the main build
set ( SRCS ${SRCS} ${kll_outputname} )


else ()
message ( AUTHOR_WARNING "Unknown Macro module, ignoring kll generation" )
endif () # PartialMap

