/**
 * @file tim_extclock.h
 * @brief STM32F429I-DISC1 Timer External Clock Source Configuration Header
 */

#ifndef TIM_EXTCLOCK_H
#define TIM_EXTCLOCK_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Configure timer to use an external clock source (TIx, ETR, or ITRx).
 * @param  htim           Timer handle
 * @param  clockSource    External clock source type
 * @param  clockPolarity  ETR mode: polarity
 * @param  clockPrescaler ETR mode: prescaler
 * @param  clockFilter    ETR mode: digital filter (0-15)
 * @retval HAL status
 * @note   For TIx/ITRx, only htim and clockSource are used; other params are for ETR mode.
 */
HAL_StatusTypeDef TIM_ExtClock_Config(
    TIM_HandleTypeDef* htim,
    uint32_t clockSource,
    uint32_t clockPolarity,
    uint32_t clockPrescaler,
    uint32_t clockFilter);

#ifdef __cplusplus
}
#endif

#endif // TIM_EXTCLOCK_H
