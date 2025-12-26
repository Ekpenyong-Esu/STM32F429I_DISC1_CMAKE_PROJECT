/**
 * @file tim_base.h
 * @brief STM32F429I-DISC1 Timer Base Feature Header
 */

#ifndef TIM_BASE_H
#define TIM_BASE_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal_tim.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef TIM_Base_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period);
HAL_StatusTypeDef TIM_Base_Start(TIM_HandleTypeDef* htim);
HAL_StatusTypeDef TIM_Base_Stop(TIM_HandleTypeDef* htim);


HAL_StatusTypeDef TIM_Interrupt_Start_IT(TIM_HandleTypeDef* htim);
HAL_StatusTypeDef TIM_Interrupt_Stop_IT(TIM_HandleTypeDef* htim);
#ifdef __cplusplus
}
#endif

#endif // TIM_BASE_H
