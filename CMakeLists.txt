###| CMAKE Kiibohd Controller |###
#
# Written by Jacob Alexander in 2011 for the Kiibohd Controller
# Due to this file's usefulness:
#
# Released into the Public Domain
#
###

##| TODO List |##
# - Add avr-gcc as requirement
# - Add avr-objcopy as a requirement
# - Generate .lst files
# - Extend .lst to .lss
# - Autoloader to Teensy
# - specify on commandline which atmel mcu

#| Set the Compilers
include( CMakeForceCompiler )
set( CMAKE_SYSTEM_NAME Generic )
cmake_force_c_compiler  ( avr-gcc AVRCCompiler )
cmake_force_cxx_compiler( avr-g++ AVRCxxCompiler )


#| Project
project( kiibohd_controller )


#| General Settings
cmake_minimum_required( VERSION 2.8 )






#| Sources 
set( SRCS
	./main.c
        ./print.c
	./usb_keyboard_debug.c
	./scan_loop.c
)

#| Target Name
set( TARGET kiibohd )







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


#| MCU Name
#| You MUST set this to match the board you are using
#| type "make clean" after changing this, so all files will be rebuilt
#|
#| "at90usb162"       # Teensy   1.0
#| "atmega32u4"       # Teensy   2.0
#| "at90usb646"       # Teensy++ 1.0
#| "at90usb1286"      # Teensy++ 2.0
set( MCU "at90usb1286" )


#| Processor frequency.
#|   Normally the first thing your program should do is set the clock prescaler,
#|   so your program will run at the correct speed.  You should also set this
#|   variable to same clock speed.  The _delay_ms() macro uses this, and many
#|   examples use this variable to calculate timings.  Do not add a "UL" here.
set( F_CPU "16000000" )
set( CDEFS "-DF_CPU=${F_CPU}" )


#| Dependency Files
#| Compiler flags to generate dependency files.
set( GENDEPFLAGS "-MMD -MP" )


#| Listing file
set( TARGET_LST ${TARGET}.lst )


#| Compiler Flags
add_definitions( "-mmcu=${MCU} -DF_CPU=${F_CPU} -O${OPT} ${TUNING} ${WARN} -Wa,-adhlns ${CSTANDARD} ${GENDEPFLAGS}" )


#| Linker Flags
set( LINKER_FLAGS "-mmcu=${MCU} -Wl,-Map=${TARGET}.map,--cref -Wl,--relax -Wl,--gc-sections" )


#| Hex Flags (XXX, CMake seems to have issues if you quote the arguments for the custom commands...)
set( HEX_FLAGS -O ${FORMAT} -R .eeprom -R .fuse -R .lock -R .signature )


#| Eep Flags
set( EEP_FLAGS -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ${FORMAT} )


#| Lss Flags
set( LSS_FLAGS -h -S -z )









#| Create the .ELF file
set( TARGET_ELF ${TARGET}.elf )
add_executable( ${TARGET_ELF} ${SRCS} )


#| .ELF Properties
set_target_properties( ${TARGET_ELF} PROPERTIES
	LINK_FLAGS ${LINKER_FLAGS}
)


#| Convert the .ELF into a .HEX to load onto the Teensy
set( TARGET_HEX ${TARGET}.hex )
add_custom_command( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND avr-objcopy ${HEX_FLAGS} ${TARGET_ELF} ${TARGET_HEX}
	COMMENT "Creating load file for Flash:  ${TARGET_HEX}"
)


#| Convert the .ELF into a .EEP
set( TARGET_EEP ${TARGET}.eep )
add_custom_command( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND avr-objcopy ${EEP_FLAGS} ${TARGET_ELF} ${TARGET_EEP}
	COMMENT "Creating load file for EEPROM: ${TARGET_EEP}"
)


#| Generate the Extended .LSS TODO Incomplete
set( TARGET_LSS ${TARGET}.lss )
add_custom_command( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND avr-objcopy ${LSS_FLAGS} ${TARGET_ELF} > ${TARGET_LSS}
	COMMENT "Creating Extended Listing:     ${TARGET_LSS}"
)


#| Generate the Symbol Table .SYM
set( TARGET_SYM ${TARGET}.sym )
add_custom_command( TARGET ${TARGET_ELF} POST_BUILD
	COMMAND avr-nm -n ${TARGET_ELF} > ${TARGET_SYM}
	COMMENT "Creating Symbol Table:         ${TARGET_SYM}"
)






#| Before Changes Size Information (and make sure it is run before ${TARGET_ELF} is generated) TODO Only call if the files exist
#add_dependencies( ${TARGET_ELF} SizeBefore )
#add_custom_target( SizeBefore ALL avr-size --target=${FORMAT} ${TARGET_HEX} ${TARGET_ELF}
#	COMMENT "Size before generation:"
#)


#| After Changes Size Information
add_custom_target( SizeAfter ALL avr-size --target=${FORMAT} ${TARGET_HEX} ${TARGET_ELF}
	DEPENDS ${TARGET_ELF}
	COMMENT "Size after generation:"
)


