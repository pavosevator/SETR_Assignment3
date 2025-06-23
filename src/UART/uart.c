/**
 * @file uart.c
 * @brief UART interface implementation for nRF52840 using Zephyr.
 *
 * This module provides UART initialization, asynchronous TX/RX handling, 
 * and APIs to send and receive data using Zephyr RTOS features.
 */

#include "uart.h"
#include <zephyr/sys/ring_buffer.h>   /* Ring buffer utilities (unused here but available) */
#include <zephyr/kernel.h>           /* Kernel APIs: semaphores, msgq, sleep */
#include <zephyr/drivers/uart.h>      /* UART driver API */
#include <zephyr/sys/printk.h>        /* printk logging */

/** 
 *@brief Semaphore used to signal that new data has arrived
 **/
static struct k_sem uart_rx_sem;

/**
 * UART device instance from DeviceTree
 * -------------------------------------------------------------
 * The UART_NODE label must be defined in your board's devicetree.
 **/
const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);

/** 
 * RX buffers to hold incoming data
 * -------------------------------------------------------------
 * rx_buf: DMA buffer provided to driver
 * rx_chars: accumulation buffer for processed bytes
 **/
static uint8_t rx_buf[RXBUF_SIZE];
static uint8_t rx_chars[RXBUF_SIZE];
volatile int uart_rxbuf_nchar = 0;    /* Number of bytes currently stored in rx_chars */

/** 
 * Message queue for received bytes
 * -------------------------------------------------------------
 * Stores individual bytes from RX_RDY events for later processing
 **/
K_MSGQ_DEFINE(uart_msgq, sizeof(uint8_t), 32, 4);

/** 
 * Generic error and message buffers
 **/
int err = 0;
uint8_t rep_mesg[MSG_BUF_SIZE];    /* Buffer for user to fill with reply data */

/** 
 * UART configuration settings
 * -------------------------------------------------------------
 * If dynamic configuration is enabled (CONFIG_UART_USE_RUNTIME_CONFIGURE),
 * these runtime parameters are applied. Otherwise, devicetree settings prevail.
 **/
const struct uart_config uart_cfg = {
    .baudrate = 115200,
    .parity = UART_CFG_PARITY_NONE,
    .stop_bits = UART_CFG_STOP_BITS_1,
    .data_bits = UART_CFG_DATA_BITS_8,
    .flow_ctrl = UART_CFG_FLOW_CTRL_NONE
};

/**
 * @brief Initialize and configure UART peripheral
 *
 * Steps:
 *  1. Verify device readiness
 *  2. Apply runtime configuration
 *  3. Register interrupt-driven callback
 *  4. Enable RX with timeout
 *  5. Initialize RX semaphore
 *
 * @return 0 on success or FATAL_ERR on failure
 **/
int uart_init(void)
{
    /* Ensure the UART device is accessible */
    if (!device_is_ready(uart_dev)) {
        printk("device_is_ready(uart) returned error! Aborting!\n\r");
        return FATAL_ERR;
    }
    printk("UART init\n");

    /* Apply UART settings at runtime */
    err = uart_configure(uart_dev, &uart_cfg);
    if (err == -ENOSYS) {
        printk("uart_configure() error. Invalid configuration\n\r");
        return FATAL_ERR;
    }

    /* Register callback for TX/RX events */
    err = uart_callback_set(uart_dev, uart_cb, NULL);
    if (err) {
        printk("uart_callback_set() error. Error code:%d\n\r", err);
        return FATAL_ERR;
    }

    /* Enable RX with DMA buffer and timeout */
    err = uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), RX_TIMEOUT);
    if (err) {
        printk("uart_rx_enable() error. Error code:%d\n\r", err);
        return FATAL_ERR;
    }
    
    printk("Start testing...\n");
    /* Initialize semaphore to 0; will be given in RX callback */
    k_sem_init(&uart_rx_sem, 0, 1);

    return 0;
}

/**
 * @brief Check and retrieve data from RX buffer
 *
 * If characters have been accumulated, terminate as string,
 * reset counter, and return pointer and length to caller.
 *
 * @param buf Pointer to store address of received data
 * @param len Pointer to store number of bytes received
 * @return 0 if data available, otherwise unspecified
 **/
int uart_check_buffer(unsigned char **buf, int *len)
{
    if (uart_rxbuf_nchar > 0) {
        int n = uart_rxbuf_nchar;
        rx_chars[n] = '\0';                /* Null-terminate */
        uart_rxbuf_nchar = 0;              /* Reset for next data */

        *buf = rx_chars;
        *len = n;
        return 0;
    }
    return -1;
}

/**
 * @brief Clear received character count
 */
void uart_resetRxBuffer(void)
{
    uart_rxbuf_nchar = 0;
}

/**
 * @brief Send data over UART
 *
 * @param buf Pointer to data buffer
 * @param len Number of bytes to send
 * @return UART driver return code
 **/
int uart_send(const uint8_t *buf, size_t len)
{
    return uart_tx(uart_dev, buf, len, SYS_FOREVER_MS);
}

/**
 * @brief UART interrupt/event callback
 *
 * Handles asynchronous TX and RX events. Keep ISR logic minimal;
 * heavy processing (e.g., parsing) should occur in separate threads.
 **/
void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    int ret;

    switch (evt->type) {
        case UART_TX_DONE:
            printk("UART_TX_DONE event\n\r");
            break;

        case UART_TX_ABORTED:
            printk("UART_TX_ABORTED event\n\r");
            break;

        case UART_RX_RDY:
            /* On new data ready, enqueue each byte */
            printk("UART_RX_RDY event\n\r");
            for (int i = 0; i < evt->data.rx.len; i++) {
                uint8_t b = rx_buf[evt->data.rx.offset + i];
                k_msgq_put(&uart_msgq, &b, K_NO_WAIT);
            }
            /* Signal waiting thread that data is available */
            k_sem_give(&uart_rx_sem);
            break;

        case UART_RX_BUF_RELEASED:
            printk("UART_RX_BUF_RELEASED event\n\r");
            break;

        case UART_RX_BUF_REQUEST:
            printk("UART_RX_BUF_REQUEST event\n\r");
            break;

        case UART_RX_DISABLED:
            /* RX buffer full: re-enable reception */
            printk("UART_RX_DISABLED event\n\r");
            ret = uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), RX_TIMEOUT);
            if (ret) {
                printk("uart_rx_enable() error. Code:%d\n\r", ret);
                /* In critical failure, abort application */
                k_oops();
            }
            break;

        case UART_RX_STOPPED:
            printk("UART_RX_STOPPED event\n\r");
            break;

        default:
            printk("UART: unknown event\n\r");
            break;
    }
}

/**
 * @brief Block until at least one RX event occurs
 **/
void uart_wait_for_rx(void)
{
    k_sem_take(&uart_rx_sem, K_FOREVER);
}
