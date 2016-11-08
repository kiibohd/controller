FROM ubuntu:xenial

RUN apt-get update
RUN apt-get install -y git cmake ctags tmux
RUN apt-get install -y libusb-1.0-0-dev binutils-arm-none-eabi gcc-arm-none-eabi libnewlib-arm-none-eabi dfu-util
RUN apt-get install -y python3 python3-pil
