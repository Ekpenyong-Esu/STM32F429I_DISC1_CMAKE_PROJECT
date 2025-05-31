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

    /* Disable all interrupts to ensure blocking mode */
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_IDLE);

    handle->isInitialized = true;
    /* Nothing special needed for blocking mode */
    return UART_OK;
}

UART_Status_t UART_Blocking_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("Blocking UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit(handle->huart, (uint8_t*)data, size, timeout);
    if (status != HAL_OK) {
        DEBUG_PRINT("Blocking UART Transmit failed: %d", status);
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

    uint16_t rxSize = 0;
    HAL_StatusTypeDef status = HAL_ERROR;

    // Use ReceiveToIdle which will return when data is received or IDLE line is detected
    status = HAL_UARTEx_ReceiveToIdle(handle->huart, data, size, &rxSize, timeout);

    if (status != HAL_OK) {
        if (status == HAL_TIMEOUT) {
            // If it's a timeout but we did receive some data, consider it a success
            if (rxSize > 0) {
                return UART_OK;
            }
            DEBUG_PRINT("Blocking UART Receive timeout: %d bytes received", rxSize);
            return UART_TIMEOUT_ERROR;
        }
    }

    // If we received less than requested but more than 0, still consider it a success
    if (rxSize < size && rxSize > 0) {
        // Fill remaining buffer with zeros if we got less than requested
        memset(data + rxSize, 0, size - rxSize);
    }

    return UART_OK;
}
