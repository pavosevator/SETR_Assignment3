#include "unity.h"
#include "i2c.h"
#include "uart.h"
#include "heater.h"
#include "cmdproc.h"
#include "data.h"

#include <string.h>

// === Configuration ===
extern unsigned char UARTRxBuffer[];
extern unsigned char UARTTxBuffer[];
extern unsigned char rxBufLen;
extern unsigned char txBufLen;


void setUp(void) {
    resetRxBuffer();
    resetTxBuffer();
    k_mutex_init(&sensor_data.mutex);
    sensor_data.current_temp = 25;
    k_mutex_init(&ctrl_state.mutex);
    ctrl_state.max_temp = 60;
}

void tearDown(void) {
    resetRxBuffer();
    resetTxBuffer();
}


void sendFrameToRxBuffer(const char *frame) {
    for (size_t i = 0; i < strlen(frame); i++) {
        rxChar(frame[i]);
    }
}

void buildCommand(char command, const char *payload, char *frame) {
    char temp[16];
    snprintf(temp, sizeof(temp), "%c%s", command, payload);
    int checksum = calcChecksum((unsigned char*)temp, strlen(temp));
    char checksumStr[4];
    snprintf(checksumStr, sizeof(checksumStr), "%03d", checksum);
    sprintf(frame, "#%s%s!", temp, checksumStr);
}


// === UART test ===
void test_uart_init_should_return_OK(void) {
    TEST_ASSERT_EQUAL(0, uart_init());
}

void test_uart_send_single_char(void) {
    resetTxBuffer();
    TEST_ASSERT_EQUAL(0, txChar('A'));
    unsigned char *buf;
    int len;
    getTxBuffer(&buf, &len);
    TEST_ASSERT_EQUAL(1, len);
    TEST_ASSERT_EQUAL('A', buf[0]);
}

void test_uart_tx_overflow_should_fail(void) {
    resetTxBuffer();
    for (int i = 0; i < UART_TX_SIZE; i++) {
        TEST_ASSERT_EQUAL(0, txChar('X'));
    }
    TEST_ASSERT_EQUAL(CMD_BUFFER_FULL, txChar('Y'));
}

void test_uart_rx_should_store_data_correctly(void) {
    resetRxBuffer();
    TEST_ASSERT_EQUAL(0, rxChar('Z'));
    TEST_ASSERT_EQUAL(1, rxBufLen);
    TEST_ASSERT_EQUAL('Z', UARTRxBuffer[0]);
}

void test_uart_rx_overflow_should_fail(void) {
    resetRxBuffer();
    for (int i = 0; i < UART_RX_SIZE; i++) {
        TEST_ASSERT_EQUAL(0, rxChar('M'));
    }
    TEST_ASSERT_EQUAL(CMD_BUFFER_FULL, rxChar('N'));
}


// === HEATER test ===
void test_heater_init_should_return_OK(void) {
    TEST_ASSERT_EQUAL(0, heater_init());
}

// === I2C test ===
void test_i2c_init_should_return_OK(void) {
    TEST_ASSERT_EQUAL(0, i2c_init());
}

void test_i2c_read_temperature_should_fail_on_null_pointer(void) {
    TEST_ASSERT_EQUAL(-5, i2c_read_temperature(NULL));
}

// === CMDPROC test ===
void test_cmdproc_set_max_temp_should_succeed(void) {
    char frame[16];
    buildCommand('M', "045", frame);
    sendFrameToRxBuffer(frame);
    TEST_ASSERT_EQUAL(CMD_OK, cmdProcessor());
    TEST_ASSERT_EQUAL(45, ctrl_state.max_temp);
}

void test_cmdproc_invalid_checksum_should_fail(void) {
    sendFrameToRxBuffer("#M045999!"); // Wrong checksum
    TEST_ASSERT_EQUAL(CMD_CS_ERROR, cmdProcessor());
}

void test_cmdproc_temperature_request_should_succeed(void) {
    char frame[16];
    buildCommand('C', "", frame);
    sendFrameToRxBuffer(frame);
    TEST_ASSERT_EQUAL(CMD_OK, cmdProcessor());

    unsigned char *txbuf;
    int len;
    getTxBuffer(&txbuf, &len);
    TEST_ASSERT_EQUAL('#', txbuf[0]);
    TEST_ASSERT_EQUAL('c', txbuf[1]);
}

void test_cmdproc_set_duty_cycle_valid(void) {
    char frame[16];
    buildCommand('S', "075", frame);
    sendFrameToRxBuffer(frame);
    TEST_ASSERT_EQUAL(CMD_OK, cmdProcessor());

    unsigned char *txbuf;
    int len;
    getTxBuffer(&txbuf, &len);
    TEST_ASSERT_EQUAL('9', txbuf[2]); // 'E9'
}

void test_cmdproc_set_duty_cycle_invalid(void) {
    char frame[16];
    buildCommand('S', "200", frame);
    sendFrameToRxBuffer(frame);
    TEST_ASSERT_EQUAL(CMD_OK, cmdProcessor());

    unsigned char *txbuf;
    int len;
    getTxBuffer(&txbuf, &len);
    TEST_ASSERT_EQUAL('i', txbuf[2]); // 'Ei'
}

// === Main Test Runner ===
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_uart_init_should_return_OK);
    RUN_TEST(test_uart_send_single_char);
    RUN_TEST(test_uart_tx_overflow_should_fail);
    RUN_TEST(test_uart_rx_should_store_data_correctly);
    RUN_TEST(test_uart_rx_overflow_should_fail);
    RUN_TEST(test_heater_init_should_return_OK);
    RUN_TEST(test_i2c_init_should_return_OK);
    RUN_TEST(test_i2c_read_temperature_should_fail_on_null_pointer); 
    RUN_TEST(test_cmdproc_set_max_temp_should_succeed);
    RUN_TEST(test_cmdproc_invalid_checksum_should_fail);
    RUN_TEST(test_cmdproc_temperature_request_should_succeed);
    RUN_TEST(test_cmdproc_set_duty_cycle_valid);
    RUN_TEST(test_cmdproc_set_duty_cycle_invalid);

    return UNITY_END();
}
