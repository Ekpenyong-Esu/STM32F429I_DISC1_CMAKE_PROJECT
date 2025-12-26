#include "dht.h"
#include <stdio.h>

static DHT_Handle_t dht22;

void DHT_Example_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    DHT_Init(&dht22, DHT_TYPE_DHT22, GPIOC, GPIO_PIN_2); // adjust pin as wired
}

void DHT_Example_Task(void)
{
    if (DHT_Read(&dht22) == HAL_OK) {
        float h = DHT_GetHumidity(&dht22);
        float t = DHT_GetTemperatureC(&dht22);
        printf("DHT22: H=%.1f%% T=%.1fC\r\n", h, t);
    }
    HAL_Delay(2000); // DHT22 min interval
}
