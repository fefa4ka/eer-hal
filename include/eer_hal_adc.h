/**
 * @file eer_hal_adc.h
 * @brief ADC hardware abstraction layer interface for the EER Framework
 * 
 * This file defines the interface for ADC operations across all supported
 * platforms. Platform-specific implementations will implement these functions.
 */
#pragma once

#include "eer_hal_errors.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief ADC reference voltage sources
 */
typedef enum {
    EER_ADC_REF_VCC,       /*!< Use VCC as reference */
    EER_ADC_REF_EXTERNAL,  /*!< Use external reference */
    EER_ADC_REF_INTERNAL   /*!< Use internal reference */
} eer_adc_reference_t;

/**
 * @brief ADC prescaler values
 */
typedef enum {
    EER_ADC_PRESCALER_2,   /*!< Divide clock by 2 */
    EER_ADC_PRESCALER_4,   /*!< Divide clock by 4 */
    EER_ADC_PRESCALER_8,   /*!< Divide clock by 8 */
    EER_ADC_PRESCALER_16,  /*!< Divide clock by 16 */
    EER_ADC_PRESCALER_32,  /*!< Divide clock by 32 */
    EER_ADC_PRESCALER_64,  /*!< Divide clock by 64 */
    EER_ADC_PRESCALER_128  /*!< Divide clock by 128 */
} eer_adc_prescaler_t;

/**
 * @brief ADC resolution options
 */
typedef enum {
    EER_ADC_RESOLUTION_8BIT,   /*!< 8-bit resolution */
    EER_ADC_RESOLUTION_10BIT,  /*!< 10-bit resolution */
    EER_ADC_RESOLUTION_12BIT,  /*!< 12-bit resolution */
    EER_ADC_RESOLUTION_16BIT   /*!< 16-bit resolution */
} eer_adc_resolution_t;

/**
 * @brief ADC operating modes
 */
typedef enum {
    EER_ADC_MODE_SINGLE,      /*!< Single conversion mode */
    EER_ADC_MODE_CONTINUOUS   /*!< Continuous conversion mode */
} eer_adc_mode_t;

/**
 * @brief ADC configuration parameters
 */
typedef struct {
    eer_adc_reference_t  reference;  /*!< Reference voltage source */
    eer_adc_prescaler_t  prescaler;  /*!< ADC clock prescaler */
    eer_adc_resolution_t resolution; /*!< ADC resolution */
    eer_adc_mode_t       mode;       /*!< ADC operating mode */
} eer_adc_config_t;

/**
 * @brief ADC conversion result information
 */
typedef struct {
    void*     channel;    /*!< Channel that completed conversion */
    uint16_t  value;      /*!< Conversion result value */
    void*     user_data;  /*!< User data passed during registration */
} eer_adc_conversion_t;

/**
 * @brief ADC conversion complete callback function type
 * @param conversion Information about the completed conversion
 */
typedef void (*eer_adc_conversion_complete_handler_t)(eer_adc_conversion_t* conversion);

/**
 * @brief ADC hardware abstraction layer interface
 * 
 * This structure provides a consistent interface for ADC operations
 * across different hardware platforms.
 */
typedef struct {
    /**
     * @brief Initialize the ADC hardware
     * @param config Configuration parameters
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*init)(eer_adc_config_t* config);
    
    /**
     * @brief Deinitialize the ADC hardware
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*deinit)(void);
    
    /**
     * @brief Start an ADC conversion
     * @param channel Channel identifier
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*start_conversion)(void* channel);
    
    /**
     * @brief Stop an ongoing ADC conversion
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*stop_conversion)(void);
    
    /**
     * @brief Check if an ADC conversion is complete
     * @param channel Channel identifier
     * @param[out] complete Pointer to store completion status
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*is_conversion_complete)(void* channel, bool* complete);
    
    /**
     * @brief Read the raw ADC conversion result
     * @param channel Channel identifier
     * @param[out] value Pointer to store the raw ADC value
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*read)(void* channel, uint16_t* value);
    
    /**
     * @brief Read the ADC result as a voltage
     * @param channel Channel identifier
     * @param[out] voltage Pointer to store the voltage value
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*read_voltage)(void* channel, float* voltage);
    
    /**
     * @brief Register a callback for ADC conversion complete events
     * @param channel Channel identifier
     * @param handler Callback function
     * @param user_data User data to pass to the callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*register_callback)(void* channel, 
                                         eer_adc_conversion_complete_handler_t handler, 
                                         void* user_data);
    
    /**
     * @brief Unregister an ADC conversion complete callback
     * @param channel Channel identifier
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*unregister_callback)(void* channel);
} eer_adc_handler_t;
