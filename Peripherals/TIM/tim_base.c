/**
 * @file tim_base.c
 * @brief STM32F429I-DISC1 Timer Base Feature Implementation
 */

#include "tim_base.h"


/**
 * @brief  Initialize a basic timer as an up-counter.
 * @param  htim      Pointer to HAL timer handle structure
 * @param  instance  Timer instance (e.g., TIM2, TIM3)
 * @param  prescaler Timer prescaler value (divides input clock)
 * @param  period    Timer period (auto-reload value)
 */
/**
 * @note Timer clock and GPIO alternate function must be enabled/configured before calling this function.
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Base_Init(TIM_HandleTypeDef* htim, TIM_TypeDef* instance, uint32_t prescaler, uint32_t period) {
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }
    htim->Instance = instance;                        // Select timer peripheral
    htim->Init.Prescaler = prescaler;                 // Set prescaler (timer clock division)
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;      // Count up mode
    htim->Init.Period = period;                       // Set auto-reload (overflow) value
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;// No additional clock division
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; // Update ARR immediately
    return HAL_TIM_Base_Init(htim);                   // Call HAL to initialize timer
}

/**
 * @brief  Start the timer in base mode (no interrupts or DMA).
 * @param  htim Pointer to HAL timer handle structure
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Base_Start(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Start(htim); // Start timer counting
}

/**
 * @brief  Stop the timer in base mode.
 * @param  htim Pointer to HAL timer handle structure
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Base_Stop(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Stop(htim); // Stop timer counting
}

/**
 * @brief  Start timer in interrupt mode.
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Interrupt_Start_IT(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Start_IT(htim); // Start timer with interrupt
}

/**
 * @brief  Stop timer in interrupt mode.
 */
/**
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Interrupt_Stop_IT(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Stop_IT(htim); // Stop timer with interrupt
}
