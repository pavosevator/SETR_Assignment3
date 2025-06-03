/**
 * @file cmdproc.c
 * @brief Source file for command processor module for a thermal process control system.
 *
 * This module processes commands received via UART for a thermal process control system.
 * It is designed to handle various commands related to temperature regulation, including
 * setting and reading temperature values, configuring controller parameters, and managing
 * system status. The module ensures robust communication by validating command frames
 * and checksums, and it maintains a history of temperature readings for monitoring and
 * diagnostic purposes.
 *
 * @author Ivan PAVOSEVIC, Enzo DOS SANTOS.
 * @date 03 Jun 2025
 *
 * @section Overview
 * The command processor module is responsible for interpreting UART commands and executing
 * corresponding actions on the thermal control system. It supports a range of commands for
 * real-time temperature monitoring and system configuration. The module uses a circular buffer
 * to maintain a history of temperature readings, which can be useful for diagnostics and logging.
 *
 * @section Commands
 * The module supports the following commands:
 * - 'C': Reads the current temperature value from the sensor. Format: #Cyyy! where yyy is the checksum.
 * - 'M': Sets the maximum allowed temperature. Format: #Mxxxyyy! where xxx is the temperature and yyy is the checksum.
 * - 'S': Sets the controller parameters. Format: #Sxxx...xxxyyy! where xxx...xxx are the parameters and yyy is the checksum.
 *
 * @section Data Structures
 * The module uses the following key data structures:
 * - UARTRxBuffer: A buffer to store incoming UART data.
 * - UARTTxBuffer: A buffer to store outgoing UART data.
 * - tHistory: A circular buffer to store the history of temperature readings.
 *
 * @section Functions
 * The module includes the following key functions:
 * - cmdProcessor(): Processes the incoming commands and executes corresponding actions.
 * - checkSofEof(): Checks for the presence of start-of-frame (SOF) and end-of-frame (EOF) markers.
 * - calcChecksum(): Calculates the checksum for a given data buffer.
 * - checkRxChecksum(): Verifies the checksum of received commands.
 * - rxChar() and txChar(): Functions to receive and transmit characters via UART.
 * - addInHistory(): Adds a temperature reading to the history buffer.
 * - generateCharArray(): Converts numerical values to character arrays for transmission.
 *
 * @section Usage
 * To use this module, initialize the UART interface and call the cmdProcessor() function
 * periodically to process incoming commands. Ensure that the UART buffers are properly
 * managed to avoid overflows and data corruption.
 *
 */

#include "cmdproc.h"
#include "i2c.h"

/* Internal variables */
/* Used as part of the UART emulation */
unsigned char UARTRxBuffer[UART_RX_SIZE];
unsigned char rxBufLen = 0; 

unsigned char UARTTxBuffer[UART_TX_SIZE];
unsigned char txBufLen = 0; 

/* Used as part of storing history*/
static signed char tHistory[HISTORY_SIZE];
unsigned char tHistoryLen = 0;

/* 
 * cmdProcessor
 */ 
int cmdProcessor(void)
{
	int i;
	unsigned char sid;

	/* Measured values: temperature [-40, 99] °C*/
	signed char temp;

	/* Char arrays for output */
	char tempChar[3];
	char checksumchar[3];

	/* Pointers to start and end of frames*/
	int sofIndex;
	int eofIndex;

	if(checkSofEof(&sofIndex, &eofIndex) == CMD_EMPTY_STRING) 
		return CMD_EMPTY_STRING;  // empty string

	if(checkSofEof(&sofIndex, &eofIndex) == CMD_MISSING_SOF_ERROR) 
		return CMD_MISSING_SOF_ERROR; 	// # not found	
	
	if(checkSofEof(&sofIndex, &eofIndex) == CMD_MISSING_EOF_ERROR) 
		return CMD_MISSING_EOF_ERROR; 	// ! not found	
	
	int frameLen, newLen;
	/* Check if frame has valid checksum*/
	if(checkRxChecksum(&sofIndex, &eofIndex) == CMD_OK) {
		

		switch(UARTRxBuffer[sofIndex+1]) { 
			
			case 'C': // Request for current temperature
            {
                temp = (signed char)psrnd(-40, 99); // Simulate temperature reading
                addInHistory(&temp, 't');

                txChar('#');
                txChar('c');
                generateCharArray('t', temp, tempChar);
                for (int j = 0; j < 3; j++)
                {
                    txChar(tempChar[j]);
                }

                snprintf(checksumchar, 4, "%03d", calcChecksum(UARTTxBuffer + 1, 3));
                for (int j = 0; j < 3; j++)
                {
                    txChar(checksumchar[j]);
                }
                txChar('!');

                frameLen = eofIndex - sofIndex + 1;
                newLen = rxBufLen - frameLen;
                memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
                rxBufLen = newLen;
                memset(UARTRxBuffer + newLen, '0', frameLen);

                return CMD_OK;
            }

            case 'M': // Set maximum temperature
            {
                if ((eofIndex - sofIndex + 1) != RX_CMD_M_LEN)
                    return CMD_INVALID;

                char maxTempStr[4];
                strncpy(maxTempStr, (char *)(UARTRxBuffer + sofIndex + 2), 3);
                maxTempStr[3] = '\0';
                maxTemp = atoi(maxTempStr);

                // Logic to set maximum temperature
                txChar('#');
                txChar('E');
                txChar('0');

                snprintf(checksumchar, 4, "%03d", calcChecksum(UARTTxBuffer + 1, 2));
                for (int j = 0; j < 3; j++)
                {
                    txChar(checksumchar[j]);
                }
                txChar('!');

                frameLen = eofIndex - sofIndex + 1;
                newLen = rxBufLen - frameLen;
                memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
                rxBufLen = newLen;
                memset(UARTRxBuffer + newLen, '0', frameLen);

                return CMD_OK;
            }

            case 'S': // Set controller parameters
            {
                // Logic to set controller parameters
                txChar('#');
                txChar('E');
                txChar('0');

                snprintf(checksumchar, 4, "%03d", calcChecksum(UARTTxBuffer + 1, 2));
                for (int j = 0; j < 3; j++)
                {
                    txChar(checksumchar[j]);
                }
                txChar('!');

                frameLen = eofIndex - sofIndex + 1;
                newLen = rxBufLen - frameLen;
                memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
                rxBufLen = newLen;
                memset(UARTRxBuffer + newLen, '0', frameLen);

                return CMD_OK;
            }

            default:
                frameLen = eofIndex - sofIndex + 1;
                newLen = rxBufLen - frameLen;
                memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
                rxBufLen = newLen;
                memset(UARTRxBuffer + newLen, '0', frameLen);
                return CMD_INVALID;
        }
		
		
	}
	// Checksum is wrong, delete data from buffer since its corrupted
	frameLen = eofIndex - sofIndex + 1;
	newLen = rxBufLen - frameLen;
	memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
	rxBufLen = newLen;
	memset(UARTRxBuffer	+ newLen, '0', frameLen);
	return CMD_CS_ERROR; // checksum not aligning

}

/* Checks if the Rx data is ready for processing*/
int checkSofEof(int * sofIndex, int * eofIndex)
{	
	/* Detect empty cmd string */
	if(rxBufLen == 0)
		return CMD_EMPTY_STRING; 
	
	int sof = -1;
	int eof = -1;

	/* Find index of SOF */
	for(int j = 0; j < rxBufLen; j++) {	
		if(UARTRxBuffer[j] == SOF_SYM) {
			sof = j;
			break;
		}
	}
	if(sof == -1) return CMD_MISSING_SOF_ERROR; // '#' not found

	/* Find index of EOF */
	for(int j = sof; j < rxBufLen; j++) {	
		if(UARTRxBuffer[j] == EOF_SYM) {
			eof = j;
			break;
		}
	}
	if(eof == -1) return CMD_MISSING_EOF_ERROR; // '!' not found
	
	*sofIndex = sof;
	*eofIndex = eof;

	return CMD_OK; // command is valid

}

/* 
 * calcChecksum
 */ 
int calcChecksum(unsigned char * buffer, int nbytes) 
{
	/* Here you are supposed to compute the modulo 256 checksum */
	/* of the first n bytes of buf. Then you should convert the */
	/* checksum to ascii (3 digitas/chars) and compare each one */
	/* of these digits/characters to the ones in the RxBuffer,	*/
	/* positions nbytes, nbytes + 1 and nbytes +2. 				*/
	
	/* That is your work to do. In this example I just assume 	*/
	/* that the checksum is always OK.							*/
	int checksum=0;
	for(int j = 0; j<nbytes; j++)
	{
		checksum+=buffer[j];
	}

	return (checksum % 256);		
}

int checkRxChecksum(int * sofIndex, int * eofIndex)
{
	// calculate checksum
	int payloadLen;
	payloadLen = *eofIndex - CS_DIGITS - 1 - *sofIndex; // length of the payload
	int checksum = calcChecksum(UARTRxBuffer + *sofIndex + 1, payloadLen);
	char checksumchar[4];
	snprintf(checksumchar, 4, "%03d", checksum);

	for(int i = 0; i < strlen(checksumchar); i++) {
		if(checksumchar[i] != UARTRxBuffer[*eofIndex - 3 + i]) {
			return CMD_CS_ERROR; // digits are not aligning
		}
	}
	return CMD_OK; // test is passed
}

/*
 * rxChar
 */
int rxChar(unsigned char car)
{
	/* If rxbuff not full add char to it */
	if (rxBufLen < UART_RX_SIZE) {
		UARTRxBuffer[rxBufLen] = car;
		rxBufLen += 1;
		return 0;		
	}
	return CMD_BUFFER_FULL;
}

/*
 * txChar
 */
int txChar(unsigned char car)
{
	/* If rxbuff not full add char to it */
	if (txBufLen < UART_TX_SIZE) {
		UARTTxBuffer[txBufLen] = car;
		txBufLen += 1;
		return 0;		
	} else {
		/* If cmd string full return error */
		return CMD_BUFFER_FULL;
	} 
}

/*
 * resetRxBuffer
 */
void resetRxBuffer(void)
{
	if(rxBufLen > 0) {
		memset(UARTRxBuffer, '0', UART_TX_SIZE);
		rxBufLen = 0;	
	}
			
	return;
}

/*
 * resetTxBuffer
 */
void resetTxBuffer(void)
{	
	if(txBufLen > 0) {
		memset(UARTTxBuffer, '0', UART_TX_SIZE);
		txBufLen = 0;	
	}
	
	return;
}

/*
 * getTxBuffer
 */
void getTxBuffer(unsigned char * buf, int * len)
{
	*len = txBufLen;
	if(txBufLen > 0) {
		memcpy(buf,UARTTxBuffer,*len);
	}		
	return;
}

// Pseudo number generator: Linear Congruential Generator
int psrnd(int min,int max) 
{
    seed = (25173 * seed + 13849) % 65536; // xn = (a * xn-1 + c) % m, m = 2^16, 
    
	// Scale the result to the desired range
    int range = max - min + 1;
    int scaled = (seed % range) + min;

	return scaled;

}

/*
	Separate function for adding values in history using circular buffer
*/
int addInHistory(void *measuredValue, char sensorType)
{
    if (sensorType == 't')
    {
        tHistory[tHistoryLen] = *(signed char *)measuredValue;
        tHistoryLen = (tHistoryLen + 1) % HISTORY_SIZE;
        return CMD_OK;
    }
    return CMD_INVALID;
}

void generateCharArray(char flag, int value, char *buffer)
{
    switch (flag)
    {
        case 't':
            snprintf(buffer, 4, "%c%02d", (value < 0 ? '-' : '+'), abs(value));
            break;
        default:
            buffer[0] = '\0';
    }
	return;
}
