/**
 * @file uart_example.c
 * @brief Example usage of UART communication with different modes
 */

#include "uart_example.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_uart.h"
#include "uart.h"
#include "uart_config.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "uart_blocking.h"

/* Constants for UART configuration */
#define STATUS_MSG_SIZE          256   /* Maximum size for status message */
#define DEFAULT_BAUD_RATE       115200 /* Default UART baud rate */
#define UART_INIT_DELAY          100   /* Delay after UART initialization in ms */
#define UART_POLLING_DELAY       10    /* Delay between polling cycles in ms */




UART_HandleTypeDef huart1;  /* UART handle for USART1 */
UART_Handle_t uartHandle;

static uint8_t rxBuffer[RX_BUFFER_SIZE];
static uint8_t txBuffer[TX_BUFFER_SIZE];
static uint8_t singleBuffer[SINGLE_CHAR_BUFFER_SIZE];
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

    /* Initialize UART handle structure with default mode */
    memset(&uartHandle, 0, sizeof(UART_Handle_t));

    /* Initialize buffers */
    memset(rxBuffer, 0, RX_BUFFER_SIZE);
    memset(txBuffer, 0, TX_BUFFER_SIZE);
    memset(singleBuffer, 0, SINGLE_CHAR_BUFFER_SIZE);
    memset(cmdBuffer, 0, RX_BUFFER_SIZE);

    rxIndex = 0;
    rxComplete = 0;
    txComplete = 0;
}

UART_Status_t UART_Example_Init(UART_Mode_t mode)
{
    DEBUG_PRINT("Initializing UART with mode: %d", mode);
    UART_Example_InitStructures();

    /* Configure UART */
    UART_Config_t config = {
        .instance = USART1,       /* Use USART1. Can be changed to USART2, USART3, etc. */
        .baudRate = DEFAULT_BAUD_RATE,
        .wordLength = UART_WORDLENGTH_8B,
        .stopBits = UART_STOPBITS_1,
        .parity = UART_PARITY_NONE,
        .mode = mode  /* Use the mode passed as parameter */
    };

    DEBUG_PRINT("Config created with mode: %d", config.mode);

    /* Initialize UART */
    uartHandle.huart = &huart1;
    uartHandle.rxBuffer = (mode == UART_MODE_DMA || mode == UART_MODE_INTERRUPT) ? rxBuffer : singleBuffer; // Use singleBuffer for interrupt/blocking mode
    uartHandle.txBuffer = txBuffer;
    uartHandle.rxSize = (mode == UART_MODE_DMA || mode == UART_MODE_INTERRUPT) ? RX_BUFFER_SIZE : SINGLE_CHAR_BUFFER_SIZE; // Use singleBuffer for interrupt/blocking mode, RX_BUFFER_SIZE;
    uartHandle.txSize = TX_BUFFER_SIZE;
    uartHandle.config = config;  // Store config in handle

    UART_Status_t status = UART_Init(&uartHandle, &config);
    if (status != UART_OK) {
        DEBUG_PRINT("UART initialization failed");
        return status;
    }

    if (config.mode == UART_MODE_DMA) {
        status = UART_Receive(&uartHandle, uartHandle.rxBuffer, uartHandle.rxSize, 0);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to start DMA reception");
            return status;
        }
    } else if (config.mode == UART_MODE_INTERRUPT) {
        // Start interrupt mode with larger buffer using IDLE detection
        status = UART_Receive(&uartHandle, uartHandle.rxBuffer, uartHandle.rxSize, 0);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to start interrupt reception");
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

    if (handle->config.mode == UART_MODE_DMA) {
        uint16_t bytesAvailable = RingBuffer_Available(&rxRingBuffer);
        if (bytesAvailable == 0) {
            DEBUG_PRINT("Ring buffer empty");
            return;
        }

        uint8_t tempBuf[RX_BUFFER_SIZE];
        if (UART_RingBuffer_Receive(handle, tempBuf, bytesAvailable) != UART_OK) {
            DEBUG_PRINT("Failed to get data from ring buffer");
            return;
        }

        for (uint16_t i = 0; i < bytesAvailable; i++) {
            if (rxIndex < RX_BUFFER_SIZE - 1) {
                cmdBuffer[rxIndex++] = tempBuf[i];
                if (tempBuf[i] == '\r' || tempBuf[i] == '\n' || rxIndex >= RX_BUFFER_SIZE - 1) {
                    cmdBuffer[rxIndex] = '\0';
                    if (rxIndex > 0) {
                        UART_Example_ProcessCommand((char*)cmdBuffer);
                    }
                    rxIndex = 0;
                    memset(cmdBuffer, 0, RX_BUFFER_SIZE);
                }
            } else {
                rxIndex = 0;
                memset(cmdBuffer, 0, RX_BUFFER_SIZE);
            }
        }
    } else if (handle->config.mode == UART_MODE_INTERRUPT) {

        uint16_t bytesAvailable = RingBuffer_Available(&rxRingBuffer);
        if (bytesAvailable == 0) {
            // No data available in the ring buffer
            return;
        }

        uint8_t tempBuf[RX_BUFFER_SIZE];
        // Get data from ring buffer
        if (UART_RingBuffer_Receive(handle, tempBuf, bytesAvailable) != UART_OK) {
            DEBUG_PRINT("Failed to get data from ring buffer");
            return;
        }

        // Process received data
        for (uint16_t i = 0; i < bytesAvailable; i++) {
            if (rxIndex < RX_BUFFER_SIZE - 1) {
                cmdBuffer[rxIndex++] = tempBuf[i];

                if (tempBuf[i] == '\r' || tempBuf[i] == '\n' || rxIndex >= RX_BUFFER_SIZE - 1) {
                    cmdBuffer[rxIndex] = '\0';
                    if (rxIndex > 0) {
                        UART_Example_ProcessCommand((char*)cmdBuffer);
                    }
                    // Reset buffer after processing
                    rxIndex = 0;
                    memset(cmdBuffer, 0, RX_BUFFER_SIZE);
                }
            } else {
                rxIndex = 0;
                memset(cmdBuffer, 0, RX_BUFFER_SIZE);
            }
        }
    } else if (handle->config.mode == UART_MODE_BLOCKING) {
        uint8_t tempBuffer[8] = {0}; // Read multiple bytes at once for efficiency
        uint16_t bytesToRead = 8;

        // Use a shorter timeout for better responsiveness
        UART_Status_t status = UART_Blocking_Receive(handle, tempBuffer, bytesToRead, UART_TIMEOUT);
        if (status != UART_OK) {
            DEBUG_PRINT("Blocking receive failed");
            return;
        }
        // Process any received bytes, even on timeout
        for (uint16_t i = 0; i < bytesToRead && tempBuffer[i] != 0; i++) {
            uint8_t receivedByte = tempBuffer[i];
            if (rxIndex < RX_BUFFER_SIZE - 1) {
                cmdBuffer[rxIndex++] = receivedByte;
                if (receivedByte == '\r' || receivedByte == '\n' || rxIndex >= RX_BUFFER_SIZE - 1) {
                    cmdBuffer[rxIndex] = '\0';
                    if (rxIndex > 0) {
                        UART_Example_ProcessCommand((char*)cmdBuffer);
                    }
                    rxIndex = 0;
                    memset(cmdBuffer, 0, RX_BUFFER_SIZE);
                }
            } else {
                // Buffer overflow protection
                rxIndex = 0;
                memset(cmdBuffer, 0, RX_BUFFER_SIZE);
            }
        }
    }
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

    char cleanCmd[RX_BUFFER_SIZE];
    size_t cmdLen = strlen(cmd);
    size_t cleanIndex = 0;

    for (size_t i = 0; i < cmdLen && cleanIndex < RX_BUFFER_SIZE - 1; i++) {
        if (cmd[i] != '\r' && cmd[i] != '\n') {
            cleanCmd[cleanIndex++] = cmd[i];
        }
    }
    cleanCmd[cleanIndex] = '\0';

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
        return UART_Example_SendMessage(statusMsg);
    }

    if (strcmp(cleanCmd, CMD_DMA) == 0) {
        UART_Status_t status = UART_DeInit(&uartHandle);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to deinitialize UART");
            return status;
        }

        /* Use UART_Example_Init which will handle the full initialization sequence */
        status = UART_Example_Init(UART_MODE_DMA);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to initialize UART in DMA mode");
            return status;
        }

        return UART_Example_SendMessage(ANSI_COLOR_GREEN "Switched to DMA mode\r\n" ANSI_COLOR_RESET "> ");
    }

    if (strcmp(cleanCmd, CMD_INTERRUPT) == 0) {
        UART_Status_t status = UART_DeInit(&uartHandle);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to deinitialize UART");
            return status;
        }

        /* Use UART_Example_Init which will handle the full initialization sequence */
        status = UART_Example_Init(UART_MODE_INTERRUPT);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to initialize UART in Interrupt mode");
            return status;
        }

        return UART_Example_SendMessage(ANSI_COLOR_GREEN "Switched to Interrupt mode\r\n" ANSI_COLOR_RESET "> ");
    }

    if (strcmp(cleanCmd, CMD_BLOCKING) == 0) {
        UART_Status_t status = UART_DeInit(&uartHandle);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to deinitialize UART");
            return status;
        }

        /* Use UART_Example_Init which will handle the full initialization sequence */
        status = UART_Example_Init(UART_MODE_BLOCKING);
        if (status != UART_OK) {
            DEBUG_PRINT("Failed to initialize UART in Blocking mode");
            return status;
        }

        return UART_Example_SendMessage(ANSI_COLOR_GREEN "Switched to Blocking mode\r\n" ANSI_COLOR_RESET "> ");
    }

    if (strcmp(cleanCmd, CMD_ECHO) == 0) {
        char echoMsg[STATUS_MSG_SIZE];
        snprintf(echoMsg, sizeof(echoMsg),
                ANSI_COLOR_GREEN "Echo: %s\r\n" ANSI_COLOR_RESET "> ",
                CMD_ECHO);
        return UART_Example_SendMessage(echoMsg);
    }

    if (strcmp(cleanCmd, "help") == 0) {
        return UART_Example_SendMessage(welcomeMsg);
    }

    char unknownCmdMsg[STATUS_MSG_SIZE];
    snprintf(unknownCmdMsg, sizeof(unknownCmdMsg), ANSI_COLOR_RED "Unknown command: %s\r\n" ANSI_COLOR_RESET "> ", cleanCmd);
    return UART_Example_SendMessage(unknownCmdMsg);
}

/**
 * @brief Send message using current UART mode
 * @param msg Message to send
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Example_SendMessage(const char* msg)
{
    if (msg == NULL || uartHandle.huart == NULL) {
        DEBUG_PRINT("UART handle or message is NULL");
        return UART_ERROR;
    }

    uint16_t length = strlen(msg);
    if (length == 0 || length >= TX_BUFFER_SIZE - 1) {
        DEBUG_PRINT("Message length is invalid");
        return UART_ERROR;
    }

    txComplete = 0;
    memcpy(txBuffer, msg, length);
    txBuffer[length] = '\0';

    UART_Status_t status = UART_Transmit(&uartHandle, uartHandle.txBuffer, length, UART_TIMEOUT);
    if (status != UART_OK) {
        DEBUG_PRINT("Send message failed: %d", status);
    }

    return status;
}

/* HAL callbacks are now implemented in uart.c to avoid multiple definitions */
void UART_Example_MainLoop(void)
{

    if (UART_Example_Init(UART_MODE_INTERRUPT) != UART_OK) {
        DEBUG_PRINT("UART Example initialization failed");
        return;
    }

    DEBUG_PRINT("UART Example main loop started - Mode: %s",
                uartHandle.config.mode == UART_MODE_DMA ? "DMA" :
                uartHandle.config.mode == UART_MODE_INTERRUPT ? "IT" : "BLOCKING");

    HAL_Delay(UART_INIT_DELAY); // Allow more time for initialization
    while (1) {
        UART_Example_PreProcess(&uartHandle);
        HAL_Delay(UART_POLLING_DELAY); // Small delay to prevent overwhelming the system
    }
}
