#include "platforms/avr/i2c.h"
#include "macros.h"
#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

// Default I2C instance for AVR
static eer_i2c_t i2c0 = eer_hal_i2c0();

// Callback handler and user data
static struct {
    eer_i2c_transfer_handler_t handler;
    void* user_data;
} i2c_callback = {0};

// Current I2C configuration
static eer_i2c_config_t current_config = {0};

// I2C status codes
#define I2C_START_TRANSMITTED      0x08
#define I2C_RESTART_TRANSMITTED    0x10
#define I2C_SLA_W_ACK              0x18
#define I2C_SLA_W_NACK             0x20
#define I2C_DATA_TRANSMITTED_ACK   0x28
#define I2C_DATA_TRANSMITTED_NACK  0x30
#define I2C_ARBITRATION_LOST       0x38
#define I2C_SLA_R_ACK              0x40
#define I2C_SLA_R_NACK             0x48
#define I2C_DATA_RECEIVED_ACK      0x50
#define I2C_DATA_RECEIVED_NACK     0x58

/**
 * @brief Calculate TWBR value for the given SCL frequency
 * @param scl_freq SCL frequency in Hz
 * @param prescaler TWI prescaler value (0-3)
 * @return TWBR value
 */
static uint8_t i2c_calculate_twbr(uint32_t scl_freq, uint8_t prescaler) {
    uint8_t twps;
    uint32_t prescaler_value;
    
    switch (prescaler) {
        case 0: prescaler_value = 1; twps = 0; break;
        case 1: prescaler_value = 4; twps = 1; break;
        case 2: prescaler_value = 16; twps = 2; break;
        case 3: prescaler_value = 64; twps = 3; break;
        default: prescaler_value = 1; twps = 0; break;
    }
    
    uint8_t twbr = ((F_CPU / scl_freq) - 16) / (2 * prescaler_value);
    
    // Set prescaler bits in TWSR
    *i2c0.twsr = (*i2c0.twsr & 0xFC) | twps;
    
    return twbr;
}

/**
 * @brief Wait for I2C operation to complete with timeout
 * @param timeout Timeout in milliseconds
 * @return Status code indicating success or failure
 */
static eer_hal_status_t i2c_wait_for_completion(uint32_t timeout) {
    uint32_t start_time = 0; // In a real implementation, get current time
    
    // Wait for TWINT flag to be set
    while (!(*i2c0.twcr & (1 << TWINT))) {
        if (timeout > 0) {
            uint32_t current_time = 0; // In a real implementation, get current time
            if ((current_time - start_time) >= timeout) {
                return EER_HAL_TIMEOUT;
            }
        }
    }
    
    return EER_HAL_OK;
}

/**
 * @brief Send I2C start condition
 * @param timeout Timeout in milliseconds
 * @return Status code indicating success or failure
 */
static eer_hal_status_t i2c_start(uint32_t timeout) {
    // Send START condition
    *i2c0.twcr = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    // Wait for TWINT flag to be set
    eer_hal_status_t status = i2c_wait_for_completion(timeout);
    if (status != EER_HAL_OK) {
        return status;
    }
    
    // Check if START was sent successfully
    if ((*i2c0.twsr & 0xF8) != I2C_START_TRANSMITTED) {
        return EER_HAL_ERROR;
    }
    
    return EER_HAL_OK;
}

/**
 * @brief Send I2C repeated start condition
 * @param timeout Timeout in milliseconds
 * @return Status code indicating success or failure
 */
static eer_hal_status_t i2c_restart(uint32_t timeout) {
    // Send RESTART condition
    *i2c0.twcr = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    // Wait for TWINT flag to be set
    eer_hal_status_t status = i2c_wait_for_completion(timeout);
    if (status != EER_HAL_OK) {
        return status;
    }
    
    // Check if RESTART was sent successfully
    if ((*i2c0.twsr & 0xF8) != I2C_RESTART_TRANSMITTED) {
        return EER_HAL_ERROR;
    }
    
    return EER_HAL_OK;
}

/**
 * @brief Send I2C stop condition
 * @return Status code indicating success or failure
 */
static eer_hal_status_t i2c_stop(void) {
    // Send STOP condition
    *i2c0.twcr = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    
    // Wait for STOP to be executed (no TWINT flag is set after STOP)
    // A small delay might be needed here in some implementations
    
    return EER_HAL_OK;
}

/**
 * @brief Send I2C address
 * @param address Target device address
 * @param read Read (true) or write (false) operation
 * @param timeout Timeout in milliseconds
 * @return Status code indicating success or failure
 */
static eer_hal_status_t i2c_send_address(uint16_t address, bool read, uint32_t timeout) {
    // For now, only support 7-bit addressing
    if (current_config.addr_mode == EER_I2C_ADDR_10BIT) {
        return EER_HAL_NOT_SUPPORTED;
    }
    
    // Load address and R/W bit
    *i2c0.twdr = (address << 1) | (read ? 1 : 0);
    
    // Start transmission
    *i2c0.twcr = (1 << TWINT) | (1 << TWEN);
    
    // Wait for TWINT flag to be set
    eer_hal_status_t status = i2c_wait_for_completion(timeout);
    if (status != EER_HAL_OK) {
        return status;
    }
    
    // Check if SLA+W/R was sent successfully
    uint8_t twsr = *i2c0.twsr & 0xF8;
    if (read) {
        if (twsr != I2C_SLA_R_ACK) {
            return EER_HAL_ERROR;
        }
    } else {
        if (twsr != I2C_SLA_W_ACK) {
            return EER_HAL_ERROR;
        }
    }
    
    return EER_HAL_OK;
}

/**
 * @brief Send data byte over I2C
 * @param data Data byte to send
 * @param timeout Timeout in milliseconds
 * @return Status code indicating success or failure
 */
static eer_hal_status_t i2c_send_data(uint8_t data, uint32_t timeout) {
    // Load data
    *i2c0.twdr = data;
    
    // Start transmission
    *i2c0.twcr = (1 << TWINT) | (1 << TWEN);
    
    // Wait for TWINT flag to be set
    eer_hal_status_t status = i2c_wait_for_completion(timeout);
    if (status != EER_HAL_OK) {
        return status;
    }
    
    // Check if data was sent successfully
    if ((*i2c0.twsr & 0xF8) != I2C_DATA_TRANSMITTED_ACK) {
        return EER_HAL_ERROR;
    }
    
    return EER_HAL_OK;
}

/**
 * @brief Receive data byte over I2C
 * @param data Pointer to store received data
 * @param send_ack Send ACK (true) or NACK (false) after reception
 * @param timeout Timeout in milliseconds
 * @return Status code indicating success or failure
 */
static eer_hal_status_t i2c_receive_data(uint8_t* data, bool send_ack, uint32_t timeout) {
    if (data == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Start reception with or without ACK
    if (send_ack) {
        *i2c0.twcr = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    } else {
        *i2c0.twcr = (1 << TWINT) | (1 << TWEN);
    }
    
    // Wait for TWINT flag to be set
    eer_hal_status_t status = i2c_wait_for_completion(timeout);
    if (status != EER_HAL_OK) {
        return status;
    }
    
    // Check if data was received successfully
    uint8_t twsr = *i2c0.twsr & 0xF8;
    if (send_ack) {
        if (twsr != I2C_DATA_RECEIVED_ACK) {
            return EER_HAL_ERROR;
        }
    } else {
        if (twsr != I2C_DATA_RECEIVED_NACK) {
            return EER_HAL_ERROR;
        }
    }
    
    // Read received data
    *data = *i2c0.twdr;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_init(eer_i2c_config_t* config) {
    if (config == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Store the configuration
    current_config = *config;
    
    // Calculate bit rate based on desired speed
    uint32_t scl_freq;
    switch (config->speed) {
        case EER_I2C_SPEED_STANDARD:
            scl_freq = 100000; // 100 kHz
            break;
        case EER_I2C_SPEED_FAST:
            scl_freq = 400000; // 400 kHz
            break;
        case EER_I2C_SPEED_FAST_PLUS:
            scl_freq = 1000000; // 1 MHz
            break;
        default:
            scl_freq = 100000; // Default to 100 kHz
            break;
    }
    
    // Override with explicit clock frequency if provided
    if (config->clock_hz > 0) {
        scl_freq = config->clock_hz;
    }
    
    // Set bit rate (use prescaler 0 for now)
    *i2c0.twbr = i2c_calculate_twbr(scl_freq, 0);
    
    // Enable TWI
    *i2c0.twcr = (1 << TWEN);
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_deinit(void) {
    // Disable TWI
    *i2c0.twcr = 0;
    
    // Clear callback handler
    i2c_callback.handler = NULL;
    i2c_callback.user_data = NULL;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_master_transmit(uint16_t address, const uint8_t* data, uint16_t size, uint32_t timeout) {
    if (data == NULL || size == 0) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Send START condition
    eer_hal_status_t status = i2c_start(timeout);
    if (status != EER_HAL_OK) {
        i2c_stop();
        return status;
    }
    
    // Send slave address with write bit
    status = i2c_send_address(address, false, timeout);
    if (status != EER_HAL_OK) {
        i2c_stop();
        return status;
    }
    
    // Send data bytes
    for (uint16_t i = 0; i < size; i++) {
        status = i2c_send_data(data[i], timeout);
        if (status != EER_HAL_OK) {
            i2c_stop();
            return status;
        }
    }
    
    // Send STOP condition
    i2c_stop();
    
    // Call the callback if registered
    if (i2c_callback.handler != NULL) {
        eer_i2c_transfer_event_t event = {
            .i2c = &i2c0,
            .address = address,
            .tx_data = (uint8_t*)data,
            .rx_data = NULL,
            .size = size,
            .user_data = i2c_callback.user_data
        };
        
        i2c_callback.handler(&event);
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_master_receive(uint16_t address, uint8_t* data, uint16_t size, uint32_t timeout) {
    if (data == NULL || size == 0) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Send START condition
    eer_hal_status_t status = i2c_start(timeout);
    if (status != EER_HAL_OK) {
        i2c_stop();
        return status;
    }
    
    // Send slave address with read bit
    status = i2c_send_address(address, true, timeout);
    if (status != EER_HAL_OK) {
        i2c_stop();
        return status;
    }
    
    // Receive data bytes
    for (uint16_t i = 0; i < size; i++) {
        // Send ACK for all bytes except the last one
        bool send_ack = (i < size - 1);
        
        status = i2c_receive_data(&data[i], send_ack, timeout);
        if (status != EER_HAL_OK) {
            i2c_stop();
            return status;
        }
    }
    
    // Send STOP condition
    i2c_stop();
    
    // Call the callback if registered
    if (i2c_callback.handler != NULL) {
        eer_i2c_transfer_event_t event = {
            .i2c = &i2c0,
            .address = address,
            .tx_data = NULL,
            .rx_data = data,
            .size = size,
            .user_data = i2c_callback.user_data
        };
        
        i2c_callback.handler(&event);
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_master_transmit_receive(uint16_t address, 
                                                      const uint8_t* tx_data, uint16_t tx_size,
                                                      uint8_t* rx_data, uint16_t rx_size,
                                                      uint32_t timeout) {
    if ((tx_data == NULL || tx_size == 0) || (rx_data == NULL || rx_size == 0)) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Send START condition
    eer_hal_status_t status = i2c_start(timeout);
    if (status != EER_HAL_OK) {
        i2c_stop();
        return status;
    }
    
    // Send slave address with write bit
    status = i2c_send_address(address, false, timeout);
    if (status != EER_HAL_OK) {
        i2c_stop();
        return status;
    }
    
    // Send data bytes
    for (uint16_t i = 0; i < tx_size; i++) {
        status = i2c_send_data(tx_data[i], timeout);
        if (status != EER_HAL_OK) {
            i2c_stop();
            return status;
        }
    }
    
    // Send RESTART condition
    status = i2c_restart(timeout);
    if (status != EER_HAL_OK) {
        i2c_stop();
        return status;
    }
    
    // Send slave address with read bit
    status = i2c_send_address(address, true, timeout);
    if (status != EER_HAL_OK) {
        i2c_stop();
        return status;
    }
    
    // Receive data bytes
    for (uint16_t i = 0; i < rx_size; i++) {
        // Send ACK for all bytes except the last one
        bool send_ack = (i < rx_size - 1);
        
        status = i2c_receive_data(&rx_data[i], send_ack, timeout);
        if (status != EER_HAL_OK) {
            i2c_stop();
            return status;
        }
    }
    
    // Send STOP condition
    i2c_stop();
    
    // Call the callback if registered
    if (i2c_callback.handler != NULL) {
        eer_i2c_transfer_event_t event = {
            .i2c = &i2c0,
            .address = address,
            .tx_data = (uint8_t*)tx_data,
            .rx_data = rx_data,
            .size = tx_size + rx_size, // Combined size
            .user_data = i2c_callback.user_data
        };
        
        i2c_callback.handler(&event);
    }
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_is_busy(bool* busy) {
    if (busy == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Check if TWINT flag is clear (operation in progress)
    *busy = (*i2c0.twcr & (1 << TWINT)) == 0;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_scan(uint16_t* devices, uint8_t max_devices, uint8_t* found_devices) {
    if (devices == NULL || found_devices == NULL || max_devices == 0) {
        return EER_HAL_INVALID_PARAM;
    }
    
    uint8_t count = 0;
    
    // Scan all possible 7-bit addresses (0-127)
    for (uint16_t addr = 0; addr < 128 && count < max_devices; addr++) {
        // Skip reserved addresses
        if (addr < 8 || (addr >= 0x78 && addr <= 0x7F)) {
            continue;
        }
        
        // Try to address the device
        eer_hal_status_t status = i2c_start(10); // 10ms timeout
        if (status != EER_HAL_OK) {
            i2c_stop();
            continue;
        }
        
        // Send address with write bit
        *i2c0.twdr = (addr << 1) | 0; // Write operation
        *i2c0.twcr = (1 << TWINT) | (1 << TWEN);
        
        // Wait for TWINT flag to be set
        status = i2c_wait_for_completion(10); // 10ms timeout
        if (status != EER_HAL_OK) {
            i2c_stop();
            continue;
        }
        
        // Check if device responded with ACK
        if ((*i2c0.twsr & 0xF8) == I2C_SLA_W_ACK) {
            devices[count++] = addr;
        }
        
        // Send STOP condition
        i2c_stop();
    }
    
    *found_devices = count;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_register_callback(eer_i2c_transfer_handler_t handler, void* user_data) {
    if (handler == NULL) {
        return EER_HAL_INVALID_PARAM;
    }
    
    // Store the handler and user data
    i2c_callback.handler = handler;
    i2c_callback.user_data = user_data;
    
    return EER_HAL_OK;
}

static eer_hal_status_t avr_i2c_unregister_callback(void) {
    // Clear the handler and user data
    i2c_callback.handler = NULL;
    i2c_callback.user_data = NULL;
    
    return EER_HAL_OK;
}

// I2C handler structure with function pointers
eer_i2c_handler_t eer_avr_i2c = {
    .init = avr_i2c_init,
    .deinit = avr_i2c_deinit,
    .master_transmit = avr_i2c_master_transmit,
    .master_receive = avr_i2c_master_receive,
    .master_transmit_receive = avr_i2c_master_transmit_receive,
    .is_busy = avr_i2c_is_busy,
    .scan = avr_i2c_scan,
    .register_callback = avr_i2c_register_callback,
    .unregister_callback = avr_i2c_unregister_callback
};
