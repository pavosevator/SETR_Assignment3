/**
 * @file gpio.h
 * @brief GPIO interface for button and LED handling using Zephyr RTOS
 *
 * This header defines initialization and control functions for buttons and LEDs.
 * It uses GPIOs defined in the device tree via DT aliases (`sw0` to `sw3`, `led0` to `led3`).
 */
 
#ifndef GPIO_H
#define GPIO_H

#include <zephyr/kernel.h>          /* for k_msleep() */
#include <zephyr/device.h>          /* for device_is_ready() and device structure */
#include <zephyr/devicetree.h>		/* for DT_NODELABEL() */
#include <zephyr/drivers/gpio.h>    /* for GPIO api*/
#include <zephyr/sys/printk.h>      /* for printk()*/

/**
 * @defgroup gpio_aliases GPIO DeviceTree Aliases
 * @brief Mappings of buttons and LEDs to DeviceTree aliases
 * @{
 */
#define BUTTON1_NODE DT_ALIAS(sw0)
#define BUTTON2_NODE DT_ALIAS(sw1)
#define BUTTON3_NODE DT_ALIAS(sw2)
#define BUTTON4_NODE DT_ALIAS(sw3)

#define LED1_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led1)
#define LED3_NODE DT_ALIAS(led2)
#define LED4_NODE DT_ALIAS(led3)
/**@}*/

/**
 * @brief Initialize all GPIOs (buttons and LEDs)
 *
 * Configures GPIO pins for input (buttons) and output (LEDs) based on DeviceTree aliases.
 *
 * @return 0 on success, or a negative error code on failure
 */
int gpio_init(void);

/**
 * @brief Task handling button press logic
 *
 * Should be called regularly (or run as a thread) to monitor button states.
 */
void button_task(void);

/**
 * @brief Task to manage LED behavior
 *
 * Handles blinking or pattern updates of LEDs depending on the system state.
 */
void led_task(void);

/**
 * @brief Toggle LED 2 state
 *
 * @param state Boolean state: true to turn ON, false to turn OFF
 */
void led2_toggle(bool state);

/**
 * @brief Toggle LED 3 state
 *
 * @param state Boolean state: true to turn ON, false to turn OFF
 */
void led3_toggle(bool state);

/**
 * @brief Toggle LED 4 state
 *
 * @param state Boolean state: true to turn ON, false to turn OFF
 */
void led4_toggle(bool state);

#endif // GPIO_H
