/**
 * @file tim_pwm.h
 * @brief STM32F429I-DISC1 Timer PWM Feature Header
 */

#ifndef TIM_PWM_H
#define TIM_PWM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef TIM_PWM_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period);
HAL_StatusTypeDef TIM_PWM_ConfigChannel(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t pulse);

HAL_StatusTypeDef TIM_PWM_Start(TIM_HandleTypeDef* htim, uint32_t channel);
HAL_StatusTypeDef TIM_PWM_Stop(TIM_HandleTypeDef* htim, uint32_t channel);

HAL_StatusTypeDef TIM_DMA_PWM_Start(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t* buffer, uint16_t length);
HAL_StatusTypeDef TIM_DMA_PWM_Stop(TIM_HandleTypeDef* htim, uint32_t channel);

HAL_StatusTypeDef TIM_PWM_Start_IT(TIM_HandleTypeDef* htim, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif // TIM_PWM_H
