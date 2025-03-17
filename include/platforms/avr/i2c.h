#pragma once

#include "eer_hal_i2c.h"
#include <avr/io.h>

/**
 * @brief AVR-specific I2C structure
 */
typedef struct {
    volatile uint8_t* twbr;  /*!< TWI Bit Rate Register */
    volatile uint8_t* twcr;  /*!< TWI Control Register */
    volatile uint8_t* twsr;  /*!< TWI Status Register */
    volatile uint8_t* twdr;  /*!< TWI Data Register */
    volatile uint8_t* twar;  /*!< TWI (Slave) Address Register */
    volatile uint8_t* twamr; /*!< TWI Address Mask Register */
} eer_i2c_t;

/**
 * @brief Macro to create an AVR I2C structure
 */
#define eer_hal_i2c0() \
    { &TWBR, &TWCR, &TWSR, &TWDR, &TWAR, &TWAMR }

/**
 * @brief AVR I2C handler structure
 * This structure contains function pointers for AVR I2C operations
 */
extern eer_i2c_handler_t eer_avr_i2c;
