#!/usr/bin/env bash
# This script runs each of the firmware build scripts
# Misc keyboard targets
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
cmd ./k-type.p2.bash
cmd ./k-type.p3.bash
#cmd ./ps2_converter.bash
cmd ./template.bash

# Tally results
result
exit $?

