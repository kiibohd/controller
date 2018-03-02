#!/usr/bin/env bash
#
# Builds each of the IC keyboards with host mode in the current directory
# Jacob Alexander 2018

# Settings
export EnableHostOnlyBuild=true
export BuildPathPrepend="Extra/"
export CMakeListsPath="../../.."

# Change working directory
cd ..

# Import build script
"${BASH_SOURCE%/*}/ic_keyboards.bash"

