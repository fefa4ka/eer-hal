/**
 * @file eer_hal_gpio.h
 * @brief GPIO hardware abstraction layer interface for the EER Framework
 * 
 * This file defines the interface for GPIO operations across all supported
 * platforms. Platform-specific implementations will implement these functions.
 */
#pragma once

#include "eer_hal_errors.h"
#include "macros.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief GPIO pin operating modes
 */
typedef enum {
    EER_GPIO_MODE_INPUT,           /*!< Input floating */
    EER_GPIO_MODE_INPUT_PULLUP,    /*!< Input with pull-up resistor */
    EER_GPIO_MODE_INPUT_PULLDOWN,  /*!< Input with pull-down resistor */
    EER_GPIO_MODE_OUTPUT,          /*!< Output push-pull */
    EER_GPIO_MODE_OUTPUT_OD,       /*!< Output open-drain */
    EER_GPIO_MODE_ANALOG,          /*!< Analog mode */
    EER_GPIO_MODE_ALTERNATE,       /*!< Alternate function */
    EER_GPIO_MODE_ALTERNATE_OD     /*!< Alternate function open-drain */
} eer_gpio_mode_t;

/**
 * @brief GPIO pin speed settings
 */
typedef enum {
    EER_GPIO_SPEED_LOW,            /*!< Low speed */
    EER_GPIO_SPEED_MEDIUM,         /*!< Medium speed */
    EER_GPIO_SPEED_HIGH,           /*!< High speed */
    EER_GPIO_SPEED_VERY_HIGH       /*!< Very high speed */
} eer_gpio_speed_t;

/**
 * @brief GPIO pin interrupt trigger modes
 */
typedef enum {
    EER_GPIO_TRIGGER_NONE,         /*!< No interrupt */
    EER_GPIO_TRIGGER_RISING,       /*!< Rising edge trigger */
    EER_GPIO_TRIGGER_FALLING,      /*!< Falling edge trigger */
    EER_GPIO_TRIGGER_BOTH          /*!< Both edges trigger */
} eer_gpio_trigger_t;

/**
 * @brief Configuration parameters for a GPIO pin
 */
typedef struct {
    eer_gpio_mode_t    mode;       /*!< Pin operating mode */
    eer_gpio_speed_t   speed;      /*!< Pin speed (if applicable) */
    eer_gpio_trigger_t trigger;    /*!< Interrupt trigger mode */
    uint8_t            alternate;  /*!< Alternate function number (if applicable) */
} eer_gpio_config_t;

/**
 * @brief GPIO interrupt callback information
 */
typedef struct {
    void*          pin;            /*!< Pin that triggered the interrupt */
    void*          user_data;      /*!< User data passed during registration */
} eer_gpio_irq_t;

/**
 * @brief GPIO interrupt callback function type
 * @param irq Information about the interrupt event
 */
typedef void (*eer_gpio_irq_handler_t)(eer_gpio_irq_t* irq);

/**
 * @brief GPIO hardware abstraction layer interface
 * 
 * This structure provides a consistent interface for GPIO operations
 * across different hardware platforms.
 */
typedef struct {
    /**
     * @brief Initialize the GPIO hardware
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*init)(void);
    
    /**
     * @brief Deinitialize the GPIO hardware
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*deinit)(void);
    
    /**
     * @brief Configure a GPIO pin
     * @param pin Pin identifier
     * @param config Configuration parameters
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*configure)(void *pin, eer_gpio_config_t* config);
    
    /**
     * @brief Set a GPIO pin output to high or low
     * @param pin Pin identifier
     * @param state true for high, false for low
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*write)(void *pin, bool state);
    
    /**
     * @brief Read the current state of a GPIO pin
     * @param pin Pin identifier
     * @param[out] state Pointer to store the pin state (true for high, false for low)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*read)(void *pin, bool* state);
    
    /**
     * @brief Toggle the state of a GPIO pin
     * @param pin Pin identifier
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*toggle)(void *pin);
    
    /**
     * @brief Register a callback for GPIO interrupts
     * @param pin Pin identifier
     * @param handler Callback function
     * @param user_data User data to pass to the callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*register_irq)(void *pin, 
                                     eer_gpio_irq_handler_t handler, 
                                     void* user_data);
    
    /**
     * @brief Unregister a GPIO interrupt callback
     * @param pin Pin identifier
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*unregister_irq)(void *pin);
    
    /**
     * @brief Enable interrupts for a GPIO pin
     * @param pin Pin identifier
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*enable_irq)(void *pin);
    
    /**
     * @brief Disable interrupts for a GPIO pin
     * @param pin Pin identifier
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*disable_irq)(void *pin);
} eer_gpio_handler_t;

