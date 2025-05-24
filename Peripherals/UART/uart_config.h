/**
 * @file uart_config.h
 * @brief UART configuration parameters for STM32F429I-DISC1
 */

#ifndef UART_CONFIG_H
#define UART_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "uart.h"
#include "uart_ring_buffer.h"

/* Default UART configuration */
#define UART_DEFAULT_BAUDRATE     115200
#define UART_DEFAULT_WORDLENGTH   UART_WORDLENGTH_8B
#define UART_DEFAULT_STOPBITS     UART_STOPBITS_1
#define UART_DEFAULT_PARITY       UART_PARITY_NONE
#define UART_DEFAULT_MODE         UART_MODE_TX_RX

/* Default timeout value in milliseconds */
#define UART_TIMEOUT         5000

/* Buffer sizes */
#define UART_RX_BUFFER_SIZE      512   /* Must match RING_BUFFER_SIZE */
#define UART_TX_BUFFER_SIZE      512   /* Keep TX buffer same size */

/* DMA configuration */
#define UART_DMA_TX_CHANNEL      DMA_CHANNEL_4
#define UART_DMA_RX_CHANNEL      DMA_CHANNEL_4
#define UART_DMA_TX_STREAM       DMA2_Stream7
#define UART_DMA_RX_STREAM       DMA2_Stream5

/* Global DMA handles */
extern DMA_HandleTypeDef hdma_uart1_tx;
extern DMA_HandleTypeDef hdma_uart1_rx;

#ifdef __cplusplus
}
#endif

#endif /* UART_CONFIG_H */
