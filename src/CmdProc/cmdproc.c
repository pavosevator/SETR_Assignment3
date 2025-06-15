/**
 * @file cmdproc.c
 * @brief Command processor for UART-based control of the sensor node.
 *        Parses received frames, validates checksum, and executes commands:
 *          - 'M': set max temperature threshold
 *          - 'C': request current temperature
 *          - 'S': set duty cycle
 *        Uses SOF ('#') and EOF ('!') delimiters and a 3-digit checksum.
 *
 * Authors: Ivan PAVOSEVIC, Enzo DOS SANTOS
 */

#include "cmdproc.h"
#include "i2c.h"
#include "data.h"
#include "gpio.h"

// UART receive buffer and current length
unsigned char UARTRxBuffer[UART_RX_SIZE];
unsigned char rxBufLen = 0;

// UART transmit buffer and current length
unsigned char UARTTxBuffer[UART_TX_SIZE];
unsigned char txBufLen = 0;

/**
 * Main command processor routine.
 * - Scans RX buffer for a complete frame (#...!)
 * - Validates checksum
 * - Executes command based on command ID
 * - Builds and queues response frame in TX buffer
 * - Cleans up processed bytes from RX buffer
 * @returns CMD_OK or error code
 */
int cmdProcessor(void)
{
    int i;
    unsigned char sid;  // command ID character
    signed char temp;   // temperature value [-40, +99]

    // Working arrays for building response
    char tempChar[3];       // holds ASCII digits of temperature
    char checksumchar[3];   // holds ASCII digits of checksum

    // Indices of start-of-frame and end-of-frame markers
    int sofIndex;
    int eofIndex;

    // If no data in RX buffer, nothing to do
    if (checkSofEof(&sofIndex, &eofIndex) == CMD_EMPTY_STRING)
        return CMD_EMPTY_STRING;

    // Missing '#' start marker
    if (checkSofEof(&sofIndex, &eofIndex) == CMD_MISSING_SOF_ERROR)
        return CMD_MISSING_SOF_ERROR;

    // Missing '!' end marker
    if (checkSofEof(&sofIndex, &eofIndex) == CMD_MISSING_EOF_ERROR)
        return CMD_MISSING_EOF_ERROR;

    int frameLen, newLen;
    char val[4];  // temporary buffer for numeric parsing

    // Validate checksum of received frame
    if (checkRxChecksum(&sofIndex, &eofIndex) == CMD_OK) {
        // Determine command ID character (immediately after '#')
        sid = UARTRxBuffer[sofIndex + 1];
        switch (sid) {
            case 'M': // "Mnnn!": set maximum temperature threshold
                // Copy three ASCII digits into val[], append '\0'
                for (int j = 0; j < 3; j++) {
                    val[j] = UARTRxBuffer[sofIndex + 2 + j];
                }
                val[3] = '\0';
                // Convert string to integer
                {
                    int value = atoi(val);
                    // Protect shared state with mutex
                    k_mutex_lock(&ctrl_state.mutex, K_FOREVER);
                    ctrl_state.max_temp = value;
                    k_mutex_unlock(&ctrl_state.mutex);
                }
                // Build acknowledgement frame: #E0<checksum>!
                txChar('#');        // SOF
                txChar('E');
                txChar('0');        // "E0" = OK for 'M' command
                // Calculate checksum on payload "E0"
                snprintf(checksumchar, CS_DIGITS + 1,
                         "%03d", calcChecksum(UARTTxBuffer + 1, 2));
                // Append checksum digits
                for (int i = 0; i < CS_DIGITS; i++) {
                    txChar(checksumchar[i]);
                }
                txChar('!');        // EOF

                // Remove processed frame from RX buffer
                frameLen = eofIndex - sofIndex + 1;
                newLen = rxBufLen - frameLen;
                memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
                rxBufLen = newLen;
                memset(UARTRxBuffer + newLen, '0', frameLen);
                return CMD_OK;

            case 'C': // "C!": request current temperature
                {
                    uint8_t currTemp;
                    // Read sensor data under mutex
                    k_mutex_lock(&sensor_data.mutex, K_FOREVER);
                    currTemp = sensor_data.current_temp;
                    k_mutex_unlock(&sensor_data.mutex);
                    // Format temperature as signed char array
                    generateCharArray('t', currTemp, tempChar);
                }
                // Build response: #cTT<checksum>!
                txChar('#');
                txChar('c');     // lowercase 'c' indicates temperature response
                for (int j = 0; j < T_DIGITS; j++) {
                    txChar(tempChar[j]);
                }
                // Compute checksum of payload "cTT"
                snprintf(checksumchar, CS_DIGITS + 1,
                         "%03d", calcChecksum(UARTTxBuffer + 1,
                                 1 + T_DIGITS));
                for (int i = 0; i < CS_DIGITS; i++) {
                    txChar(checksumchar[i]);
                }
                txChar('!');

                // Clean up RX buffer
                frameLen = eofIndex - sofIndex + 1;
                newLen = rxBufLen - frameLen;
                memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
                rxBufLen = newLen;
                memset(UARTRxBuffer + newLen, '0', frameLen);
                return CMD_OK;

            case 'S': // "Snnn!": set duty cycle (0-100)
                // Parse duty cycle value
                for (int j = 0; j < 3; j++) {
                    val[j] = UARTRxBuffer[sofIndex + 2 + j];
                }
                val[3] = '\0';
                {
                    int duty = atoi(val);
                    // Validate range
                    if (duty < 0 || duty > 100) {
                        txChar('#'); txChar('E'); txChar('i');
                        // 'Ei' indicates invalid input
                    } else {
                        txChar('#'); txChar('E'); txChar('9');
                        // 'E9' indicates success for duty command
                    }
                }
                // Append checksum
                snprintf(checksumchar, CS_DIGITS + 1,
                         "%03d", calcChecksum(UARTTxBuffer + 1, 2));
                for (int i = 0; i < CS_DIGITS; i++) {
                    txChar(checksumchar[i]);
                }
                txChar('!');

                // Cleanup RX buffer
                frameLen = eofIndex - sofIndex + 1;
                newLen = rxBufLen - frameLen;
                memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
                rxBufLen = newLen;
                memset(UARTRxBuffer + newLen, '0', frameLen);
                return CMD_OK;

            default:
                // Unknown command: discard frame and report error
                frameLen = eofIndex - sofIndex + 1;
                newLen = rxBufLen - frameLen;
                memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
                rxBufLen = newLen;
                memset(UARTRxBuffer + newLen, '0', frameLen);
                return CMD_INVALID;
        }
    }

    // Checksum error: discard corrupted frame
    frameLen = eofIndex - sofIndex + 1;
    newLen = rxBufLen - frameLen;
    memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
    rxBufLen = newLen;
    memset(UARTRxBuffer + newLen, '0', frameLen);
    return CMD_CS_ERROR;
}

/**
 * Finds start-of-frame ('#') and end-of-frame ('!') in RX buffer.
 * @param sofIndex pointer to store index of '#'
 * @param eofIndex pointer to store index of '!'
 * @returns CMD_OK, CMD_EMPTY_STRING, CMD_MISSING_SOF_ERROR, or CMD_MISSING_EOF_ERROR
 */
int checkSofEof(int *sofIndex, int *eofIndex)
{
    if (rxBufLen == 0)
        return CMD_EMPTY_STRING;

    int sof = -1, eof = -1;
    // Search for SOF marker
    for (int j = 0; j < rxBufLen; j++) {
        if (UARTRxBuffer[j] == SOF_SYM) {
            sof = j;
            break;
        }
    }
    if (sof == -1)
        return CMD_MISSING_SOF_ERROR;

    // Search for EOF marker starting at SOF
    for (int j = sof; j < rxBufLen; j++) {
        if (UARTRxBuffer[j] == EOF_SYM) {
            eof = j;
            break;
        }
    }
    if (eof == -1)
        return CMD_MISSING_EOF_ERROR;

    *sofIndex = sof;
    *eofIndex = eof;
    return CMD_OK;
}

/**
 * Calculates modulo-256 checksum of nbytes in buffer.
 * @param buffer pointer to data bytes
 * @param nbytes number of bytes to include
 * @returns checksum value [0..255]
 */
int calcChecksum(unsigned char *buffer, int nbytes)
{
    int checksum = 0;
    for (int j = 0; j < nbytes; j++) {
        checksum += buffer[j];
    }
    return (checksum % 256);
}

/**
 * Verifies received checksum against calculated value.
 * @param sofIndex index of SOF in RX buffer
 * @param eofIndex index of EOF in RX buffer
 * @returns CMD_OK or CMD_CS_ERROR
 */
int checkRxChecksum(int *sofIndex, int *eofIndex)
{
    // Payload length excludes SOF, EOF, and 3 checksum chars
    int payloadLen = *eofIndex - CS_DIGITS - 1 - *sofIndex;
    int checksum = calcChecksum(UARTRxBuffer + *sofIndex + 1, payloadLen);
    char checksumchar[4];
    snprintf(checksumchar, sizeof(checksumchar), "%03d", checksum);

    // Compare each checksum digit
    for (int i = 0; i < strlen(checksumchar); i++) {
        if (checksumchar[i] != UARTRxBuffer[*eofIndex - CS_DIGITS + i]) {
            return CMD_CS_ERROR;
        }
    }
    return CMD_OK;
}

/**
 * Adds a received character to RX buffer if space available.
 */
int rxChar(unsigned char car)
{
    if (rxBufLen < UART_RX_SIZE) {
        UARTRxBuffer[rxBufLen++] = car;
        return 0;
    }
    return CMD_BUFFER_FULL;
}

/**
 * Queues a character into TX buffer if space available.
 */
int txChar(unsigned char car)
{
    if (txBufLen < UART_TX_SIZE) {
        UARTTxBuffer[txBufLen++] = car;
        return 0;
    }
    return CMD_BUFFER_FULL;
}

/**
 * Clears RX buffer.
 */
void resetRxBuffer(void)
{
    if (rxBufLen > 0) {
        memset(UARTRxBuffer, '0', UART_RX_SIZE);
        rxBufLen = 0;
    }
}

/**
 * Clears TX buffer.
 */
void resetTxBuffer(void)
{
    if (txBufLen > 0) {
        memset(UARTTxBuffer, '0', UART_TX_SIZE);
        txBufLen = 0;
    }
}

/**
 * Provides pointer to TX buffer and its length.
 */
void getTxBuffer(unsigned char **buf, int *len)
{
    *buf = UARTTxBuffer;
    *len = txBufLen;
}

/**
 * Formats numeric values into ASCII digit arrays based on flag:
 *  't': signed temperature ("+NN" or "-NN")
 *  'h': 3-digit humidity
 *  'c': 5-digit counter
 */
void generateCharArray(char flag, int value, char *buffer)
{
    switch (flag) {
        case 't':
            snprintf(buffer, 4, "%c%02d",
                     (value < 0 ? '-' : '+'), abs(value));
            break;
        default:
            buffer[0] = '0';
    }
}
