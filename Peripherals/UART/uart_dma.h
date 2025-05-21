/**
 * @file uart_dma.h
 * @brief UART DMA mode implementation for STM32F429I-DISC1
 */

#ifndef UART_DMA_H
#define UART_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart.h"

/**
 * @brief Initialize UART DMA mode
 * @param handle UART handle pointer
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_DMA_Init(UART_Handle_t* handle);

/**
 * @brief Transmit data using UART DMA
 * @param handle UART handle pointer
 * @param data Pointer to data buffer
 * @param size Size of data to transmit
 * @param timeout Timeout duration in milliseconds
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_DMA_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout);

/**
 * @brief Receive data using UART DMA
 * @param handle UART handle pointer
 * @param data Pointer to data buffer
 * @param size Size of data to receive
 * @param timeout Timeout duration in milliseconds
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_DMA_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout);

/**
 * @brief DMA transmission complete callback
 * @param handle UART handle pointer
 */
void UART_DMA_TxCpltCallback(UART_Handle_t* handle);

/**
 * @brief DMA reception complete callback
 * @param handle UART handle pointer
 */
void UART_DMA_RxCpltCallback(UART_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_DMA_H */
