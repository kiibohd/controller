#!/usr/bin/env bash
# This script runs each of the firmware build scripts
# Input Club supported keyboard targets
# Jacob Alexander 2017-2018

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
<<<<<<< HEAD
	cmd "bash ./geminiduskdawn.bash"
	cmd "bash ./infinity.alphabet.bash"
	cmd "bash ./infinity.hacker.bash"
	cmd "bash ./infinity.standard.bash"
	cmd "bash ./infinity_led.alphabet.bash"
	cmd "bash ./infinity_led.hacker.bash"
	cmd "bash ./infinity_led.standard.bash"
	cmd "bash ./k-type.bash"
	cmd "bash ./kira.bash"
	cmd "bash ./whitefox.aria.bash"
	cmd "bash ./whitefox.iso.bash"
	cmd "bash ./whitefox.jackofalltrades.bash"
	cmd "bash ./whitefox.truefox.bash"
	cmd "bash ./whitefox.vanilla.bash"
	cmd "bash ./whitefox.winkeyless.bash"
	cmd "bash ./whitefox_sam4s.aria.bash"
	cmd "bash ./whitefox_sam4s.iso.bash"
	cmd "bash ./whitefox_sam4s.jackofalltrades.bash"
	cmd "bash ./whitefox_sam4s.truefox.bash"
	cmd "bash ./whitefox_sam4s.vanilla.bash"
	cmd "bash ./whitefox_sam4s.winkeyless.bash"

	# Tally results
	result
	exit $?
fi

# Run builds, normal
if [ "${1}" != "win" ]; then
	cmd_cpy "bash ./ergodox-l.bash"                kiibohd.dfu.bin firmware/ergodox.left.dfu.bin
	cmd_cpy "bash ./ergodox-r.bash"                kiibohd.dfu.bin firmware/ergodox.right.dfu.bin
	cmd_cpy "bash ./geminiduskdawn.bash"           kiibohd.dfu.bin firmware/geminiduskdawn.dfu.bin
	cmd_cpy "bash ./infinity.alphabet.bash"        kiibohd.dfu.bin firmware/infinity.alphabet.dfu.bin
	cmd_cpy "bash ./infinity.hacker.bash"          kiibohd.dfu.bin firmware/infinity.hacker.dfu.bin
	cmd_cpy "bash ./infinity.standard.bash"        kiibohd.dfu.bin firmware/infinity.standard.dfu.bin
	cmd_cpy "bash ./infinity_led.alphabet.bash"    kiibohd.dfu.bin firmware/infinity_led.alphabet.dfu.bin
	cmd_cpy "bash ./infinity_led.hacker.bash"      kiibohd.dfu.bin firmware/infinity_led.hacker.dfu.bin
	cmd_cpy "bash ./infinity_led.standard.bash"    kiibohd.dfu.bin firmware/infinity_led.standard.dfu.bin
	cmd_cpy "bash ./k-type.bash"                   kiibohd.dfu.bin firmware/k-type.dfu.bin
	cmd_cpy "bash ./kira.bash"                     kiibohd.dfu.bin firmware/kira.dfu.bin
	cmd_cpy "bash ./whitefox.aria.bash"            kiibohd.dfu.bin firmware/whitefox.aria.dfu.bin
	cmd_cpy "bash ./whitefox.iso.bash"             kiibohd.dfu.bin firmware/whitefox.iso.dfu.bin
	cmd_cpy "bash ./whitefox.jackofalltrades.bash" kiibohd.dfu.bin firmware/whitefox.jackofalltrades.dfu.bin
	cmd_cpy "bash ./whitefox.truefox.bash"         kiibohd.dfu.bin firmware/whitefox.truefox.dfu.bin
	cmd_cpy "bash ./whitefox.vanilla.bash"         kiibohd.dfu.bin firmware/whitefox.vanilla.dfu.bin
	cmd_cpy "bash ./whitefox.winkeyless.bash"      kiibohd.dfu.bin firmware/whitefox.winkeyless.dfu.bin
	cmd_cpy "bash ./whitefox_sam4s.aria.bash"            kiibohd.dfu.bin firmware/whitefox_sam4s.aria.dfu.bin
	cmd_cpy "bash ./whitefox_sam4s.iso.bash"             kiibohd.dfu.bin firmware/whitefox_sam4s.iso.dfu.bin
	cmd_cpy "bash ./whitefox_sam4s.jackofalltrades.bash" kiibohd.dfu.bin firmware/whitefox_sam4s.jackofalltrades.dfu.bin
	cmd_cpy "bash ./whitefox_sam4s.truefox.bash"         kiibohd.dfu.bin firmware/whitefox_sam4s.truefox.dfu.bin
	cmd_cpy "bash ./whitefox_sam4s.vanilla.bash"         kiibohd.dfu.bin firmware/whitefox_sam4s.vanilla.dfu.bin
	cmd_cpy "bash ./whitefox_sam4s.winkeyless.bash"      kiibohd.dfu.bin firmware/whitefox_sam4s.winkeyless.dfu.bin

# Windows (no symlinks)
else
	env

	cmd_cpy "bash ./geminiduskdawn.bash"           kiibohd.dfu.bin firmware/geminiduskdawn.dfu.bin
	cmd_cpy "bash ./ergodox-l.bash"                kiibohd.dfu.bin firmware/ergodox.left.dfu.bin
	cmd_cpy "bash ./ergodox-r.bash"                kiibohd.dfu.bin firmware/ergodox.right.dfu.bin
	cmd_cpy "bash ./infinity.bash"                 kiibohd.dfu.bin firmware/infinity.dfu.bin
	cmd_cpy "bash ./infinity_led.bash"             kiibohd.dfu.bin firmware/infinity_led.dfu.bin
	cmd_cpy "bash ./k-type.bash"                   kiibohd.dfu.bin firmware/k-type.dfu.bin
	cmd_cpy "bash ./kira.bash"                     kiibohd.dfu.bin firmware/kira.dfu.bin
	cmd_cpy "bash ./whitefox.bash"                 kiibohd.dfu.bin firmware/whitefox.dfu.bin
	cmd_cpy "bash ./whitefox_sam4s.bash"           kiibohd.dfu.bin firmware/whitefox_sam4s.dfu.bin
fi

# Tally results
result
exit $?

