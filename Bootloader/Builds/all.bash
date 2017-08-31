#!/usr/bin/env bash
# This script runs each of the bootloader build scripts
# Jacob Alexander 2017



########################
# Bash Library Include #
########################

# Shouldn't need to touch this section

# Check if the library can be found
if [ ! -f common.bash ]; then
	echo "ERROR: Cannot find 'common.bash'"
	exit 1
fi

# Load common functions
source "common.bash"



###########
# Scripts #
###########

# Run builds
cmd ./mk20dx128vlf5.bash
cmd ./mk20dx256vlh7.bash
cmd ./ergodox.bash
cmd ./infinity.bash
cmd ./infinity_led.bash
cmd ./ktype.bash
cmd ./whitefox.bash

# Tally results
result
exit $?

