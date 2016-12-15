#!/usr/bin/env bash
# Loads firmware image using an SWD Flasher
# Uses MCHCK ruby flasher toolchain
# NOTE: Only tested with a buspirate on Linux

# Arg 1: Path to firmware image
# Arg 2: Address to flash to (byte address)

# Must have two args
if [ "$#" -ne 2 ]; then
	echo "Usage:   `basename $0` <firmware binary> <starting address>"
	echo "Example: `basename $0` kiibohd_bootloader.bin 0"
	exit 1
fi

# First check to see if the flasher toolchain is available
if [ ! -d "programmer" ]; then
	# Use git to download the toolchain
	git clone https://github.com/kiibohd/programmer.git
fi

# Make sure the toolchain is up to date
cd programmer
#git pull --rebase
cd ..

# Attempt to flash
# Udev rules have been applied to name the buspirate as /dev/buspirate (instead of something like /dev/ttyUSB0)
# By default only root can access serial devices on Linux
#ruby programmer/flash.rb name=buspirate:dev=/dev/buspirate --mass-erase
ruby programmer/flash.rb name=buspirate:dev=/dev/buspirate "$1" "$2"
#ruby programmer/flash.rb name=buspirate:dev=/dev/buspirate --mass-erase "$1" "$2"

