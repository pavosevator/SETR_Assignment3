#ifndef UART_H_
#define UART_H_

#include <zephyr/kernel.h>          /* for k_msleep() */
#include <zephyr/device.h>          /* for device_is_ready() and device structure */
#include <zephyr/devicetree.h>	    /* for DT_NODELABEL() */
#include <zephyr/drivers/uart.h>    /* for UART API*/
#include <zephyr/sys/printk.h>      /* for printk()*/
#include <zephyr/types.h>
#include <stdio.h>                  /* for sprintf() */
#include <stdlib.h>
#include <string.h>

#define UART_NODE DT_NODELABEL(uart0)   /* UART0 node ID*/
#define MAIN_SLEEP_TIME_MS 1000 /* Time between main() activations */ 

#define FATAL_ERR -1 /* Fatal error return code, app terminates */

#define RXBUF_SIZE 60                   /* RX buffer size */
#define TXBUF_SIZE 60                   /* TX buffer size */
#define MSG_BUF_SIZE 100                /* Buffer for messages sent vai UART */
#define RX_TIMEOUT 1000                 /* Inactivity period after the instant when last char was received that triggers an rx event (in us) */


extern struct k_msgq uart_msgq;

/* UART callback function prototype */
void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);

int uart_init(void);
int uart_check_buffer(unsigned char **buf, int *len);
void uart_resetRxBuffer(void);
int uart_send(const uint8_t *buf, size_t len);
void uart_wait_for_rx(void);

#endif