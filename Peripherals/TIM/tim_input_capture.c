/**
 * @file tim_input_capture.c
 * @brief STM32F429I-DISC1 Timer Input Capture Feature Implementation
 */

#include "tim_input_capture.h"

/**
 * @brief  Initialize timer for input capture mode.
 * @param  htim      Pointer to HAL timer handle structure
 * @param  instance  Timer instance (e.g., TIM2, TIM3)
 * @param  prescaler Timer prescaler value
 * @param  period    Timer period (auto-reload value)
 */
/**
 * @note Timer clock and GPIO alternate function must be enabled/configured before calling this function.
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_IC_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period) {
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }
    htim->Instance = instance;                        // Select timer peripheral
    htim->Init.Prescaler = prescaler;                 // Set prescaler
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;      // Count up mode
    htim->Init.Period = period;                       // Set period
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;// No additional clock division
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; // Update ARR immediately
    return HAL_TIM_IC_Init(htim);                     // Initialize timer for input capture
}

/**
 * @brief  Configure an input capture channel.
 * @param  htim     Pointer to HAL timer handle structure
 * @param  channel  Timer channel (e.g., TIM_CHANNEL_1)
 * @param  polarity Edge polarity to capture (rising/falling)
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_IC_ConfigChannel(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t polarity) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = polarity;                  // Set edge polarity
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI; // Direct input
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;           // No prescaler
    sConfigIC.ICFilter = 0;                           // No filter
    return HAL_TIM_IC_ConfigChannel(htim, &sConfigIC, channel); // Configure channel
}

/**
 * @brief  Start input capture on a channel.
 * @param  htim     Pointer to HAL timer handle structure
 * @param  channel  Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_IC_Start(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Start(htim, channel); // Start input capture
}

/**
 * @brief  Start input capture on a channel.
 * @param  htim     Pointer to HAL timer handle structure
 * @param  channel  Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Start_IT(htim, channel); // Start input capture
}

/**
 * @brief  Stop input capture on a channel.
 * @param  htim     Pointer to HAL timer handle structure
 * @param  channel  Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_IC_Stop(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Stop(htim, channel); // Stop input capture
}

/**
 * @brief  Stop input capture on a channel.
 * @param  htim     Pointer to HAL timer handle structure
 * @param  channel  Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_IC_IT_Stop(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Stop_IT(htim, channel); // Stop input capture
}
