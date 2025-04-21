/**
  ******************************************************************************
  * @file    tim.c
  * @brief   TIM module implementation
  * @details This file provides code for the configuration
  *          and initialization of the Timer peripheral.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "tim.h"
#include "../SYS/sys.h"

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   TIM1 handle structure
 * @details Used by HAL functions for TIM1 peripheral operations
 */
TIM_HandleTypeDef htim1;

/**
  * @brief  TIM Initialization Function
  * @details Configures the TIM1 peripheral with the following settings:
  *          - Prescaler: 0 (no division)
  *          - Counter mode: Up-counting
  *          - Period: 65535 (maximum for 16-bit timer)
  *          - Clock division: No division
  *          - Repetition counter: 0 (no repetition)
  *          - Auto-reload preload: Disabled
  *          - Clock source: Internal clock
  *          - Master/Slave mode: Disabled
  *
  * @note   TIM1 is an advanced timer commonly used for precise timing,
  *         PWM generation, or input capture applications
  * @param  None
  * @retval None
  */
void TIM_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};  /* Clock source configuration */
  TIM_MasterConfigTypeDef sMasterConfig = {0};      /* Master mode configuration */

  /* TIM1 basic configuration */
  htim1.Instance = TIM1;                            /* Select TIM1 peripheral */
  htim1.Init.Prescaler = 0;                         /* No prescaling (1:1) */
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;      /* Up-counting mode */
  htim1.Init.Period = 65535;                        /* Maximum period (16-bit) */
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; /* No clock division */
  htim1.Init.RepetitionCounter = 0;                 /* No repetition */
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; /* Auto-reload preload disabled */

  /* Initialize the timer base with the specified parameters */
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }

  /* Configure the timer clock source */
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL; /* Use internal clock */
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if configuration fails */
  }

  /* Configure the timer master mode */
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET; /* Reset trigger output (TRGO) */
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE; /* No master/slave mode */
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if configuration fails */
  }
}
