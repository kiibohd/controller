#!/usr/bin/env bash
# This is a build and test script used to test KLL functionality
# It runs on the host system and doesn't require a device to flash onto
# Jacob Alexander 2016



#################
# Configuration #
#################

# Feel free to change the variables in this section to configure your keyboard

BuildPath="macrotest"

## KLL Configuration ##

# Generally shouldn't be changed, this will affect every layer
BaseMap="scancode_map"

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
PartialMaps[1]="hhkbpro2"
PartialMaps[2]="colemak"



##########################
# Advanced Configuration #
##########################

# Don't change the variables in this section unless you know what you're doing
# These are useful for completely custom keyboards
# NOTE: Changing any of these variables will require a force build to compile correctly

# Keyboard Module Configuration
ScanModule="TestIn"
#MacroModule="PartialMap"
MacroModule="PixelMap"
OutputModule="TestOut"
DebugModule="full"

# Microcontroller
Chip="host"

# Compiler Selection
Compiler="gcc"



########################
# Bash Library Include #
########################

# Shouldn't need to touch this section

# Check if the library can be found
if [ ! -f ../cmake.bash ]; then
	echo "ERROR: Cannot find 'cmake.bash'"
	exit 1
fi

# Override CMakeLists path
CMakeListsPath="../../.."

# Load the library
source "../cmake.bash"

# Load common functions
source "../common.bash"

# Run tests
cd "${BuildPath}"

cmd python3 Tests/test.py
cmd python3 Tests/animation.py

# Tally results
result
exit $?

