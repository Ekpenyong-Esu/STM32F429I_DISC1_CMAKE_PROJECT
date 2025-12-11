/**
 * @file tim_slave.c
 * @brief STM32F429I-DISC1 Timer Slave Mode Configuration Implementation
 */

#include "tim_slave.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

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
    uint32_t inputTrigger)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    sSlaveConfig.SlaveMode = slaveMode;
    sSlaveConfig.InputTrigger = inputTrigger;
    sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    return HAL_TIM_SlaveConfigSynchro(htim, &sSlaveConfig);
}
