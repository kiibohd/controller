#!/bin/bash
# Common functions for running kiibohd unit tests
# Jacob Alexander 2016-2018

PASSED=0
FAILED=0
FAILED_TESTS=()

COLOR_RED="\e[1;91m"
COLOR_GREEN="\e[1;92m"
COLOR_YELLOW="\e[1;93m"
COLOR_CYAN="\e[1;96m"
COLOR_NORMAL="\e[0m"
EnableSaniziter=${EnableSaniziter:-false}

# Sanitizer lookup
if $EnableSaniziter; then
	case "$OSTYPE" in
	# Linux
	"linux-gnu")
		# TODO (HaaTa) This may not work using clang as the compiler (but it may work ok)
		# It's still recommended to have llvm installed to show backtrace
		# Leak sanitizer is finding leaks in Python, these are likely bugs but the code we are
		# testing doesn't actually malloc so it's not a big deal.
		export ASAN_OPTIONS=detect_leaks=0
		export LD_PRELOAD=libasan.so:libubsan.so
		;;
	# macOS
	"darwin"*)
		# TODO (HaaTa) This may possibly point to the wrong clang
		#              It would be better to get this path directly from CMake
		# This says gcc, but on Darwin, it's actually clang
		# (specifying clang seems to have problems)
		export DYLD_INSERT_LIBRARIES=$(gcc -print-resource-dir)/lib/darwin/libclang_rt.asan_osx_dynamic.dylib
		;;
	esac
fi

# Results
result() {
	echo "### Final Results ###"
	if (( FAILED == 0 )); then
		printf "${COLOR_GREEN}${PASSED}/$((PASSED+FAILED))${COLOR_NORMAL}\n"
		return 0
	else
		printf "${COLOR_YELLOW}${PASSED}/$((PASSED+FAILED))${COLOR_NORMAL}\n"
		printf "${COLOR_RED} -- FAILED -- ${COLOR_NORMAL}\n"
		for i in "${FAILED_TESTS[@]}"; do
			printf "${COLOR_RED}${i}${COLOR_NORMAL}\n"
		done
		return 1
	fi
}

# Runs a command, increments test passed/failed
# Args: Command
cmd() {
	# Run command
	printf "${COLOR_CYAN} ==== Test $((PASSED+FAILED+1)) ==== ${COLOR_NORMAL}\n"
	echo "CMD: $@"
	$@
	local RET=$?

	# Check command
	if [[ ${RET} -ne 0 ]]; then
		((FAILED++))
		FAILED_TESTS+=("'${@}'")
		printf "${COLOR_RED} ==> FAILED: $@ ❌${COLOR_NORMAL}\n"
	else
		((PASSED++))
		printf "${COLOR_GREEN} ==> PASSED: $@ ✓${COLOR_NORMAL}\n"
	fi
	echo

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
		printf "${COLOR_RED}Build failed for '${1}' removing firmware '${3}'${COLOR_NORMAL}\n"
		rm -f ${3}
	else
		# Copy file
		cp -f ${src_path}/${2} ${3}
	fi

	return ${RET}
}

