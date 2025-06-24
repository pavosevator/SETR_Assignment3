/**
 * @file control.c
 * @brief Implementation of the temperature control loops.
 */
#include "control.h"

#include "heater.h"
#include "gpio.h"
#include "data.h"

static struct k_sem control_sem;
static struct k_sem actuator_sem;
static struct k_timer control_timer;
static struct k_timer actuator_timer;

/* Timer callback giving the control semaphore. */
static void control_timer_handler(struct k_timer *t)
{
    k_sem_give(&control_sem);
}

/* Timer callback giving the actuator semaphore. */
static void actuator_timer_handler(struct k_timer *t)
{
    k_sem_give(&actuator_sem);
}

/**
 * @brief Initialise control module.
 *
 * Sets up semaphores and periodic timers driving the control and
 * actuator threads.
 */
int control_init(void)
{
    k_sem_init(&control_sem, 0, 1);
    k_sem_init(&actuator_sem, 0, 1);

    k_timer_init(&control_timer, control_timer_handler, NULL);
    k_timer_start(&control_timer, K_NO_WAIT, K_MSEC(200));

    k_timer_init(&actuator_timer, actuator_timer_handler, NULL);
    k_timer_start(&actuator_timer, K_NO_WAIT, K_MSEC(200));

    return 0;
}

/**
 * @brief Thread implementing the control algorithm.
 */
void control_thread_func(void *argA, void *argB, void *argC)
{
    int delta;
    int hys_hb;
    k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
    hys_hb = ctrl_state.hys_half_band;
    k_mutex_unlock(&ctrl_state.mutex);
    while (1) {
        k_sem_take(&control_sem, K_FOREVER);
        if (ctrl_state.system_on) {
            delta = sensor_data.current_temp - ctrl_state.max_temp;

            if (delta > hys_hb) {
                led2_toggle(false);
                led3_toggle(false);
                led4_toggle(true);
            } else if (delta < (-1*hys_hb)) {
                led2_toggle(false);
                led3_toggle(true);
                led4_toggle(false);
            } else {
                led2_toggle(true);
                led3_toggle(false);
                led4_toggle(false);
            }
        } else {
            led2_toggle(false);
            led3_toggle(false);
            led4_toggle(false);
        }
    }
}

/**
 * @brief Thread toggling the heater based on temperature readings.
 */
void actuator_thread_func(void *argA, void *argB, void *argC)
{
    static int actuator_period = 200;
    while (1) {
        k_sem_take(&actuator_sem, K_FOREVER);
        if (!ctrl_state.system_on) {
            continue;
        }
        bool sys_state;
        int max_t;
        k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
        sys_state = ctrl_state.system_on;
        max_t = ctrl_state.max_temp;
        k_mutex_unlock(&ctrl_state.mutex);
        k_mutex_lock(&sensor_data.mutex, K_FOREVER);
        int curr_t = sensor_data.current_temp;
        if (ctrl_state.system_on) {
            if (max_t > curr_t) {
                heater_on();
            } else {
                heater_off();
            }
        }
        k_mutex_unlock(&sensor_data.mutex);

        int desired = sys_state ? 100 : 200;
        if (desired != actuator_period) {
            actuator_period = desired;
            k_timer_start(&actuator_timer, K_NO_WAIT, K_MSEC(desired));
        }
    }
}
