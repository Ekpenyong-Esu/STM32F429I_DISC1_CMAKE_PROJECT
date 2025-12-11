/**
 * @file dma_example.c
 * @brief DMA Driver Usage Examples for STM32F429I-DISC1
 * @author STM32 Team
 * @date 2025-09-04
 *
 * This file demonstrates how to use the DMA driver for various scenarios.
 */

#include "dma.h"
#include "main.h"
#include "stdbool.h"
#include <stdio.h>
#include <string.h>

/* Example data buffers */
static uint32_t src_buffer[256];
static uint32_t dst_buffer[256];


/* DMA handles for different use cases */
static DMA_Handle_t dma_mem2mem_handle;
static DMA_Handle_t dma_adc_handle;
static DMA_Handle_t dma_uart_handle;

/* Status flags */
static volatile bool mem2mem_transfer_complete = false;
static volatile bool adc_transfer_complete = false;
static volatile bool uart_transfer_complete = false;

/**
 * @brief Example 1: Memory to Memory DMA Transfer
 * @details Demonstrates copying data from one memory location to another
 */
void DMA_Example_MemoryToMemory(void) {
    printf("DMA Example: Memory to Memory Transfer\r\n");

    /* Initialize source buffer with test pattern */
    for (int i = 0; i < 256; i++) {
        src_buffer[i] = i * 2;
    }

    /* Clear destination buffer */
    memset(dst_buffer, 0, sizeof(dst_buffer));

    /* Configure DMA for Memory to Memory transfer */
    DMA_Config_t mem2mem_config = {
        .stream = DMA2_Stream0,                    /* Use DMA2 Stream 0 */
        .channel = DMA_CHANNEL_0,                  /* Channel 0 */
        .direction = DMA_MEMORY_TO_MEMORY,         /* Memory to Memory */
        .mode = DMA_NORMAL,                        /* Normal mode (single transfer) */
        .priority = DMA_PRIORITY_MEDIUM,           /* Medium priority */
        .dataSize = DMA_DATA_SIZE_WORD,            /* 32-bit data */
        .memInc = DMA_MINC_ENABLE,                 /* Enable memory increment */
        .periphInc = DMA_PINC_ENABLE,              /* Enable peripheral increment (source in this case) */
        .fifoMode = DMA_FIFOMODE_DISABLE,          /* Disable FIFO */
        .fifoThreshold = DMA_FIFO_THRESHOLD_FULL   /* Not used when FIFO disabled */
    };

    /* Initialize DMA */
    if (DMA_Init(&dma_mem2mem_handle, &mem2mem_config) != HAL_OK) {
        printf("Error: DMA Memory to Memory initialization failed\r\n");
        return;
    }

    /* Reset transfer complete flag */
    mem2mem_transfer_complete = false;

    /* Start DMA transfer */
    if (DMA_StartTransfer(&dma_mem2mem_handle, (uint32_t)src_buffer, (uint32_t)dst_buffer, 256) != HAL_OK) {
        printf("Error: Failed to start DMA transfer\r\n");
        return;
    }

    printf("DMA transfer started...\r\n");

    /* Wait for transfer completion (or implement timeout) */
    uint32_t timeout = HAL_GetTick() + 1000; /* 1 second timeout */
    while (!mem2mem_transfer_complete && HAL_GetTick() < timeout) {
        /* Wait or do other tasks */
    }

    if (mem2mem_transfer_complete) {
        printf("DMA transfer completed successfully!\r\n");

        /* Verify transfer */
        bool transfer_ok = true;
        for (int i = 0; i < 256; i++) {
            if (src_buffer[i] != dst_buffer[i]) {
                transfer_ok = false;
                break;
            }
        }

        if (transfer_ok) {
            printf("Data verification: PASSED\r\n");
        } else {
            printf("Data verification: FAILED\r\n");
        }
    } else {
        printf("DMA transfer timed out!\r\n");
    }

    /* Cleanup */
    DMA_DeInit(&dma_mem2mem_handle);
}

/**
 * @brief Example 2: ADC with DMA (Peripheral to Memory)
 * @details Demonstrates using DMA to transfer ADC conversion results
 */
void DMA_Example_ADC_WithDMA(void) {
    printf("DMA Example: ADC with DMA (Peripheral to Memory)\r\n");

    /* Configure DMA for ADC (Peripheral to Memory) */
    DMA_Config_t adc_config = {
        .stream = DMA2_Stream4,                    /* ADC1 typically uses DMA2 Stream 4 */
        .channel = DMA_CHANNEL_0,                  /* ADC1 uses Channel 0 */
        .direction = DMA_PERIPH_TO_MEMORY,         /* Peripheral to Memory */
        .mode = DMA_CIRCULAR,                      /* Circular mode for continuous ADC */
        .priority = DMA_PRIORITY_HIGH,             /* High priority for ADC */
        .dataSize = DMA_DATA_SIZE_HALFWORD,        /* 16-bit ADC data */
        .memInc = DMA_MINC_ENABLE,                 /* Enable memory increment */
        .periphInc = DMA_PINC_DISABLE,             /* Disable peripheral increment (single ADC register) */
        .fifoMode = DMA_FIFOMODE_DISABLE,          /* Disable FIFO for simple transfer */
        .fifoThreshold = DMA_FIFO_THRESHOLD_FULL   /* Not used when FIFO disabled */
    };

    /* Initialize DMA */
    if (DMA_Init(&dma_adc_handle, &adc_config) != HAL_OK) {
        printf("Error: DMA ADC initialization failed\r\n");
        return;
    }

    /* Note: In real implementation, you would:
     * 1. Initialize ADC peripheral
     * 2. Link DMA handle to ADC handle
     * 3. Start ADC with DMA
     *
     * Example (pseudo-code):
     * hadc1.DMA_Handle = &dma_adc_handle.hdma;
     * HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 128);
     */

    printf("ADC DMA configuration completed\r\n");
    printf("Note: ADC peripheral initialization required for actual operation\r\n");

    /* Cleanup */
    DMA_DeInit(&dma_adc_handle);
}

/**
 * @brief Example 3: UART TX with DMA (Memory to Peripheral)
 * @details Demonstrates using DMA to transmit data via UART
 */
void DMA_Example_UART_TX_WithDMA(void) {
    printf("DMA Example: UART TX with DMA (Memory to Peripheral)\r\n");

    /* Configure DMA for UART TX (Memory to Peripheral) */
    DMA_Config_t uart_config = {
        .stream = DMA1_Stream6,                    /* USART2 TX typically uses DMA1 Stream 6 */
        .channel = DMA_CHANNEL_4,                  /* USART2 uses Channel 4 */
        .direction = DMA_MEMORY_TO_PERIPH,         /* Memory to Peripheral */
        .mode = DMA_NORMAL,                        /* Normal mode (single transfer) */
        .priority = DMA_PRIORITY_LOW,              /* Low priority for UART */
        .dataSize = DMA_DATA_SIZE_BYTE,            /* 8-bit UART data */
        .memInc = DMA_MINC_ENABLE,                 /* Enable memory increment */
        .periphInc = DMA_PINC_DISABLE,             /* Disable peripheral increment (single UART register) */
        .fifoMode = DMA_FIFOMODE_DISABLE,          /* Disable FIFO for simple transfer */
        .fifoThreshold = DMA_FIFO_THRESHOLD_FULL   /* Not used when FIFO disabled */
    };

    /* Initialize DMA */
    if (DMA_Init(&dma_uart_handle, &uart_config) != HAL_OK) {
        printf("Error: DMA UART initialization failed\r\n");
        return;
    }

    /* Note: In real implementation, you would:
     * 1. Initialize UART peripheral
     * 2. Link DMA handle to UART handle
     * 3. Start UART transmission with DMA
     *
     * Example (pseudo-code):
     * huart2.hdmatx = &dma_uart_handle.hdma;
     * HAL_UART_Transmit_DMA(&huart2, uart_tx_buffer, strlen((char*)uart_tx_buffer));
     */

    printf("UART DMA configuration completed\r\n");
    printf("Note: UART peripheral initialization required for actual operation\r\n");

    /* Cleanup */
    DMA_DeInit(&dma_uart_handle);
}

/**
 * @brief DMA Transfer Complete Callback Implementation
 * @details This callback is called when DMA transfer completes
 */
void DMA_TransferCompleteCallback(DMA_HandleTypeDef *hdma) {
    /* Determine which DMA handle triggered the callback */
    DMA_Handle_t *dma_handle = (DMA_Handle_t *)hdma->Parent;

    if (dma_handle == &dma_mem2mem_handle) {
        mem2mem_transfer_complete = true;
        printf("Memory to Memory DMA transfer completed\r\n");
    } else if (dma_handle == &dma_adc_handle) {
        adc_transfer_complete = true;
        printf("ADC DMA transfer completed\r\n");
    } else if (dma_handle == &dma_uart_handle) {
        uart_transfer_complete = true;
        printf("UART DMA transfer completed\r\n");
    }
}

/**
 * @brief DMA Transfer Error Callback Implementation
 * @details This callback is called when DMA transfer encounters an error
 */
void DMA_TransferErrorCallback(DMA_HandleTypeDef *hdma) {
    /* Determine which DMA handle triggered the callback */
    DMA_Handle_t *dma_handle = (DMA_Handle_t *)hdma->Parent;
    uint32_t error_code = HAL_DMA_GetError(hdma);

    printf("DMA Transfer Error! Error Code: 0x%08lX\r\n", error_code);

    if (dma_handle == &dma_mem2mem_handle) {
        printf("Error in Memory to Memory DMA\r\n");
    } else if (dma_handle == &dma_adc_handle) {
        printf("Error in ADC DMA\r\n");
    } else if (dma_handle == &dma_uart_handle) {
        printf("Error in UART DMA\r\n");
    }
}

/**
 * @brief Run all DMA examples
 * @details Main function to execute all DMA examples
 */
void DMA_RunAllExamples(void) {
    printf("\r\n=== STM32F429I-DISC1 DMA Driver Examples ===\r\n\r\n");

    /* Run Memory to Memory example */
    DMA_Example_MemoryToMemory();
    printf("\r\n");

    /* Run ADC DMA example */
    DMA_Example_ADC_WithDMA();
    printf("\r\n");

    /* Run UART DMA example */
    DMA_Example_UART_TX_WithDMA();
    printf("\r\n");

    printf("=== All DMA Examples Completed ===\r\n");
}

/**
 * @brief DMA Stream IRQ Handlers
 * @details These functions should be called from the corresponding IRQ handlers in stm32f4xx_it.c
 */

/* Example IRQ handler for DMA2_Stream0 (Memory to Memory) */
void DMA2_Stream0_IRQHandler(void) {
    DMA_IRQHandler(&dma_mem2mem_handle);
}

/* Example IRQ handler for DMA2_Stream4 (ADC) */
void DMA2_Stream4_IRQHandler(void) {
    DMA_IRQHandler(&dma_adc_handle);
}

/* Example IRQ handler for DMA1_Stream6 (UART TX) */
void DMA1_Stream6_IRQHandler(void) {
    DMA_IRQHandler(&dma_uart_handle);
}
