#pragma once

#include "eer_hal_gpio.h"
#include "avr.h"

typedef struct {
    struct {
        volatile unsigned char *ddr;
        volatile unsigned char *port;
        volatile unsigned char *pin;
    } port;
    unsigned char number;
} eer_avr_pin_t;

#define eer_hw_pin(port, pin)                                                  \
    {                                                                          \
        {&DDR##port, &PORT##port, &PIN##port}, pin                             \
    }                                                                
