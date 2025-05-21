/**
 * @file uart_blocking.h
 * @brief UART Blocking mode implementation for STM32F429I-DISC1
 */

#ifndef UART_BLOCKING_H
#define UART_BLOCKING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart.h"

/**
 * @brief Initialize UART Blocking mode
 * @param handle UART handle pointer
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Blocking_Init(UART_Handle_t* handle);

/**
 * @brief Transmit data using UART in blocking mode
 * @param handle UART handle pointer
 * @param data Pointer to data buffer
 * @param size Size of data to transmit
 * @param timeout Timeout duration in milliseconds
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Blocking_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout);

/**
 * @brief Receive data using UART in blocking mode
 * @param handle UART handle pointer
 * @param data Pointer to data buffer
 * @param size Size of data to receive
 * @param timeout Timeout duration in milliseconds
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Blocking_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* UART_BLOCKING_H */
