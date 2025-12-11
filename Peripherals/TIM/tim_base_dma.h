/**
 * @file tim_base_dma.h
 * @brief STM32F429I-DISC1 Timer Base DMA Feature Header
 */

#ifndef TIM_BASE_DMA_H
#define TIM_BASE_DMA_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Configure and link DMA for timer base update event.
 * @param  htim   Pointer to HAL timer handle structure
 * @retval HAL status
 * @note   Call this once during initialization before using DMA start/stop.
 */
HAL_StatusTypeDef TIM_Base_DMA_Setup(TIM_HandleTypeDef* htim);

/**
 * @brief  Start timer base in DMA mode.
 * @param  htim   Pointer to HAL timer handle structure
 * @param  pData  Pointer to data buffer
 * @param  Length Number of data items
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Base_Start_DMA(TIM_HandleTypeDef* htim, uint32_t* pData, uint16_t Length);

/**
 * @brief  Stop timer base in DMA mode.
 * @param  htim Pointer to HAL timer handle structure
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Base_Stop_DMA(TIM_HandleTypeDef* htim);

HAL_StatusTypeDef TIM_Base_LowLevel_DMA_Blink(TIM_HandleTypeDef* htim, DMA_HandleTypeDef* hdma, uint32_t* src, uint32_t* dst, uint16_t length);
HAL_StatusTypeDef TIM_Base_LowLevel_DMA_Blink_IT(TIM_HandleTypeDef* htim, DMA_HandleTypeDef* hdma, uint32_t* src, uint32_t* dst, uint16_t length);
#ifdef __cplusplus
}
#endif

#endif // TIM_BASE_DMA_H
