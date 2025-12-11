/**
 * @file tim_output_compare.c
 * @brief STM32F429I-DISC1 Timer Output Compare Feature Implementation
 */

#include "tim_output_compare.h"

/**
 * @brief  Initialize timer for output compare mode.
 * @param  htim      Pointer to HAL timer handle structure
 * @param  instance  Timer instance (e.g., TIM2, TIM3)
 * @param  prescaler Timer prescaler value
 * @param  period    Timer period (auto-reload value)
 */
/**
 * @note Timer clock and GPIO alternate function must be enabled/configured before calling this function.
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_OC_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period) {
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }
    htim->Instance = instance;                        // Select timer peripheral
    htim->Init.Prescaler = prescaler;                 // Set prescaler
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;      // Count up mode
    htim->Init.Period = period;                       // Set period
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;// No additional clock division
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; // Update ARR immediately
    return HAL_TIM_OC_Init(htim);                     // Initialize timer for output compare
}

/**
 * @brief  Configure an output compare channel.
 * @param  htim     Pointer to HAL timer handle structure
 * @param  channel  Timer channel (e.g., TIM_CHANNEL_1)
 * @param  pulse    Compare value (when to toggle/set output)
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_OC_ConfigChannel(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t pulse) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;             // Toggle output on match
    sConfigOC.Pulse = pulse;                          // Set compare value
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;       // Output polarity
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;        // No fast mode
    return HAL_TIM_OC_ConfigChannel(htim, &sConfigOC, channel); // Configure channel
}

/**
 * @brief  Start output compare on a channel.
 * @param  htim     Pointer to HAL timer handle structure
 * @param  channel  Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_OC_Start(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Start(htim, channel); // Start output compare
}

HAL_StatusTypeDef TIM_OC_Start_IT(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Start_IT(htim, channel); // Start output compare with interrupt
}

/**
 * @brief  Stop output compare on a channel.
 * @param  htim     Pointer to HAL timer handle structure
 * @param  channel  Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_OC_Stop(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Stop(htim, channel); // Stop output compare
}
