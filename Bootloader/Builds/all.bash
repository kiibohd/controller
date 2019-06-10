#!/usr/bin/env bash
# This script runs each of the bootloader build scripts
# Jacob Alexander 2017-2018

# If git tag is set, append to each copied firmware file
git_tag=${git_tag:-""}
isuffix="_bootloader.bin"
if [[ "${git_tag}" != "" ]]; then
	suffix=".${git_tag}.bootloader.bin"
else
	suffix=".bootloader.bin"
fi



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
test -d bootloader && rm -rf bootloader
cmd_cpy "bash ./mk20dx128vlf5.bash"   kiibohd${isuffix} bootloader/mk20dx128vlf5${suffix}
cmd_cpy "bash ./mk20dx256vlh7.bash"   kiibohd${isuffix} bootloader/mk20dx256vlh7${suffix}
cmd_cpy "bash ./sam4s2a.bash"         kiibohd${isuffix} bootloader/sam4s8b${suffix}
cmd_cpy "bash ./sam4s4b.bash"         kiibohd${isuffix} bootloader/sam4s8b${suffix}
cmd_cpy "bash ./sam4s8b.bash"         kiibohd${isuffix} bootloader/sam4s8b${suffix}
cmd_cpy "bash ./sam4s8c.bash"         kiibohd${isuffix} bootloader/sam4s8c${suffix}
cmd_cpy "bash ./60v2.bash"            kiibohd${isuffix} bootloader/60v2${suffix}
cmd_cpy "bash ./ergodox.bash"         kiibohd${isuffix} bootloader/ergodox${suffix}
cmd_cpy "bash ./geminiduskdawn.bash"  kiibohd${isuffix} bootloader/geminiduskdawn${suffix}
cmd_cpy "bash ./infinity.bash"        kiibohd${isuffix} bootloader/infinity${suffix}
cmd_cpy "bash ./infinity_led.bash"    kiibohd${isuffix} bootloader/infinity_led${suffix}
cmd_cpy "bash ./keystone.bash"        kiibohd${isuffix} bootloader/keystone${suffix}
cmd_cpy "bash ./kira.bash"            kiibohd${isuffix} bootloader/kira${suffix}
cmd_cpy "bash ./ktype.bash"           kiibohd${isuffix} bootloader/ktype${suffix}
cmd_cpy "bash ./whitefox.bash"        kiibohd${isuffix} bootloader/whitefox${suffix}

# Tally results
result
exit $?

