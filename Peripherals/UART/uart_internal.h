/**
  ******************************************************************************
  * @file    uart_internal.h
  * @brief   UART module internal header
  * @details Internal header file with shared declarations for UART subsystems
  * @version 1.0
  * @date    2025-04-19
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_INTERNAL_H
#define __UART_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "uart.h"

/* Shared Variables Declarations */

extern uint8_t RxData;
extern volatile uint8_t RxReady;

#ifdef __cplusplus
}
#endif

#endif /* __UART_INTERNAL_H */
