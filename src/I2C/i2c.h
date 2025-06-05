#ifndef I2C_H_
#define I2C_H_
#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h> 
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <errno.h>
#include <stdint.h>

/**
 * @brief  Check if the I²C bus and TC74 device are ready.
 *
 * @return
 *   -  0        if the bus/device is ready
 *   - –ENODEV   if the I²C bus or device is not ready
 */
int i2c_check_bus_ready(void);

/**
 * @brief  Initialize the I²C bus and TC74 sensor.
 *
 *   1. Binds to I2C0 (DT_NODELABEL(tc74sensor)),
 *   2. Verifies readiness (i2c_check_bus_ready),
 *   3. Wakes TC74 if necessary (i2c_wake_tc74),
 *   4. Sets the RTR pointer (i2c_set_temperature_pointer).
 *
 * Must be called before any other I²C operations.  
 *
 * @return
 *   -  0        on success
 *   - –ENODEV   if the I²C bus or device isn’t ready
 *   - –EIO      on any I²C transfer failure
 */
int i2c_init(void);

/**
 * @brief  Perform a single synchronous read of the TC74’s temperature register.
 *
 * Calls i2c_read_temperature() internally. The bus must have been initialized
 * by i2c_init() first.
 *
 * @param[out] temp
 *    Pointer to an int8_t that will receive the temperature (°C).
 *
 * @return
 *   -  0        if the read succeeded (and *temp is valid)
 *   - –ENODEV   if the I²C bus/device isn’t ready
 *   - –EIO      if the I²C transfer failed
 */
int i2c_read_temperature(uint8_t *temp);


# endif