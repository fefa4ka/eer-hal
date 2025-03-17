#include "platforms/avr/timer.h"
#include "macros.h"
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Default Timer instance for AVR
static eer_timer_t timer1 = eer_hal_timer1();

// Callback handlers and user data for different timer events
static struct {
    eer_timer_event_handler_t overflow_handler;
    void* overflow_user_data;
    eer_timer_event_handler_t compare_a_handler;
    void* compare_a_user_data;
    eer_timer_event_handler_t compare_b_handler;
    void* compare_b_user_data;
    eer_timer_event_handler_t capture_handler;
    void* capture_user_data;
} timer_callbacks = {0};

// Current timer configuration
static eer_timer_config_t current_config = {0};

static eer_hal_status_t avr_timer_init(eer_timer_config_t* config) {
    if (config == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Store the configuration
    current_config = *config;
    
    // Reset timer registers
    *timer1.tccra = 0;
    *timer1.tccrb = 0;
    *timer1.timsk = 0;
    *timer1.tcnt = 0;
    
    // Configure timer mode
    switch (config->mode) {
        case EER_TIMER_MODE_ONE_SHOT:
        case EER_TIMER_MODE_CONTINUOUS:
            // Normal mode (0)
            *timer1.tccra &= ~((1 << WGM11) | (1 << WGM10));
            *timer1.tccrb &= ~((1 << WGM13) | (1 << WGM12));
            break;
            
        case EER_TIMER_MODE_PWM:
            // Fast PWM mode, TOP = ICR1 (14)
            *timer1.tccra |= (1 << WGM11);
            *timer1.tccra &= ~(1 << WGM10);
            *timer1.tccrb |= ((1 << WGM13) | (1 << WGM12));
            
            // Set TOP value based on period
            *timer1.icr = config->period;
            
            // Configure PWM outputs (non-inverting mode)
            *timer1.tccra |= ((1 << COM1A1) | (1 << COM1B1));
            *timer1.tccra &= ~((1 << COM1A0) | (1 << COM1B0));
            break;
    }
    
    // Set prescaler based on desired frequency
    // For simplicity, we'll use a fixed prescaler of 8
    // A more complete implementation would calculate the best prescaler
    *timer1.tccrb |= (1 << CS11);  // Prescaler = 8
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_timer_deinit(void) {
    // Stop the timer
    *timer1.tccrb &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
    
    // Disable all interrupts
    *timer1.timsk = 0;
    
    // Clear all callback handlers
    timer_callbacks.overflow_handler = NULL;
    timer_callbacks.overflow_user_data = NULL;
    timer_callbacks.compare_a_handler = NULL;
    timer_callbacks.compare_a_user_data = NULL;
    timer_callbacks.compare_b_handler = NULL;
    timer_callbacks.compare_b_user_data = NULL;
    timer_callbacks.capture_handler = NULL;
    timer_callbacks.capture_user_data = NULL;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_timer_start(void) {
    // Reset counter
    *timer1.tcnt = 0;
    
    // Start timer with configured prescaler
    // For simplicity, we're using a fixed prescaler of 8
    *timer1.tccrb |= (1 << CS11);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_timer_stop(void) {
    // Stop the timer by clearing the clock select bits
    *timer1.tccrb &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_timer_set_period(uint32_t period) {
    if (period > 0xFFFF) {
        // 16-bit timer can't handle periods larger than 0xFFFF
        return EER_HAL_INVALID_PARAM;
    }
    
    if (current_config.mode == EER_TIMER_MODE_PWM) {
        // In PWM mode, period is set via ICR1
        *timer1.icr = period;
    } else {
        // In other modes, we use the overflow interrupt
        // and reset the counter in the ISR if needed
        if (timer_callbacks.overflow_handler != NULL) {
            // Enable overflow interrupt
            *timer1.timsk |= (1 << TOIE1);
        }
    }
    
    current_config.period = period;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_timer_get_value(uint32_t* value) {
    if (value == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    *value = *timer1.tcnt;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_timer_set_compare(uint8_t channel, uint32_t value) {
    if (value > 0xFFFF) {
        // 16-bit timer can't handle values larger than 0xFFFF
        return EER_HAL_INVALID_PARAM;
    }
    
    switch (channel) {
        case 0:
            *timer1.ocra = value;
            break;
        case 1:
            *timer1.ocrb = value;
            break;
        default:
            return EER_HAL_INVALID_PARAM;
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_timer_set_pwm_duty_cycle(uint8_t channel, uint8_t duty_cycle) {
    if (duty_cycle > 100 || current_config.mode != EER_TIMER_MODE_PWM) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Calculate the compare value based on duty cycle and period
    uint16_t compare_value = (current_config.period * duty_cycle) / 100;
    
    switch (channel) {
        case 0:
            *timer1.ocra = compare_value;
            break;
        case 1:
            *timer1.ocrb = compare_value;
            break;
        default:
            return EER_HAL_INVALID_PARAM;
    }
    
    return EER_HAL_OK;
}

static uint32_t avr_timer_us_to_ticks(uint32_t us) {
    // With a prescaler of 8 and assuming F_CPU = 16MHz,
    // each tick is 0.5us, so multiply by 2
    return us * 2;
}

static uint32_t avr_timer_ticks_to_us(uint32_t ticks) {
    // With a prescaler of 8 and assuming F_CPU = 16MHz,
    // each tick is 0.5us, so divide by 2
    return ticks / 2;
}

static eer_hal_status_t avr_timer_register_callback(eer_timer_event_t event, 
                                                  uint8_t channel,
                                                  eer_timer_event_handler_t handler, 
                                                  void* user_data) {
    if (handler == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    switch (event) {
        case EER_TIMER_EVENT_OVERFLOW:
            timer_callbacks.overflow_handler = handler;
            timer_callbacks.overflow_user_data = user_data;
            *timer1.timsk |= (1 << TOIE1);  // Enable overflow interrupt
            break;
            
        case EER_TIMER_EVENT_COMPARE:
            if (channel == 0) {
                timer_callbacks.compare_a_handler = handler;
                timer_callbacks.compare_a_user_data = user_data;
                *timer1.timsk |= (1 << OCIE1A);  // Enable compare A interrupt
            } else if (channel == 1) {
                timer_callbacks.compare_b_handler = handler;
                timer_callbacks.compare_b_user_data = user_data;
                *timer1.timsk |= (1 << OCIE1B);  // Enable compare B interrupt
            } else {
                return EER_HAL_INVALID_PARAM;
            }
            break;
            
        case EER_TIMER_EVENT_CAPTURE:
            timer_callbacks.capture_handler = handler;
            timer_callbacks.capture_user_data = user_data;
            *timer1.timsk |= (1 << ICIE1);  // Enable input capture interrupt
            break;
            
        default:
            return EER_HAL_INVALID_PARAM;
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_timer_unregister_callback(eer_timer_event_t event, uint8_t channel) {
    switch (event) {
        case EER_TIMER_EVENT_OVERFLOW:
            timer_callbacks.overflow_handler = NULL;
            timer_callbacks.overflow_user_data = NULL;
            *timer1.timsk &= ~(1 << TOIE1);  // Disable overflow interrupt
            break;
            
        case EER_TIMER_EVENT_COMPARE:
            if (channel == 0) {
                timer_callbacks.compare_a_handler = NULL;
                timer_callbacks.compare_a_user_data = NULL;
                *timer1.timsk &= ~(1 << OCIE1A);  // Disable compare A interrupt
            } else if (channel == 1) {
                timer_callbacks.compare_b_handler = NULL;
                timer_callbacks.compare_b_user_data = NULL;
                *timer1.timsk &= ~(1 << OCIE1B);  // Disable compare B interrupt
            } else {
                return EER_HAL_INVALID_PARAM;
            }
            break;
            
        case EER_TIMER_EVENT_CAPTURE:
            timer_callbacks.capture_handler = NULL;
            timer_callbacks.capture_user_data = NULL;
            *timer1.timsk &= ~(1 << ICIE1);  // Disable input capture interrupt
            break;
            
        default:
            return EER_HAL_INVALID_PARAM;
    }
    
    return EER_HAL_OK;
}

// Timer1 Overflow ISR
ISR(TIMER1_OVF_vect) {
    if (timer_callbacks.overflow_handler != NULL) {
        eer_timer_event_info_t event = {
            .timer = &timer1,
            .event = EER_TIMER_EVENT_OVERFLOW,
            .value = 0,  // Overflow means the counter wrapped to 0
            .user_data = timer_callbacks.overflow_user_data
        };
        
        timer_callbacks.overflow_handler(&event);
    }
    
    // If in one-shot mode, stop the timer
    if (current_config.mode == EER_TIMER_MODE_ONE_SHOT) {
        *timer1.tccrb &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
    }
}

// Timer1 Compare A ISR
ISR(TIMER1_COMPA_vect) {
    if (timer_callbacks.compare_a_handler != NULL) {
        eer_timer_event_info_t event = {
            .timer = &timer1,
            .event = EER_TIMER_EVENT_COMPARE,
            .value = *timer1.ocra,
            .user_data = timer_callbacks.compare_a_user_data
        };
        
        timer_callbacks.compare_a_handler(&event);
    }
    
    // If in one-shot mode, stop the timer
    if (current_config.mode == EER_TIMER_MODE_ONE_SHOT) {
        *timer1.tccrb &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
    }
}

// Timer1 Compare B ISR
ISR(TIMER1_COMPB_vect) {
    if (timer_callbacks.compare_b_handler != NULL) {
        eer_timer_event_info_t event = {
            .timer = &timer1,
            .event = EER_TIMER_EVENT_COMPARE,
            .value = *timer1.ocrb,
            .user_data = timer_callbacks.compare_b_user_data
        };
        
        timer_callbacks.compare_b_handler(&event);
    }
}

// Timer1 Capture ISR
ISR(TIMER1_CAPT_vect) {
    if (timer_callbacks.capture_handler != NULL) {
        eer_timer_event_info_t event = {
            .timer = &timer1,
            .event = EER_TIMER_EVENT_CAPTURE,
            .value = *timer1.icr,
            .user_data = timer_callbacks.capture_user_data
        };
        
        timer_callbacks.capture_handler(&event);
    }
}

// Timer handler structure with function pointers
eer_timer_handler_t eer_avr_timer = {
    .init = avr_timer_init,
    .deinit = avr_timer_deinit,
    .start = avr_timer_start,
    .stop = avr_timer_stop,
    .set_period = avr_timer_set_period,
    .get_value = avr_timer_get_value,
    .set_compare = avr_timer_set_compare,
    .set_pwm_duty_cycle = avr_timer_set_pwm_duty_cycle,
    .us_to_ticks = avr_timer_us_to_ticks,
    .ticks_to_us = avr_timer_ticks_to_us,
    .register_callback = avr_timer_register_callback,
    .unregister_callback = avr_timer_unregister_callback
};
