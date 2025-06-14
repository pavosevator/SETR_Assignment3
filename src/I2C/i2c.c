/* i2c.c - I2C interface implementation for TC74 sensor */

#include "i2c.h"
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>

#define TC74_CMD_RTR  0x00   /* Read temperature command */
#define TC74_CMD_RWCR 0x01   /* Configuration register command */

#define ERR_FATAL -1   /* If I2C fails ...*/

/* Device Tree configuration */
#define I2C0_NID DT_NODELABEL(tc74sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NID);

/* Initialize I2C sensor */
int i2c_init(void)
{
    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus not ready!\n");
        return ERR_FATAL;
    }
    return 0;
}

void sensor_thread_func(void *argA , void *argB, void *argC)
{
    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus not ready\n");
        return;
    }

    uint8_t cmd = TC74_CMD_RTR;
    uint8_t temp_raw;

    while (1) {
        if (!ctrl_state.system_on) {
            k_sleep(K_MSEC(100));
            continue;
        }
        if (i2c_write_read_dt(&dev_i2c, &cmd, 1, &temp_raw, 1) == 0) {
            int temp = (int8_t)temp_raw;
            k_mutex_lock(&sensor_data.mutex, K_FOREVER);
            sensor_data.current_temp = temp;
            k_mutex_unlock(&sensor_data.mutex);
        } 
        k_msleep(500);
    }
}


/* Read temperature from sensor */
int i2c_read_temperature(uint8_t *current_temp)
{
    uint8_t reg_ptr = TC74_CMD_RTR; // command for temperature register
    int ret;

    ret = i2c_write_read_dt(&dev_i2c, &reg_ptr, 1, current_temp, 1);

    if (ret != 0) {
        printk("I2C write_read failed: %d\n", ret);
    }

    return ret;
}
