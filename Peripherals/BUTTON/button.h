/**
  ******************************************************************************
  * @file    button_simple.h
  * @brief   Simplified Button interface for STM32F429
  * @details Streamlined button driver with essential functionality only
  * @version 2.0
  * @date    2025-09-27
  ******************************************************************************
  */

#ifndef BUTTON_SIMPLE_H
#define BUTTON_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Button state enumeration
 */
typedef enum {
    BUTTON_RELEASED = 0,    /**< Button is released */
    BUTTON_PRESSED = 1      /**< Button is pressed */
} ButtonState_t;

/**
 * @brief Button configuration structure
 */
typedef struct {
    GPIO_TypeDef* port;     /**< GPIO port */
    uint16_t pin;           /**< GPIO pin */
    bool activeLow;         /**< True if button is active low */
    uint32_t debounceMs;    /**< Debounce time in milliseconds */
    bool enableInterrupt;   /**< Enable EXTI interrupt */
} ButtonConfig_t;

/**
 * @brief Button handle structure
 */
typedef struct {
    ButtonConfig_t config;      /**< Button configuration */
    ButtonState_t state;        /**< Current button state */
    ButtonState_t lastState;    /**< Previous button state */
    uint32_t lastChangeTime;    /**< Time of last state change */
    bool initialized;           /**< Initialization flag */
} ButtonHandle_t;

/* Exported constants --------------------------------------------------------*/
#define BUTTON_DEBOUNCE_DEFAULT     50      /**< Default debounce time (ms) */

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize button with default settings
 * @param   handle Pointer to button handle
 * @param   port GPIO port
 * @param   pin GPIO pin number
 * @retval  true if successful, false otherwise
 */
bool Button_Init(ButtonHandle_t* handle, GPIO_TypeDef* port, uint16_t pin);

/**
 * @brief   Initialize button with custom configuration
 * @param   handle Pointer to button handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Button_InitCustom(ButtonHandle_t* handle, const ButtonConfig_t* config);

/**
 * @brief   Read current button state (with debouncing)
 * @param   handle Pointer to button handle
 * @retval  Current button state
 */
ButtonState_t Button_Read(ButtonHandle_t* handle);

/**
 * @brief   Check if button is currently pressed
 * @param   handle Pointer to button handle
 * @retval  true if pressed, false otherwise
 */
bool Button_IsPressed(ButtonHandle_t* handle);

/**
 * @brief   Check if button was just pressed (rising edge)
 * @param   handle Pointer to button handle
 * @retval  true if just pressed, false otherwise
 */
bool Button_WasPressed(ButtonHandle_t* handle);

/**
 * @brief   Check if button was just released (falling edge)
 * @param   handle Pointer to button handle
 * @retval  true if just released, false otherwise
 */
bool Button_WasReleased(ButtonHandle_t* handle);

/**
 * @brief   Get raw button state (no debouncing)
 * @param   handle Pointer to button handle
 * @retval  Raw button state
 */
ButtonState_t Button_ReadRaw(ButtonHandle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_SIMPLE_H */
