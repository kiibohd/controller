#!/usr/bin/env bash
# STLcd
# Virtual Serial Port API Example
# Jacob Alexander 2015

if [ $# -eq 0 ]; then
  echo "You must specify your virtual serialport. (/dev/ttyACM0 on linux, /dev/cu.usbmodemXXXX on OSX)"
  echo "  ex: $0 /dev/ttyACM0"
  exit 1
fi
# XXX Set this to match your virtual serialport
# TODO Show example for Cygwin/Windows
# For Mac OSX it will be something like /dev/cu.usbmodem1413 (number may differ)
SERIALPORT=$1

# NOTE: Make sure you don't write too quickly to the serial port, it can get overwhelmed by a modern computer
#       Generally this just means commands will get ignored
#       I'm using 100 ms sleeps here, but much smaller are probably sufficient

# Clear out cli buffer
printf "\r" > $SERIALPORT

# Change backlight color
# 3 16-bit numbers (hex or decimal) Red, Green and Blue
sleep 0.1
printf "lcdColor 0x100 0x2000 0x4000\r" > $SERIALPORT # Light blue

# Change the lcd image
# Arguments:
#  - page
#  - starting address
#  - pixels (1 bit per pixel)
#
# There are 9 total pages of display memory, but only 4 are visable at time (it is possible to scroll though)
# Each page is 128 bits wide (16 bytes)
# See the datasheet for full details http://www.newhavendisplay.com/specs/NHD-C12832A1Z-FSRGB-FBW-3V.pdf
sleep 0.1
printf "lcdDisp 0x0 0x0  0xFF 0x13 0xFF 0x11 0xFF\r" > $SERIALPORT
sleep 0.1
printf "lcdDisp 0x1 0x10 0xFF 0x13 0xFF 0x11 0xFF 0x44\r" > $SERIALPORT
sleep 0.1
printf "lcdDisp 0x2 0x20 0xFF 0x13 0xFF 0x11 0xFF\r" > $SERIALPORT
sleep 0.1
printf "lcdDisp 0x3 0x30 0xFF 0x13 0xFF 0x11 0xFF\r" > $SERIALPORT

# Send command directly to the lcd
# See the datasheet for full details http://www.newhavendisplay.com/specs/NHD-C12832A1Z-FSRGB-FBW-3V.pdf
sleep 0.1
printf "lcdCmd 0xA7\r" > $SERIALPORT # Reverse display (0xA6 is Normal)

