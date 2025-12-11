/**
 * @file tim_slave.h
 * @brief STM32F429I-DISC1 Timer Slave Mode Configuration Header
 */

#ifndef TIM_SLAVE_H
#define TIM_SLAVE_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Configure timer in slave mode (e.g., for synchronization or external trigger).
 * @param  htim         Timer handle
 * @param  slaveMode    Slave mode selection (e.g., TIM_SLAVEMODE_EXTERNAL1, TIM_SLAVEMODE_GATED, etc.)
 * @param  inputTrigger Trigger input selection (e.g., TIM_TS_ITR0, TIM_TS_TI1FP1, etc.)
 * @retval HAL status
 * @note   Ensure timer and trigger source are properly initialized before calling.
 */
HAL_StatusTypeDef TIM_SlaveConfig(
    TIM_HandleTypeDef* htim,
    uint32_t slaveMode,
    uint32_t inputTrigger);

#ifdef __cplusplus
}
#endif

#endif // TIM_SLAVE_H
