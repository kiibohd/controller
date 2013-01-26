###| CMAKE Kiibohd Controller |###
#
# Jacob Alexander 2011-2013
# Due to this file's usefulness:
#
# Released into the Public Domain
#
# avr-gcc CMake Build Configuration
#
###



#| Set the Compilers (must be set first)
include( CMakeForceCompiler )
cmake_force_c_compiler  ( avr-gcc AVRCCompiler )
cmake_force_cxx_compiler( avr-g++ AVRCxxCompiler )


#| Compiler Binaries
set( OBJCOPY "avr-objcopy" )
set( OBJDUMP "avr-objdump" )
set( NM      "avr-nm"      )
set( SIZE    "avr-size"    )



###
# Atmel Defines and Linker Options
#

#| MCU Name
#| You _MUST_ set this to match the board you are using
#| type "make clean" after changing this, so all files will be rebuilt
#|
#| "at90usb162"       # Teensy   1.0
#| "atmega32u4"       # Teensy   2.0
#| "at90usb646"       # Teensy++ 1.0
#| "at90usb1286"      # Teensy++ 2.0
#set( MCU "atmega32u4" )
set( MCU "at90usb1286" )

message( STATUS "MCU Selected:" )
message( "${MCU}" )


#| Extra Compiler Sources
#| Mostly for convenience functions like interrupt handlers
set( COMPILER_SRCS
	# XXX Not needed for avr-gcc
)


#| Compiler flag to set the C Standard level.
#|     c89   = "ANSI" C
#|     gnu89 = c89 plus GCC extensions
#|     c99   = ISO C99 standard (not yet fully implemented)
#|     gnu99 = c99 plus GCC extensions
set( CSTANDARD "-std=gnu99" )


#| Warning Options
#|  -Wall...:     warning level
set( WARN "-Wall -Wstrict-prototypes" )


#| Tuning Options
#|  -f...:        tuning, see GCC manual and avr-libc documentation
set( TUNING "-funsigned-char -funsigned-bitfields -ffunction-sections -fpack-struct -fshort-enums" )


#| Optimization level, can be [0, 1, 2, 3, s]. 
#|     0 = turn off optimization. s = optimize for size.
#|     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
set( OPT "s" )


#| Output Format
#| srec, ihex, binary
set( FORMAT "ihex" )


#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.
set( F_CPU "16000000" )


#| Dependency Files
#| Compiler flags to generate dependency files.
set( GENDEPFLAGS "-MMD -MP" )


#| Compiler Flags
add_definitions( "-mmcu=${MCU} -DF_CPU=${F_CPU} -D_${MCU}_=1 -O${OPT} ${TUNING} ${WARN} ${CSTANDARD} ${GENDEPFLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}" )


#| Linker Flags
set( LINKER_FLAGS "-mmcu=${MCU} -Wl,-Map=${TARGET}.map,--cref -Wl,--relax -Wl,--gc-sections" )


#| Hex Flags (XXX, CMake seems to have issues if you quote the arguments for the custom commands...)
set( HEX_FLAGS -O ${FORMAT} -R .eeprom -R .fuse -R .lock -R .signature )


#| Eep Flags (XXX, I've removed this target from the builds, but keeping the set line as a note)
set( EEP_FLAGS -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ${FORMAT} )


#| Lss Flags
set( LSS_FLAGS -h -S -z )

