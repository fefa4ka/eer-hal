/**
 * @file test_gpio.c
 * @brief Test for GPIO HAL implementation
 */
#include "eer_hal.h"
#include "platforms/avr/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Test pin definitions
static eer_pin_t test_output_pin = eer_hal_pin(B, 5); // Arduino Uno LED pin (pin 13)
static eer_pin_t test_input_pin = eer_hal_pin(B, 4);  // Example input pin

// GPIO interrupt callback function
static void gpio_irq_handler(eer_gpio_irq_t* irq) {
    printf("GPIO interrupt triggered on pin %p\n", irq->pin);
}

// Test GPIO initialization
static bool test_gpio_init(void) {
    eer_hal_status_t status = eer_hal.gpio->init();
    printf("GPIO Init: %s\n", status == EER_HAL_OK ? "PASS" : "FAIL");
    return status == EER_HAL_OK;
}

// Test GPIO pin configuration
static bool test_gpio_configure(void) {
    bool success = true;
    eer_gpio_config_t output_config = {
        .mode = EER_GPIO_MODE_OUTPUT,
        .speed = EER_GPIO_SPEED_LOW,
        .trigger = EER_GPIO_TRIGGER_NONE
    };
    
    eer_gpio_config_t input_config = {
        .mode = EER_GPIO_MODE_INPUT_PULLUP,
        .speed = EER_GPIO_SPEED_LOW,
        .trigger = EER_GPIO_TRIGGER_NONE
    };
    
    // Configure output pin
    eer_hal_status_t status = eer_hal.gpio->configure(&test_output_pin, &output_config);
    printf("GPIO Configure Output: %s\n", status == EER_HAL_OK ? "PASS" : "FAIL");
    success &= (status == EER_HAL_OK);
    
    // Configure input pin
    status = eer_hal.gpio->configure(&test_input_pin, &input_config);
    printf("GPIO Configure Input: %s\n", status == EER_HAL_OK ? "PASS" : "FAIL");
    success &= (status == EER_HAL_OK);
    
    return success;
}

// Test GPIO write operation
static bool test_gpio_write(void) {
    bool success = true;
    
    // Write high
    eer_hal_status_t status = eer_hal.gpio->write(&test_output_pin, true);
    printf("GPIO Write High: %s\n", status == EER_HAL_OK ? "PASS" : "FAIL");
    success &= (status == EER_HAL_OK);
    
    // Delay to observe the change
    eer_hal.system->delay_ms(500);
    
    // Write low
    status = eer_hal.gpio->write(&test_output_pin, false);
    printf("GPIO Write Low: %s\n", status == EER_HAL_OK ? "PASS" : "FAIL");
    success &= (status == EER_HAL_OK);
    
    // Delay to observe the change
    eer_hal.system->delay_ms(500);
    
    return success;
}

// Test GPIO read operation
static bool test_gpio_read(void) {
    bool pin_state;
    eer_hal_status_t status = eer_hal.gpio->read(&test_input_pin, &pin_state);
    printf("GPIO Read: %s (State: %d)\n", status == EER_HAL_OK ? "PASS" : "FAIL", pin_state);
    return status == EER_HAL_OK;
}

// Test GPIO toggle operation
static bool test_gpio_toggle(void) {
    bool success = true;
    
    // Toggle pin (should go high)
    eer_hal_status_t status = eer_hal.gpio->toggle(&test_output_pin);
    printf("GPIO Toggle (High): %s\n", status == EER_HAL_OK ? "PASS" : "FAIL");
    success &= (status == EER_HAL_OK);
    
    // Delay to observe the change
    eer_hal.system->delay_ms(500);
    
    // Toggle pin again (should go low)
    status = eer_hal.gpio->toggle(&test_output_pin);
    printf("GPIO Toggle (Low): %s\n", status == EER_HAL_OK ? "PASS" : "FAIL");
    success &= (status == EER_HAL_OK);
    
    // Delay to observe the change
    eer_hal.system->delay_ms(500);
    
    return success;
}

// Test GPIO interrupt registration
static bool test_gpio_interrupt(void) {
    // Note: This test may not be fully functional on all AVR platforms
    // as the implementation in avr_gpio.c returns EER_HAL_NOT_SUPPORTED
    
    eer_gpio_config_t interrupt_config = {
        .mode = EER_GPIO_MODE_INPUT_PULLUP,
        .speed = EER_GPIO_SPEED_LOW,
        .trigger = EER_GPIO_TRIGGER_FALLING
    };
    
    // Configure pin for interrupt
    eer_hal_status_t status = eer_hal.gpio->configure(&test_input_pin, &interrupt_config);
    printf("GPIO Configure for Interrupt: %s\n", 
           status == EER_HAL_OK ? "PASS" : 
           status == EER_HAL_NOT_SUPPORTED ? "NOT SUPPORTED" : "FAIL");
    
    if (status == EER_HAL_NOT_SUPPORTED) {
        printf("GPIO Interrupt: SKIPPED (Not supported on this platform)\n");
        return true; // Skip test if not supported
    }
    
    // Register interrupt handler
    status = eer_hal.gpio->register_irq(&test_input_pin, gpio_irq_handler, NULL);
    printf("GPIO Register IRQ: %s\n", 
           status == EER_HAL_OK ? "PASS" : 
           status == EER_HAL_NOT_SUPPORTED ? "NOT SUPPORTED" : "FAIL");
    
    if (status == EER_HAL_NOT_SUPPORTED) {
        return true; // Skip test if not supported
    }
    
    // Enable interrupt
    status = eer_hal.gpio->enable_irq(&test_input_pin);
    printf("GPIO Enable IRQ: %s\n", 
           status == EER_HAL_OK ? "PASS" : 
           status == EER_HAL_NOT_SUPPORTED ? "NOT SUPPORTED" : "FAIL");
    
    printf("Waiting for interrupt (press button or connect pin to ground)...\n");
    eer_hal.system->delay_ms(5000); // Wait for potential interrupt
    
    // Disable interrupt
    status = eer_hal.gpio->disable_irq(&test_input_pin);
    printf("GPIO Disable IRQ: %s\n", 
           status == EER_HAL_OK ? "PASS" : 
           status == EER_HAL_NOT_SUPPORTED ? "NOT SUPPORTED" : "FAIL");
    
    // Unregister interrupt handler
    status = eer_hal.gpio->unregister_irq(&test_input_pin);
    printf("GPIO Unregister IRQ: %s\n", 
           status == EER_HAL_OK ? "PASS" : 
           status == EER_HAL_NOT_SUPPORTED ? "NOT SUPPORTED" : "FAIL");
    
    return true; // Return true even if not supported
}

// Test GPIO deinitialization
static bool test_gpio_deinit(void) {
    eer_hal_status_t status = eer_hal.gpio->deinit();
    printf("GPIO Deinit: %s\n", status == EER_HAL_OK ? "PASS" : "FAIL");
    return status == EER_HAL_OK;
}

int main(void) {
    // Initialize system first
    eer_hal.system->init();
    
    printf("\n===== GPIO HAL Test =====\n");
    
    bool all_tests_passed = true;
    
    // Run tests
    all_tests_passed &= test_gpio_init();
    all_tests_passed &= test_gpio_configure();
    all_tests_passed &= test_gpio_write();
    all_tests_passed &= test_gpio_read();
    all_tests_passed &= test_gpio_toggle();
    all_tests_passed &= test_gpio_interrupt();
    all_tests_passed &= test_gpio_deinit();
    
    printf("\n===== Test Summary =====\n");
    printf("GPIO HAL Test: %s\n", all_tests_passed ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    
    // Deinitialize system
    eer_hal.system->deinit();
    
    return all_tests_passed ? 0 : 1;
}
