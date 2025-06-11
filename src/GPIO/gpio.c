#include "gpio.h"
#include "data.h"
#include <zephyr/drivers/gpio.h>    /* for GPIO api*/

/* Define GPIO specifications for buttons */
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET(BUTTON1_NODE, gpios);
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET(BUTTON2_NODE, gpios);
static const struct gpio_dt_spec button4 = GPIO_DT_SPEC_GET(BUTTON4_NODE, gpios);

/* Define GPIO specifications for LEDs */
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED4_NODE, gpios);

/* Initialize GPIO module */
int gpio_init(void) {
    if (!device_is_ready(button1.port) || !device_is_ready(button2.port) || !device_is_ready(button4.port) ||
        !device_is_ready(led1.port) || !device_is_ready(led2.port) || !device_is_ready(led3.port) || !device_is_ready(led4.port)) {
        printk("Error: GPIO devices are not ready\n");
        return -1;
    }

    /* Configure buttons as inputs */
    gpio_pin_configure_dt(&button1, GPIO_INPUT);
    gpio_pin_configure_dt(&button2, GPIO_INPUT);
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
    bool prev = false;
    while (1) {
        bool pressed = gpio_pin_get_dt(&button1);

        if (pressed && !prev) {
            system_on = !system_on;
            gpio_pin_set_dt(&led1, system_on);
            printk("System %s\n", system_on ? "ON" : "OFF");
        
        k_sleep(K_MSEC(400)); // Sleep to debounce and reduce CPU usage
        }

        //gpio_pin_set_dt(&led2, !system_on);
    }
}