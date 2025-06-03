/**
 * @file cmdproc.c
 * @brief Source file for command processor module for smart sensor node.
 *
 * This module processes commands received via UART for a smart sensor node
 * that measures temperature, relative humidity, and CO2 levels. 
 * 
 * @author  Ivan PAVOSEVIC, Enzo DOS SANTOS.
 * @date 08 Apr 2025
 */
#include "cmdproc.h"
#include "i2c.h"
#include "data.h"


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
			
			case 'A': /*  reads the real-time values of the variables provided by the sensor */
				if((eofIndex - sofIndex + 1)  != RX_CMD_A_LEN) {
					return CMD_INVALID;
				}
				/* Sending of the data */
				/* Emulate pseudo random generation of sensor outputs */
				temp = (signed char)psrnd(-50,60);

				/* Store read values in history */
				addInHistory(&temp, 't');

				/* Use txChar func() */
				// Start of frame
				txChar('#');

				// Start of response
				txChar('a');

				// Send temperature data
				txChar('t');
				for(int j = 0; j < T_DIGITS; j++){
					txChar(tempChar[j]);
				}

				// Send checksum
				snprintf(checksumchar, CS_DIGITS + 1, "%03d", calcChecksum(UARTTxBuffer + 1, 1)); // what if buffer is not start of frame + 1 
				for(int j = 0; j < CS_DIGITS; j++){
					txChar(checksumchar[j]);
				}

				// End of frame
				txChar('!');

				// Clean Rx buffer from the last command
				frameLen = eofIndex - sofIndex + 1;
				newLen = rxBufLen - frameLen;
				
				memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
				rxBufLen = newLen;

				memset(UARTRxBuffer + newLen, '0', frameLen);

				return CMD_OK;

			case 'M':		
				/* Command "M" detected. */
				/* #Mxxxyyy! - Set maximum temperature to “xxx” (in oC). “yyy” is the checksum */   
				char outputChar[6];

				/* Sending of the data */
				txChar('#');

				/* Start of response */
				txChar('E');
				txChar('0'); // no error
				
				/* Implement here control logic for setting the temperature*/
				/* ************************IMPORTANT*********************************/

				/* Send checksum */
				snprintf(checksumchar, CS_DIGITS + 1, "%03d", calcChecksum(UARTTxBuffer + 1, strlen(UARTTxBuffer) + 2 )); // two because of 'E' and '0'
				for(int i = 0; i < CS_DIGITS; i++) {
					txChar(checksumchar[i]);
				}
				
				/* End of frame */
				txChar('!');

				frameLen = eofIndex - sofIndex + 1;
				newLen = rxBufLen - frameLen;
				
				memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
				rxBufLen = newLen;

				// Clear the rest of the buffer
				memset(UARTRxBuffer + newLen, '0', frameLen);

				return CMD_OK;
			case 'C': // Request for current temperature
				unsigned char historyChar[20];
				
				// Initialize index for historyChar
				int historyIndex = 0;
				
				int8_t temperature_copy;

    			k_mutex_lock(&sensor_data.mutex, K_FOREVER);
    			temperature_copy = sensor_data.temperature;
    			k_mutex_unlock(&sensor_data.mutex);

				generateCharArray('t', temperature_copy, tempChar);


				txChar('#');
				txChar('c');
				for(int j = 0; j < T_DIGITS; j++){
					txChar(tempChar[j]);
				}

				// send one by one the last 20 results
				for(int i = 0; i < 20 /* one value is 3 or 5 chars depends on mesaurement */; i++) {
					txChar(historyChar[i]);
				}

				/* Send checksum */
				snprintf(checksumchar, CS_DIGITS + 1, "%03d", calcChecksum(UARTTxBuffer + 1, strlen(UARTTxBuffer) + 2 )); // two because of 'p' and 'sid'
				for(int i = 0; i < CS_DIGITS; i++) {
					txChar(checksumchar[i]);
				}
				txChar('!'); 
				return CMD_OK;
			case 'R': // reset the history 
				memset(tHistory	, '\0', HISTORY_SIZE);
				txChar('#');
				txChar('r');
				/* Send checksum */
				snprintf(checksumchar, CS_DIGITS + 1, "%03d", calcChecksum(UARTTxBuffer + 1, 1 ));
				for(int i = 0; i < CS_DIGITS; i++) {
					txChar(checksumchar[i]);
				}
				txChar('!');

				return CMD_OK;			
			default:
				/* If code reaches this place, the command is not recognized */
				// delete leftover of command
				frameLen = eofIndex - sofIndex + 1;
				newLen = rxBufLen - frameLen;
				memmove(UARTRxBuffer, UARTRxBuffer + frameLen, newLen);
				rxBufLen = newLen;
				memset(UARTRxBuffer	+ newLen, '0', frameLen);
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

/*
	Separate function for adding values in history using circular buffer
*/
int addInHistory(void *measuredValue, char sensorType) 
{
	switch (sensorType) {
        case 't': // Temperature
            tHistory[tHistoryLen] = *(signed char *)measuredValue; // Cast to signed char
            tHistoryLen = (tHistoryLen + 1) % HISTORY_SIZE; // Move to the next position in a circular manner
            return CMD_OK;
        default:
            // Invalid sensor type
            return CMD_INVALID;
    }
}

void generateCharArray(char flag, int value, char* buffer) {
    switch(flag) {
        case 't':
		/* sign + two digits + \0 = 4 bytes*/
            snprintf(buffer, 4, "%c%02d", (value < 0 ? '-' : '+'), abs(value));
            break;
        case 'h':
		/* three digits + \0 = 4 bytes*/
            snprintf(buffer, 4, "%03d", value);
            break;
        case 'c':
		/* five digits + \0 = 6 bytes*/
            snprintf(buffer, 6, "%05d", value);
            break;
        default:
            buffer[0] = '0';
    }
	return;
}
