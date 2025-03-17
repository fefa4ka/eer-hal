#include "platforms/avr/adc.h"
#include "macros.h"
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Array to store interrupt handlers and user data for each ADC channel
static struct {
    eer_adc_conversion_complete_handler_t handler;
    void* user_data;
} adc_irq_handlers[8] = {0}; // 8 ADC channels on most AVR MCUs

static eer_hal_status_t avr_adc_init(eer_adc_config_t* config) {
    if (config == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // AREF = AVcc by default
    ADMUX = (1 << REFS0);
    
    // Set prescaler based on config
    uint8_t prescaler_bits = 0;
    switch (config->prescaler) {
        case EER_ADC_PRESCALER_2:
            prescaler_bits = (0 << ADPS2) | (0 << ADPS1) | (1 << ADPS0);
            break;
        case EER_ADC_PRESCALER_4:
            prescaler_bits = (0 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);
            break;
        case EER_ADC_PRESCALER_8:
            prescaler_bits = (0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
            break;
        case EER_ADC_PRESCALER_16:
            prescaler_bits = (1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0);
            break;
        case EER_ADC_PRESCALER_32:
            prescaler_bits = (1 << ADPS2) | (0 << ADPS1) | (1 << ADPS0);
            break;
        case EER_ADC_PRESCALER_64:
            prescaler_bits = (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0);
            break;
        case EER_ADC_PRESCALER_128:
        default:
            prescaler_bits = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
            break;
    }
    
    // ADC Enable and set prescaler
    ADCSRA = (1 << ADEN) | prescaler_bits;
    
    // Set reference voltage
    switch (config->reference) {
        case EER_ADC_REF_INTERNAL:
            // Internal 1.1V reference
            ADMUX |= (1 << REFS1) | (1 << REFS0);
            break;
        case EER_ADC_REF_EXTERNAL:
            // External AREF pin
            ADMUX &= ~((1 << REFS1) | (1 << REFS0));
            break;
        case EER_ADC_REF_VCC:
        default:
            // AVcc with external capacitor at AREF pin
            ADMUX = (ADMUX & ~(1 << REFS1)) | (1 << REFS0);
            break;
    }
    
    // Enable ADC interrupt if continuous mode is requested
    if (config->mode == EER_ADC_MODE_CONTINUOUS) {
        ADCSRA |= (1 << ADIE);
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_adc_deinit(void) {
    // Disable ADC and ADC interrupt
    ADCSRA &= ~((1 << ADEN) | (1 << ADIE));
    
    // Clear all handlers
    for (int i = 0; i < 8; i++) {
        adc_irq_handlers[i].handler = NULL;
        adc_irq_handlers[i].user_data = NULL;
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_adc_start_conversion(void* channel) {
    if (channel == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    eer_adc_channel_t* adc_channel = (eer_adc_channel_t*)channel;
    
    // Select the ADC channel
    uint8_t ch = adc_channel->channel & 0x07; // Ensure channel is 0-7
    ADMUX = (ADMUX & 0xF8) | ch;
    
    // Start conversion
    ADCSRA |= (1 << ADSC);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_adc_stop_conversion(void) {
    // Stop any ongoing conversion
    ADCSRA &= ~(1 << ADSC);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_adc_is_conversion_complete(void* channel, bool* complete) {
    if (channel == NULL || complete == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Check if conversion is complete (ADSC bit is cleared when done)
    *complete = (ADCSRA & (1 << ADSC)) == 0;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_adc_read(void* channel, uint16_t* value) {
    if (channel == NULL || value == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    eer_adc_channel_t* adc_channel = (eer_adc_channel_t*)channel;
    bool conversion_complete = false;
    
    // Select the ADC channel
    uint8_t ch = adc_channel->channel & 0x07; // Ensure channel is 0-7
    ADMUX = (ADMUX & 0xF8) | ch;
    
    // Start conversion if not already started
    if ((ADCSRA & (1 << ADSC)) == 0) {
        ADCSRA |= (1 << ADSC);
    }
    
    // Wait for conversion to complete
    while ((ADCSRA & (1 << ADSC)) != 0);
    
    // Read the ADC value
    *value = ADC;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_adc_read_voltage(void* channel, float* voltage) {
    if (channel == NULL || voltage == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    uint16_t raw_value;
    eer_hal_status_t status = avr_adc_read(channel, &raw_value);
    
    if (status != EER_HAL_OK) {
        return status;
    }
    
    // Calculate voltage based on reference
    // This is a simplified calculation assuming VCC = 5.0V
    // For more accurate results, the actual reference voltage should be measured
    float reference_voltage = 5.0f;
    
    // Check if internal reference is used
    if ((ADMUX & ((1 << REFS1) | (1 << REFS0))) == ((1 << REFS1) | (1 << REFS0))) {
        reference_voltage = 1.1f; // Internal 1.1V reference
    }
    
    *voltage = (raw_value * reference_voltage) / 1023.0f;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_adc_register_callback(void* channel, 
                                                 eer_adc_conversion_complete_handler_t handler, 
                                                 void* user_data) {
    if (channel == NULL || handler == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    eer_adc_channel_t* adc_channel = (eer_adc_channel_t*)channel;
    uint8_t ch = adc_channel->channel & 0x07; // Ensure channel is 0-7
    
    // Store the handler and user data
    adc_irq_handlers[ch].handler = handler;
    adc_irq_handlers[ch].user_data = user_data;
    
    // Enable ADC interrupt
    ADCSRA |= (1 << ADIE);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_adc_unregister_callback(void* channel) {
    if (channel == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    eer_adc_channel_t* adc_channel = (eer_adc_channel_t*)channel;
    uint8_t ch = adc_channel->channel & 0x07; // Ensure channel is 0-7
    
    // Clear the handler and user data
    adc_irq_handlers[ch].handler = NULL;
    adc_irq_handlers[ch].user_data = NULL;
    
    // Check if any handlers are still registered
    bool any_handlers = false;
    for (int i = 0; i < 8; i++) {
        if (adc_irq_handlers[i].handler != NULL) {
            any_handlers = true;
            break;
        }
    }
    
    // Disable ADC interrupt if no handlers are registered
    if (!any_handlers) {
        ADCSRA &= ~(1 << ADIE);
    }
    
    return EER_HAL_OK;
}

// ADC Conversion Complete ISR
ISR(ADC_vect) {
    // Get the current channel from ADMUX
    uint8_t channel = ADMUX & 0x07;
    
    // Call the handler if registered
    if (adc_irq_handlers[channel].handler != NULL) {
        eer_adc_conversion_t conversion = {
            .channel = &(eer_adc_channel_t){channel},
            .value = ADC,
            .user_data = adc_irq_handlers[channel].user_data
        };
        
        adc_irq_handlers[channel].handler(&conversion);
    }
    
    // If in continuous mode, start the next conversion
    if (ADCSRA & (1 << ADIE)) {
        ADCSRA |= (1 << ADSC);
    }
}

// ADC handler structure with function pointers
eer_adc_handler_t eer_avr_adc = {
    .init = avr_adc_init,
    .deinit = avr_adc_deinit,
    .start_conversion = avr_adc_start_conversion,
    .stop_conversion = avr_adc_stop_conversion,
    .is_conversion_complete = avr_adc_is_conversion_complete,
    .read = avr_adc_read,
    .read_voltage = avr_adc_read_voltage,
    .register_callback = avr_adc_register_callback,
    .unregister_callback = avr_adc_unregister_callback
};
