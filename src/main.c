/* main.c - I2C sensor test application */

#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "i2c.h"
#include "cmdproc.h"
#include "data.h"
#include "uart.h"

#define UPDATE_INTERVAL_MS 1000

void main(void)
{
    int ret;
    int8_t temperature = 0;

    printk("Starting sensor/control application\n");

    /* 1) Initialize I2C (so cmdproc can read sensor if needed) */
    ret = i2c_init();
    if (ret != 0) {
        printk("i2c_init() failed: %d\n", ret);
        return;
    }

    /* 2) Initialize UART (sets up callback + rx_buf) */
    ret = uart_init();
    if (ret != 0) {
        printk("uart_init() failed: %d\n", ret);
        return;
    }

    /* 3) Main loop: poll for incoming UART data */
    while (1) {

        /* Read I2C data */
        ret = i2c_read_temperature(&temperature);
        if (ret == 0) {
            printk("Temperature: %d°C\n", temperature);
        } else {
            printk("Error reading temperature: %d\n", ret);
        }

        /* Send command from UART to cmdProcessor() buffer */
        
        uint8_t b;
        while (uart_poll_rx(&b)) {
            rxChar(b);
        }

        /* 4) Run the parser each time (it returns CMD_OK only when a full frame is ready) */
        int result = cmdproc_run();
        if (result == CMD_OK) {
            /* 5) A reply is ready: get it and send it */
            uint8_t *reply;
            size_t reply_len;
            if (cmdproc_get_reply(&reply, &reply_len)) {
                //uart_send(reply, reply_len);
                resetTxBuffer();
            }
        } else {
            /* Optionally send an error response, or just clear Rx */
            resetRxBuffer();
        }


        /* If UART received bytes, uart_rxbuf_nchar > 0 */
        if (uart_rxbuf_nchar > 0) {
            /* Copy all bytes from uart.c’s rx_buf into cmdproc’s UARTRxBuffer */
            for (int i = 0; i < uart_rxbuf_nchar; i++) {
                rxChar(rx_buf[i]);
            }
            /* Reset uart_rxbuf_nchar so we only process each character once */
            uart_rxbuf_nchar = 0;

            /* 4) Run the command processor on any new data */
            int cmd_ret = cmdProcessor();
            if (cmd_ret == CMD_OK) {
                /* 5) If cmdProcessor placed a reply in UARTTxBuffer, fetch it */
                unsigned char *reply;
                int len;
                getTxBuffer(&reply, &len);
                if (len > 0) {
                    /* Send reply over UART */
                    uart_tx(uart_dev, reply, len, SYS_FOREVER_MS);
                    /* Clear cmdproc’s TX buffer for next command */
                    resetTxBuffer();
                }
            }
            /* You may want to check for CMD_INVALID or other return codes here */
        }
    
        k_msleep(UPDATE_INTERVAL_MS);
    }
}