#!/bin/bash
# This is a build script template
# These build scripts are just a convenience for configuring your keyboard (less daunting than CMake)
# Jacob Alexander 2016-2017



#################
# Configuration #
#################

# Feel free to change the variables in this section to configure your keyboard

BuildPath="K-Type"

## KLL Configuration ##

# Generally shouldn't be changed, this will affect every layer
BaseMap="defaultMap"

# This is the default layer of the keyboard
# NOTE: To combine kll files into a single layout, separate them by spaces
# e.g.  DefaultMap="mylayout mylayoutmod"
DefaultMap="animation_test stdFuncMap"

# This is where you set the additional layers
# NOTE: Indexing starts at 1
# NOTE: Each new layer is another array entry
# e.g.  PartialMaps[1]="layer1 layer1mod"
#       PartialMaps[2]="layer2"
#       PartialMaps[3]="layer3"
PartialMaps[1]="k-type/unset_v1 k-type/rainbow_wipe"
PartialMaps[2]="k-type/unset_v1 k-type/color_painter"



##########################
# Advanced Configuration #
##########################

# Don't change the variables in this section unless you know what you're doing
# These are useful for completely custom keyboards
# NOTE: Changing any of these variables will require a force build to compile correctly

# Keyboard Module Configuration
ScanModule="K-Type"
MacroModule="PixelMap"
OutputModule="pjrcUSB"
DebugModule="full"

# Microcontroller
Chip="mk20dx256vlh7"

# Compiler Selection
Compiler="gcc"



########################
# Bash Library Include #
########################

# Shouldn't need to touch this section

# Check if the library can be found
if [ ! -f cmake.bash ]; then
	echo "ERROR: Cannot find 'cmake.bash'"
	exit 1
fi

# Load the library
source cmake.bash

