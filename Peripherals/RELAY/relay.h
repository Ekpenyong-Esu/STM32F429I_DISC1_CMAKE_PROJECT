#ifndef RELAY_H
#define RELAY_H

#include "stm32f4xx.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RELAY_ACTIVE_HIGH = 0,
    RELAY_ACTIVE_LOW
} RelayPolarity_t;

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    RelayPolarity_t polarity;
} Relay_t;

HAL_StatusTypeDef Relay_Init(Relay_t *relay, GPIO_TypeDef *port, uint16_t pin, RelayPolarity_t polarity);
void Relay_On(Relay_t *relay);
void Relay_Off(Relay_t *relay);
bool Relay_IsOn(const Relay_t *relay);

#ifdef __cplusplus
}
#endif

#endif /* RELAY_H */
