#ifndef BUZZER_H
#define BUZZER_H

#include "stm32f4xx.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BUZZER_MODE_ACTIVE_GPIO = 0,  // Active buzzer driven by GPIO high/low
    BUZZER_MODE_PASSIVE_PWM       // Passive buzzer driven by PWM
} BuzzerMode_t;

typedef struct {
    BuzzerMode_t mode;
    GPIO_TypeDef *port;
    uint16_t pin;
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint32_t pwmTimerClockHz; // Timer input clock (for passive mode)
} Buzzer_t;

HAL_StatusTypeDef Buzzer_InitActive(Buzzer_t *buzzer, GPIO_TypeDef *port, uint16_t pin);
HAL_StatusTypeDef Buzzer_InitPassive(Buzzer_t *buzzer, TIM_HandleTypeDef *htim, uint32_t channel, uint32_t timerClockHz);
void Buzzer_On(Buzzer_t *buzzer);
void Buzzer_Off(Buzzer_t *buzzer);
HAL_StatusTypeDef Buzzer_Tone(Buzzer_t *buzzer, uint32_t frequencyHz, uint8_t dutyPercent);
bool Buzzer_IsOn(const Buzzer_t *buzzer);

#ifdef __cplusplus
}
#endif

#endif /* BUZZER_H */
