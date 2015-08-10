#!/usr/bin/env bash
# ISSILed
# Virtual Serial Port API Example
# Jacob Alexander 2015

if [ $# -eq 0 ]; then
  echo "You must specify your virtual serialport. (/dev/ttyACM0 on linux, /dev/cu.usbmodemXXXX on OSX)"
  echo "  ex: $0 /dev/ttyACM0"
  exit 1
fi
# XXX Set this to match your virtual serialport
# TODO Show examples for Cygwin/Windows
# For Mac OSX it will be something like /dev/cu.usbmodem1413 (number may differ)
SERIALPORT=$1

# NOTE: Make sure you don't write too quickly to the serial port, it can get overwhelmed by a modern computer
#       Generally this just means commands will get ignored
#       I'm using 100 ms sleeps here, but much smaller are probably sufficient

# Clear out cli buffer
printf "\r" > $SERIALPORT

# Write to ISSI Page
# Arguments
#  - page
#  - starting address
#  - data (usually brightness) (8 bits)
#
# For brightness control, set the starting address to 0x24
# By default only page 0x00 is used
# There are 8 pages of memory (these can be cycled through for animiations)
# 144 led channels
# Page 0x0A is used for configuration
# See the datasheet for full details http://www.issi.com/WW/pdf/31FL3731C.pdf
sleep 0.1
printf "ledWPage 0xE8 0x00 0x24 0x10 0x20 0x30 0x40 0x50\r" > $SERIALPORT # Channel 1
printf "ledWPage 0xEA 0x00 0x24 0x10 0x20 0x30 0x40 0x50\r" > $SERIALPORT # Channel 2
printf "ledWPage 0xEC 0x00 0x24 0x10 0x20 0x30 0x40 0x50\r" > $SERIALPORT # Channel 3
printf "ledWPage 0xEE 0x00 0x24 0x10 0x20 0x30 0x40 0x50\r" > $SERIALPORT # Channel 4

