/**
 * @file uart.h
 * @brief Main UART interface for STM32F429I-DISC1
 */

#ifndef UART_H
#define UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief UART transfer mode enumeration
 */
typedef enum {
    UART_MODE_BLOCKING,    /*!< Blocking mode */
    UART_MODE_INTERRUPT,   /*!< Interrupt mode */
    UART_MODE_DMA         /*!< DMA mode */
} UART_Mode_t;

/**
 * @brief UART status enumeration
 */
typedef enum {
    UART_OK = 0,          /*!< Operation successful */
    UART_ERROR,           /*!< Generic error */
    UART_BUSY,           /*!< UART is busy */
    UART_TIMEOUT_ERROR        /*!< Operation timed out */
} UART_Status_t;

/**
 * @brief UART configuration structure
 */
typedef struct {
    USART_TypeDef* instance;  /*!< UART instance (USART1, USART2, etc.) */
    uint32_t baudRate;     /*!< Baud rate */
    uint8_t wordLength;    /*!< Word length (8 or 9 bits) */
    uint8_t stopBits;      /*!< Number of stop bits */
    uint8_t parity;        /*!< Parity mode */
    UART_Mode_t mode;      /*!< Transfer mode */
} UART_Config_t;

/**
 * @brief UART handle structure
 */
typedef struct {
    UART_HandleTypeDef* huart;     /*!< HAL UART handle */
    UART_Config_t config;          /*!< UART configuration */
    uint8_t* rxBuffer;            /*!< Receive buffer */
    uint8_t* txBuffer;            /*!< Transmit buffer */
    uint16_t rxSize;              /*!< Size of receive buffer */
    uint16_t txSize;              /*!< Size of transmit buffer */
    bool isInitialized;           /*!< Initialization status */
} UART_Handle_t;

/**
 * @brief Initialize UART peripheral
 * @param handle UART handle pointer
 * @param config UART configuration structure
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Init(UART_Handle_t* handle, const UART_Config_t* config);

/**
 * @brief Deinitialize UART peripheral
 * @param handle UART handle pointer
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_DeInit(UART_Handle_t* handle);

/**
 * @brief Transmit data over UART
 * @param handle UART handle pointer
 * @param data Pointer to data buffer
 * @param size Size of data to transmit
 * @param timeout Timeout duration in milliseconds
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Transmit(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout);

/**
 * @brief Receive data over UART
 * @param handle UART handle pointer
 * @param data Pointer to data buffer
 * @param size Size of data to receive
 * @param timeout Timeout duration in milliseconds
 * @return UART_Status_t Status of operation
 */
UART_Status_t UART_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
