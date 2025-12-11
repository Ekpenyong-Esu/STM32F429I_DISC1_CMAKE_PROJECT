/**
 * @file tim_extclock.c
 * @brief STM32F429I-DISC1 Timer External Clock Source Configuration Implementation
 */

#include "tim_extclock.h"

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
    uint32_t clockFilter)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    sClockSourceConfig.ClockSource = clockSource;
    if (clockSource == TIM_CLOCKSOURCE_ETRMODE2) {
        sClockSourceConfig.ClockPolarity  = clockPolarity;
        sClockSourceConfig.ClockPrescaler = clockPrescaler;
        sClockSourceConfig.ClockFilter    = clockFilter;
    }
    // For TIx/ITRx, only ClockSource is used; other fields ignored
    return HAL_TIM_ConfigClockSource(htim, &sClockSourceConfig);
}

/*
------------------------------------------------------------
// Example usage of TIM_ExtClock_Config
//------------------------------------------------------------
// Assume htim2 is a properly initialized TIM_HandleTypeDef for TIM2
//
// 1. Use external clock on TI1 (external clock mode 1)
//   TIM_ExtClock_Config(&htim2, TIM_CLOCKSOURCE_TI1, 0, 0, 0);
//
// 2. Use external clock on ETR (external clock mode 2, with filter)
//   TIM_ExtClock_Config(&htim2, TIM_CLOCKSOURCE_ETRMODE2, TIM_ETRPOLARITY_NONINVERTED, TIM_ETRPRESCALER_DIV1, 0x0F);
//
// 3. Use internal trigger from another timer (e.g., ITR0)
//   TIM_ExtClock_Config(&htim2, TIM_CLOCKSOURCE_ITR0, 0, 0, 0);
//------------------------------------------------------------
*/
