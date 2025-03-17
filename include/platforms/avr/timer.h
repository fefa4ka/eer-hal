#pragma once

#include "eer_hal_timer.h"
#include <avr/io.h>

/**
 * @brief AVR-specific timer structure
 */
typedef struct {
    volatile uint16_t* tcnt;   /*!< Timer/Counter Register */
    volatile uint8_t*  tccra;  /*!< Timer/Counter Control Register A */
    volatile uint8_t*  tccrb;  /*!< Timer/Counter Control Register B */
    volatile uint8_t*  timsk;  /*!< Timer Interrupt Mask Register */
    volatile uint8_t*  tifr;   /*!< Timer Interrupt Flag Register */
    volatile uint16_t* ocra;   /*!< Output Compare Register A */
    volatile uint16_t* ocrb;   /*!< Output Compare Register B */
    volatile uint16_t* icr;    /*!< Input Capture Register */
} eer_timer_t;

/**
 * @brief Macro to create an AVR Timer1 structure
 */
#define eer_hal_timer1() \
    { &TCNT1, &TCCR1A, &TCCR1B, &TIMSK1, &TIFR1, &OCR1A, &OCR1B, &ICR1 }

/**
 * @brief AVR Timer handler structure
 * This structure contains function pointers for AVR Timer operations
 */
extern eer_timer_handler_t eer_avr_timer;
