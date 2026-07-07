#include "relay.h"

static inline GPIO_PinState relay_active_state(RelayPolarity_t polarity)
{
    return (polarity == RELAY_ACTIVE_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

static inline GPIO_PinState relay_inactive_state(RelayPolarity_t polarity)
{
    return (polarity == RELAY_ACTIVE_HIGH) ? GPIO_PIN_RESET : GPIO_PIN_SET;
}

HAL_StatusTypeDef Relay_Init(Relay_t *relay, GPIO_TypeDef *port, uint16_t pin, RelayPolarity_t polarity)
{
    if (relay == NULL || port == NULL) {
        return HAL_ERROR;
    }

    relay->port = port;
    relay->pin = pin;
    relay->polarity = polarity;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(port, pin, relay_inactive_state(polarity));
    return HAL_OK;
}

void Relay_On(Relay_t *relay)
{
    if (relay == NULL) {
        return;
    }
    HAL_GPIO_WritePin(relay->port, relay->pin, relay_active_state(relay->polarity));
}

void Relay_Off(Relay_t *relay)
{
    if (relay == NULL) {
        return;
    }
    HAL_GPIO_WritePin(relay->port, relay->pin, relay_inactive_state(relay->polarity));
}

bool Relay_IsOn(const Relay_t *relay)
{
    if (relay == NULL) {
        return false;
    }
    GPIO_PinState state = HAL_GPIO_ReadPin(relay->port, relay->pin);
    return (relay->polarity == RELAY_ACTIVE_HIGH) ? (state == GPIO_PIN_SET) : (state == GPIO_PIN_RESET);
}
