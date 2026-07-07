#include "buzzer.h"

HAL_StatusTypeDef Buzzer_InitActive(Buzzer_t *buzzer, GPIO_TypeDef *port, uint16_t pin)
{
    if (buzzer == NULL || port == NULL) {
        return HAL_ERROR;
    }
    buzzer->mode = BUZZER_MODE_ACTIVE_GPIO;
    buzzer->port = port;
    buzzer->pin = pin;
    buzzer->htim = NULL;
    buzzer->channel = 0;
    buzzer->pwmTimerClockHz = 0;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    return HAL_OK;
}

HAL_StatusTypeDef Buzzer_InitPassive(Buzzer_t *buzzer, TIM_HandleTypeDef *htim, uint32_t channel, uint32_t timerClockHz)
{
    if (buzzer == NULL || htim == NULL || timerClockHz == 0) {
        return HAL_ERROR;
    }
    buzzer->mode = BUZZER_MODE_PASSIVE_PWM;
    buzzer->htim = htim;
    buzzer->channel = channel;
    buzzer->pwmTimerClockHz = timerClockHz;
    buzzer->port = NULL;
    buzzer->pin = 0;

    return HAL_TIM_PWM_Start(htim, channel);
}

void Buzzer_On(Buzzer_t *buzzer)
{
    if (buzzer == NULL) {
        return;
    }
    if (buzzer->mode == BUZZER_MODE_ACTIVE_GPIO) {
        HAL_GPIO_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_SET);
    }
}

void Buzzer_Off(Buzzer_t *buzzer)
{
    if (buzzer == NULL) {
        return;
    }
    if (buzzer->mode == BUZZER_MODE_ACTIVE_GPIO) {
        HAL_GPIO_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_RESET);
    } else if (buzzer->mode == BUZZER_MODE_PASSIVE_PWM) {
        __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, 0);
    }
}

HAL_StatusTypeDef Buzzer_Tone(Buzzer_t *buzzer, uint32_t frequencyHz, uint8_t dutyPercent)
{
    if (buzzer == NULL || buzzer->mode != BUZZER_MODE_PASSIVE_PWM) {
        return HAL_ERROR;
    }
    if (frequencyHz == 0 || dutyPercent > 100) {
        return HAL_ERROR;
    }

    uint32_t timerClk = buzzer->pwmTimerClockHz;
    uint32_t period = (timerClk / frequencyHz) - 1U;
    uint32_t pulse = (period + 1U) * dutyPercent / 100U;

    __HAL_TIM_SET_AUTORELOAD(buzzer->htim, period);
    __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, pulse);

    // Generate an update event to load registers
    buzzer->htim->Instance->EGR = TIM_EGR_UG;

    return HAL_OK;
}

bool Buzzer_IsOn(const Buzzer_t *buzzer)
{
    if (buzzer == NULL) {
        return false;
    }
    if (buzzer->mode == BUZZER_MODE_ACTIVE_GPIO) {
        return HAL_GPIO_ReadPin(buzzer->port, buzzer->pin) == GPIO_PIN_SET;
    } else {
        return __HAL_TIM_GET_COMPARE(buzzer->htim, buzzer->channel) > 0;
    }
}
