/**
  ******************************************************************************
  * @file    sys.h
  * @brief   System module interface
  * @details This file contains all the function prototypes for
  *          system configuration and error handling.
  *          It provides core system functions for the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __SYS_H__
#define __SYS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes the system core components
 * @details Performs initial setup of system components and peripherals
 * @param   None
 * @retval  None
 */
void SYS_Init(void);

/**
 * @brief   Configures the system clock
 * @details Sets up the PLL, prescalers and clock sources for optimal performance
 * @note    This function should be called before any peripheral initialization
 * @param   None
 * @retval  None
 */
void SystemClock_Config(void);

/**
 * @brief   Error handler function
 * @details Called when a critical error occurs during peripheral initialization
 *          or operation. Implements failsafe behavior.
 * @param   None
 * @retval  None
 */
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_H__ */
