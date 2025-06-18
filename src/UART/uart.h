/**
 * @file uart.h
 * @brief UART driver interface for the temperature‑controller project running on
 *        the nRF52840‑DK under Zephyr RTOS.
 *
 * The module wraps Zephyr’s asynchronous UART API to offer:
 *   - Non‑blocking transmission with a simple @ref uart_send helper.
 *   - Interrupt‑driven reception that collects characters in an internal buffer
 *     and dispatches complete frames through a Zephyr message queue
 *     (@ref uart_msgq).
 *   - Basic runtime initialisation (@ref uart_init) and buffer‑management
 *     helpers.
 *
 * All definitions are implemented in @c uart.c.
 */

#ifndef UART_H_
#define UART_H_

#include <zephyr/kernel.h>          /**< Zephyr kernel helpers such as k_msleep(). */
#include <zephyr/device.h>          /**< Device driver structures and helpers.   */
#include <zephyr/devicetree.h>      /**< DeviceTree access macros (DT_NODELABEL). */
#include <zephyr/drivers/uart.h>    /**< Zephyr UART driver API.                 */
#include <zephyr/sys/printk.h>      /**< printk() for low‑level logging.         */
#include <zephyr/types.h>
#include <stdio.h>                  /**< sprintf().                              */
#include <stdlib.h>
#include <string.h>

/** @brief DeviceTree node of the UART peripheral used by this application. */
#define UART_NODE DT_NODELABEL(uart0)

/** @brief Delay between successive iterations of the @c main thread (ms). */
#define MAIN_SLEEP_TIME_MS 1000

/** @brief Fatal error return code. The application terminates when returned. */
#define FATAL_ERR -1

/** @brief Size of the receive buffer in bytes. */
#define RXBUF_SIZE 60

/** @brief Size of the transmit buffer in bytes. */
#define TXBUF_SIZE 60

/** @brief Size of the application‑level message buffer in bytes. */
#define MSG_BUF_SIZE 100

/**
 * @brief Idle time (µs) after the last received character that triggers a
 *        UART RX timeout event.
 */
#define RX_TIMEOUT 1000


/**
 * @brief Message queue receiving complete UART frames.
 *
 * Each queue element is a null‑terminated string whose maximum length is
 * @ref RXBUF_SIZE bytes.  Application threads can block on this queue using
 * @c k_msgq_get() to wait for new frames.
 */
extern struct k_msgq uart_msgq;

/**
 * @brief UART driver callback.
 *
 * Registered with the Zephyr UART driver to handle TX and RX events.  When a
 * full frame is received or a timeout occurs, the function enqueues the data
 * into @ref uart_msgq.
 *
 * @param dev       Pointer to the UART device that raised the callback.
 * @param evt       Pointer to the UART event description provided by Zephyr.
 * @param user_data User context pointer supplied during registration (unused).
 */
void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data);

/**
 * @brief Initialise the UART peripheral and associated kernel objects.
 *
 * This function configures the UART device defined by @ref UART_NODE for
 * asynchronous operation, allocates the RX buffer, and registers
 * @ref uart_cb as the event callback.
 *
 * @retval 0        Success.
 * @retval -errno   Negative error code propagated from Zephyr drivers.
 */
int uart_init(void);

/**
 * @brief Test whether a complete frame has been received.
 *
 * @param[out] buf  Pointer to the internal RX buffer pointer.  Updated on
 *                  success.
 * @param[out] len  Pointer to an integer that receives the length of the frame
 *                  (bytes).
 *
 * @retval 0        No complete frame is available.
 * @retval 1        Frame available and @p buf / @p len were updated.
 * @retval <0       Negative error code.
 */
int uart_check_buffer(unsigned char **buf, int *len);

/**
 * @brief Clear the internal RX buffer.
 *
 * Applications should call this after they have processed the frame obtained
 * through @ref uart_check_buffer().
 */
void uart_resetRxBuffer(void);

/**
 * @brief Blocking transmit helper.
 *
 * @param[in] buf  Pointer to the data to send.
 * @param[in] len  Number of bytes to transmit.
 *
 * @return 0 on success, negative errno on failure.
 */
int uart_send(const uint8_t *buf, size_t len);

/**
 * @brief Block the calling thread until a frame is received.
 *
 * Internally this function waits on @ref uart_msgq, forwarding the timeout of
 * the queue call to the caller.
 */
void uart_wait_for_rx(void);

#endif /* UART_H_ */
