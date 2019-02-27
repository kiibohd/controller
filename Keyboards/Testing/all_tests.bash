#!/usr/bin/env bash
# This script runs each of the firmware build scripts
# Input Club produced keyboard targets
# Jacob Alexander 2017-2018



########################
# Bash Library Include #
########################

# Shouldn't need to touch this section

# Check if the library can be found
if [ ! -f ../common.bash ]; then
	echo "ERROR: Cannot find 'common.bash'"
	exit 1
fi

# Load common functions
source "../common.bash"



###########
# Scripts #
###########

# Run builds
cmd "bash ./macrotest.bash"
cmd "bash ./klltest.bash"
cmd "bash ./mk20test.bash"
cmd "bash ./mk22test.bash"
cmd "bash ./mk64test.bash"
cmd "bash ./mk66test.bash"
cmd "bash ./none.bash"
cmd "bash ./nrf52832.bash"
cmd "bash ./rttout.bash"
cmd "bash ./sam4s2a.bash"
cmd "bash ./sam4s8b.bash"
cmd "bash ./sam4s8c.bash"
cmd "bash ./sam4sd32c.bash"
cmd "bash ./template.bash"
cmd "bash ./uartout.bash"
cmd "bash ./usbxrtt.bash"
cmd "bash ./usbxuart.bash"

# Tally results
result
exit $?

