/**
 * @file eer_hal_timer.h
 * @brief Timer hardware abstraction layer interface for the EER Framework
 * 
 * This file defines the interface for Timer operations across all supported
 * platforms. Platform-specific implementations will implement these functions.
 */
#pragma once

#include "eer_hal_errors.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Timer operating modes
 */
typedef enum {
    EER_TIMER_MODE_ONE_SHOT,    /*!< One-shot timer mode */
    EER_TIMER_MODE_CONTINUOUS,  /*!< Continuous timer mode */
    EER_TIMER_MODE_PWM          /*!< PWM mode */
} eer_timer_mode_t;

/**
 * @brief Timer event types
 */
typedef enum {
    EER_TIMER_EVENT_OVERFLOW,   /*!< Timer overflow event */
    EER_TIMER_EVENT_COMPARE,    /*!< Timer compare match event */
    EER_TIMER_EVENT_CAPTURE     /*!< Timer input capture event */
} eer_timer_event_t;

/**
 * @brief Timer configuration parameters
 */
typedef struct {
    uint32_t        frequency;   /*!< Timer clock frequency in Hz */
    eer_timer_mode_t mode;       /*!< Timer operating mode */
    uint32_t        period;      /*!< Timer period in ticks */
    uint8_t         channel;     /*!< Timer channel (if applicable) */
} eer_timer_config_t;

/**
 * @brief Timer event information
 */
typedef struct {
    void*           timer;      /*!< Timer instance that triggered the event */
    eer_timer_event_t event;    /*!< Type of timer event */
    uint32_t        value;      /*!< Current timer value */
    void*           user_data;  /*!< User data passed during registration */
} eer_timer_event_info_t;

/**
 * @brief Timer event callback function type
 * @param event Information about the timer event
 */
typedef void (*eer_timer_event_handler_t)(eer_timer_event_info_t* event);

/**
 * @brief Timer hardware abstraction layer interface
 * 
 * This structure provides a consistent interface for Timer operations
 * across different hardware platforms.
 */
typedef struct {
    /**
     * @brief Initialize the timer hardware
     * @param config Configuration parameters
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*init)(eer_timer_config_t* config);
    
    /**
     * @brief Deinitialize the timer hardware
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*deinit)(void);
    
    /**
     * @brief Start the timer
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*start)(void);
    
    /**
     * @brief Stop the timer
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*stop)(void);
    
    /**
     * @brief Set the timer period
     * @param period Timer period in ticks
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*set_period)(uint32_t period);
    
    /**
     * @brief Get the current timer value
     * @param[out] value Pointer to store the timer value
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*get_value)(uint32_t* value);
    
    /**
     * @brief Set the timer compare value
     * @param channel Compare channel (if applicable)
     * @param value Compare value
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*set_compare)(uint8_t channel, uint32_t value);
    
    /**
     * @brief Set the PWM duty cycle
     * @param channel PWM channel
     * @param duty_cycle Duty cycle value (0-100%)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*set_pwm_duty_cycle)(uint8_t channel, uint8_t duty_cycle);
    
    /**
     * @brief Convert time in microseconds to timer ticks
     * @param us Time in microseconds
     * @return Equivalent timer ticks
     */
    uint32_t (*us_to_ticks)(uint32_t us);
    
    /**
     * @brief Convert timer ticks to time in microseconds
     * @param ticks Timer ticks
     * @return Equivalent time in microseconds
     */
    uint32_t (*ticks_to_us)(uint32_t ticks);
    
    /**
     * @brief Register a callback for timer events
     * @param event Event type
     * @param channel Channel (if applicable)
     * @param handler Callback function
     * @param user_data User data to pass to the callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*register_callback)(eer_timer_event_t event, 
                                         uint8_t channel,
                                         eer_timer_event_handler_t handler, 
                                         void* user_data);
    
    /**
     * @brief Unregister a timer event callback
     * @param event Event type
     * @param channel Channel (if applicable)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*unregister_callback)(eer_timer_event_t event, uint8_t channel);
} eer_timer_handler_t;
