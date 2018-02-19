#!/usr/bin/env bash
# This script runs each of the dockerfile test scripts
# Jacob Alexander 2018


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

cmd ./archlinux.bash
cmd ./ubuntu.bash

# Tally results
result
exit $?

