/**
 * @file eer_hal_system.h
 * @brief System control hardware abstraction layer interface for the EER Framework
 * 
 * This file defines the interface for system control operations across all supported
 * platforms. Platform-specific implementations will implement these functions.
 */
#pragma once

#include "eer_hal_errors.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief System reset types
 */
typedef enum {
    EER_SYSTEM_RESET_SOFT,    /*!< Software reset */
    EER_SYSTEM_RESET_HARD,    /*!< Hardware reset */
    EER_SYSTEM_RESET_WATCHDOG /*!< Watchdog reset */
} eer_system_reset_type_t;

/**
 * @brief System hardware abstraction layer interface
 * 
 * This structure provides a consistent interface for system control operations
 * across different hardware platforms.
 */
typedef struct {
    /**
     * @brief Initialize the system
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*init)(void);
    
    /**
     * @brief Deinitialize the system
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*deinit)(void);
    
    /**
     * @brief Reset the system
     * @param reset_type Type of reset to perform
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*reset)(eer_system_reset_type_t reset_type);
    
    /**
     * @brief Disable global interrupts
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*disable_interrupts)(void);
    
    /**
     * @brief Enable global interrupts
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*enable_interrupts)(void);
    
    /**
     * @brief Delay execution for a specified number of milliseconds
     * @param ms Number of milliseconds to delay
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*delay_ms)(uint32_t ms);
    
    /**
     * @brief Delay execution for a specified number of microseconds
     * @param us Number of microseconds to delay
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*delay_us)(uint32_t us);
    
    /**
     * @brief Get system tick count
     * @param[out] ticks Pointer to store the tick count
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*get_tick)(uint32_t* ticks);
    
    /**
     * @brief Get system uptime in milliseconds
     * @param[out] uptime Pointer to store the uptime
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*get_uptime_ms)(uint32_t* uptime);
} eer_system_handler_t;
