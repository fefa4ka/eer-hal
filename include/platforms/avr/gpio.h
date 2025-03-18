#pragma once

#include "eer_hal_gpio.h"
#include <avr/io.h>

/**
 * @brief AVR-specific pin structure
 */
typedef struct {
    struct {
        volatile unsigned char *ddr;   /*!< Data Direction Register */
        volatile unsigned char *port;  /*!< Port Output Register */
        volatile unsigned char *pin;   /*!< Port Input Register */
    } port;
    unsigned char number;              /*!< Pin number (0-7) */
} eer_pin_t;

/**
 * @brief Macro to create an AVR pin structure
 * @param port Port letter (A, B, C, etc.)
 * @param pin Pin number (0-7)
 */
#define eer_hal_pin(port, pin)                                                  \
    {                                                                          \
        {&DDR##port, &PORT##port, &PIN##port}, pin                             \
    }

/**
 * @brief AVR GPIO handler structure
 * This structure contains function pointers for AVR GPIO operations
 */
extern eer_gpio_handler_t eer_avr_gpio;
