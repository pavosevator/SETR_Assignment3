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

/* Check if I2C bus is ready */
int i2c_check_bus_ready(void)
{
    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus not ready!\n");
        return ERR_FATAL;
    }
    return 0;
}

int i2c_wake_tc74(void)
{
    /* Send: write to register 0x01 the value 0x00 */
    return i2c_write_dt(&dev_i2c, TC74_CMD_RTR, 1);
}

/* Initialize I2C sensor */
int i2c_init(void)
{
    int ret = 0;
    ret = i2c_check_bus_ready();
    if (ret != 0) {
        return ret;
    }

    ret = i2c_write_dt(&dev_i2c, (uint8_t) TC74_CMD_RTR, 1);
    if (ret != 0) {
        printk("Error setting TC74 pointer to temp reg\n");
        return ret;
    }
return 0;
}



/* Read temperature from sensor */
int i2c_read_temperature(uint8_t *temp)
{   
    if (!temp) {
        return ERR_FATAL;
    }
    int read;

    read = i2c_read_dt(&dev_i2c, temp, 1);
    if (read != 0) {
        printk("Error reading temperature\n");
        return read;
    }

    return 0;
}
