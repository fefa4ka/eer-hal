cmake_minimum_required(VERSION 3.12)

# Add test executables
add_executable(test_gpio test_gpio.c test_hal.c)
target_link_libraries(test_gpio eer_hal)

# Make sure the test can find the HAL implementation
target_include_directories(test_gpio PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/platforms/${EER_PLATFORM})

# Ensure the test has access to the HAL implementation
target_compile_options(test_gpio PRIVATE -Wl,--undefined=eer_hal)

