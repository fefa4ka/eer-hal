#include "platforms/avr/system.h"
#include "macros.h"
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>

// System tick counter with atomic access protection
static volatile uint32_t system_ticks = 0;

// Flag to track if system is initialized
static bool system_initialized = false;

// Uptime calculation
#define TICKS_PER_MS (F_CPU / 1000)

// Structure for atomic access to 32-bit tick counter
typedef union {
    uint32_t value;
    struct {
        uint8_t byte0;
        uint8_t byte1;
        uint8_t byte2;
        uint8_t byte3;
    } bytes;
} atomic_u32_t;

static eer_hal_status_t avr_system_init(void) {
    if (system_initialized) {
        return EER_HAL_OK;
    }
    
    // Reset system tick counter
    uint8_t sreg = SREG;
    cli();
    system_ticks = 0;
    SREG = sreg;
    
    // Initialize system tick timer (Timer0)
    // Configure Timer0 for 1ms overflow
    TCCR0A = (1 << WGM01);  // CTC mode
    TCCR0B = 0;  // Stop timer initially
    
    // Calculate the compare value for 1ms period
    // F_CPU / (prescaler * 1000Hz) - 1
    uint8_t compare_value = (F_CPU / 64 / 1000) - 1;
    
    // Set compare value
    OCR0A = compare_value;
    
    // Clear any pending interrupts
    TIFR0 = (1 << OCF0A);
    
    // Enable compare match interrupt
    TIMSK0 = (1 << OCIE0A);
    
    // Start timer with prescaler 64
    TCCR0B = (1 << CS01) | (1 << CS00);
    
    // Enable global interrupts
    sei();
    
    system_initialized = true;
    return EER_HAL_OK;
}

static eer_hal_status_t avr_system_deinit(void) {
    if (!system_initialized) {
        return EER_HAL_OK;
    }
    
    // Stop the timer
    TCCR0B = 0;
    
    // Disable Timer0 interrupt
    TIMSK0 &= ~(1 << OCIE0A);
    
    // Clear any pending interrupts
    TIFR0 = (1 << OCF0A);
    
    system_initialized = false;
    return EER_HAL_OK;
}

static eer_hal_status_t avr_system_reset(eer_system_reset_type_t reset_type) {
    switch (reset_type) {
        case EER_SYSTEM_RESET_SOFT:
            // Jump to reset vector
            asm volatile ("jmp 0");
            break;
            
        case EER_SYSTEM_RESET_WATCHDOG:
            // Enable watchdog with shortest timeout
            wdt_enable(WDTO_15MS);
            // Wait for watchdog to reset the device
            while (1) {}
            break;
            
        case EER_SYSTEM_RESET_HARD:
            // Not directly possible in software
            // Fall back to watchdog reset
            wdt_enable(WDTO_15MS);
            while (1) {}
            break;
            
        default:
            return EER_HAL_INVALID_PARAM;
    }
    
    // Should never reach here
    return EER_HAL_ERROR;
}

static eer_hal_status_t avr_system_disable_interrupts(void) {
    cli();
    return EER_HAL_OK;
}

static eer_hal_status_t avr_system_enable_interrupts(void) {
    sei();
    return EER_HAL_OK;
}


static eer_hal_status_t avr_system_delay_ms(uint32_t ms) {
    while (ms--) {
        _delay_ms(1);
    }
    return EER_HAL_OK;
}

static eer_hal_status_t avr_system_delay_us(uint32_t us) {
    while (us--) {
        _delay_us(1);
    }
    return EER_HAL_OK;
}

static eer_hal_status_t avr_system_get_tick(uint32_t* ticks) {
    if (ticks == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Use a local union for atomic access
    atomic_u32_t local_ticks;
    
    // Disable interrupts to ensure atomic read of 32-bit value
    uint8_t sreg = SREG;
    cli();
    
    // Copy the volatile counter to our local union
    local_ticks.value = system_ticks;
    
    // Restore interrupt state
    SREG = sreg;
    
    // Return the safely copied value
    *ticks = local_ticks.value;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_system_get_uptime_ms(uint32_t* uptime) {
    if (uptime == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    uint32_t ticks;
    avr_system_get_tick(&ticks);
    
    // Each tick is 1ms
    *uptime = ticks;
    
    return EER_HAL_OK;
}

// Timer0 Compare Match A ISR for system tick
ISR(TIMER0_COMPA_vect) {
    // Increment the system tick counter
    // This is safe because the ISR cannot be interrupted
    system_ticks++;
    
    // If we need to perform additional operations on overflow,
    // we could check for it here
    if (system_ticks == 0) {
        // Handle 32-bit overflow if needed
    }
}

// System handler structure with function pointers
eer_system_handler_t eer_avr_system = {
    .init = avr_system_init,
    .deinit = avr_system_deinit,
    .reset = avr_system_reset,
    .disable_interrupts = avr_system_disable_interrupts,
    .enable_interrupts = avr_system_enable_interrupts,
    .delay_ms = avr_system_delay_ms,
    .delay_us = avr_system_delay_us,
    .get_tick = avr_system_get_tick,
    .get_uptime_ms = avr_system_get_uptime_ms
};
