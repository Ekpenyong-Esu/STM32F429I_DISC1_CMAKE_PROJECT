/**
 * @file uart_interrupt.c
 * @brief UART Interrupt mode implementation for STM32F429I-DISC1
 */

#include "uart_interrupt.h"
#include "uart_config.h"

/* External reference to global UART handle */
extern UART_Handle_t uartHandle;

UART_Status_t UART_IT_Init(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        DEBUG_PRINT("UART handle or huart is NULL");
        return UART_ERROR;
    }

    /* Initialize ring buffer for continuous reception */
    UART_RingBuffer_Init();

    /* Enable UART IDLE line detection - helps with command detection */
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);


    handle->isInitialized = true;
    return UART_OK;
}

UART_Status_t UART_IT_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    txComplete = 0;
    HAL_StatusTypeDef status = HAL_UART_Transmit_IT(handle->huart, (uint8_t*)data, size);
    if (status != HAL_OK) {
        DEBUG_PRINT("UART Transmit failed: %d", status);
        return UART_ERROR;
    }

    if (timeout > 0) {
        uint32_t tickstart = HAL_GetTick();
        while (!txComplete) {
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

    uartExampleRxComplete = 0;
    /* Use HAL_UARTEx_ReceiveToIdle_IT for better command reception with IDLE detection */
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_IT(handle->huart, data, size);
    if (status != HAL_OK) {
        DEBUG_PRINT("UART ReceiveToIdle failed: %d", status);
        /* Fall back to regular interrupt reception if ReceiveToIdle fails */
        status = HAL_UART_Receive_IT(handle->huart, data, size);
        if (status != HAL_OK) {
            DEBUG_PRINT("Regular UART Receive_IT also failed: %d", status);
            return UART_ERROR;
        }
    }


    if (timeout > 0) {
        uint32_t tickstart = HAL_GetTick();
        while (!uartExampleRxComplete) {
            if ((HAL_GetTick() - tickstart) > timeout) {
                DEBUG_PRINT("UART Receive timeout");
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}

/* UART interrupt handlers */
void USART1_IRQHandler(void)
{
    if (uartHandle.huart && uartHandle.huart->Instance == USART1) {
        HAL_UART_IRQHandler(uartHandle.huart);
    }
}
