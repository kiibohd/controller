#!/usr/bin/env bash
# This is bash lib file for the convenience build scripts
# Don't call this script directly
# Jacob Alexander 2015-2017

# Check if compiler has been overridden by the environment
Compiler=${COMPILER:-${Compiler}}

# Append to BuildPath, depending on which compiler this is
BuildPath=${BuildPath}.${Compiler}

# Make sure all of the relevant variables have been set
# NOTE: PartialMaps and DefaultMap do not have to be set
VariablesList=(BuildPath Chip Compiler MANUFACTURER BOOT_PRODUCT_STR)
ExitEarly=false
for var in ${VariablesList[@]}; do
	if [ -z ${!var+x} ]; then
		echo "ERROR: Unset variable => '${var}'"
		ExitEarly=true
	fi
done

# Error was detected, exit immediately
if $ExitEarly; then
	exit 1
fi


# Prepare PartialMaps
PartialMapsExpanded="${PartialMaps[1]}"
count=2 # Start the loop at index 2
while [ "$count" -le "${#PartialMaps[@]}" ]; do
	PartialMapsExpanded="${PartialMapsExpanded};${PartialMaps[count]}"
	count=$(($count+1))
done


# Internal Variables
CMakeListsPath="../.."
PROG_NAME=$(basename $0)


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
	-o|--output-path)
		BuildPath="$2"
		shift
		;;
	-h|--help)
		echo "Usage: $PROG_NAME [options...]"
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
	*)
		echo "INVALID ARG: '$1'"
		exit 2
		;;
	esac

	# Shift to the next argument
	shift
done


# Detect which OS
case "$OSTYPE" in
# Linux
"linux-gnu")
	echo "${OSTYPE}/Linux is supported."
	echo "If you are having issues, make sure you are using a very recent version of gcc. i.e. Ubuntu might not have a recent enough version in their repos."
	;;
# macOS
"darwin"*)
	echo "${OSTYPE}/macOS is unsupported for bootloader."
	echo "Might work, usually requires a very recent version of gcc in order to hit the space optimization requirements."
	;;
# Cygwin
"cygwin")
	echo "${OSTYPE} is unsupported for bootloader."
	echo "Might work, usually requires a very recent version of gcc in order to hit the space optimization requirements."
	;;
# Others
*)
	echo "${OSTYPE} is unsupported for bootloader."
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
echo "Selected Generator: ${CMAKE_GENERATOR}"


# Prepend OSType (so not to clobber builds if using the same storage medium, i.e. dropbox)
BuildPath="${OSTYPE}.${BuildPath}"
echo "${BuildPath}"


# Info
echo "Manufacturer: ${MANUFACTURER}"
echo "Boot Product Str: ${BOOT_PRODUCT_STR}"


# Run CMake commands
mkdir -p "${BuildPath}"
cd "${BuildPath}"

# Cygwin
if [[ $(uname -s) == MINGW32_NT* ]] || [[ $(uname -s) == CYGWIN* ]]; then
	if [[ -z "$wincmake_path" ]]; then
		echo "Error wincmake_path environment variable has not been set, see -> https://github.com/kiibohd/controller/wiki/Windows-Setup"
		exit 1
	fi
	echo "Cygwin Build"
	PATH="$wincmake_path":"${PATH}" cmake -DCHIP="${Chip}" -DCOMPILER="${Compiler}" -DBOOT_PRODUCT_STR="${BOOT_PRODUCT_STR}" -DMANUFACTURER="${MANUFACTURER}" "${CMakeListsPath}" -G "${CMAKE_GENERATOR}"

# Linux / Mac (and everything else)
else
	cmake -DCHIP="${Chip}" -DCOMPILER="${Compiler}" -DBOOT_PRODUCT_STR="${BOOT_PRODUCT_STR}" -DMANUFACTURER="${MANUFACTURER}" "${CMakeListsPath}" -G "${CMAKE_GENERATOR}"
	return_code=$?

fi

if [[ "$return_code" -ne "0" ]] ; then
	echo "Error in cmake. Exiting..."
	exit $return_code
fi

# Automatically determines the build system and initiates it
cmake --build .
return_code=$?
if [[ "$return_code" -ne "0" ]] ; then
	echo "Error in make. Exiting..."
	exit $return_code
fi

echo "Firmware has been compiled into: '${BuildPath}'"
cd -

