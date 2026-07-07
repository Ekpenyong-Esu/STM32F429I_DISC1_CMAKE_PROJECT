/**
 * @file uart.c
 * @brief Main UART implementation for STM32F429I-DISC1
 */

#include "uart.h"
#include "uart_config.h"
#include "uart_dma.h"
#include "uart_interrupt.h"
#include "uart_blocking.h"
#include "sys.h"
#include "log.h"

/** @brief Delay after deinitialization to allow hardware to settle */
#define UART_DEINIT_DELAY_MS (1000U)

/* Transfer completion flags (set from HAL callbacks, polled by interrupt/DMA transfers) */
volatile uint8_t txComplete = 0;
volatile uint8_t uartExampleRxComplete = 0;

/* Global UART handle instance (declared extern in uart.h / used across UART sources) */
UART_Handle_t uartHandle;


UART_Status_t UART_Init(UART_Handle_t* handle, const UART_Config_t* config)
{
    log_debug("UART: Initializing UART");

    if (handle == NULL || config == NULL || config->instance == NULL) {
        DEBUG_PRINT("UART handle or config is NULL");
        return UART_ERROR;
    }

    /* Deinitialize previous configuration if initialized */
    if (handle->isInitialized) {
        UART_DeInit(handle);
    }

    /* Store configuration */
    handle->config = *config;

    /* Configure UART base settings */
    if (handle->huart == NULL) {
        DEBUG_PRINT("UART HAL handle is NULL");
        return UART_ERROR;
    }

    memset(handle->huart, 0, sizeof(UART_HandleTypeDef));

    __HAL_RCC_USART1_CLK_ENABLE();

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
        log_debug("UART: UART initialized successfully");
    }

    return status;
}

UART_Status_t UART_DeInit(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        DEBUG_PRINT("UART handle or huart is NULL");
        return UART_ERROR;
    }

    /* Disable all UART interrupts */
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_IDLE);

    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        DEBUG_PRINT("UART deinitialization failed");
        return UART_ERROR;
    }

    /* Deinitialize MSP resources */
    HAL_UART_MspDeInit(handle->huart);

    handle->isInitialized = false;

    /* Reset handle state */
    handle->isInitialized = false;
    handle->rxBuffer = NULL;
    handle->txBuffer = NULL;
    handle->rxSize = 0;
    handle->txSize = 0;
    memset(&handle->config, 0, sizeof(UART_Config_t));

    HAL_Delay(UART_DEINIT_DELAY_MS); // Allow time for deinitialization

    return UART_OK;

}

/* Helper function to handle UART mode-specific operations */
static UART_Status_t UART_HandleMode(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout, bool isTransmit)
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
            return isTransmit ? UART_DMA_Transmit(handle, data, size, timeout) : UART_DMA_Receive(handle, (uint8_t*)data, size, timeout);
        case UART_MODE_INTERRUPT:
            return isTransmit ? UART_IT_Transmit(handle, data, size, timeout) : UART_IT_Receive(handle, (uint8_t*)data, size, timeout);
        case UART_MODE_BLOCKING:
            return isTransmit ? UART_Blocking_Transmit(handle, data, size, timeout) : UART_Blocking_Receive(handle, (uint8_t*)data, size, timeout);
        default:
            return UART_ERROR;
    }
}

UART_Status_t UART_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    return UART_HandleMode(handle, data, size, timeout, true);
}

UART_Status_t UART_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    return UART_HandleMode(handle, data, size, timeout, false);
}


/**
 * @brief HAL UART Rx Event callback (for IDLE detection)
 * @param huart UART handle pointer
 * @param Size Number of bytes received
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart != uartHandle.huart) {
        return;  // Not our UART
    }

    if (uartHandle.config.mode == UART_MODE_DMA) {
        DEBUG_PRINT("UART Rx Event (IDLE) - Size: %d", Size);
        if (Size > 0) {
            uartExampleRxComplete = 1;
            UART_RingBuffer_PutData(uartHandle.rxBuffer, Size);
            HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
            if (status != HAL_OK) {
                DEBUG_PRINT("Failed to restart DMA ReceiveToIdle: %d", status);
                HAL_UART_Receive_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
            }
        }
    }else if (uartHandle.config.mode == UART_MODE_INTERRUPT) {
        DEBUG_PRINT("UART Rx Event (IDLE) - Size: %d", Size);
        if (Size > 0) {
            // Put received data into ring buffer
            UART_RingBuffer_PutData(uartHandle.rxBuffer, Size);
            uartExampleRxComplete = 1;

            HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_IT(huart, uartHandle.rxBuffer, uartHandle.rxSize);
            if (status != HAL_OK) {
                DEBUG_PRINT("Failed to restart interrupt reception: %d", status);
                /* Fall back to basic interrupt reception if ReceiveToIdle fails */
                status = HAL_UART_Receive_IT(huart, uartHandle.rxBuffer, uartHandle.rxSize);
                if (status != HAL_OK) {
                    DEBUG_PRINT("Failed to fall back to regular interrupt reception: %d", status);
                }
            }
        }
    }
}

/**
 * @brief HAL UART transmit complete callback
 * @param huart UART handle pointer
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != uartHandle.huart) {
        return;  // Not our UART
    }

    /* Set transmission complete flag */
    txComplete = 1;
    DEBUG_PRINT("UART TX Complete");
}

/**
 * @brief HAL UART receive complete callback
 * @param huart UART handle pointer
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != uartHandle.huart) {
        return;  // Not our UART
    }

    if (uartHandle.config.mode == UART_MODE_INTERRUPT) {
        uartExampleRxComplete = 1;
        DEBUG_PRINT("IT Received data complete");

        // Put received data into the ring buffer (only one byte for HAL_UART_Receive_IT)
        if (huart->RxXferSize == 1) {
            UART_RingBuffer_PutData(uartHandle.rxBuffer, 1);
        }

        /* Restart interrupt reception immediately rather than waiting for PreProcess */
        HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_IT(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        if (status != HAL_OK) {
            DEBUG_PRINT("Failed to restart interrupt reception: %d", status);
            /* Fall back to basic interrupt reception if ReceiveToIdle fails */
            status = HAL_UART_Receive_IT(huart, uartHandle.rxBuffer, uartHandle.rxSize);
            if (status != HAL_OK) {
                DEBUG_PRINT("Failed to fall back to regular interrupt reception: %d", status);
            }
        }
    } else if (uartHandle.config.mode == UART_MODE_DMA) {
        uartExampleRxComplete = 1;
        DEBUG_PRINT("DMA Full Buffer Received: %d bytes", uartHandle.rxSize);
        UART_RingBuffer_PutData(uartHandle.rxBuffer, uartHandle.rxSize);
        HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        if (status != HAL_OK) {
            DEBUG_PRINT("Failed to restart DMA ReceiveToIdle: %d", status);
            HAL_UART_Receive_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        }
    }
}

/**
 * @brief HAL UART error callback
 * @param huart UART handle pointer
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart != uartHandle.huart) {
        return;  // Not our UART
    }

    if (huart->ErrorCode & HAL_UART_ERROR_ORE) {
        DEBUG_PRINT("UART Overrun Error");
        __HAL_UART_CLEAR_OREFLAG(huart);
    }
    if (huart->ErrorCode & HAL_UART_ERROR_NE) {
        DEBUG_PRINT("UART Noise Error");
        __HAL_UART_CLEAR_NEFLAG(huart);
    }
    if (huart->ErrorCode & HAL_UART_ERROR_FE) {
        DEBUG_PRINT("UART Frame Error");
        __HAL_UART_CLEAR_FEFLAG(huart);
    }
    if (huart->ErrorCode & HAL_UART_ERROR_PE) {
        DEBUG_PRINT("UART Parity Error");
        __HAL_UART_CLEAR_PEFLAG(huart);
    }

    /* Clear error flags */
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    /* Reset the UART peripheral first to clear stuck errors */
    HAL_UART_AbortReceive(huart);

    /* Clear error flags */
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    /* Short delay to ensure UART is ready */
    volatile uint32_t delayCounter = 0;
    for (delayCounter = 0; delayCounter < UART_ERROR_RECOVERY_DELAY; delayCounter++) {
        /* Short delay */
    }

    /* Restart reception if it was stopped due to error */
    if (uartHandle.config.mode == UART_MODE_DMA) {
        HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        if (status != HAL_OK) {
            DEBUG_PRINT("Failed to restart DMA ReceiveToIdle after error: %d", status);
            /* Fallback to regular DMA if ReceiveToIdle fails */
            HAL_UART_Receive_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        }
    } else if (uartHandle.config.mode == UART_MODE_INTERRUPT) {
        HAL_UART_Init(huart);
        /* Restart interrupt reception after error using ReceiveToIdle */
        HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_IT(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        if (status != HAL_OK) {
            DEBUG_PRINT("Failed to restart ReceiveToIdle after error: %d", status);
            /* Fall back to regular interrupt reception */
            status = HAL_UART_Receive_IT(huart, uartHandle.rxBuffer, uartHandle.rxSize);
            if (status != HAL_OK) {
                DEBUG_PRINT("Failed to restart interrupt reception after error: %d", status);
            }
        }
    }
}

/**
  * @brief UART MSP Initialization
  * This function configures the hardware resources used in this example
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    static DMA_HandleTypeDef hdma_usart1_tx;
    static DMA_HandleTypeDef hdma_usart1_rx;

    if (huart->Instance == USART1) {


        /* USART1 GPIO Configuration: PA9 -> USART1_TX, PA10 -> USART1_RX */
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        if (uartHandle.config.mode == UART_MODE_DMA) {
            __HAL_RCC_DMA2_CLK_ENABLE();
            /* DMA configuration for USART1_TX */
            hdma_usart1_tx.Instance = DMA2_Stream7;
            hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
            hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
            hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
            hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
            hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
            hdma_usart1_tx.Init.Mode = DMA_NORMAL;
            hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
            hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

            if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) {
                Error_Handler();
            }
            __HAL_LINKDMA(huart, hdmatx, hdma_usart1_tx);

            /* DMA configuration for USART1_RX */
            hdma_usart1_rx.Instance = DMA2_Stream5;
            hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
            hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
            hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
            hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
            hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
            hdma_usart1_rx.Init.Mode = DMA_NORMAL;
            hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
            hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

            if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK) {
                Error_Handler();
            }
            __HAL_LINKDMA(huart, hdmarx, hdma_usart1_rx);

            /* NVIC configuration for DMA interrupts */
            HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 2, 0);
            HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
            HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 2, 0);
            HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
        }
        /* Always enable USART1 IRQ for both DMA and interrupt modes */
        if (uartHandle.config.mode == UART_MODE_DMA || uartHandle.config.mode == UART_MODE_INTERRUPT) {
            HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
        }
    }
}

/**
  * @brief UART MSP De-Initialization
  * This function frees the hardware resources used in this example
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART1) {
        /* Disable UART peripheral clock */
        __HAL_RCC_USART1_CLK_DISABLE();

        /* Deinitialize GPIO pins */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        /* Disable DMA if used */
        if (uartHandle.config.mode == UART_MODE_DMA) {
            if (huart->hdmatx != NULL) {
                HAL_DMA_DeInit(huart->hdmatx);
            }
            if (huart->hdmarx != NULL) {
                HAL_DMA_DeInit(huart->hdmarx);
            }
            HAL_NVIC_DisableIRQ(DMA2_Stream7_IRQn);
            HAL_NVIC_DisableIRQ(DMA2_Stream5_IRQn);
        }

        /* Always disable UART interrupt */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
}
