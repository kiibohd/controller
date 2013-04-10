#!/bin/bash

#temp jo
#export PATH="/c/WinAVR-20100110/bin:/c/Program Files (x86)/CMake 2.8/bin:${PATH}"
#echo $PATH
export PATH="/c/WinAVR-20100110/bin:/c/Program Files (x86)/CMake 2.8/bin:/usr/local/bin:/usr/bin:/bin:/c/Windows/system32"
echo $PATH


which cmake.exe
which -a cmake.exe
which avr-gcc
which make
#alias cmake="cmake.exe"
#alias make="avr-nm"

#cd build
cmake -G "Unix Makefiles" -D \
CMAKE_C_COMPILER="C:/WinAVR-20100110/bin/avr-gcc.exe" \
-D CMAKE_CXX_COMPILER="C:/WinAVR-20100110/bin/avr-g++.exe" .
#cd ..
make
#nm

#./buildall.bash
