cmake_minimum_required(VERSION 3.12)
project(eer_hal C)

# Determine platform from toolchain or explicit setting
if(NOT DEFINED EER_PLATFORM)
  message(FATAL_ERROR "Unknown platform. Please set EER_PLATFORM.")
  return()
endif()

# Prevent host system flags from being added
set(CMAKE_SYSTEM_NAME Generic)

# Platform-specific source files
include(toolchain/generic-gcc-${EER_PLATFORM}.cmake)

set(PLATFORM_SOURCES 
src/platforms/${EER_PLATFORM}/gpio.c
src/platforms/${EER_PLATFORM}/adc.c
src/platforms/${EER_PLATFORM}/uart.c
src/platforms/${EER_PLATFORM}/timer.c
src/platforms/${EER_PLATFORM}/spi.c
src/platforms/${EER_PLATFORM}/i2c.c
src/platforms/${EER_PLATFORM}/system.c
src/platforms/${EER_PLATFORM}/power.c
src/platforms/${EER_PLATFORM}/hal.c)

# Create HAL library
add_library(eer_hal STATIC ${PLATFORM_SOURCES})

target_include_directories(
  eer_hal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                 ${CMAKE_CURRENT_SOURCE_DIR}/include/platforms/${EER_PLATFORM})

# Make sure all source files can find the necessary headers
target_compile_options(eer_hal PRIVATE -I${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add tests directory
add_subdirectory(tests)

# Make sure the HAL implementation is properly exported
target_compile_definitions(eer_hal PUBLIC EER_PLATFORM=${EER_PLATFORM})

# Generate HEX file for AVR platform
if(EER_PLATFORM STREQUAL "avr")
    add_custom_command(
        TARGET eer_hal
        POST_BUILD
        COMMAND ${AVR_OBJCOPY} -O ihex -R .eeprom eer_hal ${CMAKE_BINARY_DIR}/eer_hal.hex
        COMMENT "Creating HEX file: eer_hal.hex"
    )
endif()
