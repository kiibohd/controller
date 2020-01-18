#!/usr/bin/env bash
# Uses filename to determine which build script to use
# Jacob Alexander 2018

Basename=$(basename $0)

# Extract layout
Layout=$(echo $Basename | cut -d'.' -f2)

# Extract build script
BuildScript=$(echo $Basename | cut -d'.' -f1).bash

# Run build
source "${BASH_SOURCE%/*}/${BuildScript}"

