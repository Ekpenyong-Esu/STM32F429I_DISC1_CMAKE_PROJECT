/**
 * @file tim_sync.c
 * @brief STM32F429I-DISC1 Timer Master-Slave Synchronization Implementation
 */

#include "tim_sync.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @brief  Synchronize master and slave timers.
 * @param  htimMaster   Master timer handle
 * @param  triggerOut   Master trigger output (e.g., TIM_TRGO_UPDATE)
 * @param  htimSlave    Slave timer handle
 * @param  slaveMode    Slave mode (e.g., TIM_SLAVEMODE_EXTERNAL1)
 * @param  inputTrigger Slave input trigger (e.g., TIM_TS_ITR0)
 * @retval HAL status
 * @note   Timers must be initialized before calling.
 */
HAL_StatusTypeDef TIM_SyncMasterSlave(
    TIM_HandleTypeDef* htimMaster,
    uint32_t triggerOut,
    TIM_HandleTypeDef* htimSlave,
    uint32_t slaveMode,
    uint32_t inputTrigger)
{
    if (htimMaster == NULL || htimSlave == NULL) {
        return HAL_ERROR;
    }
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    sMasterConfig.MasterOutputTrigger = triggerOut;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(htimMaster, &sMasterConfig) != HAL_OK) {
        return HAL_ERROR;
    }
    TIM_SlaveConfigTypeDef sSlaveConfig = {0};
    sSlaveConfig.SlaveMode = slaveMode;
    sSlaveConfig.InputTrigger = inputTrigger;
    sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
    sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    return HAL_TIM_SlaveConfigSynchro(htimSlave, &sSlaveConfig);
}
