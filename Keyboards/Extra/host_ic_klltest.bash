#!/usr/bin/env bash
#
# Builds each of the IC keyboards with host mode in the current directory
# Jacob Alexander 2018

# Settings
export HostTest=${HostTest:-kll.py}

# Import build script
"${BASH_SOURCE%/*}/host_ic_keyboards.bash"

