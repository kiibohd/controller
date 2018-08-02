FROM ubuntu:bionic

RUN apt-get update && \
    apt-get install -qy locales

RUN echo "LANG=en_US.UTF-8" > /etc/locale.conf
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
ENV LANG en_US.UTF-8
ENV LC_ALL en_US.UTF-8
ENV LANGUAGE en_US:en
RUN locale-gen

RUN apt-get install -qy git cmake ctags tmux libusb-1.0-0-dev binutils-arm-none-eabi lsb-core \
    gcc-arm-none-eabi libnewlib-arm-none-eabi dfu-util python3 python3-pil git ninja-build python3-pip && \
    rm -rf /var/lib/apt/lists/* && \
    pip3 install pipenv kll

VOLUME /controller
WORKDIR /controller/Keyboards
CMD /bin/bash

# 1. Build the image after the initial cloning of this repo
# docker build -t controller .. # notice the dots at the end
# cd ..

# 2. Run the image from within the repository root
# docker run -it --rm -v "$(pwd):/controller" controller

# 3. Build the firmware
# ./ergodox.bash

# 4. Exit the container and load the firmware
#   a. exit
#   b. cd ./Keyboards/ICED-L.gcc/
#   c. ./load
