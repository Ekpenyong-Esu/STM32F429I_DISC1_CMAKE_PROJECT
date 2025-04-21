/**
  ******************************************************************************
  * @file    uart_io.c
  * @brief   UART input/output operations
  * @details Implements reading and writing functions for the UART peripheral
  * @version 1.0
  * @date    2025-04-19
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "uart.h"
#include "uart_internal.h"

/* Private variables ---------------------------------------------------------*/
static uint8_t RxBuffer[UART_RX_BUFFER_SIZE];

/**
  * @brief  Send a string over UART
  * @param  str: Pointer to the string to send
  * @retval None
  */
void UART_SendString(char *str)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/**
  * @brief  Send a single byte over UART
  * @param  byte: Byte to send
  * @retval None
  */
void UART_SendByte(uint8_t byte)
{
  HAL_UART_Transmit(&huart1, &byte, 1, HAL_MAX_DELAY);
}

/**
  * @brief  Check if data is available to read
  * @retval 1 if data is available, 0 otherwise
  */
uint8_t UART_IsDataAvailable(void)
{
  return RxReady;
}

/**
  * @brief  Read a string from UART with timeout
  * @param  buffer: Pointer to buffer to store received data
  * @param  max_size: Maximum size of the buffer
  * @param  timeout_ms: Timeout in milliseconds
  * @retval Number of bytes read
  */
uint16_t UART_ReadString(char *buffer, uint16_t max_size, uint32_t timeout_ms)
{
  uint32_t startTime = HAL_GetTick();
  uint16_t size = 0;

  while ((HAL_GetTick() - startTime) < timeout_ms && size < max_size - 1)
  {
    if (RxReady)
    {
      buffer[size++] = RxData;
      RxReady = 0;

      /* Start receiving next byte */
      HAL_UART_Receive_IT(&huart1, &RxData, 1);

      /* If we received end of line, stop */
      if (buffer[size-1] == '\n' || buffer[size-1] == '\r')
      {
        break;
      }
    }
  }

  /* Null-terminate the string */
  buffer[size] = '\0';
  return size;
}

/**
  * @brief  Printf-style interface for UART output
  * @param  format: Format string
  * @param  ...: Variable arguments
  * @retval None
  */
void UART_printf(const char *format, ...)
{
  char buffer[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  UART_SendString(buffer);
}
