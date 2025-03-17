#include "platforms/avr/spi.h"
#include "macros.h"
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Default SPI instance for AVR
static eer_spi_t spi0 = eer_hal_spi0();

// Callback handler and user data
static struct {
    eer_spi_transfer_handler_t handler;
    void* user_data;
} spi_callback = {0};

// Current SPI configuration
static eer_spi_config_t current_config = {0};

static eer_hal_status_t avr_spi_init(eer_spi_config_t* config) {
    if (config == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Store the configuration
    current_config = *config;
    
    // Configure SPI pins
    if (config->master) {
        // In master mode, set MOSI, SCK, and SS as outputs
        bit_set(*(spi0.pins.ddr), spi0.pins.mosi);
        bit_set(*(spi0.pins.ddr), spi0.pins.sck);
        bit_set(*(spi0.pins.ddr), spi0.pins.ss);
        
        // Set SS high (inactive)
        bit_set(*(spi0.pins.port), spi0.pins.ss);
        
        // Make sure MISO is input
        bit_clear(*(spi0.pins.ddr), spi0.pins.miso);
    } else {
        // In slave mode, set MISO as output
        bit_set(*(spi0.pins.ddr), spi0.pins.miso);
        
        // Make sure MOSI, SCK, and SS are inputs
        bit_clear(*(spi0.pins.ddr), spi0.pins.mosi);
        bit_clear(*(spi0.pins.ddr), spi0.pins.sck);
        bit_clear(*(spi0.pins.ddr), spi0.pins.ss);
    }
    
    // Configure SPI control register
    uint8_t spcr_value = (1 << SPE); // SPI Enable
    
    // Set master/slave mode
    if (config->master) {
        spcr_value |= (1 << MSTR);
    }
    
    // Set data order
    if (config->bit_order == EER_SPI_BIT_ORDER_LSB) {
        spcr_value |= (1 << DORD);
    }
    
    // Set clock polarity and phase based on mode
    switch (config->mode) {
        case EER_SPI_MODE_0:
            // CPOL=0, CPHA=0 (default)
            break;
        case EER_SPI_MODE_1:
            // CPOL=0, CPHA=1
            spcr_value |= (1 << CPHA);
            break;
        case EER_SPI_MODE_2:
            // CPOL=1, CPHA=0
            spcr_value |= (1 << CPOL);
            break;
        case EER_SPI_MODE_3:
            // CPOL=1, CPHA=1
            spcr_value |= (1 << CPOL) | (1 << CPHA);
            break;
    }
    
    // Set clock rate
    uint8_t spsr_value = 0;
    
    switch (config->prescaler) {
        case EER_SPI_PRESCALER_2:
            spsr_value |= (1 << SPI2X);
            break;
        case EER_SPI_PRESCALER_4:
            // SPR1=0, SPR0=0, SPI2X=0 (default)
            break;
        case EER_SPI_PRESCALER_8:
            spsr_value |= (1 << SPI2X);
            spcr_value |= (1 << SPR0);
            break;
        case EER_SPI_PRESCALER_16:
            spcr_value |= (1 << SPR0);
            break;
        case EER_SPI_PRESCALER_32:
            spsr_value |= (1 << SPI2X);
            spcr_value |= (1 << SPR1);
            break;
        case EER_SPI_PRESCALER_64:
            spcr_value |= (1 << SPR1);
            break;
        case EER_SPI_PRESCALER_128:
        default:
            spcr_value |= (1 << SPR1) | (1 << SPR0);
            break;
    }
    
    // Apply settings
    *spi0.spcr = spcr_value;
    *spi0.spsr = spsr_value;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_spi_deinit(void) {
    // Disable SPI
    *spi0.spcr = 0;
    
    // Clear callback handler
    spi_callback.handler = NULL;
    spi_callback.user_data = NULL;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_spi_transfer(const uint8_t* tx_data, uint8_t* rx_data, uint16_t size, uint32_t timeout) {
    if (size == 0 || (tx_data == NULL && rx_data == NULL)) {
        return EER_HAL_INVALID_PARAM;
    }
    
    uint32_t start_time = 0; // In a real implementation, get current time
    
    for (uint16_t i = 0; i < size; i++) {
        // Load data into the SPI data register
        if (tx_data != NULL) {
            *spi0.spdr = tx_data[i];
        } else {
            // If no TX data, send dummy byte
            *spi0.spdr = 0xFF;
        }
        
        // Wait for transmission to complete
        while (!(*spi0.spsr & (1 << SPIF))) {
            if (timeout > 0) {
                uint32_t current_time = 0; // In a real implementation, get current time
                if ((current_time - start_time) >= timeout) {
                    return EER_HAL_TIMEOUT;
                }
            }
        }
        
        // Read received data
        if (rx_data != NULL) {
            rx_data[i] = *spi0.spdr;
        } else {
            // If no RX buffer, read and discard
            (void)*spi0.spdr;
        }
    }
    
    // Call the callback if registered
    if (spi_callback.handler != NULL) {
        eer_spi_transfer_event_t event = {
            .spi = &spi0,
            .tx_data = (uint8_t*)tx_data,
            .rx_data = rx_data,
            .size = size,
            .user_data = spi_callback.user_data
        };
        
        spi_callback.handler(&event);
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_spi_transmit(const uint8_t* data, uint16_t size, uint32_t timeout) {
    return avr_spi_transfer(data, NULL, size, timeout);
}

static eer_hal_status_t avr_spi_receive(uint8_t* data, uint16_t size, uint32_t timeout) {
    return avr_spi_transfer(NULL, data, size, timeout);
}

static eer_hal_status_t avr_spi_is_ready(bool* ready) {
    if (ready == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // SPI is ready when SPIF flag is set
    *ready = (*spi0.spsr & (1 << SPIF)) != 0;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_spi_chip_select(void* pin, bool state) {
    if (pin == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Cast to pin structure
    eer_pin_t* gpio_pin = (eer_pin_t*)pin;
    
    // Set pin state (active low)
    if (state) {
        // Select (low)
        bit_clear(*(gpio_pin->port.port), gpio_pin->number);
    } else {
        // Deselect (high)
        bit_set(*(gpio_pin->port.port), gpio_pin->number);
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_spi_register_callback(eer_spi_transfer_handler_t handler, void* user_data) {
    if (handler == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Store the handler and user data
    spi_callback.handler = handler;
    spi_callback.user_data = user_data;
    
    // Enable SPI interrupt
    *spi0.spcr |= (1 << SPIE);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_spi_unregister_callback(void) {
    // Clear the handler and user data
    spi_callback.handler = NULL;
    spi_callback.user_data = NULL;
    
    // Disable SPI interrupt
    *spi0.spcr &= ~(1 << SPIE);
    
    return EER_HAL_OK;
}

// SPI Transfer Complete ISR
ISR(SPI_STC_vect) {
    // This ISR is called when a SPI transfer is complete
    // In interrupt-driven mode, we would handle the transfer here
    // For now, we just call the callback if registered
    
    if (spi_callback.handler != NULL) {
        // Note: In a real implementation, we would need to track the transfer
        // state and provide the actual data buffers to the callback
        eer_spi_transfer_event_t event = {
            .spi = &spi0,
            .tx_data = NULL,
            .rx_data = NULL,
            .size = 0,
            .user_data = spi_callback.user_data
        };
        
        spi_callback.handler(&event);
    }
}

// SPI handler structure with function pointers
eer_spi_handler_t eer_avr_spi = {
    .init = avr_spi_init,
    .deinit = avr_spi_deinit,
    .transfer = avr_spi_transfer,
    .transmit = avr_spi_transmit,
    .receive = avr_spi_receive,
    .is_ready = avr_spi_is_ready,
    .chip_select = avr_spi_chip_select,
    .register_callback = avr_spi_register_callback,
    .unregister_callback = avr_spi_unregister_callback
};
