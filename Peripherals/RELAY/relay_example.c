#include "relay.h"

// Example: drive a relay on PB0, active high
static Relay_t relay1;

void Relay_Example_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    Relay_Init(&relay1, GPIOB, GPIO_PIN_0, RELAY_ACTIVE_HIGH);
}

void Relay_Example_Task(void)
{
    Relay_On(&relay1);
    HAL_Delay(500);
    Relay_Off(&relay1);
    HAL_Delay(500);
}
