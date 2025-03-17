#include "platforms/avr/uart.h"
#include "macros.h"
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Default UART instance for AVR
static eer_uart_t uart0 = eer_hal_uart0();

// Callback handlers and user data
static struct {
    eer_uart_rx_handler_t rx_handler;
    void* rx_user_data;
    eer_uart_tx_handler_t tx_handler;
    void* tx_user_data;
} uart_callbacks = {0};

// Buffer for interrupt-driven reception
static uint8_t rx_buffer[64];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

/**
 * @brief Calculate UBRR value for the given baud rate
 * @param baudrate Desired baud rate
 * @return UBRR value
 */
static uint16_t uart_calculate_ubrr(uint32_t baudrate) {
    return (((F_CPU) + 4UL * (baudrate)) / (8UL * (baudrate)) - 1UL);
}

static eer_hal_status_t avr_uart_init(eer_uart_config_t* config) {
    if (config == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Calculate baud rate register value
    uint16_t ubrr = uart_calculate_ubrr(config->baudrate);
    
    // Set baud rate
    *uart0.ubrrh = (uint8_t)(ubrr >> 8);
    *uart0.ubrrl = (uint8_t)ubrr;
    
    // Enable double speed mode
    bit_set(*uart0.ucsra, U2X0);
    
    // Set frame format: data bits, parity, stop bits
    uint8_t ucsrc_value = 0;
    
    // Set data bits
    switch (config->data_bits) {
        case EER_UART_DATA_BITS_5:
            // 5 data bits (000)
            break;
        case EER_UART_DATA_BITS_6:
            // 6 data bits (001)
            ucsrc_value |= (1 << UCSZ00);
            break;
        case EER_UART_DATA_BITS_7:
            // 7 data bits (010)
            ucsrc_value |= (1 << UCSZ01);
            break;
        case EER_UART_DATA_BITS_9:
            // 9 data bits (111)
            ucsrc_value |= (1 << UCSZ02) | (1 << UCSZ01) | (1 << UCSZ00);
            break;
        case EER_UART_DATA_BITS_8:
        default:
            // 8 data bits (011)
            ucsrc_value |= (1 << UCSZ01) | (1 << UCSZ00);
            break;
    }
    
    // Set parity
    switch (config->parity) {
        case EER_UART_PARITY_EVEN:
            ucsrc_value |= (1 << UPM01);
            break;
        case EER_UART_PARITY_ODD:
            ucsrc_value |= (1 << UPM01) | (1 << UPM00);
            break;
        case EER_UART_PARITY_NONE:
        default:
            // No parity is default
            break;
    }
    
    // Set stop bits
    if (config->stop_bits == EER_UART_STOP_BITS_2) {
        ucsrc_value |= (1 << USBS0);
    }
    
    // Apply UCSRC settings
    *uart0.ucsrc = ucsrc_value;
    
    // Enable receiver and transmitter
    *uart0.ucsrb = (1 << RXEN0) | (1 << TXEN0);
    
    // Reset buffer indices
    rx_head = 0;
    rx_tail = 0;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_deinit(void) {
    // Disable receiver and transmitter
    *uart0.ucsrb = 0;
    
    // Clear callback handlers
    uart_callbacks.rx_handler = NULL;
    uart_callbacks.rx_user_data = NULL;
    uart_callbacks.tx_handler = NULL;
    uart_callbacks.tx_user_data = NULL;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_transmit(const uint8_t* data, uint16_t size, uint32_t timeout) {
    if (data == NULL || size == 0) {
        return EER_HAL_INVALID_PARAM;
    }
    
    uint32_t start_time = 0; // In a real implementation, get current time
    
    for (uint16_t i = 0; i < size; i++) {
        // Wait for transmit buffer to be empty
        while (!(*uart0.ucsra & (1 << UDRE0))) {
            if (timeout > 0) {
                uint32_t current_time = 0; // In a real implementation, get current time
                if ((current_time - start_time) >= timeout) {
                    return EER_HAL_TIMEOUT;
                }
            }
        }
        
        // Send data
        *uart0.udr = data[i];
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_receive(uint8_t* data, uint16_t size, uint32_t timeout) {
    if (data == NULL || size == 0) {
        return EER_HAL_INVALID_PARAM;
    }
    
    uint32_t start_time = 0; // In a real implementation, get current time
    
    for (uint16_t i = 0; i < size; i++) {
        // Wait for data to be received
        while (!(*uart0.ucsra & (1 << RXC0))) {
            if (timeout > 0) {
                uint32_t current_time = 0; // In a real implementation, get current time
                if ((current_time - start_time) >= timeout) {
                    return EER_HAL_TIMEOUT;
                }
            }
        }
        
        // Get received data
        data[i] = *uart0.udr;
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_is_tx_ready(bool* ready) {
    if (ready == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    *ready = (*uart0.ucsra & (1 << UDRE0)) != 0;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_is_rx_ready(bool* ready) {
    if (ready == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    *ready = (*uart0.ucsra & (1 << RXC0)) != 0;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_register_rx_callback(eer_uart_rx_handler_t handler, void* user_data) {
    if (handler == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Store the handler and user data
    uart_callbacks.rx_handler = handler;
    uart_callbacks.rx_user_data = user_data;
    
    // Enable receive complete interrupt
    *uart0.ucsrb |= (1 << RXCIE0);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_unregister_rx_callback(void) {
    // Clear the handler and user data
    uart_callbacks.rx_handler = NULL;
    uart_callbacks.rx_user_data = NULL;
    
    // Disable receive complete interrupt
    *uart0.ucsrb &= ~(1 << RXCIE0);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_register_tx_callback(eer_uart_tx_handler_t handler, void* user_data) {
    if (handler == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Store the handler and user data
    uart_callbacks.tx_handler = handler;
    uart_callbacks.tx_user_data = user_data;
    
    // Enable transmit complete interrupt
    *uart0.ucsrb |= (1 << TXCIE0);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_uart_unregister_tx_callback(void) {
    // Clear the handler and user data
    uart_callbacks.tx_handler = NULL;
    uart_callbacks.tx_user_data = NULL;
    
    // Disable transmit complete interrupt
    *uart0.ucsrb &= ~(1 << TXCIE0);
    
    return EER_HAL_OK;
}

// UART Receive Complete ISR
ISR(USART_RX_vect) {
    uint8_t data = UDR0;
    
    // Store received byte in buffer
    uint8_t next_head = (rx_head + 1) % sizeof(rx_buffer);
    if (next_head != rx_tail) {
        rx_buffer[rx_head] = data;
        rx_head = next_head;
    }
    
    // Call the handler if registered
    if (uart_callbacks.rx_handler != NULL) {
        eer_uart_rx_event_t event = {
            .uart = &uart0,
            .data = &rx_buffer[rx_head - 1],
            .size = 1,
            .user_data = uart_callbacks.rx_user_data
        };
        
        uart_callbacks.rx_handler(&event);
    }
}

// UART Transmit Complete ISR
ISR(USART_TX_vect) {
    // Call the handler if registered
    if (uart_callbacks.tx_handler != NULL) {
        eer_uart_tx_event_t event = {
            .uart = &uart0,
            .user_data = uart_callbacks.tx_user_data
        };
        
        uart_callbacks.tx_handler(&event);
    }
}

// UART handler structure with function pointers
eer_uart_handler_t eer_avr_uart = {
    .init = avr_uart_init,
    .deinit = avr_uart_deinit,
    .transmit = avr_uart_transmit,
    .receive = avr_uart_receive,
    .is_tx_ready = avr_uart_is_tx_ready,
    .is_rx_ready = avr_uart_is_rx_ready,
    .register_rx_callback = avr_uart_register_rx_callback,
    .unregister_rx_callback = avr_uart_unregister_rx_callback,
    .register_tx_callback = avr_uart_register_tx_callback,
    .unregister_tx_callback = avr_uart_unregister_tx_callback
};
