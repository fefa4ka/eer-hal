#pragma once

#include <stdint.h>
#include <stdbool.h>

// Common types used across all platforms
typedef void (*eer_callback_fn)(void* argument, void* trigger);

typedef struct {
    eer_callback_fn method;
    void* argument;
} eer_callback_t;


// Include all peripheral interface definitions
#include "eer_hal_gpio.h"
#include "eer_hal_adc.h"
#include "eer_hal_uart.h"
#include "eer_hal_spi.h"
#include "eer_hal_i2c.h"
#include "eer_hal_timer.h"
#include "eer_hal_system.h"
#include "eer_hal_power.h"

// Master HAL structure that combines all peripherals
typedef struct {
    eer_gpio_handler_t*    gpio;
    eer_adc_handler_t*     adc;
    eer_uart_handler_t*    uart;
    eer_spi_handler_t*     spi;
    eer_i2c_handler_t*     i2c;
    eer_timer_handler_t*   timer;
    eer_system_handler_t*  system;
    eer_power_handler_t*   power;
} eer_hal_t;


// Global HAL instance - defined by platform implementation
extern eer_hal_t eer_hal;
