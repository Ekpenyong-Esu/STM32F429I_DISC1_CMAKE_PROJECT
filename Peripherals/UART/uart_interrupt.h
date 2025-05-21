/**
 * @file uart_interrupt.h
 * @brief UART Interrupt mode implementation for STM32F429I-DISC1
 */

#ifndef UART_INTERRUPT_H
#define UART_INTERRUPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart.h"

/**
 * @brief Initialize UART Interrupt mode
 * @param handle UART handle pointer
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_IT_Init(UART_Handle_t* handle);

/**
 * @brief Transmit data using UART Interrupts
 * @param handle UART handle pointer
 * @param data Pointer to data buffer
 * @param size Size of data to transmit
 * @param timeout Timeout duration in milliseconds
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_IT_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout);

/**
 * @brief Receive data using UART Interrupts
 * @param handle UART handle pointer
 * @param data Pointer to data buffer
 * @param size Size of data to receive
 * @param timeout Timeout duration in milliseconds
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_IT_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout);

/**
 * @brief UART transmission complete callback
 * @param handle UART handle pointer
 */
void UART_IT_TxCpltCallback(UART_Handle_t* handle);

/**
 * @brief UART reception complete callback
 * @param handle UART handle pointer
 */
void UART_IT_RxCpltCallback(UART_Handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* UART_INTERRUPT_H */
