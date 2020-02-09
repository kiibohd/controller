#!/usr/bin/env bash
# This script runs each of the firmware build scripts
# Input Club supported keyboard targets
# Jacob Alexander 2017-2020

###########
# Options #
###########

# Enables host-build tests
export EnableHostOnlyBuild=${EnableHostOnlyBuild:-false}

# Option to forceably disable the address sanitizer
DisableSanitizer=${DisableSanitizer:-false}

# If git tag is set, append to each copied firmware file
git_tag=${git_tag:-""}
isuffix=".dfu.bin"
if [[ "${git_tag}" != "" ]]; then
	suffix=".${git_tag}${isuffix}"
else
	suffix="${isuffix}"
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

# Run test builds
if [[ "${1}" != "win" ]] && ${EnableHostOnlyBuild}; then
	# Enable run-time sanitizers
	export EnableSanitizer=true
	export CMakeExtraArgs="-DSANITIZER=1"
	if $TRAVIS; then
		if [ "$TRAVIS_OS_NAME" = "osx" ]; then
			export EnableSanitizer=false
			export CMakeExtraArgs=""
			echo "macOS builds on Travis-CI don't seem to like the DYLD_INSERT_LIBRARIES preload, disabling sanitization."
		fi
	fi
	# Force disable the sanitizer
	if ${DisableSanitizer}; then
		export EnableSanitizer=false
		export CMakeExtraArgs=""
		echo "Sanitizers disabled!"
	fi

	# NOTE: Infinity Ergodox is not tested as Interconnect and LCD needs more infrastructure to test
	cmd "bash ./60v2.bash"
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

	# Tally results
	result
	exit $?
fi

# Run builds, normal
if [ "${1}" != "win" ]; then
	test -d firmware && rm -rf firmware
	cmd_cpy "bash ./60v2.bash"                     kiibohd${isuffix} firmware/60v2${suffix}
	cmd_cpy "bash ./ergodox-l.bash"                kiibohd${isuffix} firmware/ergodox.left${suffix}
	cmd_cpy "bash ./ergodox-r.bash"                kiibohd${isuffix} firmware/ergodox.right${suffix}
	cmd_cpy "bash ./geminiduskdawn.bash"           kiibohd${isuffix} firmware/geminiduskdawn${suffix}
	cmd_cpy "bash ./infinity.alphabet.bash"        kiibohd${isuffix} firmware/infinity.alphabet${suffix}
	cmd_cpy "bash ./infinity.hacker.bash"          kiibohd${isuffix} firmware/infinity.hacker${suffix}
	cmd_cpy "bash ./infinity.standard.bash"        kiibohd${isuffix} firmware/infinity.standard${suffix}
	cmd_cpy "bash ./infinity_led.alphabet.bash"    kiibohd${isuffix} firmware/infinity_led.alphabet${suffix}
	cmd_cpy "bash ./infinity_led.hacker.bash"      kiibohd${isuffix} firmware/infinity_led.hacker${suffix}
	cmd_cpy "bash ./infinity_led.standard.bash"    kiibohd${isuffix} firmware/infinity_led.standard${suffix}
	cmd_cpy "bash ./k-type.bash"                   kiibohd${isuffix} firmware/k-type${suffix}
	cmd_cpy "bash ./kira.bash"                     kiibohd${isuffix} firmware/kira${suffix}
	cmd_cpy "bash ./whitefox.aria.bash"            kiibohd${isuffix} firmware/whitefox.aria${suffix}
	cmd_cpy "bash ./whitefox.iso.bash"             kiibohd${isuffix} firmware/whitefox.iso${suffix}
	cmd_cpy "bash ./whitefox.jackofalltrades.bash" kiibohd${isuffix} firmware/whitefox.jackofalltrades${suffix}
	cmd_cpy "bash ./whitefox.truefox.bash"         kiibohd${isuffix} firmware/whitefox.truefox${suffix}
	cmd_cpy "bash ./whitefox.vanilla.bash"         kiibohd${isuffix} firmware/whitefox.vanilla${suffix}
	cmd_cpy "bash ./whitefox.winkeyless.bash"      kiibohd${isuffix} firmware/whitefox.winkeyless${suffix}

# Windows (no symlinks)
else
	env

	test -d firmware && rm -rf firmware
	cmd_cpy "bash ./60v2.bash"                     kiibohd${isuffix} firmware/60v2${suffix}
	cmd_cpy "bash ./geminiduskdawn.bash"           kiibohd${isuffix} firmware/geminiduskdawn${suffix}
	cmd_cpy "bash ./ergodox-l.bash"                kiibohd${isuffix} firmware/ergodox.left${suffix}
	cmd_cpy "bash ./ergodox-r.bash"                kiibohd${isuffix} firmware/ergodox.right${suffix}
	cmd_cpy "bash ./infinity.bash"                 kiibohd${isuffix} firmware/infinity${suffix}
	cmd_cpy "bash ./infinity_led.bash"             kiibohd${isuffix} firmware/infinity_led${suffix}
	cmd_cpy "bash ./k-type.bash"                   kiibohd${isuffix} firmware/k-type${suffix}
	cmd_cpy "bash ./kira.bash"                     kiibohd${isuffix} firmware/kira${suffix}
	cmd_cpy "bash ./whitefox.bash"                 kiibohd${isuffix} firmware/whitefox${suffix}
fi

# Tally results
result
exit $?

