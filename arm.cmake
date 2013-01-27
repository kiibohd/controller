###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2013
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# Freescale ARM CMake Build Configuration
#
###



#| Set the Compilers (must be set first)
include( CMakeForceCompiler )
cmake_force_c_compiler  ( arm-none-eabi-gcc AVRCCompiler )
cmake_force_cxx_compiler( arm-none-eabi-g++ AVRCxxCompiler )


#| Compiler Binaries
set( OBJCOPY "arm-none-eabi-objcopy" )
set( OBJDUMP "arm-none-eabi-objdump" )
set( NM      "arm-none-eabi-nm"      )
set( SIZE    "arm-none-eabi-size"    )



###
# ARM Defines and Linker Options
#

#| Chip Name (Linker)
#| You _MUST_ set this to match the board you are using
#| type "make clean" after changing this, so all files will be rebuilt
#|
#| "mk20dx128"        # Teensy   3.0
set( CHIP "mk20dx128" )

message( STATUS "Chip Selected:" )
message( "${CHIP}" )


#| CPU Type
#| You _MUST_ set this to match the board you are using
#| type "make clean" after changing this, so all files will be rebuilt
#|
#| "cortex-m4"        # Teensy   3.0
set( CPU "cortex-m4" )

message( STATUS "CPU Selected:" )
message( "${CPU}" )


#| Extra Compiler Sources
#| Mostly for convenience functions like interrupt handlers
set( COMPILER_SRCS
	Lib/${CHIP}.c
	Lib/delay.c
)


#| Compiler flag to set the C Standard level.
#|     c89   = "ANSI" C
#|     gnu89 = c89 plus GCC extensions
#|     c99   = ISO C99 standard (not yet fully implemented)
#|     gnu99 = c99 plus GCC extensions
set( CSTANDARD "-std=gnu99" )


#| Warning Options
#|  -Wall...:     warning level
set( WARN "-Wall -g" )


#| Tuning Options
#|  -f...:        tuning, see GCC manual
set( TUNING "-mthumb -nostdlib" )


#| Optimization level, can be [0, 1, 2, 3, s]. 
#|     0 = turn off optimization. s = optimize for size.
#|     (Note: 3 is not always the best optimization level.)
set( OPT "s" )


#| Output Format
#| srec, ihex, binary
set( FORMAT "ihex" )


#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.
set( F_CPU "48000000" )


#| Dependency Files
#| Compiler flags to generate dependency files.
set( GENDEPFLAGS "-MMD" )


#| Compiler Flags
add_definitions( "-mcpu=${CPU} -DF_CPU=${F_CPU} -D_${CHIP}_=1 -O${OPT} ${TUNING} ${WARN} ${CSTANDARD} ${GENDEPFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}" )


#| Linker Flags
set( LINKER_FLAGS "-mcpu=${CPU} -Wl,-Map=${TARGET}.map,--cref -Wl,--gc-sections -mthumb -T${CMAKE_CURRENT_SOURCE_DIR}/Lib/${CHIP}.ld" )


#| Hex Flags (XXX, CMake seems to have issues if you quote the arguments for the custom commands...)
set( HEX_FLAGS -O ${FORMAT} -R .eeprom )


#| Lss Flags
set( LSS_FLAGS -h -S -z )

