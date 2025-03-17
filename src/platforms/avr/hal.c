#include "eer_hal.h"
#include "platforms/avr/hal.h"
#include "platforms/avr/gpio.h"
#include "platforms/avr/adc.h"
#include "platforms/avr/uart.h"
#include "platforms/avr/spi.h"
#include "platforms/avr/i2c.h"
#include "platforms/avr/timer.h"
#include "platforms/avr/system.h"
#include "platforms/avr/power.h"

// Global HAL instance for AVR platform
// Using __attribute__((used)) to prevent the linker from optimizing it out
eer_hal_t eer_hal __attribute__((used)) = {
    .gpio = &eer_avr_gpio,
    .adc = &eer_avr_adc,
    .uart = &eer_avr_uart,
    .spi = &eer_avr_spi,
    .i2c = &eer_avr_i2c,
    .timer = &eer_avr_timer,
    .system = &eer_avr_system,
    .power = &eer_avr_power
};
