#!/usr/bin/env bash
#
# Builds each of the IC keyboards with host mode in the current directory
# Jacob Alexander 2018

# Settings
export EnableHostOnlyBuild=true
export BuildPathPrepend="Extra/"
export CMakeListsPath="../../.."

# Enable run-time sanitizers
export EnableSaniziter=true
export CMakeExtraArgs="-DSANITIZER=1"

# Change working directory
cd ..

# Import build script
"${BASH_SOURCE%/*}/ic_keyboards.bash"

