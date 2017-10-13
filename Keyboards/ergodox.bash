#!/usr/bin/env bash
#
# Keyboard: Ergodox
#
# These build scripts are just a convenience for configuring your keyboard (less daunting than CMake)
# Jacob Alexander 2015-2017


# Build the Left Side
"${BASH_SOURCE%/*}/ergodox-l.bash"

# Build the Right Side
"${BASH_SOURCE%/*}/ergodox-r.bash"