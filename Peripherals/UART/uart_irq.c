/**
  ******************************************************************************
  * @file    uart_irq.c
  * @brief   UART interrupt handlers
  * @details Implements the interrupt handlers and callbacks for UART
  * @version 1.0
  * @date    2025-04-19
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include "uart_internal.h"

/**
  * @brief  UART Received callback
  * @param  huart: UART handle
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    /* Set flag to indicate data received */
    RxReady = 1;
  }
}

/**
  * @brief  USART1 IRQ Handler
  * @retval None
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}
