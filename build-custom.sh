#!/bin/bash

# NOTE: the cmake directivea are specific to the KLL files in the build-custom/ directory.
# if the set of KLL files changes in there you will need to change the DefaultMap and PartialMaps
# values.
#
# Also note that only the Default, 1, 2 and 7 layers are defined. the lcdFuncMap is still defined for
# the remaining 'blank' layers.

# Build the left-hand firmware
mkdir -p build-left
cd build-left
cp ../build-custom/*.kll .
cmake .. -DScanModule=MDErgo1 -DCHIP=mk20dx256vlh7 '-DBaseMap=defaultMap leftHand slave1 rightHand' -DMacroModule=PartialMap -DOutputModule=pjrcUSB -DDebugModule=full '-DDefaultMap=MDErgo1-Default-0 lcdFuncMap' '-DPartialMaps=MDErgo1-Default-1 lcdFuncMap;MDErgo1-Default-2 lcdFuncMap; lcdFuncMap; lcdFuncMap; lcdFuncMap; lcdFuncMap;MDErgo1-Default-7 lcdFuncMap'
make

# back to the base dir
cd ..

# Build the right-hand firmware
mkdir -p build-right
cd build-right
cp ../build-custom/*.kll .
cmake .. -DScanModule=MDErgo1 -DCHIP=mk20dx256vlh7 '-DBaseMap=defaultMap rightHand slave1 leftHand' -DMacroModule=PartialMap -DOutputModule=pjrcUSB -DDebugModule=full '-DDefaultMap=MDErgo1-Default-0 lcdFuncMap' '-DPartialMaps=MDErgo1-Default-1 lcdFuncMap;MDErgo1-Default-2 lcdFuncMap; lcdFuncMap; lcdFuncMap; lcdFuncMap; lcdFuncMap;MDErgo1-Default-7 lcdFuncMap'
make

# back to the base dir
cd ..
