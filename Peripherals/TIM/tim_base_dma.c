
/*
Usage notes:
// High-level: Use HAL_TIM_Base_Start_DMA for timer-driven DMA to timer registers (e.g., waveform, PWM, CCRx)
HAL_TIM_Base_Start_DMA(&htim2, data, length);

// Low-level: Use TIM_Base_LowLevel_DMA_Blink for timer-driven DMA to arbitrary peripherals (e.g., GPIO)
TIM_Base_LowLevel_DMA_Blink(&htim2, &hdma_tim2_up, led_pattern, &(GPIOG->ODR), 2);
*/
/**
 * @file tim_base_dma.c
 * @brief STM32F429I-DISC1 Timer Base DMA Feature Implementation
 */

#include "tim_base_dma.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_rcc.h"

// Example DMA handle (adjust as needed for your timer/stream/channel)
extern DMA_HandleTypeDef hdma_tim2_up;

/**
 * @brief  Configure and link DMA for timer base update event.
 * @param  htim   Pointer to HAL timer handle structure
 * @retval HAL status
 * @note   Call this once during initialization before using DMA start/stop.
 */
HAL_StatusTypeDef TIM_Base_DMA_Setup(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    __HAL_RCC_DMA1_CLK_ENABLE(); // Enable DMA1 clock (adjust for your hardware)
    // Example: TIM2 update event, DMA1 Stream1 Channel3 (adjust for your hardware)
    hdma_tim2_up.Instance = DMA1_Stream1;
    hdma_tim2_up.Init.Channel = DMA_CHANNEL_3;
    hdma_tim2_up.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim2_up.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim2_up.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim2_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tim2_up.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_tim2_up.Init.Mode = DMA_CIRCULAR;
    hdma_tim2_up.Init.Priority = DMA_PRIORITY_LOW;
    hdma_tim2_up.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_tim2_up) != HAL_OK) {
        return HAL_ERROR;
    }
    // Link DMA to timer
    __HAL_LINKDMA(htim, hdma[TIM_DMA_ID_UPDATE], hdma_tim2_up);
    // NVIC setup for DMA interrupt
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    return HAL_OK;
}

/**
 * @brief  Start timer base in DMA mode.
 * @param  htim   Pointer to HAL timer handle structure
 * @param  pData  Pointer to data buffer
 * @param  Length Number of data items
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Base_Start_DMA(TIM_HandleTypeDef* htim, uint32_t* pData, uint16_t Length) {
    if (htim == NULL || pData == NULL || Length == 0) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Start_DMA(htim, pData, Length);
}

/**
 * @brief  Stop timer base in DMA mode.
 * @param  htim Pointer to HAL timer handle structure
 * @retval HAL status
 */
HAL_StatusTypeDef TIM_Base_Stop_DMA(TIM_HandleTypeDef* htim) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Stop_DMA(htim);
}

/**
 * @brief  Use low-level HAL_DMA_Start and __HAL_TIM_ENABLE_DMA for timer-triggered DMA to GPIO.
 * @param  htim      Pointer to HAL timer handle structure
 * @param  hdma      Pointer to DMA handle structure
 * @param  src       Pointer to source buffer (e.g., LED pattern)
 * @param  dst       Pointer to destination register (e.g., GPIOG->ODR)
 * @param  length    Number of data items
 * @retval HAL status
 * @note   Use this for non-standard DMA destinations (e.g., GPIO), not supported by HAL_TIM_Base_Start_DMA.
 *         For timer register transfers (e.g., PWM, CCRx), use HAL_TIM_Base_Start_DMA (high-level).
 */
HAL_StatusTypeDef TIM_Base_LowLevel_DMA_Blink(TIM_HandleTypeDef* htim, DMA_HandleTypeDef* hdma, uint32_t* src, uint32_t* dst, uint16_t length) {
    if (htim == NULL || hdma == NULL || src == NULL || dst == NULL || length == 0) {
        return HAL_ERROR;
    }
    // Start DMA transfer from src buffer to destination register (e.g., GPIOG->ODR)
    if (HAL_DMA_Start(hdma, (uint32_t)src, (uint32_t)dst, length) != HAL_OK) {
        return HAL_ERROR;
    }
    // Enable timer DMA request on update event
    __HAL_TIM_ENABLE_DMA(htim, TIM_DMA_UPDATE);

    return HAL_OK;
}


HAL_StatusTypeDef TIM_Base_LowLevel_DMA_Blink_IT(TIM_HandleTypeDef* htim, DMA_HandleTypeDef* hdma, uint32_t* src, uint32_t* dst, uint16_t length) {
    if (htim == NULL || hdma == NULL || src == NULL || dst == NULL || length == 0) {
        return HAL_ERROR;
    }
    // Start DMA transfer from src buffer to destination register (e.g., GPIOG->ODR)
    if (HAL_DMA_Start_IT(hdma, (uint32_t)src, (uint32_t)dst, length) != HAL_OK) {
        return HAL_ERROR;
    }
    // Enable timer DMA request on update event
    __HAL_TIM_ENABLE_DMA(htim, TIM_DMA_UPDATE);

    return HAL_OK;
}
