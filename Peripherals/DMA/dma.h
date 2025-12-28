/**
 * @file dma.h
 * @brief DMA Driver Header File
 * @author Generated for STM32F429
 * @date 2025
 *
 * This DMA driver provides a reusable interface for DMA operations on STM32F4 series.
 * It wraps around STM32 HAL DMA functions for better abstraction and reusability.
 */

#ifndef DMA_H
#define DMA_H

#include "stm32f4xx.h"
#include <stdbool.h>

/* DMA Data Size Definitions */
#define DMA_DATA_SIZE_BYTE       0x00U
#define DMA_DATA_SIZE_HALFWORD   0x01U
#define DMA_DATA_SIZE_WORD       0x02U

/* DMA Configuration Structure */
typedef struct {
    DMA_Stream_TypeDef *stream;      /* DMA Stream (e.g., DMA2_Stream0) */
    uint32_t channel;                /* DMA Channel (e.g., DMA_CHANNEL_0) */
    uint32_t direction;              /* Transfer direction */
    uint32_t mode;                   /* DMA mode (Normal/Circular) */
    uint32_t priority;               /* DMA priority */
    uint32_t dataSize;               /* Data size (Byte/HalfWord/Word) */
    uint32_t memInc;                 /* Memory increment mode */
    uint32_t periphInc;              /* Peripheral increment mode */
    uint32_t fifoMode;               /* FIFO mode */
    uint32_t fifoThreshold;          /* FIFO threshold */
} DMA_Config_t;

/* DMA Handle Structure */
typedef struct {
    DMA_HandleTypeDef hdma;          /* HAL DMA handle */
    DMA_Config_t config;             /* Configuration */
    bool initialized;                /* Initialization status */
    IRQn_Type irqn;                  /* DMA stream IRQ number */
} DMA_Handle_t;

/* Function Prototypes */

/**
 * @brief Initialize DMA with given configuration
 * @param handle: Pointer to DMA handle
 * @param config: Pointer to DMA configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef DMA_Init(DMA_Handle_t *handle, DMA_Config_t *config);

/**
 * @brief Deinitialize DMA
 * @param handle: Pointer to DMA handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef DMA_DeInit(DMA_Handle_t *handle);

/**
 * @brief Start DMA transfer (Memory to Memory)
 * @param handle: Pointer to DMA handle
 * @param srcAddr: Source address
 * @param dstAddr: Destination address
 * @param dataLength: Number of data items to transfer
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef DMA_StartTransfer(DMA_Handle_t *handle, uint32_t srcAddr, uint32_t dstAddr, uint32_t dataLength);

/**
 * @brief Start DMA transfer (Peripheral to Memory)
 * @param handle: Pointer to DMA handle
 * @param periphAddr: Peripheral address
 * @param memAddr: Memory address
 * @param dataLength: Number of data items to transfer
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef DMA_StartPeriphToMem(DMA_Handle_t *handle, uint32_t periphAddr, uint32_t memAddr, uint32_t dataLength);

/**
 * @brief Start DMA transfer (Memory to Peripheral)
 * @param handle: Pointer to DMA handle
 * @param memAddr: Memory address
 * @param periphAddr: Peripheral address
 * @param dataLength: Number of data items to transfer
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef DMA_StartMemToPeriph(DMA_Handle_t *handle, uint32_t memAddr, uint32_t periphAddr, uint32_t dataLength);

/**
 * @brief Stop DMA transfer
 * @param handle: Pointer to DMA handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef DMA_StopTransfer(DMA_Handle_t *handle);

/**
 * @brief Check if DMA transfer is complete
 * @param handle: Pointer to DMA handle
 * @return bool: True if transfer complete, false otherwise
 */
bool DMA_IsTransferComplete(DMA_Handle_t *handle);

/**
 * @brief Get DMA error status
 * @param handle: Pointer to DMA handle
 * @return uint32_t: Error code
 */
uint32_t DMA_GetError(DMA_Handle_t *handle);

/**
 * @brief Enable DMA clock and configure NVIC
 * @param handle: Pointer to DMA handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef DMA_EnableClockAndIRQ(DMA_Handle_t *handle);

/**
 * @brief Get DMA stream IRQ number from stream instance
 * @param stream: DMA stream instance
 * @return IRQn_Type: IRQ number
 */
IRQn_Type DMA_GetStreamIRQ(DMA_Stream_TypeDef *stream);

/**
 * @brief DMA IRQ Handler - to be called from DMA stream IRQ handler
 * @param handle: Pointer to DMA handle
 */
void DMA_IRQHandler(DMA_Handle_t *handle);

/**
 * @brief DMA Transfer Complete Callback (to be implemented by user)
 * @param hdma: Pointer to HAL DMA handle
 */
void DMA_TransferCompleteCallback(DMA_HandleTypeDef *hdma);

/**
 * @brief DMA Transfer Error Callback (to be implemented by user)
 * @param hdma: Pointer to HAL DMA handle
 */
void DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma);

#endif /* DMA_H */
