/**
  ******************************************************************************
  * @file    uart.h
  * @brief   UART module main header
  * @details Header file for UART configuration and operations
  * @version 1.0
  * @date    2025-04-19
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdarg.h>
#include <stdint.h>


/* Defines -------------------------------------------------------------------*/
#define UART_RX_BUFFER_SIZE     256

/* Public function prototypes -------------------------------------------------------*/
/* Initialization Functions */
void UART_Init(void);

/* Input/Output Functions */
void UART_SendString(char *str);
void UART_SendByte(uint8_t byte);
uint8_t UART_IsDataAvailable(void);
uint16_t UART_ReadString(char *buffer, uint16_t max_size, uint32_t timeout_ms);
void UART_printf(const char *format, ...);

/* Public Variables */
extern UART_HandleTypeDef huart1;

#ifdef __cplusplus
}
#endif

#endif /* __UART_H */
