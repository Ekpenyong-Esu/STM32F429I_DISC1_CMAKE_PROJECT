/**
 * @file tim_input_capture.h
 * @brief STM32F429I-DISC1 Timer Input Capture Feature Header
 */

#ifndef TIM_INPUT_CAPTURE_H
#define TIM_INPUT_CAPTURE_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef TIM_IC_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period);
HAL_StatusTypeDef TIM_IC_ConfigChannel(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t polarity);
HAL_StatusTypeDef TIM_IC_Start(TIM_HandleTypeDef* htim, uint32_t channel);
HAL_StatusTypeDef TIM_IC_Stop(TIM_HandleTypeDef* htim, uint32_t channel);
HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef* htim, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif // TIM_INPUT_CAPTURE_H
