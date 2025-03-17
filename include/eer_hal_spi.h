/**
 * @file eer_hal_spi.h
 * @brief SPI hardware abstraction layer interface for the EER Framework
 * 
 * This file defines the interface for SPI operations across all supported
 * platforms. Platform-specific implementations will implement these functions.
 */
#pragma once

#include "eer_hal_errors.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief SPI clock polarity options
 */
typedef enum {
    EER_SPI_CPOL_LOW,   /*!< Clock idle state is low */
    EER_SPI_CPOL_HIGH   /*!< Clock idle state is high */
} eer_spi_cpol_t;

/**
 * @brief SPI clock phase options
 */
typedef enum {
    EER_SPI_CPHA_FIRST, /*!< Data sampled on first clock edge */
    EER_SPI_CPHA_SECOND /*!< Data sampled on second clock edge */
} eer_spi_cpha_t;

/**
 * @brief SPI bit order options
 */
typedef enum {
    EER_SPI_BIT_ORDER_MSB, /*!< Most significant bit first */
    EER_SPI_BIT_ORDER_LSB  /*!< Least significant bit first */
} eer_spi_bit_order_t;

/**
 * @brief SPI data size options
 */
typedef enum {
    EER_SPI_DATA_SIZE_8BIT,  /*!< 8-bit data size */
    EER_SPI_DATA_SIZE_16BIT  /*!< 16-bit data size */
} eer_spi_data_size_t;

/**
 * @brief SPI mode options (combinations of CPOL and CPHA)
 */
typedef enum {
    EER_SPI_MODE_0, /*!< CPOL=0, CPHA=0 */
    EER_SPI_MODE_1, /*!< CPOL=0, CPHA=1 */
    EER_SPI_MODE_2, /*!< CPOL=1, CPHA=0 */
    EER_SPI_MODE_3  /*!< CPOL=1, CPHA=1 */
} eer_spi_mode_t;

/**
 * @brief SPI clock prescaler options
 */
typedef enum {
    EER_SPI_PRESCALER_2,   /*!< Divide clock by 2 */
    EER_SPI_PRESCALER_4,   /*!< Divide clock by 4 */
    EER_SPI_PRESCALER_8,   /*!< Divide clock by 8 */
    EER_SPI_PRESCALER_16,  /*!< Divide clock by 16 */
    EER_SPI_PRESCALER_32,  /*!< Divide clock by 32 */
    EER_SPI_PRESCALER_64,  /*!< Divide clock by 64 */
    EER_SPI_PRESCALER_128  /*!< Divide clock by 128 */
} eer_spi_prescaler_t;

/**
 * @brief SPI configuration parameters
 */
typedef struct {
    eer_spi_mode_t      mode;       /*!< SPI mode (clock polarity and phase) */
    eer_spi_bit_order_t bit_order;  /*!< Bit order (MSB or LSB first) */
    eer_spi_data_size_t data_size;  /*!< Data size (8 or 16 bits) */
    eer_spi_prescaler_t prescaler;  /*!< Clock prescaler */
    bool                master;     /*!< Master mode (true) or slave mode (false) */
} eer_spi_config_t;

/**
 * @brief SPI transfer complete event information
 */
typedef struct {
    void*     spi;        /*!< SPI instance that completed transfer */
    uint8_t*  tx_data;    /*!< Pointer to transmitted data buffer */
    uint8_t*  rx_data;    /*!< Pointer to received data buffer */
    uint16_t  size;       /*!< Size of transferred data */
    void*     user_data;  /*!< User data passed during registration */
} eer_spi_transfer_event_t;

/**
 * @brief SPI transfer complete callback function type
 * @param event Information about the transfer event
 */
typedef void (*eer_spi_transfer_handler_t)(eer_spi_transfer_event_t* event);

/**
 * @brief SPI hardware abstraction layer interface
 * 
 * This structure provides a consistent interface for SPI operations
 * across different hardware platforms.
 */
typedef struct {
    /**
     * @brief Initialize the SPI hardware
     * @param config Configuration parameters
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*init)(eer_spi_config_t* config);
    
    /**
     * @brief Deinitialize the SPI hardware
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*deinit)(void);
    
    /**
     * @brief Transmit and receive data over SPI
     * @param tx_data Pointer to data to transmit (can be NULL for receive-only)
     * @param rx_data Pointer to buffer for received data (can be NULL for transmit-only)
     * @param size Size of data to transfer
     * @param timeout Timeout in milliseconds (0 for non-blocking)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*transfer)(const uint8_t* tx_data, uint8_t* rx_data, uint16_t size, uint32_t timeout);
    
    /**
     * @brief Transmit data over SPI
     * @param data Pointer to data to transmit
     * @param size Size of data to transmit
     * @param timeout Timeout in milliseconds (0 for non-blocking)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*transmit)(const uint8_t* data, uint16_t size, uint32_t timeout);
    
    /**
     * @brief Receive data from SPI
     * @param data Pointer to buffer for received data
     * @param size Size of data to receive
     * @param timeout Timeout in milliseconds (0 for non-blocking)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*receive)(uint8_t* data, uint16_t size, uint32_t timeout);
    
    /**
     * @brief Check if SPI is ready for transfer
     * @param ready Pointer to store ready status
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*is_ready)(bool* ready);
    
    /**
     * @brief Set the chip select pin state
     * @param pin Pin identifier
     * @param state true to select (active low), false to deselect
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*chip_select)(void* pin, bool state);
    
    /**
     * @brief Register a callback for SPI transfer complete events
     * @param handler Callback function
     * @param user_data User data to pass to the callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*register_callback)(eer_spi_transfer_handler_t handler, void* user_data);
    
    /**
     * @brief Unregister an SPI transfer complete callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*unregister_callback)(void);
} eer_spi_handler_t;
