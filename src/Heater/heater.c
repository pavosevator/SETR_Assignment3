#include "heater.h"

#define HEATER_NODE DT_NODELABEL(heater)

static const struct gpio_dt_spec heater = GPIO_DT_SPEC_GET(HEATER_NODE, gpios);

int heater_init(void)
{
    if (!device_is_ready(heater.port)) {
        return -1;
    }
    gpio_pin_configure_dt(&heater, GPIO_OUTPUT_ACTIVE);
    gpio_pin_set_dt(&heater, 0);
    return 0;
}

void heater_on(void)
{
    gpio_pin_set_dt(&heater, 1);
}

void heater_off(void)
{
    gpio_pin_set_dt(&heater, 0);
}

void heater_toggle(void)
{
    gpio_pin_toggle_dt(&heater);
}
