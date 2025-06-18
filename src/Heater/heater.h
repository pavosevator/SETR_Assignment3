/**
 * @file heater.h
 * @brief Heater control interface for Zephyr-based embedded systems.
 *
 * This module provides simple functions to initialize and control a heating element
 * via GPIO using the Zephyr RTOS.
 */

 #ifndef HEATER_H
#define HEATER_H

#include <stdint.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

/**
 * @brief Initialize the heater GPIO pin.
 *
 * Configures the GPIO pin connected to the heater as an output and sets its initial state.
 *
 * @return 0 on success, negative error code on failure
 */
int heater_init(void);

/**
 * @brief Turn the heater ON.
 *
 * Sets the GPIO pin to the active state, enabling power to the heater.
 */
void heater_on(void);

/**
 * @brief Turn the heater OFF.
 *
 * Clears the GPIO pin to disable the heater.
 */
void heater_off(void);

/**
 * @brief Toggle the heater state.
 *
 * If the heater is ON, turns it OFF, and vice versa.
 */
void heater_toggle(void);

#endif /* HEATER_H */