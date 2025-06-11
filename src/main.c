/* main.c - I2C sensor test application */

#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "i2c.h"
#include "cmdproc.h"
#include "data.h"
#include "uart.h"
#include "gpio.h"

#define UPDATE_INTERVAL_MS 1000

#define STACK_SIZE 1024

#define COMMAND_PRIO 1
#define SENSOR_PRIO 2
#define CONTROL_PRIO 3
#define ACTUATOR_PRIO 4
#define UI_PRIO 5

K_THREAD_STACK_DEFINE(command_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(sensor_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(control_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(actuator_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(ui_stack, STACK_SIZE);

struct k_thread command_thread;
struct k_thread sensor_thread;
struct k_thread control_thread;
struct k_thread actuator_thread;
struct k_thread ui_thread;

k_tid_t command_tid;
k_tid_t sensor_tid;
k_tid_t control_tid;
k_tid_t actuator_tid;
k_tid_t ui_tid;

static unsigned char *rx_data;
static int rx_len;
static unsigned char *tx_data;
static int tx_len;

void command_thread_func(void *argA, void *argB, void *argC);
void control_thread_func(void *argA, void *argB, void *argC);
void actuator_thread_func(void *argA, void *argB, void *argC);
void ui_thread_func(void *argA, void *argB, void *argC);


int main(void)
{
    /* 1) Init I²C and UART (no mutex) */
    if (i2c_init() != 0) {
        printk("I2C init failed\n");
        return 0;
    }

    /* 3) Init UART */
    if (uart_init() != 0) {
        printk("UART init failed\n");
        return 0;
    }

    if (gpio_init() != 0) {
        printk("UART init failed\n");
        return 0;
    }

    k_mutex_init(&sensor_data.mutex);

    int ret = 0;
    uint8_t temperature = 0;

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

    k_sleep(K_FOREVER);
    return 0;
}

void command_thread_func(void *argA , void *argB, void *argC)
{
    uint8_t b;
    uint8_t t = 0;

    while (1) {
        if (!system_on) {
            k_sleep(K_MSEC(100));
            continue;
        }
        if (k_msgq_get(&uart_msgq, &b, K_MSEC(100)) != 0) {
            continue;
        }

        /* Feed that byte into cmdproc */
        rxChar(b);

        /* Drain any additional queued bytes without blocking */
        while (k_msgq_get(&uart_msgq, &b, K_NO_WAIT) == 0) {
            rxChar(b);
        }
        resetTxBuffer();
        /* Then call cmdProcessor() as before… */
        if (cmdProcessor() == CMD_OK) {
            unsigned char *tx_data;
            int tx_len;
            getTxBuffer(&tx_data, &tx_len);
            uart_send(tx_data, tx_len);
            
        }
        if(i2c_read_temperature(&t) != 0){
            printk("majmune");
        } else {
            printk("%d\n", t);
        }

    }
}

void control_thread_func(void *argA, void *argB, void *argC){
    while (1) {
        if (!system_on) {
            k_sleep(K_MSEC(100));
            continue;
        }
        k_sleep(K_MSEC(200));
    }
}

void actuator_thread_func(void *argA, void *argB, void *argC)
{
    while (1) {
        if (!system_on) {
            k_sleep(K_MSEC(100));
            continue;
        }
        //printk("Actuator thread running\n");
        k_sleep(K_MSEC(200));
    }
}

void ui_thread_func(void *argA, void *argB, void *argC)
{
    while (1) {
        ui_task();
    }
}
  

