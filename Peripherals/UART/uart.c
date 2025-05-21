/**
 * @file uart.c
 * @brief Main UART implementation for STM32F429I-DISC1
 */

#include "uart.h"
#include "uart_config.h"
#include "uart_dma.h"
#include "uart_interrupt.h"
#include "uart_blocking.h"


UART_Status_t UART_Init(UART_Handle_t* handle, const UART_Config_t* config)
{
    if (handle == NULL || config == NULL || config->instance == NULL) {
        return UART_ERROR;
    }

    /* Store configuration */
    handle->config = *config;

    /* Configure UART */
    handle->huart->Instance = config->instance;
    handle->huart->Init.BaudRate = config->baudRate;
    handle->huart->Init.WordLength = config->wordLength;
    handle->huart->Init.StopBits = config->stopBits;
    handle->huart->Init.Parity = config->parity;
    handle->huart->Init.Mode = UART_DEFAULT_MODE;
    handle->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    handle->huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(handle->huart) != HAL_OK) {
        return UART_ERROR;
    }

    /* Initialize mode-specific functionality */
    switch (config->mode) {
        case UART_MODE_DMA:
            return UART_DMA_Init(handle);
        case UART_MODE_INTERRUPT:
            return UART_IT_Init(handle);
        case UART_MODE_BLOCKING:
            return UART_Blocking_Init(handle);
        default:
            return UART_ERROR;
    }
}

UART_Status_t UART_DeInit(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        return UART_ERROR;
    }

    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        return UART_ERROR;
    }

    handle->isInitialized = false;
    return UART_OK;
}

UART_Status_t UART_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL) {
        return UART_ERROR;
    }

    switch (handle->config.mode) {
        case UART_MODE_DMA:
            return UART_DMA_Transmit(handle, data, size, timeout);
        case UART_MODE_INTERRUPT:
            return UART_IT_Transmit(handle, data, size, timeout);
        case UART_MODE_BLOCKING:
            return UART_Blocking_Transmit(handle, data, size, timeout);
        default:
            return UART_ERROR;
    }
}

UART_Status_t UART_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL) {
        return UART_ERROR;
    }

    switch (handle->config.mode) {
        case UART_MODE_DMA:
            return UART_DMA_Receive(handle, data, size, timeout);
        case UART_MODE_INTERRUPT:
            return UART_IT_Receive(handle, data, size, timeout);
        case UART_MODE_BLOCKING:
            return UART_Blocking_Receive(handle, data, size, timeout);
        default:
            return UART_ERROR;
    }
}
