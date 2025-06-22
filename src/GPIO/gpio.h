/**
 * @file gpio.h
 * @brief GPIO interface for button and LED handling using Zephyr RTOS
 *
 * This header defines initialization and control functions for buttons and LEDs.
 * It uses GPIOs defined in the device tree via DT aliases (`sw0` to `sw3`, `led0` to `led3`).
 */
 
#ifndef GPIO_H
#define GPIO_H

#include "data.h"
#include "heater.h"
#include <zephyr/kernel.h>
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

/** Button events delivered by the interrupt callbacks. */
enum button_event {
    BTN_SYSTEM,
    BTN_INCREASE,
    BTN_TOGGLE,
    BTN_DECREASE,
};

/** Message queue publishing button events. */
extern struct k_msgq button_msgq;

/**
 * @brief Initialize all GPIOs and interrupts.
 *
 * Configures buttons with pull-ups and edge interrupts and initialises LED pins.
 */
int gpio_init(void);

/** UI thread that waits for button events and updates the system state. */
void ui_task(void);

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