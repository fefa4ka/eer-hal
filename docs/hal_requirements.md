# EER HAL - Required Abstractions

This document outlines the essential Hardware Abstraction Layer (HAL) interfaces that must be implemented for each supported platform in the EER framework.

## Core Design Principles

1. **Consistency**: All HAL interfaces follow the same patterns for initialization, configuration, and error handling.
2. **Portability**: Abstractions hide platform-specific details while exposing necessary functionality.
3. **Efficiency**: Implementations should add minimal overhead to hardware access.
4. **Extensibility**: Platform-specific extensions are allowed but not required for core functionality.

## Required HAL Interfaces

### 1. GPIO

The GPIO interface must provide:
- Pin configuration (input/output/alternate functions)
- Digital read/write operations
- Interrupt handling with callback registration
- Toggle functionality

### 2. UART/USART

The UART interface must provide:
- Configuration (baud rate, parity, stop bits)
- Transmit/receive operations (blocking and non-blocking)
- Interrupt-based communication with callbacks
- Error handling

### 3. SPI

The SPI interface must provide:
- Master/slave configuration
- Clock polarity and phase settings
- Data transfer operations
- Chip select management

### 4. I2C

The I2C interface must provide:
- Master/slave operations
- Address management
- Read/write transactions
- Clock stretching support

### 5. ADC

The ADC interface must provide:
- Channel configuration
- Single and continuous conversion modes
- Resolution settings
- Reference voltage selection

### 6. Timer/PWM

The Timer interface must provide:
- Basic timing operations
- PWM generation
- Input capture
- Period/duty cycle control

### 7. System Control

The System interface must provide:
- Basic initialization
- Reset functionality
- Interrupt enable/disable
- Delay functions

### 8. Power Management

The Power Management interface must provide:
- Sleep mode entry/exit
- Basic power state control
- Wakeup source configuration

### 9. Non-Volatile Storage

The Non-Volatile Storage interface must provide:
- Basic read/write/erase operations for EEPROM or Flash
- Wear-leveling considerations (where applicable)

## Optional but Recommended Interfaces

### 10. RTC (Real-Time Clock)

The RTC interface should provide:
- Date/time setting and getting
- Alarm functionality
- Timestamp operations

### 11. Watchdog Timer

The Watchdog interface should provide:
- Configuration
- Refresh/feed operations
- Timeout handling

### 12. External Interrupts

The External Interrupt interface should provide:
- Configuration beyond GPIO interrupts
- Priority settings
- Callback registration

## Implementation Requirements

1. All interfaces must use the standard `eer_hal_status_t` for error reporting
2. All interfaces must provide init/deinit functions
3. All interfaces must use the `eer_callback_t` structure for event handling
4. All interfaces must properly manage resources and clean up in deinit functions
5. All interfaces must be documented with clear examples

## Platform-Specific Considerations

- Platform-specific extensions are allowed but must be clearly documented
- Optional features should be gracefully degraded when not supported
- Performance-critical operations should be optimized for each platform

## Versioning and Compatibility

The HAL interface definitions are versioned. All implementations must specify which HAL version they support. Breaking changes to the HAL interface will result in a major version increment.
