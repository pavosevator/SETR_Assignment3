/*
 * heater.c - GPIO-based heater control interface for Nordic nRF52840
 *
 * This module provides initialization and on/off/toggle control of a heater
 * element wired to a GPIO pin, using Zephyr's device tree bindings.
 */

#include "heater.h"
#include <zephyr/drivers/gpio.h>

/*
 * Device Tree node label for the heater control GPIO
 * -------------------------------------------------------------
 * The DT_NODELABEL(heater) entry must be defined in your board's .dts file
 * to match the physical pin driving the heater relay or transistor.
 */
#define HEATER_NODE DT_NODELABEL(heater)

/*
 * GPIO specification for the heater control line
 * -------------------------------------------------------------
 * GPIO_DT_SPEC_GET macro reads the port and pin configuration from the
 * device tree, creating a struct gpio_dt_spec containing:
 *   - port: pointer to the GPIO device
 *   - pin: the GPIO pin number
 *   - dt_flags: pin configuration flags (e.g., active high/low)
 */
static const struct gpio_dt_spec heater = GPIO_DT_SPEC_GET(HEATER_NODE, gpios);

/**
 * @brief Initialize the heater GPIO
 *
 * Configures the heater GPIO pin as an output and drives it low (heater off)
 * after verifying the GPIO device is ready.
 *
 * @return 0 on success, -1 on failure (GPIO device not ready)
 */
int heater_init(void)
{
    /* Ensure the GPIO port for the heater is available */
    if (!device_is_ready(heater.port)) {
        return -1;
    }

    /* Configure the pin as output and set it to 'active' state flag (usually high)
     * Then immediately drive output low to ensure heater is off at startup */
    gpio_pin_configure_dt(&heater, GPIO_OUTPUT_ACTIVE);
    gpio_pin_set_dt(&heater, 0);

    return 0;
}

/**
 * @brief Turn the heater on
 *
 * Drives the heater GPIO pin to the active level (typically high),
 * energizing the heater element or relay.
 */
void heater_on(void)
{
    gpio_pin_set_dt(&heater, 1);
}

/**
 * @brief Turn the heater off
 *
 * Drives the heater GPIO pin low, de-energizing the heater element.
 */
void heater_off(void)
{
    gpio_pin_set_dt(&heater, 0);
}

/**
 * @brief Toggle the heater state
 *
 * Flips the current output level of the heater GPIO pin. If the heater
 * was on, it will turn off, and vice versa.
 */
void heater_toggle(void)
{
    gpio_pin_toggle_dt(&heater);
}
