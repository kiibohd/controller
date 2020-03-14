#!/bin/bash
# Builds the necessary targets to deploy using GitHub Actions
# Jacob Alexander 2017-2020
set -x

# Get git tag
git_tag=$(git tag -l --points-at HEAD)
export git_tag
echo "Git Tag: ${git_tag}"

# Don't do anything if there isn't a tag
if [[ "${git_tag}" = "" ]]; then
	echo "Warning: Most recent git commit does not have a tag."
	exit 0
fi

# Build Firmware
cd ../../Keyboards || exit 1
./ic_keyboards.bash
cd - || exit 1

# Build Bootloaders
cd ../../Bootloader/Builds || exit 1
./all.bash
cd - || exit 1
