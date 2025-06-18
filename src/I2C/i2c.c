/**
 * @file i2c.c
 * @brief I2C interface implementation for TC74 temperature sensor.
 *
 * This module provides functions to initialize and communicate with
 * a TC74 sensor over I2C, including a periodic thread that reads
 * temperature values and updates shared system state.
 */

#include "i2c.h"
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>

/**
 * TC74 sensor commands
 * -------------------------------------------------------------
 * TC74_CMD_RTR  (0x00): Selects the Read-Temperature register
 * TC74_CMD_RWCR (0x01): Selects the Read/Write Configuration register
 **/
#define TC74_CMD_RTR  0x00   /* Read temperature command */
#define TC74_CMD_RWCR 0x01   /* Configuration register command */

/* Generic error code for fatal I2C failures */
#define ERR_FATAL -1

/**
 * Device Tree binding for the I2C-connected TC74 sensor
 * -------------------------------------------------------------
 * The DT_NODELABEL(tc74sensor) entry must be defined in the board's .dts file
 * to match the hardware wiring. Zephyr I2C_DT_SPEC_GET macro reads bus & address.
 **/
#define I2C0_NID DT_NODELABEL(tc74sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NID);

/**
 * @brief Initialize the I2C interface to the TC74 sensor
 *
 * Checks that the I2C bus driver is ready before any transactions.
 *
 * @return 0 on success, ERR_FATAL if the bus device is not ready
 **/
int i2c_init(void)
{
    if (!device_is_ready(dev_i2c.bus)) {
        /* Bus driver not initialized or missing in DT */
        printk("I2C bus not ready!\n");
        return ERR_FATAL;
    }
    return 0;
}

/**
 * @brief Periodic sensor read thread
 *
 * This function is intended to be run as a Zephyr thread. It polls the TC74
 * at a fixed interval and stores the converted temperature into a shared
 * data structure, protecting access with a mutex.
 *
 * @param argA Unused
 * @param argB Unused
 * @param argC Unused
 */
void sensor_thread_func(void *argA, void *argB, void *argC)
{
    /* Ensure the I2C bus is still available */
    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus not ready\n");
        return;
    }

    uint8_t cmd = TC74_CMD_RTR;   /* Command to select the temperature register */
    uint8_t temp_raw;

    while (1) {
        /* Skip reading if system is turned off */
        if (!ctrl_state.system_on) {
            k_sleep(K_MSEC(100));
            continue;
        }

        /* Perform write-then-read to get raw temperature byte */
        if (i2c_write_read_dt(&dev_i2c, &cmd, 1, &temp_raw, 1) == 0) {
            /* Convert raw data (unsigned) to signed temperature (°C) */
            int temp = (int8_t)temp_raw;

            /* Safely update shared sensor data */
            k_mutex_lock(&sensor_data.mutex, K_FOREVER);
            sensor_data.current_temp = temp;
            k_mutex_unlock(&sensor_data.mutex);
        }

        /* Wait 500 ms before next measurement */
        k_msleep(500);
    }
}

/**
 * @brief Read current temperature from the TC74 sensor
 *
 * Sends the Read-Temperature command then reads one byte from the sensor.
 *
 * @param current_temp Pointer to a uint8_t where the raw temperature will be stored
 * @return 0 on success, non-zero error code on failure
 */
int i2c_read_temperature(uint8_t *current_temp)
{
    uint8_t reg_ptr = TC74_CMD_RTR;  /* Pointer to temperature register */
    int ret;

    /* Combined write (register select) and read (data) operation */
    ret = i2c_write_read_dt(&dev_i2c, &reg_ptr, 1, current_temp, 1);

    if (ret != 0) {
        /* Log the I2C transaction error code */
        printk("I2C write_read failed: %d\n", ret);
    }

    return ret;
}
