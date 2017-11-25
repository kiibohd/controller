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
cmd_cpy ./whitefox.aria.bash            kiibohd.dfu.bin firmware/whitefox.aria.bin
cmd_cpy ./whitefox.iso.bash             kiibohd.dfu.bin firmware/whitefox.iso.bin
cmd_cpy ./whitefox.jackofalltrades.bash kiibohd.dfu.bin firmware/whitefox.jackofalltrades.bin
cmd_cpy ./whitefox.truefox.bash         kiibohd.dfu.bin firmware/whitefox.truefox.bin
cmd_cpy ./whitefox.vanilla.bash         kiibohd.dfu.bin firmware/whitefox.vanilla.bin
cmd_cpy ./whitefox.winkeyless.bash      kiibohd.dfu.bin firmware/whitefox.winkeyless.bin
result
exit $?

# Tally results
result
exit $?

