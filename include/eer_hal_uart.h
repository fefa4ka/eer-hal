/**
 * @file eer_hal_uart.h
 * @brief UART hardware abstraction layer interface for the EER Framework
 * 
 * This file defines the interface for UART operations across all supported
 * platforms. Platform-specific implementations will implement these functions.
 */
#pragma once

#include "eer_hal_errors.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief UART parity options
 */
typedef enum {
    EER_UART_PARITY_NONE,  /*!< No parity */
    EER_UART_PARITY_EVEN,  /*!< Even parity */
    EER_UART_PARITY_ODD    /*!< Odd parity */
} eer_uart_parity_t;

/**
 * @brief UART stop bit options
 */
typedef enum {
    EER_UART_STOP_BITS_1,  /*!< 1 stop bit */
    EER_UART_STOP_BITS_2   /*!< 2 stop bits */
} eer_uart_stop_bits_t;

/**
 * @brief UART data bit options
 */
typedef enum {
    EER_UART_DATA_BITS_5,  /*!< 5 data bits */
    EER_UART_DATA_BITS_6,  /*!< 6 data bits */
    EER_UART_DATA_BITS_7,  /*!< 7 data bits */
    EER_UART_DATA_BITS_8,  /*!< 8 data bits */
    EER_UART_DATA_BITS_9   /*!< 9 data bits */
} eer_uart_data_bits_t;

/**
 * @brief UART configuration parameters
 */
typedef struct {
    uint32_t            baudrate;   /*!< Baud rate in bits per second */
    eer_uart_parity_t   parity;     /*!< Parity setting */
    eer_uart_stop_bits_t stop_bits; /*!< Number of stop bits */
    eer_uart_data_bits_t data_bits; /*!< Number of data bits */
    bool                flow_control; /*!< Hardware flow control enable */
} eer_uart_config_t;

/**
 * @brief UART receive event information
 */
typedef struct {
    void*     uart;       /*!< UART instance that received data */
    uint8_t*  data;       /*!< Pointer to received data buffer */
    uint16_t  size;       /*!< Size of received data */
    void*     user_data;  /*!< User data passed during registration */
} eer_uart_rx_event_t;

/**
 * @brief UART transmit complete event information
 */
typedef struct {
    void*     uart;       /*!< UART instance that completed transmission */
    void*     user_data;  /*!< User data passed during registration */
} eer_uart_tx_event_t;

/**
 * @brief UART receive callback function type
 * @param event Information about the receive event
 */
typedef void (*eer_uart_rx_handler_t)(eer_uart_rx_event_t* event);

/**
 * @brief UART transmit complete callback function type
 * @param event Information about the transmit complete event
 */
typedef void (*eer_uart_tx_handler_t)(eer_uart_tx_event_t* event);

/**
 * @brief UART hardware abstraction layer interface
 * 
 * This structure provides a consistent interface for UART operations
 * across different hardware platforms.
 */
typedef struct {
    /**
     * @brief Initialize the UART hardware
     * @param config Configuration parameters
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*init)(eer_uart_config_t* config);
    
    /**
     * @brief Deinitialize the UART hardware
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*deinit)(void);
    
    /**
     * @brief Transmit data over UART
     * @param data Pointer to data buffer
     * @param size Size of data to transmit
     * @param timeout Timeout in milliseconds (0 for non-blocking)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*transmit)(const uint8_t* data, uint16_t size, uint32_t timeout);
    
    /**
     * @brief Receive data from UART
     * @param data Pointer to data buffer
     * @param size Size of data to receive
     * @param timeout Timeout in milliseconds (0 for non-blocking)
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*receive)(uint8_t* data, uint16_t size, uint32_t timeout);
    
    /**
     * @brief Check if UART is ready to transmit
     * @param ready Pointer to store ready status
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*is_tx_ready)(bool* ready);
    
    /**
     * @brief Check if UART has received data
     * @param ready Pointer to store ready status
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*is_rx_ready)(bool* ready);
    
    /**
     * @brief Register a callback for UART receive events
     * @param handler Callback function
     * @param user_data User data to pass to the callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*register_rx_callback)(eer_uart_rx_handler_t handler, void* user_data);
    
    /**
     * @brief Unregister a UART receive callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*unregister_rx_callback)(void);
    
    /**
     * @brief Register a callback for UART transmit complete events
     * @param handler Callback function
     * @param user_data User data to pass to the callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*register_tx_callback)(eer_uart_tx_handler_t handler, void* user_data);
    
    /**
     * @brief Unregister a UART transmit complete callback
     * @return Status code indicating success or failure
     */
    eer_hal_status_t (*unregister_tx_callback)(void);
} eer_uart_handler_t;
