#!/bin/bash
# Combines a given bootloader image and firmware image into a single firmware binary
# Manufacturing deliverable

# Args
# Argument #1 Path to bootloader binary
# Argument #2 Path to firmware binary
# Argument #3 Memory location of the firmware binary (bootloader always starts at address 0x0) in bytes (hex or decimal)

# Must have three args
if [ "$#" -ne 3 ]; then
	echo "Usage:   `basename $0` <bootloader binary> <firmware binary> <memory address of firmware>"
	echo "Example: `basename $0` kiibohd_bootloader.bin kiibohd.dfu.bin 4096"
	echo "Creates a file called 'kiibohd_manufacturing_<date>.bin'"
	echo "WARNING: Make sure bootloader is smaller than or equal to the memory address of the firmware binary."
	exit 1
fi

# Copy images to /tmp
cp "$1" /tmp/.
cp "$2" /tmp/.

bootloader=$(basename "$1")
firmware=$(basename "$2")

# Pad bootloader binary to given address
truncate -s "$3" /tmp/"$bootloader"

# Concatenate firmware image onto newly sized bootloader
cat /tmp/"$bootloader" /tmp/"$firmware" > kiibohd_manufacturing_$(date +%Y-%m-%d).bin

