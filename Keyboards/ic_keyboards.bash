#!/usr/bin/env bash
# This script runs each of the firmware build scripts
# Input Club produced keyboard targets
# Jacob Alexander 2017

###########
# Options #
###########

# Enables host-build tests
export EnableHostOnlyBuild=${EnableHostOnlyBuild:-false}




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

# Run test builds
if [[ "${1}" != "win" ]] && ${EnableHostOnlyBuild}; then
	# NOTE: Infinity Ergodox is not tested as Interconnect and LCD needs more infrastructure to test
	cmd ./infinity.alphabet.bash
	cmd ./infinity.hacker.bash
	cmd ./infinity.standard.bash
	cmd ./infinity_led.alphabet.bash
	cmd ./infinity_led.hacker.bash
	cmd ./infinity_led.standard.bash
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
fi

# Run builds, normal
if [ "${1}" != "win" ]; then
	cmd_cpy ./ergodox-l.bash                kiibohd.dfu.bin firmware/ergodox.left.dfu.bin
	cmd_cpy ./ergodox-r.bash                kiibohd.dfu.bin firmware/ergodox.right.dfu.bin
	cmd_cpy ./infinity.alphabet.bash        kiibohd.dfu.bin firmware/infinity.alphabet.dfu.bin
	cmd_cpy ./infinity.hacker.bash          kiibohd.dfu.bin firmware/infinity.hacker.dfu.bin
	cmd_cpy ./infinity.standard.bash        kiibohd.dfu.bin firmware/infinity.standard.dfu.bin
	cmd_cpy ./infinity_led.alphabet.bash    kiibohd.dfu.bin firmware/infinity_led.alphabet.dfu.bin
	cmd_cpy ./infinity_led.hacker.bash      kiibohd.dfu.bin firmware/infinity_led.hacker.dfu.bin
	cmd_cpy ./infinity_led.standard.bash    kiibohd.dfu.bin firmware/infinity_led.standard.dfu.bin
	cmd_cpy ./k-type.bash                   kiibohd.dfu.bin firmware/k-type.dfu.bin
	cmd_cpy ./kira.bash                     kiibohd.dfu.bin firmware/kira.dfu.bin
	cmd_cpy ./whitefox.aria.bash            kiibohd.dfu.bin firmware/whitefox.aria.dfu.bin
	cmd_cpy ./whitefox.iso.bash             kiibohd.dfu.bin firmware/whitefox.iso.dfu.bin
	cmd_cpy ./whitefox.jackofalltrades.bash kiibohd.dfu.bin firmware/whitefox.jackofalltrades.dfu.bin
	cmd_cpy ./whitefox.truefox.bash         kiibohd.dfu.bin firmware/whitefox.truefox.dfu.bin
	cmd_cpy ./whitefox.vanilla.bash         kiibohd.dfu.bin firmware/whitefox.vanilla.dfu.bin
	cmd_cpy ./whitefox.winkeyless.bash      kiibohd.dfu.bin firmware/whitefox.winkeyless.dfu.bin

# Windows (no symlinks)
else
	env

	cmd_cpy ./ergodox-l.bash                kiibohd.dfu.bin firmware/ergodox.left.dfu.bin
	cmd_cpy ./ergodox-r.bash                kiibohd.dfu.bin firmware/ergodox.right.dfu.bin
	cmd_cpy ./infinity.bash                 kiibohd.dfu.bin firmware/infinity.dfu.bin
	cmd_cpy ./infinity_led.bash             kiibohd.dfu.bin firmware/infinity_led.dfu.bin
	cmd_cpy ./k-type.bash                   kiibohd.dfu.bin firmware/k-type.dfu.bin
	cmd_cpy ./kira.bash                     kiibohd.dfu.bin firmware/kira.dfu.bin
	cmd_cpy ./whitefox.bash                 kiibohd.dfu.bin firmware/whitefox.dfu.bin
fi

# Tally results
result
exit $?

