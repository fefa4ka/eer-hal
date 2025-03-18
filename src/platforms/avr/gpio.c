#include "platforms/avr/gpio.h"
#include "macros.h"
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Array to store interrupt handlers and user data for each pin
static struct {
    eer_gpio_irq_handler_t handler;
    void* user_data;
} gpio_irq_handlers[24] = {0}; // Assuming max 24 pins (3 ports * 8 pins)

static eer_hal_status_t avr_gpio_init(void) {
    // AVR doesn't need special initialization for GPIO
    return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_deinit(void) {
    // Nothing to deinitialize
    return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_configure(void *pin, eer_gpio_config_t* config) {
    if (pin == NULL || config == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    eer_pin_t *avr_pin = (eer_pin_t *)pin;
    
    switch (config->mode) {
        case EER_GPIO_MODE_INPUT:
            // Configure as input, no pull-up
            bit_clear(*(avr_pin->port.ddr), avr_pin->number);
            bit_clear(*(avr_pin->port.port), avr_pin->number);
            break;
            
        case EER_GPIO_MODE_INPUT_PULLUP:
            // Configure as input with pull-up
            bit_clear(*(avr_pin->port.ddr), avr_pin->number);
            bit_set(*(avr_pin->port.port), avr_pin->number);
            break;
            
        case EER_GPIO_MODE_OUTPUT:
            // Configure as output
            bit_set(*(avr_pin->port.ddr), avr_pin->number);
            break;
            
        default:
            // Other modes not supported on AVR
            return EER_HAL_NOT_SUPPORTED;
    }
    
    // Configure pin for interrupt if requested
    if (config->trigger != EER_GPIO_TRIGGER_NONE) {
        // Note: This is a simplified implementation that assumes
        // the pin is on PORTD and can use PCINT or INT interrupts
        // A complete implementation would need to handle all ports
        
        // For now, return not supported
        return EER_HAL_NOT_SUPPORTED;
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_write(void *pin, bool state) {
    if (pin == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    eer_pin_t *avr_pin = (eer_pin_t *)pin;
    
    if (state) {
        bit_set(*(avr_pin->port.port), avr_pin->number);
    } else {
        bit_clear(*(avr_pin->port.port), avr_pin->number);
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_read(void *pin, bool *state) {
    if (pin == NULL || state == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    eer_pin_t *avr_pin = (eer_pin_t *)pin;
    
    *state = bit_get(*(avr_pin->port.pin), avr_pin->number);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_toggle(void *pin) {
    if (pin == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    eer_pin_t *avr_pin = (eer_pin_t *)pin;
    
    bit_toggle(*(avr_pin->port.port), avr_pin->number);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_register_irq(void *pin, 
                                             eer_gpio_irq_handler_t handler, 
                                             void* user_data) {
    // This is a simplified implementation
    // A complete implementation would need to configure the appropriate
    // interrupt registers based on the pin
    
    // For now, return not supported
    return EER_HAL_NOT_SUPPORTED;
}

static eer_hal_status_t avr_gpio_unregister_irq(void *pin) {
    // For now, return not supported
    return EER_HAL_NOT_SUPPORTED;
}

static eer_hal_status_t avr_gpio_enable_irq(void *pin) {
    // For now, return not supported
    return EER_HAL_NOT_SUPPORTED;
}

static eer_hal_status_t avr_gpio_disable_irq(void *pin) {
    // For now, return not supported
    return EER_HAL_NOT_SUPPORTED;
}

// GPIO handler structure with function pointers
eer_gpio_handler_t eer_avr_gpio = {
    .init = avr_gpio_init,
    .deinit = avr_gpio_deinit,
    .configure = avr_gpio_configure,
    .write = avr_gpio_write,
    .read = avr_gpio_read,
    .toggle = avr_gpio_toggle,
    .register_irq = avr_gpio_register_irq,
    .unregister_irq = avr_gpio_unregister_irq,
    .enable_irq = avr_gpio_enable_irq,
    .disable_irq = avr_gpio_disable_irq
};
