/**
 * @file uart_interrupt.c
 * @brief UART Interrupt mode implementation for STM32F429I-DISC1
 */

#include "uart_interrupt.h"

UART_Status_t UART_IT_Init(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        DEBUG_PRINT("UART handle or huart is NULL");
        return UART_ERROR;
    }
    handle->isInitialized = true;
    return UART_OK;
}

UART_Status_t UART_IT_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit_IT(handle->huart, data, size);

    if (status != HAL_OK) {
        DEBUG_PRINT("UART Transmit failed");
        return UART_ERROR;
    }

    /* Wait for transmission complete if timeout is not 0 */
    if (timeout) {
        uint32_t tickstart = HAL_GetTick();
        while (handle->huart->gState != HAL_UART_STATE_READY) {
            if ((HAL_GetTick() - tickstart) > timeout) {
                DEBUG_PRINT("UART Transmit timeout");
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}

UART_Status_t UART_IT_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Receive_IT(handle->huart, data, size);

    if (status != HAL_OK) {
        DEBUG_PRINT("UART Receive failed");
        return UART_ERROR;
    }

    /* Wait for reception complete if timeout is not 0 */
    if (timeout) {
        uint32_t tickstart = HAL_GetTick();
        while (handle->huart->RxState != HAL_UART_STATE_READY) {
            if ((HAL_GetTick() - tickstart) > timeout) {
                DEBUG_PRINT("UART Receive timeout");
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}
