#!/usr/bin/env bash
# This is a build script template
# These build scripts are just a convenience for configuring your keyboard (less daunting than CMake)
# Jacob Alexander 2017-2022



#################
# Configuration #
#################

# Should be empty if not set
MANUFACTURER="Input Club"
BOOT_PRODUCT_STR="Keystone"
BOOT_ALT_PRODUCT_STR="Keystone TKL"
BOOT_VENDOR_ID="0x308F"
BOOT_PRODUCT_ID="0x0026" # Fullsize
BOOT_ALT_PRODUCT_ID="0x0028" # TKL



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

