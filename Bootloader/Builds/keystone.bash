#!/usr/bin/env bash
# This is a build script template
# These build scripts are just a convenience for configuring your keyboard (less daunting than CMake)
# Jacob Alexander 2017-2019



#################
# Configuration #
#################

# Should be empty if not set
BOOT_PRODUCT_STR=Keystone
BOOT_VENDOR_ID="0x308F"
BOOT_PRODUCT_ID="0x0026"



########################
# Bash Library Include #
########################

# Shouldn't need to touch this section

# Check if the library can be found
if [ ! -f sam4s8b.bash ]; then
	echo "ERROR: Cannot find 'sam4s8b.bash'"
	exit 1
fi

# Load the library
source sam4s8b.bash

