# executables in use
##########################################################################
find_program(AVR_CC avr-gcc REQUIRED)
find_program(AVR_CXX avr-g++ REQUIRED)
find_program(AVR_OBJCOPY avr-objcopy REQUIRED)
find_program(AVR_SIZE_TOOL avr-size REQUIRED)
find_program(AVR_STRIP avr-strip REQUIRED)
find_program(AVR_NM avr-nm REQUIRED)
find_program(AVR_OBJDUMP avr-objdump REQUIRED)

##########################################################################
# toolchain starts with defining mandatory variables
##########################################################################
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})
set(CMAKE_ASM_COMPILER ${AVR_CC})
set(CMAKE_NM ${AVR_NN})

 # Prevent host system flags from being added
# Empty out all macOS-specific settings
set(_CMAKE_APPLE_ARCHS_DEFAULT "" CACHE INTERNAL "")
set(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "Build architectures for macOS" FORCE)
set(CMAKE_OSX_DEPLOYMENT_TARGET "" CACHE STRING "Minimum macOS version to target" FORCE)
set(CMAKE_OSX_SYSROOT "" CACHE STRING "The product will be built against the headers and libraries in this SDK" FORCE)

# Prevent automatic detection of these values
set(_CMAKE_OSX_MACHINE "" CACHE INTERNAL "")
set(CMAKE_HOST_SYSTEM_PROCESSOR "")

# Disable macOS-specific linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "")
set(CMAKE_STATIC_LINKER_FLAGS_INIT "")

set(AVR 1)

# Builds compiler options
if(CMAKE_BUILD_TYPE MATCHES Release)
   set(CMAKE_C_FLAGS_RELEASE "-Os")
   set(CMAKE_CXX_FLAGS_RELEASE "-Os")
endif(CMAKE_BUILD_TYPE MATCHES Release)

if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
   set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Os -save-temps -g -gdwarf-3 -gstrict-dwarf")
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

if(CMAKE_BUILD_TYPE MATCHES Debug)
   set(CMAKE_C_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
   set(CMAKE_CXX_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES Debug)


set(F_CPU 8000000UL CACHE STRING "Frequency the crystal on your board")
# CPU, you can find the list here:
# https://gcc.gnu.org/onlinedocs/gcc/AVR-Options.html
set(MCU atmega328p CACHE STRING "ATMega family MCU")
# Read avr include/util/setbaud.h for usage
set(BAUD 9600 CACHE STRING "Baudrate for UART")
# The programmer to use, read avrdude manual for list
set(PROG_TYPE arduino CACHE STRING "AVR programmer")
set(PROG_PORT /dev/tty.usbmodem* CACHE STRING "Programmer port")
set(HAL ${MCU})


# AVR Fuses, must be in concordance with your hardware and F_CPU
# http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p
set(E_FUSE 0xfd)
set(H_FUSE 0xda)
set(L_FUSE 0xfd)
set(LOCK_BIT 0xff)

# Find avrdude
find_program(AVRDUDE avrdude REQUIRED)

# Pass defines to compiler
add_definitions(
    -DF_CPU=${F_CPU}
    -DBAUD=${BAUD}
)
# mmcu MUST be passed to both the compiler and linker
set(CMAKE_EXE_LINKER_FLAGS "-mmcu=${MCU}")

# Override any platform-specific linker flags
set(CMAKE_C_LINK_FLAGS "-mmcu=${MCU}")
set(CMAKE_CXX_LINK_FLAGS "-mmcu=${MCU}")

add_compile_options(
    -mmcu=${MCU} # MCU
    -std=gnu99 # C99 standard
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
)

 # Find AVR include paths
execute_process(
    COMMAND avr-gcc -print-search-dirs
    OUTPUT_VARIABLE AVR_GCC_SEARCH_DIRS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(REGEX MATCH "install: ([^\n]*)" _ ${AVR_GCC_SEARCH_DIRS})
set(AVR_GCC_INSTALL_DIR ${CMAKE_MATCH_1})

message(STATUS "AVR Install Dir: ${AVR_GCC_INSTALL_DIR}")
# Add AVR specific include directories
# FIX: Find a better way to do this
include_directories(
    ${AVR_GCC_INSTALL_DIR}/include
    ${AVR_GCC_INSTALL_DIR}/../../../../../../avr/include
)

# Add custom targets for flashing and erasing
add_custom_target(flash
    COMMAND ${AVRDUDE} -p ${MCU} -c ${PROG_TYPE} -P ${PROG_PORT} -b 115200 -D -U flash:w:${CMAKE_PROJECT_NAME}.hex:i
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Flashing ${CMAKE_PROJECT_NAME}.hex to ${MCU} using ${PROG_TYPE}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(erase
    COMMAND ${AVRDUDE} -p ${MCU} -c ${PROG_TYPE} -P ${PROG_PORT} -e
    COMMENT "Erasing ${MCU} flash memory"
)

add_custom_target(fuses
    COMMAND ${AVRDUDE} -p ${MCU} -c ${PROG_TYPE} -P ${PROG_PORT} -U efuse:w:${E_FUSE}:m -U hfuse:w:${H_FUSE}:m -U lfuse:w:${L_FUSE}:m
    COMMENT "Setting fuses: E=${E_FUSE}, H=${H_FUSE}, L=${L_FUSE}"
)

add_custom_target(lock
    COMMAND ${AVRDUDE} -p ${MCU} -c ${PROG_TYPE} -P ${PROG_PORT} -U lock:w:${LOCK_BIT}:m
    COMMENT "Setting lock bits to ${LOCK_BIT}"
)

add_custom_target(read_fuses
    COMMAND ${AVRDUDE} -p ${MCU} -c ${PROG_TYPE} -P ${PROG_PORT} -U efuse:r:-:h -U hfuse:r:-:h -U lfuse:r:-:h
    COMMENT "Reading fuses from ${MCU}"
)
