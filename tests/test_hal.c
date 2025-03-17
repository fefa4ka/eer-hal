/**
 * @file test_hal.c
 * @brief Helper file to ensure HAL is properly linked
 */
#include "eer_hal.h"

// This function is not called but ensures the linker includes the HAL
void reference_hal(void) {
    // Reference the HAL to make sure it's linked
    eer_hal.gpio->init();
}
