#!/usr/bin/env bash
#
# Dockerfile: Ubuntu
#
# Convience script for testing Dockerfiles
# Jacob Alexander 2018
set -e


#################
# Configuration #
#################

# Name of the dockerfile
Dockerfile="Dockerfile.ubuntu"

# Name of the dockerimage
Dockerimage="controller.ubuntu"



########################
# Bash Library Include #
########################

echo "${BASH_SOURCE%/*}/docker.bash"
if [ ! -f "${BASH_SOURCE%/*}/docker.bash" ]; then
	echo "ERROR: Cannot find 'docker.bash'"
	exit 1
fi

# Load the library
source "${BASH_SOURCE%/*}/docker.bash"

