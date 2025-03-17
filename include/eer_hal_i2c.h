/**
 * @file eer_hal_i2c.h
 * @brief I2C hardware abstraction layer interface for the EER Framework
 * 
 * This file defines the interface for I2C operations across all supported
 * platforms. Platform-specific implementations will implement these functions.
 */
#pragma once

#include "eer_hal_errors.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief I2C addressing modes
 */
typedef enum {
    EER_I2C_ADDR_7BIT,  /*!< 7-bit addressing mode */
    EER_I2C_ADDR_10BIT  /*!< 10-bit addressing mode */
} eer_i2c_addr_mode_t;

/**
 * @brief I2C speed modes
 */
typedef enum {
    EER_I2C_SPEED_STANDARD,  /*!< Standard mode (100 kHz) */
    EER_I2C_SPEED_FAST,      /*!< Fast mode (400 kHz) */
    EER_I2C_SPEED_FAST_PLUS  /*!< Fast mode plus (1 MHz) */
} eer_i2c_speed_t;

/**
 * @brief I2C configuration parameters
 */
typedef struct {
    eer_i2c_addr_mode_t addr_mode;  /*!< Addressing mode */
    eer_i2c_speed_t     speed;      /*!< Bus speed */
    uint32_t            clock_hz;   /*!< I2C clock frequency in Hz */
    bool                duty_cycle;  /*!< Duty cycle (true for 16/9, false for 2) */
} eer_i2c_config_t;

/**
 * @brief I2C transfer complete event information
 */
typedef struct {
    void*     i2c;        /*!< I2C instance that completed transfer */
    uint16_t  address;    /*!< Target device address */
    uint8_t*  tx_data;    /*!< Pointer to transmitted data buffer */
    uint8_t*  rx_data;    /*!< Pointer to received data buffer */
    uint16_t  size;       /*!< Size of transferred data */
    void*     user_data;  /*!< User data passed during registration */
} eer_i2c_transfer_event_t;

/**
 * @brief I2C transfer complete callback function type
 * @param event Information about the transfer event
 */
typedef void (*eer_i2c_transfer_handler_t)(eer_i2c_transfer_event_t* event);

/**
 * @brief I2C hardware abstraction layer interface
 * 
 * This structure provides a consistent interface for I2C operations
 * across different hardware platforms.
 */
typedef struct {
    /**
     * @brief Initialize the I2C hardware
     * @param config Configuration parameters
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*init)(eer_i2c_config_t* config);
    
    /**
     * @brief Deinitialize the I2C hardware
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*deinit)(void);
    
    /**
     * @brief Master transmit data over I2C
     * @param address Target device address
     * @param data Pointer to data to transmit
     * @param size Size of data to transmit
     * @param timeout Timeout in milliseconds (0 for non-blocking)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*master_transmit)(uint16_t address, const uint8_t* data, uint16_t size, uint32_t timeout);
    
    /**
     * @brief Master receive data from I2C
     * @param address Target device address
     * @param data Pointer to buffer for received data
     * @param size Size of data to receive
     * @param timeout Timeout in milliseconds (0 for non-blocking)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*master_receive)(uint16_t address, uint8_t* data, uint16_t size, uint32_t timeout);
    
    /**
     * @brief Master transmit then receive data over I2C (combined operation)
     * @param address Target device address
     * @param tx_data Pointer to data to transmit
     * @param tx_size Size of data to transmit
     * @param rx_data Pointer to buffer for received data
     * @param rx_size Size of data to receive
     * @param timeout Timeout in milliseconds (0 for non-blocking)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*master_transmit_receive)(uint16_t address, 
                                               const uint8_t* tx_data, uint16_t tx_size,
                                               uint8_t* rx_data, uint16_t rx_size,
                                               uint32_t timeout);
    
    /**
     * @brief Check if I2C bus is busy
     * @param busy Pointer to store busy status
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*is_busy)(bool* busy);
    
    /**
     * @brief Scan the I2C bus for connected devices
     * @param devices Array to store found device addresses
     * @param max_devices Maximum number of devices to find
     * @param found_devices Pointer to store number of devices found
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*scan)(uint16_t* devices, uint8_t max_devices, uint8_t* found_devices);
    
    /**
     * @brief Register a callback for I2C transfer complete events
     * @param handler Callback function
     * @param user_data User data to pass to the callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*register_callback)(eer_i2c_transfer_handler_t handler, void* user_data);
    
    /**
     * @brief Unregister an I2C transfer complete callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*unregister_callback)(void);
} eer_i2c_handler_t;
