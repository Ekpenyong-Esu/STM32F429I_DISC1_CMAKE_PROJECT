/**
 * @file tim_encoder.c
 * @brief STM32F429I-DISC1 Timer Encoder Feature Implementation
 */

#include "tim_encoder.h"

/**
 * @brief  Initialize timer for encoder interface mode.
 * @param  htim      Pointer to HAL timer handle structure
 * @param  instance  Timer instance (e.g., TIM2, TIM3)
 * @param  prescaler Timer prescaler value
 * @param  period    Timer period (auto-reload value)
 */
/**
 * @note Timer clock and GPIO alternate function must be enabled/configured before calling this function.
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Encoder_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period) {
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }
    htim->Instance = instance;                        // Select timer peripheral
    htim->Init.Prescaler = prescaler;                 // Set prescaler
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;      // Count up mode
    htim->Init.Period = period;                       // Set period (max count)
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;// No additional clock division
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; // Update ARR immediately
    return HAL_TIM_Encoder_Init(htim, NULL);          // Initialize timer for encoder mode
}

/**
 * @brief  Start encoder interface (counts quadrature pulses).
 * @param  htim Pointer to HAL timer handle structure
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Encoder_Start(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL); // Start encoder counting
}



HAL_StatusTypeDef TIM_Encoder_Start_IT(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Encoder_Start_IT(htim, TIM_CHANNEL_ALL); // Start encoder counting
}
/**
 * @brief  Stop encoder interface.
 * @param  htim Pointer to HAL timer handle structure
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Encoder_Stop(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Encoder_Stop(htim, TIM_CHANNEL_ALL); // Stop encoder counting
}
