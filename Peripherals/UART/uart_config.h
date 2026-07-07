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
#include "uart_ring_buffer.h"

/* Global transfer completion flags - defined in uart.c */
extern volatile uint8_t txComplete;
extern volatile uint8_t uartExampleRxComplete;

/* Default UART configuration */
#define UART_DEFAULT_BAUDRATE     115200
#define UART_DEFAULT_WORDLENGTH   UART_WORDLENGTH_8B
#define UART_DEFAULT_STOPBITS     UART_STOPBITS_1
#define UART_DEFAULT_PARITY       UART_PARITY_NONE
#define UART_DEFAULT_MODE         UART_MODE_TX_RX

/* Default timeout value in milliseconds */
#define UART_TIMEOUT         5000  /* Default timeout in milliseconds */
#define UART_CHAR_TIMEOUT   100   /* Timeout for single character reception */


/* DMA configuration */
#define UART_DMA_TX_CHANNEL      DMA_CHANNEL_4
#define UART_DMA_RX_CHANNEL      DMA_CHANNEL_4
#define UART_DMA_TX_STREAM       DMA2_Stream7
#define UART_DMA_RX_STREAM       DMA2_Stream5

/* Error recovery delay constants */
#define UART_ERROR_RECOVERY_DELAY 1000
#define UART_RETRY_DELAY          500

/* Buffer sizes */
#define RX_BUFFER_SIZE     512  /* Match UART_RX_BUFFER_SIZE */
#define TX_BUFFER_SIZE     512  /* Match UART_TX_BUFFER_SIZE */
#define SINGLE_CHAR_BUFFER_SIZE  1   /* For single character operations */

#ifdef __cplusplus
}
#endif

#endif /* UART_CONFIG_H */
