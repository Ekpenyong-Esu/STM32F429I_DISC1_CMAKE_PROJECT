#include "buzzer.h"

// Example for active buzzer on PB1
static Buzzer_t activeBuzzer;

// Example for passive buzzer on PA6 (TIM3_CH1)
extern TIM_HandleTypeDef htim3;
static Buzzer_t passiveBuzzer;

void Buzzer_Example_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    Buzzer_InitActive(&activeBuzzer, GPIOB, GPIO_PIN_1);

    // Timer clock must be provided by user; typical: TIM3 on APB1 = 84MHz
    Buzzer_InitPassive(&passiveBuzzer, &htim3, TIM_CHANNEL_1, 84000000U);
}

void Buzzer_Example_Task(void)
{
    // Active buzzer: simple on/off
    Buzzer_On(&activeBuzzer);
    HAL_Delay(200);
    Buzzer_Off(&activeBuzzer);

    // Passive buzzer: 2 kHz tone at 50% duty for 200 ms
    Buzzer_Tone(&passiveBuzzer, 2000U, 50);
    HAL_Delay(200);
    Buzzer_Off(&passiveBuzzer);

    HAL_Delay(600);
}
