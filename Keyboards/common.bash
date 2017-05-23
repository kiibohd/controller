#!/bin/bash
# Common functions for running kiibohd unit tests
# Jacob Alexander 2016

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

