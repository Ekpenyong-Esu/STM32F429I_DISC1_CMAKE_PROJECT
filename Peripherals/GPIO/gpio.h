/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   GPIO module interface
  * @details This file contains all the function prototypes for
  *          the GPIO peripheral configuration and control.
  *          It provides APIs to initialize and control different GPIO pins
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes all GPIO pins used in the application
 * @details Configures the GPIO pin modes, pull-up/down resistors,
 *          speeds, and initial output levels
 * @param   None
 * @retval  None
 */
void GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */
