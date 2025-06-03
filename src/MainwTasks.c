/* main.c - Zephyr OS task scheduling example for thermal controller */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "i2c.h"
#include "cmdproc.h"
#include "data.h"

#define STACK_SIZE 1024
#define PRIORITY_COMMAND 5
#define PRIORITY_SENSOR 4
#define PRIORITY_UI 6


/* Placeholder task functions */
void command_task(void)
{
int ret;

    while (1) {
        ret = cmdProcessor();
        if(ret == CMD_OK) {
            printk("Command processed successfully\n");
        } else if (ret == CMD_INVALID) {
            printk("Invalid command received\n");
        } else if (ret == CMD_CS_ERROR) {
            printk("Checksum error detected\n");
        } else if (ret == CMD_MISSING_SOF_ERROR || ret == CMD_MISSING_EOF_ERROR) {
            printk("Framing error detected\n");
        }
        k_msleep(100);
    }
}

void sensor_task(void)
{
    int8_t temperature;

    i2c_init();

    while (1) {
        if (i2c_read_temperature(&temperature) == 0) {
            k_mutex_lock(&sensor_data.mutex, K_FOREVER);
            sensor_data.temperature = temperature;
            k_mutex_unlock(&sensor_data.mutex);

            printk("Sensor task: Temperature: %d°C\n", temperature);
        } else {
            printk("Sensor task: Temperature read error\n");
        }
        k_msleep(1000);
    }
}

void ui_task(void)
{
    while (1) {
        /* Placeholder: buttons and LEDs handling logic */
        printk("UI task handling buttons and LEDs\n");
        k_msleep(200);
    }
}

/* Define thread stacks */
K_THREAD_STACK_DEFINE(command_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(sensor_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(ui_stack, STACK_SIZE);

/* Define thread control blocks */
struct k_thread command_thread;
struct k_thread sensor_thread;
struct k_thread ui_thread;

void main(void)
{   
    k_mutex_init(&sensor_data.mutex);

    printk("Starting thermal controller application with Zephyr tasks\n");

    /* Start command handling thread */
    k_thread_create(&command_thread, command_stack, STACK_SIZE,
                    (k_thread_entry_t)command_task, NULL, NULL, NULL,
                    PRIORITY_COMMAND, 0, K_NO_WAIT);

    /* Start temperature sensor handling thread */
    k_thread_create(&sensor_thread, sensor_stack, STACK_SIZE,
                    (k_thread_entry_t)sensor_task, NULL, NULL, NULL,
                    PRIORITY_SENSOR, 0, K_NO_WAIT);

    /* Start user interface (buttons & LEDs) handling thread */
    k_thread_create(&ui_thread, ui_stack, STACK_SIZE,
                    (k_thread_entry_t)ui_task, NULL, NULL, NULL,
                    PRIORITY_UI, 0, K_NO_WAIT);
}