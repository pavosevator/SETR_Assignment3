#include "heater.h"
#include <zephyr/drivers/pwm.h>
#include <zephyr/device.h>

#define HEATER_NODE DT_NODELABEL(heater_pwm)

static const struct pwm_dt_spec heater = PWM_DT_SPEC_GET(HEATER_NODE);

int heater_init(void)
{
    if (!device_is_ready(heater.dev)) {
        return -1;
    }
    pwm_set_pulse_dt(&heater, 0);
    return 0;
}

void heater_set(uint8_t duty)
{
    if (!device_is_ready(heater.dev)) {
        return;
    }
    uint32_t pulse = (heater.period * duty) / 100U;
    pwm_set_pulse_dt(&heater, pulse);
}