/* main.c - I2C sensor test application */

#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "i2c.h"

#define UPDATE_INTERVAL_MS 1000

void main(void)
{
    int ret;
    int8_t temperature = 0;

    printk("Initializing TC74 I2C sensor...\n");

    ret = i2c_init();
    if (ret != 0) {
        printk("Failed to initialize I2C sensor, error: %d\n", ret);
        return;
    }

    while (1) {
        ret = i2c_read_temperature(&temperature);
        if (ret == 0) {
            printk("Temperature: %d°C\n", temperature);
        } else {
            printk("Error reading temperature: %d\n", ret);
        }

        k_msleep(UPDATE_INTERVAL_MS);
    }
}