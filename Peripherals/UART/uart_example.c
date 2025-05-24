/**
 * @file uart_example.c
 * @brief Example usage of UART communication with different modes
 */

#include "uart_example.h"
#include "stm32f4xx_hal_uart.h"
#include "uart_config.h"
#include "uart_blocking.h"
#include <string.h>
#include <stdio.h>

/* Constants for UART configuration */
#define STATUS_MSG_SIZE          256   /* Maximum size for status message */
#define DEFAULT_BAUD_RATE       115200 /* Default UART baud rate */

UART_HandleTypeDef huart1;  /* UART handle for USART1 */

UART_Handle_t uartHandle;
static uint8_t rxBuffer[RX_BUFFER_SIZE];
static uint8_t txBuffer[TX_BUFFER_SIZE];
volatile uint8_t rxComplete = 0;
volatile uint8_t txComplete = 0;

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


static void UART_Example_InitStructures(void)
{
    /* Initialize HAL UART handle */
    memset(&huart1, 0, sizeof(UART_HandleTypeDef));

    /* Initialize UART handle structure */
    memset(&uartHandle, 0, sizeof(UART_Handle_t));

    /* Initialize buffers */
    memset(rxBuffer, 0, RX_BUFFER_SIZE);
    memset(txBuffer, 0, TX_BUFFER_SIZE);
    memset(cmdBuffer, 0, RX_BUFFER_SIZE);

    rxIndex = 0;
    rxComplete = 0;
    txComplete = 0;
}

UART_Status_t UART_Example_Init(void)
{
    UART_Example_InitStructures();

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
    uartHandle.huart = &huart1;
    uartHandle.rxBuffer = rxBuffer;
    uartHandle.txBuffer = txBuffer;
    uartHandle.rxSize = RX_BUFFER_SIZE;
    uartHandle.txSize = TX_BUFFER_SIZE;
    uartHandle.config = config;  // Store config in handle

    /* Initialize ring buffer first */
    UART_RingBuffer_Init();

    UART_Status_t status = UART_Init(&uartHandle, &config);
    if (status != UART_OK) {
        DEBUG_PRINT("UART initialization failed");
        return status;
    }


    if (config.mode == UART_MODE_DMA || config.mode == UART_MODE_INTERRUPT) {

        /* Start reception with full buffer size */
        UART_Status_t status = UART_Receive(&uartHandle, rxBuffer, RX_BUFFER_SIZE, 0);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to start UART reception");
            return status;
        }
    }

    /* Send welcome message */
    return UART_Example_SendMessage(welcomeMsg);
}

/**
 * @brief Efficient UART receive callback handling multiple characters
 * @param handle UART handle
 */
void UART_Example_PreProcess(UART_Handle_t* handle)
{
    if (handle == NULL || handle->rxBuffer == NULL) {
        DEBUG_PRINT("Invalid handle or buffer");
        return;
    }

    uint8_t tempBuf[RX_BUFFER_SIZE];
    uint16_t bytesAvailable = RingBuffer_Available(&rxRingBuffer);

    if (bytesAvailable == 0) {
        DEBUG_PRINT("No data available");
        return;
    }

    /* Get data from ring buffer */
    if (UART_RingBuffer_Receive(handle, tempBuf, bytesAvailable) != UART_OK) {
        DEBUG_PRINT("Failed to get data from ring buffer");
        return;
    }

    /* Process each received byte */
    for (uint16_t i = 0; i < bytesAvailable; i++) {
        /* Add byte to command buffer if there's space */
        if (rxIndex < RX_BUFFER_SIZE - 1) {
            cmdBuffer[rxIndex++] = tempBuf[i];

            /* Check for line ending */
            if (tempBuf[i] == '\r' || tempBuf[i] == '\n' || rxIndex >= RX_BUFFER_SIZE - 1) {
                /* Null terminate the string */
                cmdBuffer[rxIndex] = '\0';

                /* Only process if we have actual content */
                if (rxIndex > 0) {
                    DEBUG_PRINT("Received command: %s", cmdBuffer);
                    UART_Example_ProcessCommand((char*)cmdBuffer);
                }

                /* Reset buffer index */
                rxIndex = 0;
                memset(cmdBuffer, 0, RX_BUFFER_SIZE);
            }
        } else {
            /* Buffer full, reset */
            rxIndex = 0;
            memset(cmdBuffer, 0, RX_BUFFER_SIZE);
        }
    }

    rxComplete = 0;
}

/**
 * @brief Processes received UART commands
 * @param cmd The command string to process
 * @return UART_Status_t Result of command processing
 */
UART_Status_t UART_Example_ProcessCommand(const char* cmd)
{
    if (cmd == NULL) {
        DEBUG_PRINT("NULL command received");
        return UART_ERROR;
    }

    /* Remove CR/LF and create clean command */
    char cleanCmd[RX_BUFFER_SIZE];
    size_t cmdLen = strlen(cmd);
    size_t cleanIndex = 0;

    /* Copy command while removing CR/LF */
    for (size_t i = 0; i < cmdLen && cleanIndex < RX_BUFFER_SIZE - 1; i++) {
        if (cmd[i] != '\r' && cmd[i] != '\n') {
            cleanCmd[cleanIndex++] = cmd[i];
        }
    }
    cleanCmd[cleanIndex] = '\0';

    /* Ignore empty commands */
    if (cleanIndex == 0) {
        DEBUG_PRINT("Empty command received");
        return UART_OK;
    }

    DEBUG_PRINT("Processing command: %s", cleanCmd);

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
        DEBUG_PRINT("Sending status message");
        return UART_Example_SendMessage(statusMsg);
    }

    if (strcmp(cleanCmd, CMD_DMA) == 0) {
        /* Switch to DMA mode */
        uartHandle.config.mode = UART_MODE_DMA;
        UART_Status_t status = UART_Init(&uartHandle, &uartHandle.config);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to switch to DMA mode");
            return status;
        }
        DEBUG_PRINT("Switching to DMA mode");

        /* Start reception with full buffer */
        status = UART_Receive(&uartHandle, rxBuffer, RX_BUFFER_SIZE, 0);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to start DMA reception");
            return status;
        }

        return UART_Example_SendMessage(ANSI_COLOR_GREEN "Switched to DMA mode\r\n" ANSI_COLOR_RESET "> ");
    }

    if (strcmp(cleanCmd, CMD_INTERRUPT) == 0) {
        /* Switch to Interrupt mode */
        uartHandle.config.mode = UART_MODE_INTERRUPT;
        UART_Status_t status = UART_Init(&uartHandle, &uartHandle.config);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to switch to Interrupt mode");
            return status;
        }

        /* Start reception with full buffer */
        status = UART_Receive(&uartHandle, rxBuffer, RX_BUFFER_SIZE, 0);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to start interrupt reception");
            return status;
        }

        DEBUG_PRINT("Switching to Interrupt mode");
        return UART_Example_SendMessage(ANSI_COLOR_GREEN "Switched to Interrupt mode\r\n" ANSI_COLOR_RESET "> ");
    }

    if (strcmp(cleanCmd, CMD_BLOCKING) == 0) {
        /* Switch to Blocking mode */
        uartHandle.config.mode = UART_MODE_BLOCKING;
        UART_Status_t status = UART_Init(&uartHandle, &uartHandle.config);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to switch to Blocking mode");
            return status;
        }
        DEBUG_PRINT("Switching to Blocking mode");
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
    return UART_Example_SendMessage(ANSI_COLOR_RED "Unknown command\r\n" ANSI_COLOR_RESET "> ");
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
        DEBUG_PRINT("UART handle or message is NULL");
        return UART_ERROR;
    }

    /* Get message length */
    uint16_t length = strlen(msg);
    if (length == 0 || length >= TX_BUFFER_SIZE - 1) {  /* Leave room for null terminator */
        DEBUG_PRINT("Message length is invalid");
        return UART_ERROR;
    }

    /* Reset completion flag */
    txComplete = 0;

    /* Copy message to transmit buffer with null termination */
    memcpy(txBuffer, msg, length);
    txBuffer[length] = '\0';  /* Ensure null termination */

    UART_Status_t status = UART_Transmit(&uartHandle, txBuffer, length, UART_TIMEOUT);
    if (status != UART_OK) {
        DEBUG_PRINT("Send message failed: %d", status);
    }

    return status;
}

/* Enhanced callback for UART transmission complete */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == uartHandle.huart) {
        txComplete = 1;
        DEBUG_PRINT("UART transmission complete");
        /* Add logging or additional actions if needed */
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == uartHandle.huart) {

        if (huart->ErrorCode & HAL_UART_ERROR_ORE) {
        DEBUG_PRINT("UART Overrun Error");
        }
        if (huart->ErrorCode & HAL_UART_ERROR_NE) {
            DEBUG_PRINT("UART Noise Error");
        }
        if (huart->ErrorCode & HAL_UART_ERROR_FE) {
            DEBUG_PRINT("UART Frame Error");
        }
        if (huart->ErrorCode & HAL_UART_ERROR_PE) {
            DEBUG_PRINT("UART Parity Error");
        }

        /* Clear error flags */
        __HAL_UART_CLEAR_PEFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        __HAL_UART_CLEAR_OREFLAG(huart);

        /* Restart reception */
        rxComplete = 0;
        rxIndex = 0;
        memset(rxBuffer, 0, RX_BUFFER_SIZE);
        memset(cmdBuffer, 0, RX_BUFFER_SIZE);
        /* Restart reception based on mode */
        if (uartHandle.config.mode == UART_MODE_DMA) {
            HAL_UART_Receive_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        } else if (uartHandle.config.mode == UART_MODE_INTERRUPT) {
            HAL_UART_Receive_IT(huart, uartHandle.rxBuffer, 1);
        }
    }
}
void UART_Example_MainLoop(void)
{
    if (UART_Example_Init() != UART_OK) {
        DEBUG_PRINT("UART Example initialization failed");
        return;
    }

    while (1) {
        /* Periodically process received data */
        if (rxComplete) {
            UART_Example_PreProcess(&uartHandle);
            rxComplete = 0;  // Reset the flag after processing
        }

        HAL_Delay(PROCESS_INTERVAL_MS);  // Defined as 10ms in uart_example.h
    }
}
