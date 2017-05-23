FROM ubuntu:xenial

RUN apt-get update && \
    apt-get install -qy git cmake ctags tmux libusb-1.0-0-dev binutils-arm-none-eabi \
    gcc-arm-none-eabi libnewlib-arm-none-eabi dfu-util python3 python3-pil git ninja-build && \
    rm -rf /var/lib/apt/lists/*

VOLUME /controller
WORKDIR /controller/Keyboards
CMD /bin/bash

# 1. Build the image after the initial cloning of this repo
# docker build -t controller . # notice the dot at the end

# 2. Run the image from within the repository root
# docker run -it --rm -v "$(pwd):/controller" controller

# 3. Build the firmware
# ./ergodox.bash

# 4. Exit the container and load the firmware
#   a. exit
#   b. cd ./Keyboards/ICED-L.gcc/
#   c. ./load
