/**
 * @file uart_example.c
 * @brief Example usage of UART communication with different modes
 */

#include "uart_example.h"
#include "uart_config.h"
#include <string.h>
#include <stdio.h>

/* Constants for UART configuration */
#define STATUS_MSG_SIZE          256   /* Maximum size for status message */
#define DEFAULT_BAUD_RATE       115200 /* Default UART baud rate */

/* Static variables */
static UART_Handle_t uartHandle;
static uint8_t rxBuffer[RX_BUFFER_SIZE];
static uint8_t txBuffer[TX_BUFFER_SIZE];
static volatile uint8_t rxComplete = 0;
static volatile uint8_t txComplete = 0;

/* Buffer management for improved reception */
static uint16_t rxIndex = 0;
static uint8_t cmdBuffer[RX_BUFFER_SIZE];

/* Welcome message */
static const char* welcomeMsg = ANSI_COLOR_CYAN
    "\r\n=================================\r\n"
    "UART Communication Example\r\n"
    "Available commands:\r\n"
    "  help   - Show this help\r\n"
    "  status - Show UART status\r\n"
    "  dma    - Send using DMA\r\n"
    "  int    - Send using Interrupts\r\n"
    "  block  - Send using Blocking mode\r\n"
    "  echo   - Echo back received text\r\n"
    "================================="
    ANSI_COLOR_RESET "\r\n> ";

UART_Status_t UART_Example_Init(void)
{
    /* Configure UART */
    UART_Config_t config = {
        .instance = USART1,       /* Use USART1. Can be changed to USART2, USART3, etc. */
        .baudRate = DEFAULT_BAUD_RATE,
        .wordLength = UART_WORDLENGTH_8B,
        .stopBits = UART_STOPBITS_1,
        .parity = UART_PARITY_NONE,
        .mode = UART_MODE_DMA  /* Start with DMA mode */
    };

    /* Initialize UART */
    uartHandle.rxBuffer = rxBuffer;
    uartHandle.txBuffer = txBuffer;
    uartHandle.rxSize = RX_BUFFER_SIZE;
    uartHandle.txSize = TX_BUFFER_SIZE;

    UART_Status_t status = UART_Init(&uartHandle, &config);
    if (status != UART_OK) {
        return status;
    }

    /* Send welcome message */
    return UART_Example_SendMessage(welcomeMsg);
}

/* Override weak callback definitions from uart_dma.c */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == uartHandle.huart) {
        txComplete = 1;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

    if (huart == uartHandle.huart) {
        rxComplete = 1;
    }
}

/* DMA Completion Callbacks */
void UART_DMA_TxCpltCallback(UART_Handle_t* handle)
{
    (void)handle;  /* Prevent unused parameter warning */
    txComplete = 1;
}

void UART_DMA_RxCpltCallback(UART_Handle_t* handle)
{
    rxComplete = 1;

    /* Process received byte if it's a complete command (CR or LF) */
    if (rxBuffer[0] == '\r' || rxBuffer[0] == '\n') {
        /* Process the command */
        UART_Example_ProcessCommand((char*)rxBuffer);

        /* Clear buffer */
        memset(rxBuffer, 0, RX_BUFFER_SIZE);
    }

    /* Start receiving again */
    if (handle != NULL) {
        UART_Receive(handle, rxBuffer, 1, 0);
    }
}

/**
 * @brief Efficient UART receive callback handling multiple characters
 * @param handle UART handle
 */
void UART_Example_ProcessCommand_Efficient(UART_Handle_t* handle)
{
    rxComplete = 1;

    /* Copy received byte to command buffer */
    cmdBuffer[rxIndex] = rxBuffer[0];

    /* Check for line ending or buffer full */
    if (rxBuffer[0] == '\r' || rxBuffer[0] == '\n' || rxIndex >= RX_BUFFER_SIZE - 1) {
        /* Null terminate the string */
        cmdBuffer[rxIndex] = '\0';

        /* Only process if we have actual content */
        if (rxIndex > 0) {
            UART_Example_ProcessCommand((char*)cmdBuffer);
        }

        /* Reset buffer index */
        rxIndex = 0;
        /* Clear buffers */
        memset(cmdBuffer, 0, RX_BUFFER_SIZE);
        memset(rxBuffer, 0, RX_BUFFER_SIZE);
    } else {
        /* Move to next position if not at end */
        rxIndex++;
    }

    /* Start receiving next character */
    if (handle != NULL) {
        UART_Receive(handle, rxBuffer, 1, 0);
    }
}

/**
 * @brief Processes received UART commands
 * @param cmd The command string to process
 * @return UART_Status_t Result of command processing
 */
UART_Status_t UART_Example_ProcessCommand(const char* cmd)
{
    /* Remove any CR/LF from the command */
    char cleanCmd[RX_BUFFER_SIZE];
    strncpy(cleanCmd, cmd, RX_BUFFER_SIZE - 1);
    cleanCmd[RX_BUFFER_SIZE - 1] = '\0';

    /* Remove trailing newline or carriage return */
    char* endPtr = strchr(cleanCmd, '\r');
    if (endPtr != NULL) {
        *endPtr = '\0';
    }
    endPtr = strchr(cleanCmd, '\n');
    if (endPtr != NULL) {
        *endPtr = '\0';
    }

    /* Process commands */
    if (strcmp(cleanCmd, CMD_HELP) == 0) {
        return UART_Example_SendMessage(welcomeMsg);
    }

    if (strcmp(cleanCmd, CMD_STATUS) == 0) {
        char statusMsg[STATUS_MSG_SIZE];
        snprintf(statusMsg, sizeof(statusMsg),
            ANSI_COLOR_GREEN "\r\nUART Status:\r\n"
            "Mode: %s\r\n"
            "Baud Rate: %u\r\n"
            "Word Length: %d bits\r\n"
            "Stop Bits: %s\r\n"
            "Parity: %s\r\n"
            ANSI_COLOR_RESET "\r\n> ",
            (uartHandle.config.mode == UART_MODE_DMA) ? "DMA" :
            (uartHandle.config.mode == UART_MODE_INTERRUPT) ? "Interrupt" : "Blocking",
            (unsigned int)uartHandle.config.baudRate,
            (uartHandle.config.wordLength == UART_WORDLENGTH_8B) ? 8 : 9,
            (uartHandle.config.stopBits == UART_STOPBITS_1) ? "1" : "2",
            (uartHandle.config.parity == 0) ? "None" :
            (uartHandle.config.parity == 1) ? "Even" : "Odd"
        );
        return UART_Example_SendMessage(statusMsg);
    }

    if (strcmp(cleanCmd, CMD_DMA) == 0) {
        /* Switch to DMA mode */
        uartHandle.config.mode = UART_MODE_DMA;
        UART_Status_t status = UART_Init(&uartHandle, &uartHandle.config);
        if (status != UART_OK) {
            return status;
        }
        return UART_Example_SendMessage(ANSI_COLOR_GREEN "Switched to DMA mode\r\n" ANSI_COLOR_RESET "> ");
    }

    if (strcmp(cleanCmd, CMD_INTERRUPT) == 0) {
        /* Switch to Interrupt mode */
        uartHandle.config.mode = UART_MODE_INTERRUPT;
        UART_Status_t status = UART_Init(&uartHandle, &uartHandle.config);
        if (status != UART_OK) {
            return status;
        }
        return UART_Example_SendMessage(ANSI_COLOR_GREEN "Switched to Interrupt mode\r\n" ANSI_COLOR_RESET "> ");
    }

    if (strcmp(cleanCmd, CMD_BLOCKING) == 0) {
        /* Switch to Blocking mode */
        uartHandle.config.mode = UART_MODE_BLOCKING;
        UART_Status_t status = UART_Init(&uartHandle, &uartHandle.config);
        if (status != UART_OK) {
            return status;
        }
        return UART_Example_SendMessage(ANSI_COLOR_GREEN "Switched to Blocking mode\r\n" ANSI_COLOR_RESET "> ");
    }

    if (strncmp(cleanCmd, "echo ", 5) == 0) {
        /* Echo command - send back the received text */
        const char* textToEcho = cleanCmd + 5;  /* Skip "echo " prefix */
        
        /* Format echo message */
        char echoMsg[STATUS_MSG_SIZE];
        snprintf(echoMsg, sizeof(echoMsg), 
                ANSI_COLOR_GREEN "Echo: %s\r\n" ANSI_COLOR_RESET "> ", 
                textToEcho);
        
        return UART_Example_SendMessage(echoMsg);
    }

    /* Unknown command */
    return UART_Example_SendMessage(ANSI_COLOR_RED "Unknown command. Type 'help' for available commands.\r\n"
                                  ANSI_COLOR_RESET "> ");
}

/**
 * @brief Send message using current UART mode
 * @param msg Message to send
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Example_SendMessage(const char* msg)
{
    /* Validate input */
    if (msg == NULL || uartHandle.huart == NULL) {
        return UART_ERROR;
    }

    /* Get message length */
    uint16_t length = strlen(msg);
    if (length == 0 || length >= TX_BUFFER_SIZE - 1) {  /* Leave room for null terminator */
        return UART_ERROR;
    }

    /* Reset completion flag */
    txComplete = 0;

    /* Copy message to transmit buffer with null termination */
    memcpy(txBuffer, msg, length);
    txBuffer[length] = '\0';  /* Ensure null termination */

    /* Transmit based on current mode */
    return UART_Transmit(&uartHandle, txBuffer, length, UART_TIMEOUT);
}


/* Add this to your main.c or wherever you handle UART */
void UART_Example_MainLoop(void)
{
    /* Initialize UART Example */
    if (UART_Example_Init() != UART_OK) {
        /* Handle error */
        return;
    }

    while (1) {
        /* Your main loop code */
        HAL_Delay(1);  /* Give some time to RTOS if used */
    }
}
