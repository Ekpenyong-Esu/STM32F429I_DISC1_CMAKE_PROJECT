/**
 * @file uart_blocking.c
 * @brief UART Blocking mode implementation for STM32F429I-DISC1
 */

#include "uart_blocking.h"

UART_Status_t UART_Blocking_Init(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        DEBUG_PRINT("Blocking UART handle or huart is NULL");
        return UART_ERROR;
    }

    handle->isInitialized = true;
    /* Nothing special needed for blocking mode */
    return UART_OK;
}

UART_Status_t UART_Blocking_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("Blocking UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit(handle->huart, data, size, timeout);

    if (status != HAL_OK) {
        DEBUG_PRINT("Blocking UART Transmit failed");
        return UART_ERROR;
    }

    return UART_OK;
}

UART_Status_t UART_Blocking_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("Blocking UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Receive(handle->huart, data, size, timeout);

    if (status != HAL_OK) {
        DEBUG_PRINT("Blocking UART Receive failed");
        return UART_ERROR;
    }

    return UART_OK;
}
