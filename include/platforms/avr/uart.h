#pragma once

#include "eer_hal_uart.h"
#include <avr/io.h>

/**
 * @brief AVR-specific UART structure
 */
typedef struct {
    volatile uint8_t* udr;   /*!< UART Data Register */
    volatile uint8_t* ucsra; /*!< UART Control and Status Register A */
    volatile uint8_t* ucsrb; /*!< UART Control and Status Register B */
    volatile uint8_t* ucsrc; /*!< UART Control and Status Register C */
    volatile uint8_t* ubrrl; /*!< UART Baud Rate Register Low */
    volatile uint8_t* ubrrh; /*!< UART Baud Rate Register High */
} eer_uart_t;

/**
 * @brief Macro to create an AVR UART structure for UART0
 */
#define eer_hal_uart0() \
    { &UDR0, &UCSR0A, &UCSR0B, &UCSR0C, &UBRR0L, &UBRR0H }

/**
 * @brief AVR UART handler structure
 * This structure contains function pointers for AVR UART operations
 */
extern eer_uart_handler_t eer_avr_uart;
