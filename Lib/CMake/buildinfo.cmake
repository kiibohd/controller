###| CMAKE Kiibohd Controller Source Configurator |###
#
# Written by Jacob Alexander in 2011-2017 for the Kiibohd Controller
#
# Released into the Public Domain
#
###

find_package ( Git REQUIRED )


###
# Generate Build Defines
#

#| Modified
#| Takes a bit of work to extract the "M " using CMake, and not using it if there are no modifications
execute_process ( COMMAND ${GIT_EXECUTABLE} status -s -uno --porcelain
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Modified_INFO
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
string ( LENGTH "${Git_Modified_INFO}" Git_Modified_LENGTH )
set ( Git_Modified_Status "Clean" )
if ( ${Git_Modified_LENGTH} GREATER 2 )
	string ( SUBSTRING "${Git_Modified_INFO}" 1 2 Git_Modified_Flag_INFO )
	set ( Git_Modified_Status "Dirty" )
endif ()

#| List of modified files
execute_process ( COMMAND ${GIT_EXECUTABLE} diff-index --name-only HEAD --
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Modified_Files
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
string ( REGEX REPLACE "\n" "\\\\r\\\\n\\\\t" Git_Modified_Files "${Git_Modified_Files}" )
set ( Git_Modified_Files "\\r\\n\\t${Git_Modified_Files}" )

#| Branch
execute_process( COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Branch_INFO
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Date
execute_process ( COMMAND ${GIT_EXECUTABLE} show -s --format=%ci
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Date_INFO
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Commit Author and Email
execute_process ( COMMAND ${GIT_EXECUTABLE} show -s --format="%cn <%ce>"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Commit_Author
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Commit Revision
execute_process ( COMMAND ${GIT_EXECUTABLE} show -s --format=%H
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Commit_Revision
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Commit Number (on branch)
execute_process ( COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Commit_Number
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
	RESULT_VARIABLE res_var
)

#| Most Recent Tag (on branch)
execute_process ( COMMAND ${GIT_EXECUTABLE} for-each-ref refs/tags --sort=-taggerdate "--format=%(refname:short)" --count=1
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Recent_Tag
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
	RESULT_VARIABLE res_var
)

#| Most Recent Tag Hash (on branch)
execute_process ( COMMAND ${GIT_EXECUTABLE} rev-list -n 1 ${Git_Recent_Tag}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Recent_Tag_Revision
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
	RESULT_VARIABLE res_var
)
#| If no tags set, just use HEAD
if ( Git_Recent_Tag STREQUAL "" )
	set ( Git_Recent_Tag_Revision "${Git_Commit_Revision}" )
endif ()

#| Most Recent Tag Commit Number (on branch)
execute_process ( COMMAND ${GIT_EXECUTABLE} rev-list --count ${Git_Recent_Tag_Revision}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Recent_Tag_Commit_Number
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
	RESULT_VARIABLE res_var
)

#| Older versions of git (e.g. 1.7.1) dont' like rev-list --count
#| If there's an error, try again with a less efficient version
if ( NOT "${res_var}" STREQUAL "0" )
	execute_process ( COMMAND ${GIT_EXECUTABLE} log --pretty=oneline
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		OUTPUT_VARIABLE Git_Log_Lines
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE res_var
	)

	# Now we do some CMake wizardry to split lines in the variable so we have a list called contents
	string ( REGEX REPLACE ";" "\\\\;" contents "${Git_Log_Lines}" )
	string ( REGEX REPLACE "\n" ";" contents "${contents}" )

	# Now we just have to measure the length of the list
	list ( LENGTH contents Git_Commit_Number )
endif ()


#| Origin URL
execute_process ( COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE Git_Origin_URL
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Build Date
execute_process ( COMMAND "date" "+%Y-%m-%d %T %z"
	OUTPUT_VARIABLE Build_Date
	ERROR_QUIET
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

#| Last Commit Date
set ( GitLastCommitDate "${Git_Modified_Status} ${Git_Branch_INFO} - ${Git_Date_INFO}" )

#| Build Platform
if ( "${DETECTED_BUILD_KERNEL}" MATCHES "Darwin" )
	set( Build_OS ${CMAKE_SYSTEM} )

elseif ( "${DETECTED_BUILD_KERNEL}" MATCHES "CYGWIN" )
	set( Build_OS ${CMAKE_SYSTEM} )

elseif ( "${DETECTED_BUILD_KERNEL}" MATCHES "Linux" )
	# lsb_release is required on Linux
	find_package ( LSB REQUIRED )

	execute_process( COMMAND ${LSB_RELEASE_EXECUTABLE} -dcs
		OUTPUT_VARIABLE Build_OS
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE res_var
	)
	# Replace quotes to be compatible with C
	string( REPLACE "\"" "'" Build_OS ${Build_OS} )
	string( REPLACE "\n" " " Build_OS ${Build_OS} )
else () # Unknown
	set( Build_OS ${CMAKE_SYSTEM} )
endif ()
message( STATUS "Build OS Detected:" )
message( "${Build_OS}" )

