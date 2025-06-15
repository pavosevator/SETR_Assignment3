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

#define UPDATE_INTERVAL_MS 1000    // Not currently used but reserved for timed tasks
#define STACK_SIZE 1024            // Stack size for each thread

// Thread priorities (lower number = higher priority)
#define COMMAND_PRIO 1
#define SENSOR_PRIO  2
#define CONTROL_PRIO 3
#define ACTUATOR_PRIO 4
#define UI_PRIO 5

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

// Buffers for UART transmission (not actively used in main.c, reserved for modules)
static unsigned char *rx_data;
static int rx_len;
static unsigned char *tx_data;
static int tx_len;

// Thread entry function declarations
void command_thread_func(void *argA, void *argB, void *argC);
void control_thread_func(void *argA, void *argB, void *argC);
void actuator_thread_func(void *argA, void *argB, void *argC);
void ui_thread_func(void *argA, void *argB, void *argC);

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

    // Initialize system control state
    ctrl_state.system_on = false;
    ctrl_state.max_temp = 25;  // Default target temperature

    // Initialize shared data structures (mutex-protected)
    k_mutex_init(&sensor_data.mutex);
    k_mutex_init(&ctrl_state.mutex);

    // Initial LED state
    led4_toggle(0);
    led3_toggle(0);

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

/**
 * @brief Command thread
 * 
 * Reads bytes from UART queue, passes them into command parser, and handles output
 */
void command_thread_func(void *argA , void *argB, void *argC)
{
    uint8_t b;
    while (1) {
        if (!ctrl_state.system_on) {
            k_sleep(K_MSEC(100));
            continue;
        }

        // Wait for a byte from UART with timeout
        if (k_msgq_get(&uart_msgq, &b, K_MSEC(100)) != 0) {
            continue;
        }

        // Process the first received byte
        rxChar(b);

        // Process all other bytes in queue without blocking
        while (k_msgq_get(&uart_msgq, &b, K_NO_WAIT) == 0) {
            rxChar(b);
        }

        resetTxBuffer();

        // Process full command if valid
        if (cmdProcessor() == CMD_OK) {
            getTxBuffer(&tx_data, &tx_len);
            uart_send(tx_data, tx_len); // Send back response
        }
    }
}

/**
 * @brief Control thread
 * 
 * Compares current temperature to target and signals warning using LEDs
 */
void control_thread_func(void *argA, void *argB, void *argC)
{
    int delta;
    while (1) {
        if (ctrl_state.system_on) {
            delta = sensor_data.current_temp - ctrl_state.max_temp;

            if (delta > 2) {           // Too hot
                led2_toggle(0);
                led3_toggle(0);
                led4_toggle(1);
            } else if (delta < -2) {   // Too cold
                led2_toggle(0);
                led3_toggle(1);
                led4_toggle(0);
            } else {                   // Temperature is OK
                led2_toggle(1);
                led3_toggle(0);
                led4_toggle(0);
            }
        } else {
            // Turn off all LEDs when system is off
            led2_toggle(0);
            led3_toggle(0);
            led4_toggle(0);
        }
        k_sleep(K_MSEC(200));
    }
}

/**
 * @brief Actuator thread
 * 
 * Turns the heater on or off based on temperature and system state
 */
void actuator_thread_func(void *argA, void *argB, void *argC)
{
    while (1) {
        if (ctrl_state.system_on) {
            if (ctrl_state.max_temp >= sensor_data.current_temp) {
                heater_on();
            } else {
                heater_off();
            }
            k_sleep(K_MSEC(100));
            continue;
        }
        // System off: check less frequently
        k_sleep(K_MSEC(200));
    }
}

/**
 * @brief UI thread
 * 
 * Handles user interface logic (e.g. display or input polling)
 */
void ui_thread_func(void *argA, void *argB, void *argC)
{
    while (1) {
        ui_task();  // UI logic runs here (defined in a different module)
    }
}
