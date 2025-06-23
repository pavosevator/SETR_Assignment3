/* 
 * main.c - Temperature control application using Zephyr RTOS on NRF platform
 *
 * This application interfaces with I2C sensors, UART for command input,
 * and controls a heater element based on temperature readings. It spawns
 * several concurrent threads, each handling a specific subsystem: 
 * - Command handling
 * - Sensor reading
 * - Control logic
 * - Actuation (heater)
 * - User interface
 */

#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

#include "i2c.h"
#include "cmdproc.h"
#include "data.h"
#include "uart.h"
#include "gpio.h"
#include "heater.h"
#include "control.h"

#define STACK_SIZE 1024            // Stack size for each thread

// Thread priorities (lower number = higher priority)
#define COMMAND_PRIO 2
#define SENSOR_PRIO  3
#define CONTROL_PRIO 4
#define ACTUATOR_PRIO 5
#define UI_PRIO 1

// Define thread stacks
K_THREAD_STACK_DEFINE(command_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(sensor_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(control_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(actuator_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(ui_stack, STACK_SIZE);

// Thread control blocks
struct k_thread command_thread;
struct k_thread sensor_thread;
struct k_thread control_thread;
struct k_thread actuator_thread;
struct k_thread ui_thread;

// Thread identifiers
k_tid_t command_tid;
k_tid_t sensor_tid;
k_tid_t control_tid;
k_tid_t actuator_tid;
k_tid_t ui_tid;

/**
 * @brief Initialize all subsystems and create application threads
 * 
 * This function wraps all setup previously in `main()`, making it easier
 * to reuse during unit testing or simulate initialization.
 */
void app_init(void)
{
    // Initialize hardware drivers
    if (i2c_init() != 0) { printk("I2C failed\n"); return; }
    if (uart_init() != 0) { printk("UART failed\n"); return; }
    if (gpio_init() != 0) { printk("GPIO failed\n"); return; }
    if (heater_init() != 0) { printk("Heater failed\n"); return; }
    if (control_init() != 0) { printk("Heater failed\n"); return; }

    // Initialize system control state
    ctrl_state.system_on = false;
    ctrl_state.max_temp = 25;  // Default target temperature

    // Initial LED state
    led4_toggle(0);
    led3_toggle(0);

    //Voltage on PIN1.9 used for heater = 0 V
    heater_off();

    // Thread creation
    command_tid = k_thread_create(&command_thread, command_stack,
        K_THREAD_STACK_SIZEOF(command_stack), command_thread_func,
        NULL, NULL, NULL, COMMAND_PRIO, 0, K_NO_WAIT);

    sensor_tid = k_thread_create(&sensor_thread, sensor_stack,
        K_THREAD_STACK_SIZEOF(sensor_stack), sensor_thread_func,
        NULL, NULL, NULL, SENSOR_PRIO, 0, K_NO_WAIT);

    control_tid = k_thread_create(&control_thread, control_stack,
        K_THREAD_STACK_SIZEOF(control_stack), control_thread_func,
        NULL, NULL, NULL, CONTROL_PRIO, 0, K_NO_WAIT);

    actuator_tid = k_thread_create(&actuator_thread, actuator_stack,
        K_THREAD_STACK_SIZEOF(actuator_stack), actuator_thread_func,
        NULL, NULL, NULL, ACTUATOR_PRIO, 0, K_NO_WAIT);

    ui_tid = k_thread_create(&ui_thread, ui_stack,
        K_THREAD_STACK_SIZEOF(ui_stack), ui_thread_func,
        NULL, NULL, NULL, UI_PRIO, 0, K_NO_WAIT);

}

/**
 * @brief Application entry point
 * 
 * Only calls initialization and sleeps indefinitely (threads do the work)
 */
int main(void)
{
    app_init(); // All hardware and thread setup is done here
    k_sleep(K_FOREVER); // Main thread sleeps forever
    return 0;
}