/**
 * @file tim_output_compare.h
 * @brief STM32F429I-DISC1 Timer Output Compare Feature Header
 */

#ifndef TIM_OUTPUT_COMPARE_H
#define TIM_OUTPUT_COMPARE_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef TIM_OC_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period);
HAL_StatusTypeDef TIM_OC_ConfigChannel(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t pulse);
HAL_StatusTypeDef TIM_OC_Start(TIM_HandleTypeDef* htim, uint32_t channel);
HAL_StatusTypeDef TIM_OC_Stop(TIM_HandleTypeDef* htim, uint32_t channel);
HAL_StatusTypeDef TIM_OC_Start_IT(TIM_HandleTypeDef* htim, uint32_t channel);
HAL_StatusTypeDef TIM_OC_Stop_IT(TIM_HandleTypeDef* htim, uint32_t channel);
#ifdef __cplusplus
}
#endif

#endif // TIM_OUTPUT_COMPARE_H
