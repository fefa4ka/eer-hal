cmake_minimum_required(VERSION 3.12)

# Add test executables
add_executable(test_gpio test_gpio.c)
target_link_libraries(test_gpio eer_hal)

# Create a custom target for running the tests
add_custom_target(run_gpio_test
    COMMAND avrdude -c ${PROG_TYPE} -p ${MCU} -U flash:w:test_gpio.hex
    DEPENDS test_gpio
    COMMENT "Uploading and running GPIO test"
)

# Create hex files for AVR
add_custom_command(TARGET test_gpio POST_BUILD
    COMMAND ${AVR_OBJCOPY} -O ihex -R .eeprom test_gpio test_gpio.hex
    COMMENT "Creating HEX file for test_gpio"
)
