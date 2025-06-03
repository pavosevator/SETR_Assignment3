#include "gpio.h"


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
void gpio_init(void) {
    if (!device_is_ready(button1.port) || !device_is_ready(button2.port) || !device_is_ready(button4.port) ||
        !device_is_ready(led1.port) || !device_is_ready(led2.port) || !device_is_ready(led3.port) || !device_is_ready(led4.port)) {
        printk("Error: GPIO devices are not ready\n");
        return;
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
}

/* Task to handle button presses */
void button_task(void) {
    while (1) {
        if (gpio_pin_get_dt(&button1)) {
            printk("Button 1 pressed: Toggle system on/off\n");
            // Add logic to toggle the system
        }
        if (gpio_pin_get_dt(&button2)) {
            printk("Button 2 pressed: Increase temperature\n");
            // Add logic to increase the temperature
        }
        if (gpio_pin_get_dt(&button4)) {
            printk("Button 4 pressed: Decrease temperature\n");
            // Add logic to decrease the temperature
        }
        k_sleep(K_MSEC(100)); // Sleep to debounce and reduce CPU usage
    }
}

/* Task to handle LED updates */
void led_task(void) {
    while (1) {
        // Example logic to update LEDs based on system state
        gpio_pin_set_dt(&led1, 1); // Turn on LED1
        k_sleep(K_MSEC(500));
        gpio_pin_set_dt(&led1, 0); // Turn off LED1
        k_sleep(K_MSEC(500));
    }
}
