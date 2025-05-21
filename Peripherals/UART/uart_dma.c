/**
 * @file uart_dma.c
 * @brief UART DMA mode implementation for STM32F429I-DISC1
 */

#include "uart_dma.h"
#include "uart.h"
#include "uart_config.h"

/* Global DMA handles */
DMA_HandleTypeDef hdma_uart1_tx;
DMA_HandleTypeDef hdma_uart1_rx;

UART_Status_t UART_DMA_Init(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        return UART_ERROR;
    }

    /* Enable DMA clock */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* Configure TX DMA */
    hdma_uart1_tx.Instance = UART_DMA_TX_STREAM;
    hdma_uart1_tx.Init.Channel = UART_DMA_TX_CHANNEL;
    hdma_uart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart1_tx.Init.Mode = DMA_NORMAL;
    hdma_uart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_uart1_tx);

    /* Configure RX DMA */
    hdma_uart1_rx.Instance = UART_DMA_RX_STREAM;
    hdma_uart1_rx.Init.Channel = UART_DMA_RX_CHANNEL;
    hdma_uart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart1_rx.Init.Mode = DMA_NORMAL;
    hdma_uart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_uart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_DMA_Init(&hdma_uart1_rx);

    /* Link DMA with UART */
    __HAL_LINKDMA(handle->huart, hdmatx, hdma_uart1_tx);
    __HAL_LINKDMA(handle->huart, hdmarx, hdma_uart1_rx);

    /* DMA interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

    return UART_OK;
}

/**
 * @brief DMA TX complete callback
 */
__weak void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument warning */
    UNUSED(huart);
}

/**
 * @brief DMA RX complete callback
 */
__weak void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    /* Prevent unused argument warning */
    UNUSED(huart);
}

UART_Status_t UART_DMA_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(handle->huart, data, size);

    if (status != HAL_OK) {
        return UART_ERROR;
    }

    /* Wait for transmission complete if timeout is not 0 */
    if (timeout) {
        uint32_t tickstart = HAL_GetTick();
        while (handle->huart->gState != HAL_UART_STATE_READY) {
            if ((HAL_GetTick() - tickstart) > timeout) {
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}

UART_Status_t UART_DMA_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Receive_DMA(handle->huart, data, size);

    if (status != HAL_OK) {
        return UART_ERROR;
    }

    /* Wait for reception complete if timeout is not 0 */
    if (timeout) {
        uint32_t tickstart = HAL_GetTick();
        while (handle->huart->RxState != HAL_UART_STATE_READY) {
            if ((HAL_GetTick() - tickstart) > timeout) {
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}
