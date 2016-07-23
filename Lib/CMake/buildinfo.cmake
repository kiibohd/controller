###| CMAKE Kiibohd Controller Source Configurator |###
#
# Written by Jacob Alexander in 2011-2016 for the Kiibohd Controller
#
# Released into the Public Domain
#
###

find_package ( Git REQUIRED )


###
# Generate Build Defines
#

#| Manufacturer name
set ( MANUFACTURER "Kiibohd" )


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
)

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

