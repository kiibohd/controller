#!/bin/bash
###| Builder Script |###
#
# Builds all permutations of modules
# This script is an attempt to maintain module sanity as new ones are added
#
# Fortunately, sweeping API changes don't happen much anymore...but just in case...
#
# Written by Jacob Alexander 2013 for the Kiibohd Controller
# Released into the Public Domain
#
###

## TODO List ##
# - Complete non-Scan module permutations (will take extra work)
# - Add command line arguments
# - Add help flag for usage
# - Make sure the script is being run from the correct directory


main() {
	ERROR="\e[5;1;31mERROR\e[0m:"
	failCount=0

	# Scan for list of Scan Modules
	scanModules=$(ls Scan)

	# Prune out "invalid" modules (parent modules)
	scanModules=${scanModules[@]//matrix/}

	# Create permutation directories
	# Then run cmake, and run each build permutation
	# Keeping track of how many builds failed/passed
	for module in $scanModules; do
		# Create directory, but do not error if it exists already
		mkdir -p build/$module
		cd build/$module

		# Make sure CMake has been run, and attempt to build
		cmake -DScanModuleOverride=$module ../.. && make || let failCount++

		# Cleanup, for the next build
		cd - > /dev/null
	done

	totalModules=$(echo $scanModules | wc -w)
	if (( failCount > 0 )); then
		echo -e "$ERROR $failCount/$totalModules failed"
	else
		echo -e "Build Success!"
	fi
}


#| Main Script Entry
main "$@"


exit 0

