/**
  ******************************************************************************
  * @file    tim.h
  * @brief   TIM module interface
  * @details This file contains all the function prototypes for
  *          the Timer (TIM) peripheral configuration.
  *          It provides APIs to initialize and control timer operations
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes Timer peripheral used in the application
 * @details Configures the timer with appropriate parameters for
 *          period, prescaler, clock division and counting mode
 * @param   None
 * @retval  None
 */
void TIM_Init(void);

/* Exported variables ---------------------------------------------------------*/
/**
 * @brief   TIM1 handle structure
 * @details Used by HAL functions to manage TIM1 operations
 *          typically used for PWM generation, precise timing or capture
 */
extern TIM_HandleTypeDef htim1;

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */
