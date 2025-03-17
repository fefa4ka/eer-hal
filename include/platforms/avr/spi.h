#pragma once

#include "eer_hal_spi.h"
#include <avr/io.h>

/**
 * @brief AVR-specific SPI structure
 */
typedef struct {
    volatile uint8_t* spcr;  /*!< SPI Control Register */
    volatile uint8_t* spsr;  /*!< SPI Status Register */
    volatile uint8_t* spdr;  /*!< SPI Data Register */
    struct {
        volatile uint8_t* ddr;   /*!< Data Direction Register */
        volatile uint8_t* port;  /*!< Port Output Register */
        volatile uint8_t* pin;   /*!< Port Input Register */
        uint8_t mosi;            /*!< MOSI pin number */
        uint8_t miso;            /*!< MISO pin number */
        uint8_t sck;             /*!< SCK pin number */
        uint8_t ss;              /*!< SS pin number */
    } pins;
} eer_spi_t;

/**
 * @brief Macro to create an AVR SPI structure
 */
#define eer_hal_spi0() \
    { &SPCR, &SPSR, &SPDR, { &DDRB, &PORTB, &PINB, PORTB3, PORTB4, PORTB5, PORTB2 } }

/**
 * @brief AVR SPI handler structure
 * This structure contains function pointers for AVR SPI operations
 */
extern eer_spi_handler_t eer_avr_spi;
