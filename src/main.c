/* main.c - I2C sensor test application */

#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include "i2c.h"
#include "cmdproc.h"
#include "data.h"
#include "uart.h"

#define UPDATE_INTERVAL_MS 1000

#define STACK_SIZE 1024

#define thread_A_prio 1
#define thread_B_prio 2

K_THREAD_STACK_DEFINE(thread_A_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(thread_B_stack, STACK_SIZE);

struct k_thread thread_A_data;
struct k_thread thread_B_data;

k_tid_t thread_A_tid;
k_tid_t thread_B_tid;

static unsigned char *rx_data;
static int rx_len;
static unsigned char *tx_data;
static int tx_len;

void thread_A_code(void *argA, void *argB, void *argC);
void thread_B_code(void *argA, void *argB, void *argC);


int main(void)
{
    /* 1) Init I²C and UART (no mutex) */
    /*if (i2c_init() != 0) {
        printk("I2C init failed\n");
        return 0;
    }*/
    /* if (uart_init() != 0) {
        printk("UART init failed\n");
        return -1;
    } */

    int ret = 0;
    uint8_t temperature = 0;
    printk("Starting sensor/control application\n");


    thread_A_tid = k_thread_create(&thread_A_data, thread_A_stack,
        K_THREAD_STACK_SIZEOF(thread_A_stack), thread_A_code,
        NULL, NULL, NULL, thread_A_prio, 0, K_NO_WAIT);

    thread_B_tid = k_thread_create(&thread_B_data, thread_B_stack,
        K_THREAD_STACK_SIZEOF(thread_B_stack), thread_B_code,
        NULL, NULL, NULL, thread_B_prio, 0, K_NO_WAIT);

    /* 3) Main loop: poll for incoming UART data */
    while (1) {
        /***********************************************
        if(uart_check_buffer(&rx_data, &rx_len) == 0){
            for (int i = 0; i < rx_len; i++) {
                rxChar(rx_data[i]);     
            }
        
        if (cmdProcessor() == CMD_OK) { 
            getTxBuffer(&tx_data, &tx_len);  
            if(uart_send(tx_data, tx_len) != 0){
                printk("Error in main \n");
            }
            resetTxBuffer();            
        } 
        ***************************************************/
    
        k_msleep(UPDATE_INTERVAL_MS);
    }
    return 0;
}

void thread_A_code(void *argA , void *argB, void *argC)
{
    /* Thread loop */
    while(1) {
        
        /* BLock until sem is given to k_sem_give(&uart_rx_sem) */
        uart_wait_for_rx();
        
        /* */
        if(uart_check_buffer(&rx_data, &rx_len) == 0){
            for(int i = 0; i < rx_len; i++){
                rxChar(rx_data[i]);
            }
            uart_resetRxBuffer;
        }

        if(cmdProcessor() == CMD_OK){
            getTxBuffer(&tx_data, &tx_len);
            uart_send(tx_data,tx_len);
            resetTxBuffer;
        }
    }
}

void thread_B_code(void *argA , void *argB, void *argC)
{
    printk("Thread B init (sporadic, waits on a semaphore by task A)\n");
}
