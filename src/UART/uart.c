#include "uart.h"
#include <zephyr/sys/ring_buffer.h>

static struct k_sem uart_rx_sem;

/* UART related variables */
const struct device *uart_dev = DEVICE_DT_GET(UART_NODE);
static uint8_t rx_buf[RXBUF_SIZE];      /* RX buffer, to store received data */
static uint8_t rx_chars[RXBUF_SIZE];    /* chars actually received  */
volatile int uart_rxbuf_nchar=0;        /* Number of chars currrntly on the rx buffer */

/* FIFO queue */
K_MSGQ_DEFINE(uart_msgq,sizeof(uint8_t), 32,4);

int err=0; /* Generic error variable */
uint8_t welcome_mesg[] = "UART demo: Type a few chars in a row and then pause for a little while ...\n\r"; 

uint8_t rep_mesg[MSG_BUF_SIZE];

/* Struct for UART configuration. If using default values (check devicetree info) is not needed) */
/* Dynamic configuration option, available if CONFIG_UART_USE_RUNTIME_CONFIGURE is ser (it is by defualt)*/
/* For static UART configuration it is recommended to use the devicetree editor (Actions tab) */
const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
}; 

int uart_init(){
     /* Check if uart device is open */
    if (!device_is_ready(uart_dev)) {
        printk("device_is_ready(uart) returned error! Aborting! \n\r");
        return FATAL_ERR;
    }
    printk("Radi init\n");
    /* Configure UART */
    err = uart_configure(uart_dev, &uart_cfg);
    if (err == -ENOSYS) { /* If invalid configuration */
        printk("uart_configure() error. Invalid configuration\n\r");
        return FATAL_ERR; 
    }

    /* Register callback */
    err = uart_callback_set(uart_dev, uart_cb, NULL);
    if (err) {
        printk("uart_callback_set() error. Error code:%d\n\r",err);
        return FATAL_ERR;
    }

        /* Enable data reception */
    err =  uart_rx_enable(uart_dev ,rx_buf,sizeof(rx_buf),RX_TIMEOUT);
    if (err) {
        printk("uart_rx_enable() error. Error code:%d\n\r",err);
        return FATAL_ERR;
    }

    /* Send a welcome message */ 
    /* Last arg is timeout. Only relevant if flow controll is used */
    err = uart_tx(uart_dev, welcome_mesg, sizeof(welcome_mesg), SYS_FOREVER_MS);
    if (err) {
        printk("uart_tx() error. Error code:%d\n\r",err);
        return FATAL_ERR;
    }

    printk("Start testing...\n");
    k_sem_init(&uart_rx_sem, 0, 1);
    return 0;

}

int uart_check_buffer(unsigned char **buf, int *len){
    //k_msleep(MAIN_SLEEP_TIME_MS);
        
    /* Print string received so far. */
    /* Very basic implementation, just for showing the use of the API */
    /* E.g. it does not prevent race conditions with the callback!!!!*/
    if(uart_rxbuf_nchar > 0) {
        int n = uart_rxbuf_nchar;
        rx_chars[uart_rxbuf_nchar] = 0; /* Terminate the string */
        uart_rxbuf_nchar = 0;           /* Reset counter */
            
        *buf = rx_chars;
        *len = n;
        return 0;
    }
    //printk(".\n");
}

void uart_resetRxBuffer(void)                     
{
    uart_rxbuf_nchar = 0;

}

int uart_send(const uint8_t *buf, size_t len)
{
    return uart_tx(uart_dev, buf, len, SYS_FOREVER_MS);
}

/* UART callback implementation */
/* Note that callback functions are executed in the scope of interrupt handlers. */
/* They run asynchronously after hardware/software interrupts and have a higher priority than tasks/threads */
/* Should be kept as short and simple as possible. Heavier processing should be deferred to a task with suitable priority*/
void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    int err;

    switch (evt->type) {
	
        case UART_TX_DONE:
		    printk("UART_TX_DONE event \n\r");
            break;

    	case UART_TX_ABORTED:
	    	printk("UART_TX_ABORTED event \n\r");
		    break;
		
	    case UART_RX_RDY:
		    printk("UART_RX_RDY event \n\r");
            for(int i = 0; i < evt->data.rx.len; i++){
                uint8_t b = rx_buf[evt->data.rx.offset + i];
                k_msgq_put(&uart_msgq, &b, K_NO_WAIT);
            }
            /* memcpy(&rx_chars[uart_rxbuf_nchar],&(rx_buf[evt->data.rx.offset]),evt->data.rx.len); 
            uart_rxbuf_nchar += evt->data.rx.len; */ 
            k_sem_give(&uart_rx_sem);          
		    break;

	    case UART_RX_BUF_RELEASED:
		    printk("UART_RX_BUF_RELEASED event \n\r");
		    break;
		
        case UART_RX_BUF_REQUEST:
		    printk("UART_RX_BUF_REQUEST event \n\r");
            break;
            
	    case UART_RX_DISABLED: 
            /* When the RX_BUFF becomes full RX is disabled automaticaly.  */
            /* It must be re-enabled manually for continuous reception */
            printk("UART_RX_DISABLED event \n\r");
		    err =  uart_rx_enable(uart_dev ,rx_buf,sizeof(rx_buf),RX_TIMEOUT);
            if (err) {
                printk("uart_rx_enable() error. Error code:%d\n\r",err);
                exit(FATAL_ERR);                
            }
		    break;

	    case UART_RX_STOPPED:
		    printk("UART_RX_STOPPED event \n\r");
		    break;
		
	    default:
            printk("UART: unknown event \n\r");
		    break;
    }

}

void uart_wait_for_rx(void){
    k_sem_take(&uart_rx_sem, K_FOREVER);
}