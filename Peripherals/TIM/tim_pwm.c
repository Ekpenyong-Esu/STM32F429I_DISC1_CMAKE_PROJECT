/**
 * @file tim_pwm.c
 * @brief STM32F429I-DISC1 Timer PWM Feature Implementation
 */

#include "tim_pwm.h"
#include "stm32f4xx_hal_tim.h"

/**
 * @brief  Initialize timer for PWM output.
 * @param  htim      Pointer to HAL timer handle structure
 * @param  instance  Timer instance (e.g., TIM2, TIM3)
 * @param  prescaler Timer prescaler value
 * @param  period    PWM period (auto-reload value)
 */
/**
 * @note Timer clock and GPIO alternate function must be enabled/configured before calling this function.
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_PWM_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period) {
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }
    htim->Instance = instance;                        // Select timer peripheral
    htim->Init.Prescaler = prescaler;                 // Set prescaler
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;      // Count up mode
    htim->Init.Period = period;                       // Set PWM period
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;// No additional clock division
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; // Update ARR immediately
    return HAL_TIM_PWM_Init(htim);                    // Initialize timer for PWM
}

/**
 * @brief  Configure a PWM channel's pulse width (duty cycle).
 * @param  htim   Pointer to HAL timer handle structure
 * @param  channel Timer channel (e.g., TIM_CHANNEL_1)
 * @param  pulse  Pulse width (compare value)
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_PWM_ConfigChannel(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t pulse) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;               // PWM mode 1
    sConfigOC.Pulse = pulse;                          // Set duty cycle
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;       // Output polarity
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;        // No fast mode
    return HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel); // Configure channel
}

/**
 * @brief  Start PWM output on a channel.
 * @param  htim   Pointer to HAL timer handle structure
 * @param  channel Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_PWM_Start(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Start(htim, channel); // Start PWM output
}

HAL_StatusTypeDef TIM_PWM_Start_IT(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Start_IT(htim, channel); // Start PWM output
}


/**
 * @brief  Stop PWM output on a channel.
 * @param  htim   Pointer to HAL timer handle structure
 * @param  channel Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_PWM_Stop(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Stop(htim, channel); // Stop PWM output
}

/**
 * @brief  Start PWM output on a channel using DMA for dynamic duty cycle updates.
 * @param  htim    Pointer to HAL timer handle structure
 * @param  channel Timer channel (e.g., TIM_CHANNEL_1)
 * @param  buffer  Pointer to array of compare values (duty cycles)
 * @param  length  Number of values in buffer
 * @note   DMA and timer must be initialized before calling this function.
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_DMA_PWM_Start(TIM_HandleTypeDef* htim, uint32_t channel, uint32_t* buffer, uint16_t length) {
    if (htim == NULL || buffer == NULL || length == 0) {
        return HAL_ERROR;
    }
    // Start PWM with DMA: buffer contains duty cycle values (CCR updates)
    return HAL_TIM_PWM_Start_DMA(htim, channel, buffer, length);
}

/**
 * @brief  Stop PWM output with DMA on a channel.
 * @param  htim    Pointer to HAL timer handle structure
 * @param  channel Timer channel
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_DMA_PWM_Stop(TIM_HandleTypeDef* htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Stop_DMA(htim, channel);
}
