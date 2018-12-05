#!/bin/bash
# Builds the necessary targets to deploy using Travis-CI
# Jacob Alexander 2017-2018
set -x

# Get git tag
export git_tag=$(git tag -l --points-at HEAD)
echo "Git Tag: ${git_tag}"

# Don't do anything if there isn't a tag
if [[ "${git_tag}" = "" ]]; then
	echo "Warning: Most recent git commit does not have a tag."
	exit 0
fi

# Build Firmware
cd ../../Keyboards
./ic_keyboards.bash
cd -

# Build Bootloaders
cd ../../Bootloader/Builds
./all.bash
cd -

