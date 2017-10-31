#!/usr/bin/env bash
# This script runs each of the firmware build scripts
# Input Club produced keyboard targets
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
cmd ./ergodox.bash
cmd ./infinity.bash
cmd ./infinity_led.bash
cmd ./k-type.bash
cmd ./kira.bash
cmd ./whitefox.aria.bash
cmd ./whitefox.iso.bash
cmd ./whitefox.jackofalltrades.bash
cmd ./whitefox.truefox.bash
cmd ./whitefox.vanilla.bash
cmd ./whitefox.winkeyless.bash

# Tally results
result
exit $?

