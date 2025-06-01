#include <zephyr.h>
#include <sys/printk.h>
#include "i2c.h"

void main(void)
{
    int ret;
    int8_t temp;

    printk("I2C test starting...\n");

    /* 1) Initialize I²C + TC74 */
    ret = i2c_init();
    if (ret) {
        printk("i2c_init() failed: %d\n", ret);
        return;
    }

    /* 2) Read temperature once */
    ret = i2c_read_temperature_once(&temp);
    if (ret == 0) {
        printk("TC74 temperature: %d °C\n", temp);
    } else {
        printk("i2c_read_temperature_once() failed: %d\n", ret);
    }

    /* 3) Done. Now just idle. */
    while (1) {
        k_sleep(K_MSEC(1000));
    }
}

