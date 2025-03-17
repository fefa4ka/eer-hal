# executables in use
set(CMAKE_CROSSTOOL tc32-elf)
find_program(TC32_CC ${CMAKE_CROSSTOOL}-gcc REQUIRED)
find_program(TC32_CXX ${CMAKE_CROSSTOOL}-g++ REQUIRED)
find_program(TC32_OBJCOPY ${CMAKE_CROSSTOOL}-objcopy REQUIRED)
find_program(TC32_OBJDUMP ${CMAKE_CROSSTOOL}-objdump REQUIRED)
find_program(TC32_SIZE_TOOL ${CMAKE_CROSSTOOL}-size REQUIRED)
find_program(TC32_STRIP ${CMAKE_CROSSTOOL}-strip REQUIRED)
find_program(TC32_NM ${CMAKE_CROSSTOOL}-nm REQUIRED)
find_program(TC32_GDB ${CMAKE_CROSSTOOL}-gdb-py3 REQUIRED)

# toolchain starts with defining mandatory variables
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR tc32)
set(CMAKE_C_COMPILER ${TC32_CC})
set(CMAKE_CXX_COMPILER ${TC32_CXX})
set(CMAKE_ASM_COMPILER ${TC32_CC})
set(CMAKE_NM ${TC32_NN})
set(TC32 1)

# Builds compiler options
if(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Maximum optimization for speed")
    set(CMAKE_C_FLAGS_RELEASE "-Ofast")
    #   set(CMAKE_ASM_FLAGS_RELEASE "-Os -x assembler ${CMAKE_TC32_FLAGS}")
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    message(STATUS "Maximum optimization for speed, debug info included")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Ofast -save-temps -g -gdwarf-3 -gstrict-dwarf")
elseif (${CMAKE_BUILD_TYPE} MATCHES MinSizeRel)
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Without optimization, debug info included")
    # FIXME: -0g for minimal optimization maybe better
    set(CMAKE_C_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif()

#set(MCU ch573 CACHE STRING "TC32 family MCU")

# The programmer to use, read tc32dude manual for list
set(PROG_TYPE openocd CACHE STRING "TC32 programmer")
set(HAL ${MCU})

set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -Xlinker --gc-sections -Xlinker --print-memory-usage")

if(MCU MATCHES "ch573")
    add_compile_options(
        -std=gnu99
        -march=rv32imac
        -mabi=ilp32
        -mcmodel=medany
        -msmall-data-limit=8
        -mno-save-restore
        )
elseif(MCU MATCHES "ch32v003")
    add_compile_options(
        -std=gnu99
        -march=rv32ec
        -mabi=ilp32e
        -msmall-data-limit=0
        -mno-save-restore
    )

    add_link_options(
        -std=gnu99
        -march=rv32ec
        -mabi=ilp32e
        -msmall-data-limit=0
        -mno-save-restore
        )
endif()

add_compile_options(
    -fmessage-length=0
    -fsigned-char
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fstack-usage
    -Wl,--gc-sections
    )



