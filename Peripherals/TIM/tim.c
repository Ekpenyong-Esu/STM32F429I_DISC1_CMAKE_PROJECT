/**
 * @file tim.c
 * @brief STM32F429I-DISC1 Unified Timer Driver Implementation
 */

#include "tim.h"
#include "log.h"

/* ========================== Base Timer ========================== */

HAL_StatusTypeDef TIM_Init(TIM_HandleTypeDef *htim,
                           TIM_TypeDef *instance,
                           uint32_t prescaler,
                           uint32_t period)
{
    log_debug("TIM: Initializing Timer");

    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_StatusTypeDef status = HAL_TIM_Base_Init(htim);
    if (status == HAL_OK) {
        log_debug("TIM: Timer initialized successfully");
    }
    return status;
}

HAL_StatusTypeDef TIM_Start(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Start(htim);
}

HAL_StatusTypeDef TIM_Stop(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Stop(htim);
}

HAL_StatusTypeDef TIM_Start_IT(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Start_IT(htim);
}

HAL_StatusTypeDef TIM_Stop_IT(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Stop_IT(htim);
}

uint32_t TIM_GetCounter(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return 0U;
    }
    return __HAL_TIM_GET_COUNTER(htim);
}

void TIM_SetCounter(TIM_HandleTypeDef *htim, uint32_t value)
{
    if (htim != NULL) {
        __HAL_TIM_SET_COUNTER(htim, value);
    }
}

/* ========================== PWM Output ========================== */

HAL_StatusTypeDef TIM_PWM_Init(TIM_HandleTypeDef *htim,
                               TIM_TypeDef *instance,
                               uint32_t prescaler,
                               uint32_t period)
{
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    return HAL_TIM_PWM_Init(htim);
}

HAL_StatusTypeDef TIM_PWM_ConfigChannel(TIM_HandleTypeDef *htim,
                                        uint32_t channel,
                                        uint32_t pulse)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    return HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel);
}

HAL_StatusTypeDef TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Start(htim, channel);
}

HAL_StatusTypeDef TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Stop(htim, channel);
}

void TIM_PWM_SetDuty(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse)
{
    if (htim == NULL) {
        return;
    }
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}

/* ========================== Input Capture ========================== */

HAL_StatusTypeDef TIM_IC_Init(TIM_HandleTypeDef *htim,
                              TIM_TypeDef *instance,
                              uint32_t prescaler,
                              uint32_t period)
{
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    return HAL_TIM_IC_Init(htim);
}

HAL_StatusTypeDef TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim,
                                       uint32_t channel,
                                       uint32_t polarity)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }

    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = polarity;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0U;

    return HAL_TIM_IC_ConfigChannel(htim, &sConfigIC, channel);
}

HAL_StatusTypeDef TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Start(htim, channel);
}

HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Start_IT(htim, channel);
}

HAL_StatusTypeDef TIM_IC_Stop(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Stop(htim, channel);
}

uint32_t TIM_IC_GetCapture(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return 0U;
    }
    return HAL_TIM_ReadCapturedValue(htim, channel);
}

/* ========================== Output Compare ========================== */

HAL_StatusTypeDef TIM_OC_Init(TIM_HandleTypeDef *htim,
                              TIM_TypeDef *instance,
                              uint32_t prescaler,
                              uint32_t period)
{
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    return HAL_TIM_OC_Init(htim);
}

HAL_StatusTypeDef TIM_OC_ConfigChannel(TIM_HandleTypeDef *htim,
                                       uint32_t channel,
                                       uint32_t pulse)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
    sConfigOC.Pulse = pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    return HAL_TIM_OC_ConfigChannel(htim, &sConfigOC, channel);
}

HAL_StatusTypeDef TIM_OC_Start(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Start(htim, channel);
}

HAL_StatusTypeDef TIM_OC_Stop(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Stop(htim, channel);
}

/* ========================== Encoder Interface ========================== */

HAL_StatusTypeDef TIM_Encoder_Init(TIM_HandleTypeDef *htim,
                                   TIM_TypeDef *instance,
                                   uint32_t period)
{
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = 0U;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    /* Encoder configuration: count on both TI1 and TI2 edges */
    TIM_Encoder_InitTypeDef sEncoderConfig = {0};
    sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC1Filter = 0U;
    sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC2Filter = 0U;

    return HAL_TIM_Encoder_Init(htim, &sEncoderConfig);
}

HAL_StatusTypeDef TIM_Encoder_Start(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
}

HAL_StatusTypeDef TIM_Encoder_Stop(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Encoder_Stop(htim, TIM_CHANNEL_ALL);
}

int32_t TIM_Encoder_GetCount(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return 0;
    }
    return (int32_t)__HAL_TIM_GET_COUNTER(htim);
}

void TIM_Encoder_Reset(TIM_HandleTypeDef *htim)
{
    if (htim != NULL) {
        __HAL_TIM_SET_COUNTER(htim, 0U);
    }
}
