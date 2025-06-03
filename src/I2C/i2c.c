/* i2c.c - I2C interface implementation for TC74 sensor */

#include "i2c.h"
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>
#include <errno.h>

#define TC74_CMD_RTR  0x00   /* Read temperature command */
#define TC74_CMD_RWCR 0x01   /* Configuration register command */

/* Device Tree configuration */
#define I2C0_NID DT_NODELABEL(tc74sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NID);

/* Check if I2C bus is ready */
int i2c_check_bus_ready(void)
{
    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus not ready!\n");
        return -ENODEV;
    }
    return 0;
}

/* Wake sensor by clearing shutdown bit */
int i2c_wake_tc74(void)
{
    uint8_t buf[2] = { TC74_CMD_RWCR, 0x00 };
    return i2c_write_dt(&dev_i2c, buf, sizeof(buf));
}

/* Set sensor pointer to temperature register */
/* Write (command RTR) to set the read address to temperature */
/* Only necessary if a config done before (not the case), but let's stay in the safe side */
int i2c_set_temperature_pointer(void)
{
    return i2c_write_dt(&dev_i2c, TC74_CMD_RTR, 1);
}

/* Initialize I2C sensor */
int i2c_init(void)
{
    int ret = i2c_check_bus_ready();
    if (ret != 0) {
        return ret;
    }

    /* Set temperature pointer register */
    //ret = i2c_set_temperature_pointer();
    if (ret != 0) {
        printk("Error setting temperature pointer\n");
        return ret;
    }

    printk("I2C sensor initialized successfully\n");
    return 0;
}

/* Read temperature from sensor */
int i2c_read_temperature(int8_t *temp)
{
    if (!temp) {
        return -EINVAL;
    }

    int ret = i2c_read_dt(&dev_i2c, (uint8_t*)temp, sizeof(temp));
    if (ret != 0) {
        printk("Error reading temperature\n");
        return ret;
    }

    return 0;
}
