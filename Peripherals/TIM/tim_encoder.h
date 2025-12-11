/**
 * @file tim_encoder.h
 * @brief STM32F429I-DISC1 Timer Encoder Feature Header
 */

#ifndef TIM_ENCODER_H
#define TIM_ENCODER_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef TIM_Encoder_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period);
HAL_StatusTypeDef TIM_Encoder_Start(TIM_HandleTypeDef* htim);
HAL_StatusTypeDef TIM_Encoder_Stop(TIM_HandleTypeDef* htim);
HAL_StatusTypeDef TIM_Encoder_Start_IT(TIM_HandleTypeDef* htim);

#ifdef __cplusplus
}
#endif

#endif // TIM_ENCODER_H
