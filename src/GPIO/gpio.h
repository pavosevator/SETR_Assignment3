#ifndef GPIO_H
#define GPIO_H

#include <zephyr/kernel.h>          /* for k_msleep() */
#include <zephyr/device.h>          /* for device_is_ready() and device structure */
#include <zephyr/devicetree.h>		/* for DT_NODELABEL() */
#include <zephyr/drivers/gpio.h>    /* for GPIO api*/
#include <zephyr/sys/printk.h>      /* for printk()*/

/* Define the aliases for the buttons and LEDs */
#define BUTTON1_NODE DT_ALIAS(sw0)
#define BUTTON2_NODE DT_ALIAS(sw1)
#define BUTTON3_NODE DT_ALIAS(sw2)
#define BUTTON4_NODE DT_ALIAS(sw3)

#define LED1_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led1)
#define LED3_NODE DT_ALIAS(led2)
#define LED4_NODE DT_ALIAS(led3)

/* Function prototypes */
int gpio_init(void);
void button_task(void);
void led_task(void);
void led2_toggle(bool state);
void led3_toggle(bool state);
void led4_toggle(bool state);

#endif // GPIO_H
