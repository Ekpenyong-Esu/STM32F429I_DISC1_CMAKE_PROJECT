/**
 * @file uart_example.h
 * @brief Example usage of UART communication with different modes
 */

#ifndef UART_EXAMPLE_H
#define UART_EXAMPLE_H

#include "uart.h"

/* ANSI Color Codes */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

/* Buffer sizes */
#define RX_BUFFER_SIZE     256
#define TX_BUFFER_SIZE     256

/* Command definitions */
#define CMD_HELP           "help"
#define CMD_STATUS         "status"
#define CMD_DMA            "dma"
#define CMD_INTERRUPT      "int"
#define CMD_BLOCKING       "block"
#define CMD_ECHO          "echo"      /* New echo command */

/**
 * @brief Initialize the UART example
 * @return UART_Status_t Status of initialization
 */
UART_Status_t UART_Example_Init(void);

/**
 * @brief Process received command
 * @param cmd Command string
 * @return UART_Status_t Status of command processing
 */
UART_Status_t UART_Example_ProcessCommand(const char* cmd);

/**
 * @brief Example of DMA mode transmission
 * @param data Data to transmit
 * @return UART_Status_t Status of transmission
 */
UART_Status_t UART_Example_DMAMode(const char* data);

/**
 * @brief Example of Interrupt mode transmission
 * @param data Data to transmit
 * @return UART_Status_t Status of transmission
 */
UART_Status_t UART_Example_InterruptMode(const char* data);

/**
 * @brief Example of Blocking mode transmission
 * @param data Data to transmit
 * @return UART_Status_t Status of transmission
 */
UART_Status_t UART_Example_BlockingMode(const char* data);

/**
 * @brief Send a message using current UART mode
 * @param msg Message to send
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Example_SendMessage(const char* msg);

/**
 * @brief Main loop for UART example
 * This function should be called in the main loop to handle UART operations
 */
void UART_Example_MainLoop(void);

#endif /* UART_EXAMPLE_H */
