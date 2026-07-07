/**
 * @file uart_dma.c
 * @brief UART DMA mode implementation for STM32F429I-DISC1
 */

#include "uart_dma.h"
#include "uart_config.h"
#include "stm32f4xx_hal_dma.h"

/* External references */
extern UART_Handle_t uartHandle;  // Define this in uart.c

UART_Status_t UART_DMA_Init(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        DEBUG_PRINT("DMA UART handle or huart is NULL");
        return UART_ERROR;
    }

    /* Note: DMA hardware initialization is now handled in HAL_UART_MspInit() */
    /* This function only configures the application-level settings */

    /* For DMA mode, only enable IDLE interrupt for packet detection */
    /* Do NOT enable RXNE interrupt as it conflicts with DMA */
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);  // Keep error interrupt
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE); // Keep IDLE line detection

    /* Initialize ring buffer first */
    UART_RingBuffer_Init();

    handle->isInitialized = true;
    return UART_OK;
}


UART_Status_t UART_DMA_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("DMA UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    /* Reset transmission complete flag */
    txComplete = 0;

    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(handle->huart, data, size);
    if (status != HAL_OK) {
        DEBUG_PRINT("DMA UART Transmit failed");
        return UART_ERROR;
    }

     /* Wait for transmission complete if timeout is specified */
    if (timeout > 0) {
        uint32_t startTick = HAL_GetTick();
        while (!txComplete) {
            if ((HAL_GetTick() - startTick) > timeout) {
                DEBUG_PRINT("DMA UART Transmit timeout");
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}

UART_Status_t UART_DMA_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("DMA UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    if (size > RING_BUFFER_SIZE) {
        DEBUG_PRINT("Requested size exceeds ring buffer size");
        return UART_ERROR;
    }

    /* Reset reception complete flag */
    uartExampleRxComplete = 0;

    /* Use IDLE line detection for more responsive reception */
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_DMA(handle->huart, handle->rxBuffer, handle->rxSize);
    if (status != HAL_OK) {
        DEBUG_PRINT("DMA UART ReceiveToIdle failed: %d", status);
        /* Fallback to regular DMA receive if ReceiveToIdle fails */
        status = HAL_UART_Receive_DMA(handle->huart, handle->rxBuffer, handle->rxSize);
        if (status != HAL_OK) {
            DEBUG_PRINT("DMA UART Receive failed: %d", status);
            return UART_ERROR;
        }
    }

    /* Disable DMA Half Transfer interrupt to avoid conflicts */
    __HAL_DMA_DISABLE_IT(handle->huart->hdmarx, DMA_IT_HT);

    /* If timeout is specified, wait for completion or timeout */
    if (timeout > 0) {
        uint32_t startTick = HAL_GetTick();
        while (!uartExampleRxComplete) {
            if ((HAL_GetTick() - startTick) > timeout) {
                DEBUG_PRINT("DMA UART Receive timeout");
                return UART_TIMEOUT_ERROR;
            }

            /* Check if data is available in ring buffer */
            if (UART_RingBuffer_Receive(handle, data, size) == UART_OK) {
                return UART_OK;
            }
        }
    }

    return UART_OK;
}

/* DMA interrupt handlers */
void DMA2_Stream7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(uartHandle.huart->hdmatx);
}

void DMA2_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(uartHandle.huart->hdmarx);
}
