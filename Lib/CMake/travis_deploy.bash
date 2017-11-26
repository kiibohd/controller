#!/bin/bash
# Builds the necessary targets to deploy using Travis-CI
# Jacob Alexander 2017
set -x

# Build Firmware
cd ../../Keyboards
./ic_keyboards.bash
cd -

# Build Bootloaders
cd ../../Bootloader/Builds
./all.bash
cd -

