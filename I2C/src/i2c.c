#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#include <sys/printk.h>
#include <errno.h>
#include <stdint.h>

/* TC74 definitions */
#define TC74_ADDR     0x48
#define TC74_CMD_RTR  0x00
#define TC74_CMD_RWCR 0x01

/* DeviceTree spec */
#define I2C0_NID       DT_NODELABEL(tc74sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NID);

/* Check bus readiness */
static int i2c_check_bus_ready(void)
{
    if (!device_is_ready(dev_i2c.bus)) {
        return -ENODEV;
    }
    return 0;
}

/* Set RTR pointer */
static int i2c_set_temperature_pointer(void)
{
    uint8_t cmd = TC74_CMD_RTR;
    return i2c_write_dt(&dev_i2c, &cmd, 1);
}

/* Wake TC74 if needed */
static int i2c_wake_tc74(void)
{
    uint8_t buf[2] = { TC74_CMD_RWCR, 0x00 };
    return i2c_write_dt(&dev_i2c, buf, sizeof(buf));
}

/* Public: initialize bus + sensor */
int i2c_init(void)
{
    int ret = i2c_check_bus_ready();
    if (ret) {
        return ret;
    }

    /* Clear shutdown (ignore error if already awake) */
    (void)i2c_wake_tc74();

    /* Point to temperature register */
    ret = i2c_set_temperature_pointer();
    if (ret) {
        return -EIO;
    }

    return 0;
}

/* Public: do a single temperature read */
int i2c_read_temperature_once(int8_t *temp)
{
    if (!dev_i2c.bus || !device_is_ready(dev_i2c.bus)) {
        return -ENODEV;
    }

    int ret = i2c_read_dt(&dev_i2c, (uint8_t *)temp, 1);
    if (ret) {
        return -EIO;
    }

    return 0;
}
