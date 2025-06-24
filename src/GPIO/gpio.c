/**
 * @file gpio.c
 * @brief Interrupt driven GPIO handling for buttons and LEDs.
 */

#include "gpio.h"
#include <zephyr/sys/util.h>

/* GPIO specifications for board buttons */
static struct btn_ctx {
    struct gpio_dt_spec spec;
    struct gpio_callback cb;
    int64_t last_ts;
    enum button_event evt;
} buttons[] = {
    { .spec = GPIO_DT_SPEC_GET(BUTTON1_NODE, gpios), .evt = BTN_SYSTEM },
    { .spec = GPIO_DT_SPEC_GET(BUTTON2_NODE, gpios), .evt = BTN_INCREASE },
    { .spec = GPIO_DT_SPEC_GET(BUTTON3_NODE, gpios), .evt = BTN_TOGGLE },
    { .spec = GPIO_DT_SPEC_GET(BUTTON4_NODE, gpios), .evt = BTN_DECREASE },
};

/* LED GPIO specifications */
static const struct gpio_dt_spec leds[] = {
    GPIO_DT_SPEC_GET(LED1_NODE, gpios),
    GPIO_DT_SPEC_GET(LED2_NODE, gpios),
    GPIO_DT_SPEC_GET(LED3_NODE, gpios),
    GPIO_DT_SPEC_GET(LED4_NODE, gpios),
};

/* Queue delivering button events to the UI task */
K_MSGQ_DEFINE(button_msgq, sizeof(enum button_event), 8, 4);

/* Common ISR for all buttons */
static void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    struct btn_ctx *ctx = CONTAINER_OF(cb, struct btn_ctx, cb);
    int64_t now = k_uptime_get();

    /* Debounce: ignore events within X ms */
    if ((now - ctx->last_ts) < 200) {
        return;
    }
    ctx->last_ts = now;

    /* Signal only on press */
    if (gpio_pin_get_dt(&ctx->spec)) {
        enum button_event evt = ctx->evt;
        k_msgq_put(&button_msgq, &evt, K_NO_WAIT);
    }
}

int gpio_init(void)
{
    /* Configure buttons */
    for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
        if (!device_is_ready(buttons[i].spec.port)) {
            printk("Button device not ready\n");
            return -ENODEV;
        }
        gpio_pin_configure_dt(&buttons[i].spec, GPIO_INPUT | GPIO_PULL_UP);
        gpio_pin_interrupt_configure_dt(&buttons[i].spec,
                                        GPIO_INT_EDGE_TO_ACTIVE);
        gpio_init_callback(&buttons[i].cb, button_isr,
                           BIT(buttons[i].spec.pin));
        gpio_add_callback(buttons[i].spec.port, &buttons[i].cb);
        buttons[i].last_ts = 0;
    }

    /* Configure LEDs */
    for (int i = 0; i < ARRAY_SIZE(leds); i++) {
        if (!device_is_ready(leds[i].port)) {
            printk("LED device not ready\n");
            return -ENODEV;
        }
        gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT_INACTIVE);
    }

    return 0;
}

/**
 * @brief UI thread
 * 
 * Handles user interface logic (e.g. display or input polling)
 */
void ui_thread_func(void *argA, void *argB, void *argC)
{   
    while (1) {
        enum button_event evt;
        k_msgq_get(&button_msgq, &evt, K_FOREVER);

        switch (evt) {
        case BTN_SYSTEM:
            k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
            ctrl_state.system_on = !ctrl_state.system_on;
            bool sys_state = ctrl_state.system_on;
            k_mutex_unlock(&ctrl_state.mutex);

            gpio_pin_set_dt(&leds[0], sys_state);
            printk("System %s\n", sys_state ? "ON" : "OFF");
            if (!sys_state) {
                heater_off();
            }
            break;

        case BTN_INCREASE:
            k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
            if (ctrl_state.system_on) {
                ctrl_state.max_temp++;
                printk("Temperature goal increased to: %d\n", ctrl_state.max_temp);
            }
            k_mutex_unlock(&ctrl_state.mutex);
            break;

        case BTN_DECREASE:
            k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
            if (ctrl_state.system_on) {
                ctrl_state.max_temp--;
                printk("Temperature goal decreased to: %d\n", ctrl_state.max_temp);
                printk("HYS to: %d\n", ctrl_state.hys_half_band);
            } else {
                heater_off();
            }
            k_mutex_unlock(&ctrl_state.mutex);
            break;
        }
    }
}

void led2_toggle(bool state)
{
    gpio_pin_set_dt(&leds[1], state);
}

void led3_toggle(bool state)
{
    gpio_pin_set_dt(&leds[2], state);
}

void led4_toggle(bool state)
{
    gpio_pin_set_dt(&leds[3], state);
}