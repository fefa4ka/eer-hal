#pragma once

#include "eer_hal_adc.h"
#include <avr/io.h>

/**
 * @brief AVR-specific ADC channel structure
 */
typedef struct {
    uint8_t channel;  /*!< ADC channel number (0-7) */
} eer_adc_channel_t;

/**
 * @brief Macro to create an AVR ADC channel
 * @param ch Channel number (0-7)
 */
#define eer_hal_adc_channel(ch) \
    { ch }

/**
 * @brief AVR ADC handler structure
 * This structure contains function pointers for AVR ADC operations
 */
extern eer_adc_handler_t eer_avr_adc;
