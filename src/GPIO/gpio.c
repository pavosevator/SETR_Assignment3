/**
 * @file gpio.c
 * @brief GPIO interface for button and LED handling using Zephyr RTOS
 *
 * This header defines initialization and control functions for buttons and LEDs.
 * It uses GPIOs defined in the device tree via DT aliases (`sw0` to `sw3`, `led0` to `led3`).
 */
 
#include "gpio.h"
#include "data.h"
#include <zephyr/drivers/gpio.h>    /* for GPIO api*/

/* Define GPIO specifications for buttons */
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(BUTTON1_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(BUTTON2_NODE, gpios);
static const struct gpio_dt_spec button3 = GPIO_DT_SPEC_GET(BUTTON3_NODE, gpios);
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET(BUTTON4_NODE, gpios);

/* Define GPIO specifications for LEDs */
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED4_NODE, gpios);

/* Initialize GPIO module */
int gpio_init(void) {
    if (!device_is_ready(button1.port) || !device_is_ready(button2.port) ||
        !device_is_ready(button3.port) || !device_is_ready(button4.port) ||
        !device_is_ready(led1.port) || !device_is_ready(led2.port) ||
        !device_is_ready(led3.port) || !device_is_ready(led4.port)) {
        printk("Error: GPIO devices are not ready\n");
        return -1;
    }

    /* Configure buttons as inputs */
    gpio_pin_configure_dt(&button1, GPIO_INPUT);
    gpio_pin_configure_dt(&button2, GPIO_INPUT);
    gpio_pin_configure_dt(&button3, GPIO_INPUT);
    gpio_pin_configure_dt(&button4, GPIO_INPUT);

    /* Configure LEDs as outputs */
    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led4, GPIO_OUTPUT_INACTIVE);

    return 0;
}

/* Task to handle button presses */
void ui_task(void) {
    static bool prev_system = false;
    static bool prev_increase = false;
    static bool prev_toggle = false;
    static bool prev_decrease = false;

    bool btn_system   = gpio_pin_get_dt(&button1);
    bool btn_increase = gpio_pin_get_dt(&button2);
    bool pressed3     = gpio_pin_get_dt(&button3);
    bool btn_decrease = gpio_pin_get_dt(&button4);

    bool sys_state;
    
    k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
    sys_state = ctrl_state.system_on;
    if (btn_system && !prev_system) {
        ctrl_state.system_on = !ctrl_state.system_on;
        sys_state = ctrl_state.system_on;
    }
    k_mutex_unlock(&ctrl_state.mutex);

    if (btn_system && !prev_system) {
        gpio_pin_set_dt(&led1, sys_state);
        printk("System %s\n", sys_state ? "ON" : "OFF");
    }

    if (btn_increase && sys_state) {
        k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
        ctrl_state.max_temp++;
        printk("Temperature goal increased to: %d\n", ctrl_state.max_temp);
        k_mutex_unlock(&ctrl_state.mutex);
    }

    if (pressed3 && sys_state) {
        heater_toggle();
        printk("Heater toggled\n");
    }

    if (btn_decrease && sys_state) {
        k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
        ctrl_state.max_temp--;
        printk("Temperature goal decreased to: %d\n", ctrl_state.max_temp);
        k_mutex_unlock(&ctrl_state.mutex);
    }
    else if(!sys_state){
        heater_off();
    }

    prev_system = btn_system;
    prev_increase = btn_increase;
    prev_toggle = pressed3;
    prev_decrease = btn_decrease;
}

void led2_toggle(bool state){
    gpio_pin_set_dt(&led2, state); // when delta temperature is less than 2
}

void led3_toggle(bool state){
    gpio_pin_set_dt(&led3, state); // warning: low temp
}

void led4_toggle(bool state){
    gpio_pin_set_dt(&led4, state); // warning: high temp
}