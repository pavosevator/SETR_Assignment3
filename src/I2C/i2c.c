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

/* Initialize I2C sensor */
int i2c_init(void)
{
    int ret = 0;
    ret = i2c_check_bus_ready();
    if (ret != 0) {
        return ret;
    }
    return 0;
}



/* Read temperature from sensor */
int i2c_read_temperature(uint8_t *temp)
{   
    int ret;
    uint8_t cmd = TC74_CMD_RTR;
    printk("Looking for I2C at %p (addr=0x%02x)\n",
       (void *)dev_i2c.bus, dev_i2c.addr); 

    /* Issue the Read-Temperature command */
    ret = i2c_write_dt(&dev_i2c, &cmd, 1);
    if (ret) {
        printk("I2C write RTR failed (err %d)\n", ret);
        return ret;
    } 

    /* Read back the single-byte temperature */
    ret = i2c_read_dt(&dev_i2c, temp, 1);
    if (ret) {
        printk("I2C read temp failed (err %d)\n", ret);
    }
    return ret;
}
