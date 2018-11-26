#!/usr/bin/env bash
# This is bash lib file for the convenience build scripts
# Don't call this script directly
# Jacob Alexander 2015-2018

# Check if compiler has been overridden by the environment
Compiler=${COMPILER:-${Compiler}}

# Append to BuildPath, depending on which compiler this is
BuildPath=${BuildPath}.${Compiler}

# Default to not using HostBuild
EnableHostBuild=${EnableHostBuild:-false}
EnableHostOnlyBuild=${EnableHostOnlyBuild:-false}
HostTest=${HostTest:-""}

# Default VID:PIDs
VENDOR_ID=${VENDOR_ID:-0x1C11}
PRODUCT_ID=${PRODUCT_ID:-0xB04D}
BOOT_VENDOR_ID=${BOOT_VENDOR_ID:-0x1C11}
BOOT_PRODUCT_ID=${BOOT_PRODUCT_ID:-0xB007}

# Make sure all of the relevant variables have been set
# NOTE: PartialMaps and DefaultMap do not have to be set
VariablesList=(BuildPath BaseMap ScanModule MacroModule OutputModule DebugModule Chip Compiler)
ExitEarly=false
for var in ${VariablesList[@]}; do
	if [ -z ${!var+x} ]; then
		echo "ERROR: Unset variable => '${var}'"
		ExitEarly=true
	fi
done

# Make sure pipenv is detected
PIPENV_ACTIVE=${PIPENV_ACTIVE:-0}
if [ $PIPENV_ACTIVE -ne 1 ]; then
	CUR_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
	echo "ERROR: pipenv is not active!"
	echo "Please run the following before running this script ($(basename "$0")):"
	echo "  cd $CUR_DIR  OR  cd $CUR_DIR/../kll"
	echo "  pipenv install"
	echo "  pipenv shell"
	ExitEarly=true
fi

# Error was detected, exit immediately
if $ExitEarly; then
	exit 1
fi

# Sanitizer lookup
if $EnableSaniziter; then
	export CMakeExtraArgs="-DSANITIZER=1"

	case "$OSTYPE" in
	# Linux
	"linux-gnu")
		# TODO (HaaTa) This may not work using clang as the compiler (but it may work ok)
		# It's still recommended to have llvm installed to show backtrace
		# Leak sanitizer is finding leaks in Python, these are likely bugs but the code we are
		# testing doesn't actually malloc so it's not a big deal.
		ASAN_LIB=$(ldconfig -v 2> /dev/null | grep libasan.so | cut -d' ' -f1 | head -1 | xargs)
		UBSAN_LIB=$(ldconfig -v 2> /dev/null | grep libubsan.so | cut -d' ' -f1 | head -1 | xargs)
		export ASAN_OPTIONS=detect_leaks=0
		export PRELOADED_LIBS=${ASAN_LIB}:${UBSAN_LIB}
		echo "ASAN_OPTIONS -> ${ASAN_OPTIONS}"
		echo "PRELOADED_LIBS -> ${PRELOADED_LIBS}"
		;;
	# macOS
	"darwin"*)
		# TODO (HaaTa) This may possibly point to the wrong clang
		#              It would be better to get this path directly from CMake
		# This says gcc, but on Darwin, it's actually clang
		# (specifying clang seems to have problems)
		export DYLD_INSERT_LIBRARIES=$(gcc -print-resource-dir)/lib/darwin/libclang_rt.asan_osx_dynamic.dylib
		echo "DYLD_INSERT_LIBRARIES -> ${DYLD_INSERT_LIBRARIES}"
		;;
	esac
fi

# Prepare PartialMaps
PartialMapsExpanded="${PartialMaps[1]}"
count=2 # Start the loop at index 2
while [ "$count" -le "${#PartialMaps[@]}" ]; do
	PartialMapsExpanded="${PartialMapsExpanded};${PartialMaps[count]}"
	count=$(($count+1))
done

# Internal Variables
CMakeListsPath=${CMakeListsPath:-"../.."}
PROG_NAME=$(basename $0)

# Detect which OS
case "$OSTYPE" in
# Linux
"linux-gnu")
	echo "${OSTYPE}/Linux is supported."
	;;
# macOS
"darwin"*)
	echo "${OSTYPE}/macOS is supported."
	;;
# Cygwin
"cygwin")
	echo "${OSTYPE} is supported."
	;;
# MSYS (not tested)
"msys")
	echo "${OSTYPE} is untested..."
	;;
# Is this even possible?
"win32")
	echo "${OSTYPE} is untested..."
	;;
# FreeBSDs (not tested, but should mostly work)
"freebsd"*)
	echo "${OSTYPE} is untested..."
	;;
# Unknown OSTYPE
*)
	echo "${OSTYPE} is untested..."
	;;
esac

# Determine which CMake Makefile Generator to use
# If found, default to Ninja, otherwise use Make
if [ -z "${CMAKE_GENERATOR}" ]; then
	# First look for ninja (default), always runs a parallel build
	if type ninja &> /dev/null; then
		CMAKE_GENERATOR="Ninja"
	# Then look for make
	elif type make &> /dev/null; then
		CMAKE_GENERATOR="Unix Makefiles"
	# Error
	else
		echo "ERROR: Could not find a makefile generator"
		echo "Supported: ninja, make"
		exit 1
	fi
fi

case "${CMAKE_GENERATOR}" in
"Ninja")
	MAKE="ninja"
	;;
"Unix Makefiles")
	MAKE="make"
	;;
*)
	echo "Invalid CMAKE_GENERATOR. See cmake --help"
	exit 1
esac

# Append generator name (to support building both types on the same system)
BuildPath="${BuildPath}.${MAKE}"
# Prepend OSType (so not to clobber builds if using the same storage medium, i.e. dropbox)
BuildPath="${OSTYPE}.${BuildPath}"
# Append Extra Name if necessary
BuildPath="${BuildPath}${ExtraBuildPath}"
# Path prepend (sometimes used)
BuildPath="${BuildPathPrepend}${BuildPath}"
# Extra Path Prepend
BuildPath="${ExtraBuildPathPrepend}${BuildPath}"
# Path used for host builds
HostPath="${BuildPath}.host"

# Process the command line arguments (if any)
while (( "$#" >= "1" )); do
	# Scan each argument
	key="$1"
	case $key in
	-c|--cmakelists-path)
		CMakeListsPath="$2"
		shift
		;;
	-f|--force-rebuild)
		# Remove the old directory first
		rm -rf "${BuildPath}"
		;;
	-b|--host-build)
		# Enables host-build, tests out kll files using local simulation
		EnableHostBuild=true
		;;
	-B|--host-only-build)
		# Enables host-build, tests out kll files using local simulation
		# Does not run the main build
		EnableHostBuild=true
		EnableHostOnlyBuild=true
		;;
	-o|--output-path)
		BuildPath="$2"
		shift
		;;
	-h|--help)
		echo "Usage: $PROG_NAME [options...] [default_map] [partial_maps...]"
		echo ""
		echo "Convenience script to build the source of a given keyboard."
		echo "Edit '$PROG_NAME' to configure the keyboard options such as KLL layouts."
		echo ""
		echo "Arguments:"
		echo " -c, --cmakelists-path PATH    Set the path of CMakeLists.txt"
		echo "                               Default: ${CMakeListsPath}"
		echo " -f, --force-rebuild           Deletes the old build directory and rebuilds from scratch."
		echo " -o, --output-path PATH        Set the path of the build files."
		echo "                               Default: ${BuildPath}"
		echo " -h, --help                    This message."
		exit 1
		;;
	-*)
		echo "INVALID ARG: '$1'"
		exit 2
		;;
	*)
		echo "INVALID ARG: '$1'"
		exit 2
		;;
	esac

	# Shift to the next argument
	shift
done

# If EnableHostOnlyBuild is enabled, EnableHostBuild should be set
if ${EnableHostOnlyBuild}; then
	EnableHostBuild=true
fi



#
# Determine OS Build Type
#

# Cygwin
if [[ $(uname -s) == MINGW32_NT* ]] || [[ $(uname -s) == CYGWIN* ]]; then
	if [[ -z "$wincmake_path" ]]; then
		echo "Error wincmake_path environment variable has not been set, see -> https://github.com/kiibohd/controller/wiki/Windows-Setup"
		exit 1
	fi
	echo "Cygwin Build"
	OS_BUILD=cygwin

# Linux / Mac (and everything else)
else
	echo "Unix Build"
	OS_BUILD=unix
fi


# Override the defaults if the environment variables are set.
BaseMap=${BaseMapOverride:-${BaseMap}}
DefaultMap=${DefaultMapOverride:-${DefaultMap}}
PartialMapsExpanded=${PartialMapsExpandedOverride:-${PartialMapsExpanded}}
LayoutName=${LayoutNameOverride:-${LayoutName}}

# If Layout name is set, prepend a :
if [ "${LayoutName}" != "" ]; then
	LayoutName=:${LayoutName}
fi

echo "Selected Generator: ${CMAKE_GENERATOR}"
echo "${BuildPath}"


# Info
echo "VID: ${VENDOR_ID} (Boot: ${BOOT_VENDOR_ID})"
echo "PID: ${PRODUCT_ID} (Boot: ${BOOT_PRODUCT_ID})"



#
# Run Host Build (test kll)
#

if ${EnableHostBuild}; then
	# Prepare CMake directory
	mkdir -p "${HostPath}"
	cd "${HostPath}"

	# Cygwin
	if [[ "${OS_BUILD}" == "cygwin" ]]; then
		PATH="$wincmake_path":"${PATH}" cmake \
			-DHostBuild=1 \
			-DCHIP="host" \
			-DCOMPILER="${Compiler}" \
			-DScanModule="${ScanModule}" \
			-DMacroModule="${MacroModule}" \
			-DOutputModule="TestOut" \
			-DDebugModule="${DebugModule}" \
			-DLayoutName="${LayoutName}" \
			-DBaseMap="${BaseMap}" \
			-DDefaultMap="${DefaultMap}" \
			-DPartialMaps="${PartialMapsExpanded}" \
			-DVENDOR_ID="${VENDOR_ID}" \
			-DPRODUCT_ID="${PRODUCT_ID}" \
			-DBOOT_VENDOR_ID="${BOOT_VENDOR_ID}" \
			-DBOOT_PRODUCT_ID="${BOOT_PRODUCT_ID}" \
			${CMakeExtraArgs} "${CMakeListsPath}" \
			-G "${CMAKE_GENERATOR}"
		return_code=$?
	# Everything else
	else
		cmake \
			-DHostBuild=1 \
			-DCHIP="host" \
			-DCOMPILER="${Compiler}" \
			-DScanModule="${ScanModule}" \
			-DMacroModule="${MacroModule}" \
			-DOutputModule="TestOut" \
			-DDebugModule="${DebugModule}" \
			-DLayoutName="${LayoutName}" \
			-DBaseMap="${BaseMap}" \
			-DDefaultMap="${DefaultMap}" \
			-DPartialMaps="${PartialMapsExpanded}" \
			-DVENDOR_ID="${VENDOR_ID}" \
			-DPRODUCT_ID="${PRODUCT_ID}" \
			-DBOOT_VENDOR_ID="${BOOT_VENDOR_ID}" \
			-DBOOT_PRODUCT_ID="${BOOT_PRODUCT_ID}" \
			${CMakeExtraArgs} "${CMakeListsPath}" \
			-G "${CMAKE_GENERATOR}"
		return_code=$?
	fi

	if [[ "$return_code" -ne "0" ]] ; then
		echo "Error in host build cmake. Exiting..."
		exit $return_code
	fi

	# Cygwin
	if [[ "${OS_BUILD}" == "cygwin" ]]; then
		# Automatically determines the build system and initiates it
		PATH="$wincmake_path":"${PATH}"	cmake --build . ${CMakeExtraBuildArgs}
		return_code=$?

	# Everything else
	else
		# Automatically determines the build system and initiates it
		cmake --build . ${CMakeExtraBuildArgs}
		return_code=$?
	fi
	if [[ "$return_code" -ne "0" ]] ; then
		echo "Error in host build. Exiting..."
		exit $return_code
	fi
	echo "Host-Build has been compiled into: '${HostPath}'"

	# Determine if we are running any tests
	if [[ "$HostTest" != "" ]]; then
		TestPath="Tests/${HostTest}"
		# Make sure test exists
		if [[ -e "$TestPath" ]]; then
			# Run test
			echo "Running '$TestPath'..."
			LD_PRELOAD=${PRELOADED_LIBS} python3 ./${TestPath}
			return_code=$?

			# Results
			if [[ "$return_code" -ne "0" ]] ; then
				echo "Error in running '$TestPath'. Exiting..."
				exit $return_code
			fi
		fi
	fi

	# Only host build
	if ${EnableHostOnlyBuild}; then
		exit $return_code
	fi

	cd -
fi



#
# Run Build
#

# Prepare CMake directory
mkdir -p "${BuildPath}"
cd "${BuildPath}"

# Cygwin
if [[ "${OS_BUILD}" == "cygwin" ]]; then
	PATH="$wincmake_path":"${PATH}" cmake \
		-DCHIP="${Chip}" \
		-DCOMPILER="${Compiler}" \
		-DScanModule="${ScanModule}" \
		-DMacroModule="${MacroModule}" \
		-DOutputModule="${OutputModule}" \
		-DDebugModule="${DebugModule}" \
		-DLayoutName="${LayoutName}" \
		-DBaseMap="${BaseMap}" \
		-DDefaultMap="${DefaultMap}" \
		-DPartialMaps="${PartialMapsExpanded}" \
		-DVENDOR_ID="${VENDOR_ID}" \
		-DPRODUCT_ID="${PRODUCT_ID}" \
		-DBOOT_VENDOR_ID="${BOOT_VENDOR_ID}" \
		-DBOOT_PRODUCT_ID="${BOOT_PRODUCT_ID}" \
		${CMakeExtraArgs} "${CMakeListsPath}" \
		-G "${CMAKE_GENERATOR}"
	return_code=$?

# Linux / Mac (and everything else)
else
	cmake \
		-DCHIP="${Chip}" \
		-DCOMPILER="${Compiler}" \
		-DScanModule="${ScanModule}" \
		-DMacroModule="${MacroModule}" \
		-DOutputModule="${OutputModule}" \
		-DDebugModule="${DebugModule}" \
		-DLayoutName="${LayoutName}" \
		-DBaseMap="${BaseMap}" \
		-DDefaultMap="${DefaultMap}" \
		-DPartialMaps="${PartialMapsExpanded}" \
		-DVENDOR_ID="${VENDOR_ID}" \
		-DPRODUCT_ID="${PRODUCT_ID}" \
		-DBOOT_VENDOR_ID="${BOOT_VENDOR_ID}" \
		-DBOOT_PRODUCT_ID="${BOOT_PRODUCT_ID}" \
		${CMakeExtraArgs} "${CMakeListsPath}" \
		-G "${CMAKE_GENERATOR}"
	return_code=$?
fi

if [[ "$return_code" -ne "0" ]] ; then
	echo "Error in cmake. Exiting..."
	exit $return_code
fi

# Automatically determines the build system and initiates it
# Cygwin
if [[ $(uname -s) == MINGW32_NT* ]] || [[ $(uname -s) == CYGWIN* ]]; then
	PATH="$wincmake_path":"${PATH}"	cmake --build . ${CMakeExtraBuildArgs}
	return_code=$?

# Linux / Mac (and everything else)
else
	cmake --build . ${CMakeExtraBuildArgs}
	return_code=$?
fi

if [[ "$return_code" -ne "0" ]] ; then
	echo "Error in build. Exiting..."
	exit $return_code
fi

echo "Firmware has been compiled into: '${BuildPath}'"
cd -

