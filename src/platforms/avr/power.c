#include "platforms/avr/power.h"
#include "macros.h"
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>

// Current power mode
static eer_power_mode_t current_power_mode = EER_POWER_MODE_RUN;

// Last wakeup source
static struct {
    eer_wakeup_source_t source;
    uint8_t pin_or_id;
} last_wakeup = {0};

static eer_hal_status_t avr_power_init(void) {
    // Initialize power management
    // Nothing specific needed for AVR
    return EER_HAL_OK;
}

static eer_hal_status_t avr_power_deinit(void) {
    // Nothing to deinitialize
    return EER_HAL_OK;
}

static eer_hal_status_t avr_power_set_mode(eer_power_mode_t mode) {
    switch (mode) {
        case EER_POWER_MODE_RUN:
            // Already in run mode, nothing to do
            break;
            
        case EER_POWER_MODE_SLEEP:
            // Configure sleep mode as idle
            set_sleep_mode(SLEEP_MODE_IDLE);
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
            break;
            
        case EER_POWER_MODE_DEEP_SLEEP:
            // Configure sleep mode as power-save
            set_sleep_mode(SLEEP_MODE_PWR_SAVE);
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
            break;
            
        case EER_POWER_MODE_STANDBY:
            // Configure sleep mode as power-down
            set_sleep_mode(SLEEP_MODE_PWR_DOWN);
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
            break;
            
        default:
            return EER_HAL_INVALID_PARAM;
    }
    
    current_power_mode = mode;
    return EER_HAL_OK;
}

static eer_hal_status_t avr_power_get_mode(eer_power_mode_t* mode) {
    if (mode == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    *mode = current_power_mode;
    return EER_HAL_OK;
}

static eer_hal_status_t avr_power_enable_wakeup_source(eer_wakeup_source_t source, uint8_t pin_or_id) {
    switch (source) {
        case EER_WAKEUP_PIN:
            // Enable external interrupt for the specified pin
            // This is a simplified implementation
            if (pin_or_id == 0) {
                // INT0
                EIMSK |= (1 << INT0);
            } else if (pin_or_id == 1) {
                // INT1
                EIMSK |= (1 << INT1);
            } else {
                return EER_HAL_INVALID_PARAM;
            }
            break;
            
        case EER_WAKEUP_TIMER:
            // Enable Timer/Counter2 overflow interrupt
            TIMSK2 |= (1 << TOIE2);
            break;
            
        case EER_WAKEUP_WATCHDOG:
            // Enable watchdog interrupt
            WDTCSR |= (1 << WDIE);
            break;
            
        case EER_WAKEUP_RTC:
            // AVR doesn't have a built-in RTC
            return EER_HAL_NOT_SUPPORTED;
            
        default:
            return EER_HAL_INVALID_PARAM;
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_power_disable_wakeup_source(eer_wakeup_source_t source, uint8_t pin_or_id) {
    switch (source) {
        case EER_WAKEUP_PIN:
            // Disable external interrupt for the specified pin
            if (pin_or_id == 0) {
                // INT0
                EIMSK &= ~(1 << INT0);
            } else if (pin_or_id == 1) {
                // INT1
                EIMSK &= ~(1 << INT1);
            } else {
                return EER_HAL_INVALID_PARAM;
            }
            break;
            
        case EER_WAKEUP_TIMER:
            // Disable Timer/Counter2 overflow interrupt
            TIMSK2 &= ~(1 << TOIE2);
            break;
            
        case EER_WAKEUP_WATCHDOG:
            // Disable watchdog interrupt
            WDTCSR &= ~(1 << WDIE);
            break;
            
        case EER_WAKEUP_RTC:
            // AVR doesn't have a built-in RTC
            return EER_HAL_NOT_SUPPORTED;
            
        default:
            return EER_HAL_INVALID_PARAM;
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_power_get_wakeup_source(eer_wakeup_source_t* source, uint8_t* pin_or_id) {
    if (source == NULL || pin_or_id == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    *source = last_wakeup.source;
    *pin_or_id = last_wakeup.pin_or_id;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_power_get_voltage(uint16_t* voltage_mv) {
    if (voltage_mv == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // AVR doesn't have a built-in voltage sensor
    // This would typically require an ADC reading of VCC
    // For now, return a fixed value
    *voltage_mv = 5000; // 5V
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_power_get_power_consumption(uint16_t* power_mw) {
    if (power_mw == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // AVR doesn't have a built-in power monitor
    // Return estimated values based on power mode
    switch (current_power_mode) {
        case EER_POWER_MODE_RUN:
            *power_mw = 15; // Example value
            break;
        case EER_POWER_MODE_SLEEP:
            *power_mw = 5; // Example value
            break;
        case EER_POWER_MODE_DEEP_SLEEP:
            *power_mw = 2; // Example value
            break;
        case EER_POWER_MODE_STANDBY:
            *power_mw = 1; // Example value
            break;
        default:
            *power_mw = 0;
            break;
    }
    
    return EER_HAL_OK;
}

// External Interrupt 0 ISR
ISR(INT0_vect) {
    last_wakeup.source = EER_WAKEUP_PIN;
    last_wakeup.pin_or_id = 0;
}

// External Interrupt 1 ISR
ISR(INT1_vect) {
    last_wakeup.source = EER_WAKEUP_PIN;
    last_wakeup.pin_or_id = 1;
}

// Timer/Counter2 Overflow ISR
ISR(TIMER2_OVF_vect) {
    last_wakeup.source = EER_WAKEUP_TIMER;
    last_wakeup.pin_or_id = 2;
}

// Watchdog Timer ISR
ISR(WDT_vect) {
    last_wakeup.source = EER_WAKEUP_WATCHDOG;
    last_wakeup.pin_or_id = 0;
}

// Power handler structure with function pointers
eer_power_handler_t eer_avr_power = {
    .init = avr_power_init,
    .deinit = avr_power_deinit,
    .set_mode = avr_power_set_mode,
    .get_mode = avr_power_get_mode,
    .enable_wakeup_source = avr_power_enable_wakeup_source,
    .disable_wakeup_source = avr_power_disable_wakeup_source,
    .get_wakeup_source = avr_power_get_wakeup_source,
    .get_voltage = avr_power_get_voltage,
    .get_power_consumption = avr_power_get_power_consumption
};
