#!/bin/bash
# Common functions for running kiibohd unit tests
# Jacob Alexander 2016-2017

PASSED=0
FAILED=0

# Results
result() {
	echo "### Final Results ###"
	echo "${PASSED}/$((PASSED+FAILED))"
	if (( FAILED == 0 )); then
		return 0
	else
		return 1
	fi
}

# Runs a command, increments test passed/failed
# Args: Command
cmd() {
	# Run command
	echo "CMD: $@"
	$@
	local RET=$?

	# Check command
	if [[ ${RET} -ne 0 ]]; then
		((FAILED++))
	else
		((PASSED++))
	fi

	return ${RET}
}

# Runs a command and copies the give file to a given location
# Args:
#  1) Command to run
#  2) File to copy, uses the most recently created directory
#  3) Where to copy file
cmd_cpy() {
	cmd "${1}"
	local RET=$?

	# Store most recently created directory
	local src_path=$(ls -t | head -1)

	# Make sure the destination exists
	mkdir -p $(dirname ${3})

	# If the build failed, make sure to delete the old one
	if [[ ${RET} -ne 0 ]]; then
		echo "Build failed for '${1}' removing firmware '${3}'"
		rm -f ${3}
	else
		# Copy file
		cp -f ${src_path}/${2} ${3}
	fi

	return ${RET}
}

