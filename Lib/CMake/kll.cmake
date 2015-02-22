###| CMAKE Kiibohd Controller KLL Configurator |###
#
# Written by Jacob Alexander in 2014 for the Kiibohd Controller
#
# Released into the Public Domain
#
###


###
# Check if KLL compiler is needed
#

if ( "${MacroModule}" STREQUAL "PartialMap" )



###
# KLL Installation (Make sure repo has been cloned)
#

if ( NOT EXISTS "${PROJECT_SOURCE_DIR}/kll/kll.py" )
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

#| Search for capabilities.kll in each module directory
foreach ( DIR ${ScanModulePath} ${MacroModulePath} ${OutputModulePath} ${DebugModulePath} )
	# capabilities.kll exists, add to BaseMap
	set ( filename "${PROJECT_SOURCE_DIR}/${DIR}/capabilities.kll" )
	if ( EXISTS ${filename} )
		set ( BaseMap_Args ${BaseMap_Args} ${filename} )
		set ( KLL_DEPENDS ${KLL_DEPENDS} ${filename} )
	endif ()
endforeach ()

#| If set BaseMap cannot be found, use default map
set ( pathname "${PROJECT_SOURCE_DIR}/${ScanModulePath}" )
if ( NOT EXISTS ${pathname}/${BaseMap}.kll )
	set ( BaseMap_Args ${BaseMap_Args} ${pathname}/defaultMap.kll )
	set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/defaultMap.kll )
else ()
	set ( BaseMap_Args ${BaseMap_Args} ${pathname}/${BaseMap}.kll )
	set ( KLL_DEPENDS ${KLL_DEPENDS} ${pathname}/${BaseMap}.kll )
endif ()

#| Configure DefaultMap if specified
if ( NOT "${DefaultMap}" STREQUAL "" )
	set ( DefaultMap_Args -d )

	string ( REPLACE " " ";" MAP_LIST ${DefaultMap} ) # Change spaces to semicolons
	foreach ( MAP ${MAP_LIST} )
		# Check if kll file is in build directory, otherwise default to layout directory
		if ( EXISTS "${PROJECT_BINARY_DIR}/${MAP}.kll" )
			set ( DefaultMap_Args ${DefaultMap_Args} ${MAP}.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${MAP}.kll )
		else ()
			set ( DefaultMap_Args ${DefaultMap_Args} ${PROJECT_SOURCE_DIR}/kll/layouts/${MAP}.kll )
			set ( KLL_DEPENDS ${KLL_DEPENDS} ${PROJECT_SOURCE_DIR}/kll/layouts/${MAP}.kll )
		endif ()
	endforeach ()
endif ()

#| Configure PartialMaps if specified
if ( NOT "${PartialMaps}" STREQUAL "" )
	# For each partial layer
	foreach ( MAP ${PartialMaps} )
		set ( PartialMap_Args ${PartialMap_Args} -p )

		# Combine each layer
		string ( REPLACE " " ";" MAP_LIST ${MAP} ) # Change spaces to semicolons
		foreach ( MAP_PART ${MAP_LIST} )
			# Check if kll file is in build directory, otherwise default to layout directory
			if ( EXISTS "${PROJECT_BINARY_DIR}/${MAP_PART}.kll" )
				set ( PartialMap_Args ${PartialMap_Args} ${MAP_PART}.kll )
				set ( KLL_DEPENDS ${KLL_DEPENDS} ${MAP_PART}.kll )
			else ()
				set ( PartialMap_Args ${PartialMap_Args} ${PROJECT_SOURCE_DIR}/kll/layouts/${MAP_PART}.kll )
				set ( KLL_DEPENDS ${KLL_DEPENDS} ${PROJECT_SOURCE_DIR}/kll/layouts/${MAP_PART}.kll )
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
set ( kll_backend   --backend kiibohd )
set ( kll_template  --templates ${PROJECT_SOURCE_DIR}/kll/templates/kiibohdKeymap.h ${PROJECT_SOURCE_DIR}/kll/templates/kiibohdDefs.h )
set ( kll_outputname generatedKeymap.h kll_defs.h )
set ( kll_output    --outputs ${kll_outputname} )

#| KLL Cmd
set ( kll_cmd ${PROJECT_SOURCE_DIR}/kll/kll.py ${BaseMap_Args} ${DefaultMap_Args} ${PartialMap_Args} ${kll_backend} ${kll_template} ${kll_output} )
add_custom_command ( OUTPUT ${kll_outputname}
	COMMAND ${kll_cmd}
	DEPENDS ${KLL_DEPENDS}
	COMMENT "Generating KLL Layout"
)

#| KLL Regen Convenience Target
add_custom_target ( kll_regen
	COMMAND ${kll_cmd}
	COMMENT "Re-generating KLL Layout"
)

#| Append generated file to required sources so it becomes a dependency in the main build
set ( SRCS ${SRCS} ${kll_outputname} )



endif () # PartialMap

