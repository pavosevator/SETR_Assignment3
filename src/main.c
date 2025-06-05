/* main.c - I2C sensor test application */

#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "i2c.h"
#include "cmdproc.h"
#include "data.h"
#include "uart.h"

#define UPDATE_INTERVAL_MS 1000

int main(void)
{
    unsigned char *rx_data;
    int rx_len;
    unsigned char *tx_data;
    int tx_len;

    printk("Starting sensor/control application\n");

    /* 1) Init I²C and UART (no mutex) */
    if (i2c_init() != 0) {
        printk("I2C init failed\n");
        return;
    }
    if (uart_init() != 0) {
        printk("UART init failed\n");
        return;
    }

    /* 3) Main loop: poll for incoming UART data */
    while (1) {
        int aloba = uart_check_buffer(&rx_data, &rx_len);
        if(aloba == 0){
            for (int i = 0; i < rx_len; i++) {
                rxChar(rx_data[i]);      /* cmdproc.c */
            }
        

        /* 3) Run command processor (cmdproc.c) */
        int majmunko = cmdProcessor();
        resetRxBuffer();  
        if (majmunko == CMD_OK) {
            /* 4) Retrieve reply and send via UART (uart.c) */
            getTxBuffer(&tx_data, &tx_len);  /* cmdproc.c */
            if(uart_send(tx_data, tx_len) != 0){
                printk("Stoopid \n");
            }
            resetTxBuffer();                 /* cmdproc.c */
        }
    
        (UPDATE_INTERVAL_MS);
        }
    }
    return 0;
}