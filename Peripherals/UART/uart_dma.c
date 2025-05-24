/**
 * @file uart_dma.c
 * @brief UART DMA mode implementation for STM32F429I-DISC1
 */

#include "uart_dma.h"
#include "uart_example.h"
#include "stm32f4xx_hal_dma.h"

/* External references */
extern UART_Handle_t uartHandle;  // Define this in uart.c


/* Global DMA handles */
DMA_HandleTypeDef hdma_uart1_tx;
DMA_HandleTypeDef hdma_uart1_rx;

UART_Status_t UART_DMA_Init(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        DEBUG_PRINT("DMA UART handle or huart is NULL");
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

    if (HAL_DMA_Init(&hdma_uart1_tx) != HAL_OK) {
        DEBUG_PRINT("DMA TX initialization failed");
        return UART_ERROR;
    }

    /* Configure RX DMA */
    hdma_uart1_rx.Instance = UART_DMA_RX_STREAM;
    hdma_uart1_rx.Init.Channel = UART_DMA_RX_CHANNEL;
    hdma_uart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart1_rx.Init.Mode = DMA_CIRCULAR;  // Use circular mode for continuous reception
    hdma_uart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_uart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_uart1_rx) != HAL_OK) {
        DEBUG_PRINT("DMA RX initialization failed");
        return UART_ERROR;
    }

    /* Link DMA with UART */
    __HAL_LINKDMA(handle->huart, hdmatx, hdma_uart1_tx);
    __HAL_LINKDMA(handle->huart, hdmarx, hdma_uart1_rx);

    /* DMA interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_RXNE); // Enable Receive Data Register Not Empty interrupt
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR); // Enable Error interrupt
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE); // Enable IDLE line detection interrupt
    handle->isInitialized = true;
    return UART_OK;
}


UART_Status_t UART_DMA_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        DEBUG_PRINT("DMA UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(handle->huart, data, size);
    if (status != HAL_OK) {
        DEBUG_PRINT("DMA UART Transmit failed");
        return UART_ERROR;
    }

     /* Wait for transmission complete if timeout is specified */
    if (timeout) {
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

    /* Start the DMA transfer */
    HAL_StatusTypeDef status = HAL_UART_Receive_DMA(handle->huart, handle->rxBuffer, handle->rxSize);
    if (status != HAL_OK) {
        DEBUG_PRINT("DMA UART Receive failed");
        return UART_ERROR;
    }

    // /* Wait for requested amount of data if timeout is not 0 */
    // if (timeout) {
    //     uint32_t tickstart = HAL_GetTick();
    //     while (RingBuffer_Available(&rxRingBuffer) < size) {
    //         /* Check for timeout */
    //         if ((HAL_GetTick() - tickstart) > timeout) {
    //             DEBUG_PRINT("DMA UART Receive timeout");
    //             return UART_TIMEOUT_ERROR;
    //         }
    //         /* Check for DMA errors */
    //         if (handle->huart->RxState == HAL_UART_STATE_ERROR) {
    //             DEBUG_PRINT("DMA UART Error state");
    //             return UART_ERROR;
    //         }
    //     }
    // }

    // /* Copy data from ring buffer to user buffer */
    // return UART_RingBuffer_Receive(handle, data, size);

    return UART_OK;
}

/* UART Reception Complete Callback */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    /* Get the current UART handle */
    if (huart != uartHandle.huart) {
        return;  // Not our UART
    }

    uint16_t pos = 0;

    /* Handle based on mode */
    if (uartHandle.config.mode == UART_MODE_DMA) {
        /* Get position in circular buffer */
        pos = huart->RxXferSize - huart->hdmarx->Instance->NDTR;

        /* Process received data if any */
        if (pos > 0) {
            DEBUG_PRINT("Received %d bytes in DMA mode", pos);
            UART_RingBuffer_PutData(huart->pRxBuffPtr, pos);
            UART_Example_PreProcess(&uartHandle);
            /* Restart DMA reception */
            HAL_UART_Receive_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        }

    } else if (uartHandle.config.mode == UART_MODE_INTERRUPT) {
        /* In interrupt mode, receive one byte at a time */
        DEBUG_PRINT("Received 1 byte in Interrupt mode");
        UART_RingBuffer_PutData(huart->pRxBuffPtr, 1);
        UART_Example_PreProcess(&uartHandle);

        /* Restart reception for next byte */
        HAL_UART_Receive_IT(huart, huart->pRxBuffPtr, 1);
    }

    /* Set reception complete flag */
    rxComplete = 1;
}

/**
 * @brief DMA error callback
 * @param huart UART handle pointer
 */
