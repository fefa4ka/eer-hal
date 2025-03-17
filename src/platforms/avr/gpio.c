#include "platforms/avr/gpio.h"

static eer_hal_status_t avr_gpio_init(void) {
  // AVR doesn't need special initialization
  return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_deinit(void) {
  // Nothing to deinitialize
  return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_configure(void *pin, eer_gpio_config_t* config) {
  // Nothing to deinitialize
  return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_write(void *pin, bool state) {
  // Nothing to deinitialize
  return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_read(void *pin, bool *state) {
  // Nothing to deinitialize
  return EER_HAL_OK;
}

static eer_hal_status_t avr_gpio_toggle(void *pin) {
  // Nothing to deinitialize
  return EER_HAL_OK;
}

eer_gpio_handler_t eer_avr_gpio = {
    .init = avr_gpio_init,
    .deinit = avr_gpio_deinit,
    .configure = avr_gpio_configure,
    .write = avr_gpio_write,
    .read = avr_gpio_read,
    .toggle = avr_gpio_toggle,
    .register_irq = NULL,   // Not implemented in this example
    .unregister_irq = NULL, // Not implemented in this example
    .enable_irq = NULL,     // Not implemented in this example
    .disable_irq = NULL     // Not implemented in this example
};
