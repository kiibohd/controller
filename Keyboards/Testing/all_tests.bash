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
cmd ./macrotest.bash
cmd ./klltest.bash
cmd ./mk20test.bash
cmd ./mk22test.bash
cmd ./mk64test.bash
cmd ./mk66test.bash
cmd ./none.bash
cmd ./nrf52832.bash
cmd ./sam4sd32c.bash
cmd ./template.bash
cmd ./uartout.bash
cmd ./usbxuart.bash

# Tally results
result
exit $?

