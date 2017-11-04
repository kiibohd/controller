#!/usr/bin/env bash
# This is a build script template
# These build scripts are just a convenience for configuring your keyboard (less daunting than CMake)
# Jacob Alexander 2015-2017



#################
# Configuration #
#################

# Should be empty if not set
SubBuild=${BOOT_PRODUCT_STR}
BOOT_PRODUCT_STR=${BOOT_PRODUCT_STR:-Kiibohd DFU Bootloader}
MANUFACTURER=${MANUFACTURER:-Kiibohd}

if [[ ! -z ${SubBuild} ]]; then
	SubBuild=".${SubBuild// /_}"
fi

BuildPath="mk22fx512avlh12${SubBuild}"



##########################
# Advanced Configuration #
##########################

# Don't change the variables in this section unless you know what you're doing
# These are useful for completely custom keyboards
# NOTE: Changing any of these variables will require a force build to compile correctly

# Microcontroller
Chip="mk22fx512avlh12"

# Compiler Selection
Compiler="gcc"



########################
# Bash Library Include #
########################

# Shouldn't need to touch this section

# Check if the library can be found
if [ ! -f cmake.bash ]; then
	echo "ERROR: Cannot find 'cmake.bash'"
	exit 1
fi

# Load the library
source cmake.bash

