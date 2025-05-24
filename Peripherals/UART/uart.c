/**
 * @file uart.c
 * @brief Main UART implementation for STM32F429I-DISC1
 */

#include "uart.h"
#include "uart_config.h"
#include "uart_dma.h"
#include "uart_interrupt.h"
#include "uart_blocking.h"

/* Ensure uartHandle is declared globally */
extern UART_Handle_t uartHandle;



UART_Status_t UART_Init(UART_Handle_t* handle, const UART_Config_t* config)
{
    if (handle == NULL || config == NULL || config->instance == NULL) {
        DEBUG_PRINT("UART handle or config is NULL");
        return UART_ERROR;
    }

    /* Store configuration */
    handle->config = *config;

    /* Configure UART base settings */
    if (handle->huart == NULL) {
        DEBUG_PRINT("UART HAL handle is NULL");
        return UART_ERROR;
    }

    /* Configure UART base settings */
    handle->huart->Instance = config->instance;
    handle->huart->Init.BaudRate = config->baudRate;
    handle->huart->Init.WordLength = config->wordLength;
    handle->huart->Init.StopBits = config->stopBits;
    handle->huart->Init.Parity = config->parity;
    handle->huart->Init.Mode = UART_DEFAULT_MODE;
    handle->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    handle->huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(handle->huart) != HAL_OK) {
        DEBUG_PRINT("UART initialization failed");
        return UART_ERROR;
    }

    /* Initialize mode-specific functionality */
    UART_Status_t status = UART_ERROR;
    switch (config->mode) {
        case UART_MODE_DMA:
            status = UART_DMA_Init(handle);
            break;
        case UART_MODE_INTERRUPT:
            status = UART_IT_Init(handle);
            break;
        case UART_MODE_BLOCKING:
            status = UART_Blocking_Init(handle);
            break;
        default:
            DEBUG_PRINT("Invalid UART mode");
            return UART_ERROR;
    }

    if (status == UART_OK) {
        handle->isInitialized = true;
    }

    return status;
}

UART_Status_t UART_DeInit(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        DEBUG_PRINT("UART handle or huart is NULL");
        return UART_ERROR;
    }

    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        DEBUG_PRINT("UART deinitialization failed");
        return UART_ERROR;
    }

    handle->isInitialized = false;
    return UART_OK;
}

/* Helper function to handle UART mode-specific operations */
static UART_Status_t UART_HandleMode(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout, bool isTransmit)
{
    if (handle == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("UART handle, data is NULL or size is 0");
        return UART_ERROR;
    }

    if (!handle->isInitialized) {
        DEBUG_PRINT("UART not initialized");
        return UART_ERROR;
    }

    DEBUG_PRINT("UART mode: %d", handle->config.mode);
    switch (handle->config.mode) {
        case UART_MODE_DMA:
            return isTransmit ? UART_DMA_Transmit(handle, data, size, timeout) : UART_DMA_Receive(handle, data, size, timeout);
        case UART_MODE_INTERRUPT:
            return isTransmit ? UART_IT_Transmit(handle, data, size, timeout) : UART_IT_Receive(handle, data, size, timeout);
        case UART_MODE_BLOCKING:
            return isTransmit ? UART_Blocking_Transmit(handle, data, size, timeout) : UART_Blocking_Receive(handle, data, size, timeout);
        default:
            return UART_ERROR;
    }
}

UART_Status_t UART_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    return UART_HandleMode(handle, data, size, timeout, true);
}

UART_Status_t UART_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    return UART_HandleMode(handle, data, size, timeout, false);
}
