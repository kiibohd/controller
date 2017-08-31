#!/usr/bin/env bash
# This is a build script template
# These build scripts are just a convenience for configuring your keyboard (less daunting than CMake)
# Jacob Alexander 2017



#################
# Configuration #
#################

# Should be empty if not set
BOOT_PRODUCT_STR=K-Type



########################
# Bash Library Include #
########################

# Shouldn't need to touch this section

# Check if the library can be found
if [ ! -f mk20dx256vlh7.bash ]; then
	echo "ERROR: Cannot find 'mk20dx256vlh7.bash'"
	exit 1
fi

# Load the library
source mk20dx256vlh7.bash

