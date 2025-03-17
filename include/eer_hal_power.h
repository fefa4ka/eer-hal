/**
 * @file eer_hal_power.h
 * @brief Power management hardware abstraction layer interface for the EER Framework
 * 
 * This file defines the interface for power management operations across all supported
 * platforms. Platform-specific implementations will implement these functions.
 */
#pragma once

#include "eer_hal_errors.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Power modes
 */
typedef enum {
    EER_POWER_MODE_RUN,       /*!< Normal run mode */
    EER_POWER_MODE_SLEEP,     /*!< Sleep mode */
    EER_POWER_MODE_DEEP_SLEEP, /*!< Deep sleep mode */
    EER_POWER_MODE_STANDBY    /*!< Standby mode */
} eer_power_mode_t;

/**
 * @brief Wakeup sources
 */
typedef enum {
    EER_WAKEUP_PIN,           /*!< External pin wakeup */
    EER_WAKEUP_RTC,           /*!< RTC alarm wakeup */
    EER_WAKEUP_TIMER,         /*!< Timer wakeup */
    EER_WAKEUP_WATCHDOG       /*!< Watchdog timer wakeup */
} eer_wakeup_source_t;

/**
 * @brief Power management hardware abstraction layer interface
 * 
 * This structure provides a consistent interface for power management operations
 * across different hardware platforms.
 */
typedef struct {
    /**
     * @brief Initialize the power management subsystem
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*init)(void);
    
    /**
     * @brief Deinitialize the power management subsystem
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*deinit)(void);
    
    /**
     * @brief Set the power mode
     * @param mode Power mode to set
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*set_mode)(eer_power_mode_t mode);
    
    /**
     * @brief Get the current power mode
     * @param[out] mode Pointer to store the current power mode
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*get_mode)(eer_power_mode_t* mode);
    
    /**
     * @brief Enable a wakeup source
     * @param source Wakeup source to enable
     * @param pin_or_id Pin number or identifier for the wakeup source (if applicable)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*enable_wakeup_source)(eer_wakeup_source_t source, uint8_t pin_or_id);
    
    /**
     * @brief Disable a wakeup source
     * @param source Wakeup source to disable
     * @param pin_or_id Pin number or identifier for the wakeup source (if applicable)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*disable_wakeup_source)(eer_wakeup_source_t source, uint8_t pin_or_id);
    
    /**
     * @brief Get the last wakeup source
     * @param[out] source Pointer to store the last wakeup source
     * @param[out] pin_or_id Pointer to store the pin number or identifier (if applicable)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*get_wakeup_source)(eer_wakeup_source_t* source, uint8_t* pin_or_id);
    
    /**
     * @brief Get the current voltage level
     * @param[out] voltage_mv Pointer to store the voltage in millivolts
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*get_voltage)(uint16_t* voltage_mv);
    
    /**
     * @brief Get the current power consumption
     * @param[out] power_mw Pointer to store the power in milliwatts
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*get_power_consumption)(uint16_t* power_mw);
} eer_power_handler_t;
