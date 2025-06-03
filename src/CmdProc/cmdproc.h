/**
 * @file cmdproc.h
 * @brief Header file for command processor module for a thermal process control system.
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

#ifndef CMD_PROC_H_
#define CMD_PROC_H_

/* Including libraries */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/**
 * @def UART_RX_SIZE
 * @brief Maximum size of the UART RX buffer.
 */
#define UART_RX_SIZE 20 	

/**
 * @def UART_TX_SIZE
 * @brief Maximum size of the UART TX buffer.
 */
#define UART_TX_SIZE 20 	

/**
 * @def HISTORY_SIZE
 * @brief Maximum size of history array of sensor values.
 */
#define HISTORY_SIZE 100    

/**
 * @def SOF_SYM
 * @brief Start of Frame Symbol.
 */
#define SOF_SYM '#'	        

/**
 * @def EOF_SYM
 * @brief End of Frame Symbol.
 */
#define EOF_SYM '!'         

/**
 * @def T_DIGITS
 * @brief Number of digits for temperature (-40,+99) °C.
 */
#define T_DIGITS 3          

/**
 * @def CS_DIGITS
 * @brief Number of digits for checksum (calculated with modulo 256).
 */
#define CS_DIGITS 3       

/**
 * @def RX_CMD_A_LEN
 * @brief Length of the received message = 6 (SOF_SYM + 'A' + 3 digit checksum + EOF_SYM).
 */
#define RX_CMD_A_LEN 6

/**
 * @def CMD_OK
 * @brief Command executed successfully.
 */
#define CMD_OK 0

/**
 * @def CMD_EMPTY_STRING
 * @brief Empty string error.
 */
#define CMD_EMPTY_STRING -1

/**
 * @def CMD_INVALID
 * @brief Invalid command error.
 */
#define CMD_INVALID -2

/**
 * @def CMD_CS_ERROR
 * @brief Checksum error.
 */
#define CMD_CS_ERROR -3

/**
 * @def CMD_BUFFER_FULL
 * @brief Buffer full error.
 */
#define CMD_BUFFER_FULL -4

/**
 * @def CMD_BUFFER_EMPTY
 * @brief Buffer empty error.
 */
#define CMD_BUFFER_EMPTY -5

/**
 * @def CMD_MISSING_SOF_ERROR
 * @brief Missing start of frame ('#') error.
 */
#define CMD_MISSING_SOF_ERROR -6

/**
 * @def CMD_MISSING_EOF_ERROR
 * @brief Missing start of frame ('!') error.
 */
#define CMD_MISSING_EOF_ERROR -7

/* Extern variables for unity tests */
extern unsigned char UARTRxBuffer[];
extern unsigned char rxBufLen; 

extern unsigned char UARTTxBuffer[];
extern unsigned char txBufLen;

extern unsigned int seed;

/* Function prototypes */

/**
 * @brief Processes the chars in the RX buffer looking for commands.
 *
 * Supported commands: \n
 * C: Reads the current temperature value from the sensor. \n
 * M: Sets the maximum allowed temperature. Format: #Mxxxyyy! where xxx is the temperature and yyy is the checksum. \n
 * S: Sets the controller parameters. Format: #Sxxx...xxxyyy! where xxx...xxx are the parameters and yyy is the checksum. \n
 *
 * @return CMD_OK (0): if a valid command was found and executed.
 *         CMD_EMPTY_STRING (-1): if empty string or incomplete command found.
 *         CMD_INVALID (-2): if an invalid command was found.
 *         CMD_CS_ERROR (-3): if a checksum error is detected (command not executed).
 *         CMD_BUFFER_FULL (-4): if the buffer is full.
 *         CMD_MISSING_SOF_ERROR (-6): SOF_SYM not sent via Rx buffer.
 *         CMD_MISSING_EOF_ERROR (-7): EOF_SYM not sent via Rx buffer.
 */

int cmdProcessor(void);


/** @brief Checks if data in the Rx buffer is valid by finding position index of SOF_SYM and EOF_SYM.
* 
* @param sofIndex Position of start of frame symbol inside the UART Rx buffer.
* @param eofIndex Position of end of frame symbol inside the UART Rx buffer.
*
* @return CMD_EMPTY_STRING (-1): if the Rx buffer is empty  
*         CMD_MISSING_SOF_ERROR (-6): SOF_SYM not sent via Rx buffer                          
*         CMD_MISSING_EOF_ERROR (-7): EOF_SYM not sent via Rx buffer        
* 		  CMD_OK (0): frame contains both SOF_SYM and EOF_SYM and is not empty                		
*/
int checkSofEof(int * sofIndex, int * eofIndex);

/**
 * @brief Adds a character to the RX buffer. \n
 *I.e., the reception of commands.
 * @param car Character to add.
 * @return CMD_OK (0) if success, CMD_BUFFER_FULL (-1) if buffer full.
 */
int rxChar(unsigned char car);

/**
 * @brief Adds a character to the TX buffer. \n
 * I.e., the tranmsisison of answers.
 *
 * @param car Character to add.
 * @return CMD_OK (0) if success, CMD_BUFFER_FULL (-1) if buffer full.
 */
int txChar(unsigned char car);

/**
 * @brief Resets the RX buffer.
 */
void resetRxBuffer(void);

/**
 * @brief Resets the TX buffer.
 */
void resetTxBuffer(void);

/**
 * @brief Gets the TX buffer content.
 *
 * @param buf Buffer to copy the TX buffer content to.
 * @param len Pointer to store the length of the TX buffer content.
 */
void getTxBuffer(unsigned char * buf, int * len);

/**
 * @brief Calculates the checksum of given payload defined from buffer for n number of bytes.
 *
 * @param buffer Buffer for which payload calculation is needed.
 * @param nbytes Number of bytes of the payload.
 * 
 * @returns Checksum value calculated for payload only (without SOF_SYM and EOF_SYM) calculated with modulo 256
 */
int calcChecksum(unsigned char * buffer, int nbytes);

/**
 * @brief Compares checksum from received frame with calculated checksum to check if message was received completely.
 *
 * @param sofIndex Position of start of frame symbol.
 * @param eofIndex Position of end of frame symbol.
 * 
 * @returns CMD_CS_ERROR (-3): if compared values are not the same.
 *          CMD_OK (0): Compared values are same. 
 * 
 */
int checkRxChecksum(int * sofIndex, int * eofIndex);

/**
 * @brief Adds a measured value to the history using a circular buffer.
 *
 * @param measuredValue Pointer to the measured value.
 * @param sensorType Type of sensor ('t' for temperature, 'h' for humidity, 'c' for CO2).
 * @return CMD_OK (0) if success, CMD_INVALID (-2) if invalid sensor type i.e. invalid command.
 */
int addInHistory(void *measuredValue, char sensorType);

/**
 * @brief Pseudo number generator using Linear Congruential Generator.
 *
 * @param min Minimum value.
 * @param max Maximum value.
 * @return Pseudo-random number in the range [min, max].
 */ 
int psrnd(signed int min, signed int max);

/**
 * @brief Generates a character array representation of a sensor value.
 *
 * @param flag Flag indicating the type of sensor ('t', 'h', 'c').
 * @param value Sensor value.
 * @param buffer Buffer to store the character array representation.
 */
void generateCharArray(char flag, int value, char* buffer);

#endif
