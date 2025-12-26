/**
 * @file led_simple.h
 * @brief Simplified LED driver for STM32F429
 * @details Streamlined LED driver with essential functionality only
 * @version 2.0
 * @date 2025-09-27
 */

#ifndef LED_SIMPLE_H
#define LED_SIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief LED state enumeration
 */
typedef enum {
    LED_OFF = 0,        /**< LED is off */
    LED_ON = 1          /**< LED is on */
} LedState_t;

/**
 * @brief LED configuration structure
 */
typedef struct {
    GPIO_TypeDef* port;     /**< GPIO port */
    uint16_t pin;           /**< GPIO pin */
    bool activeLow;         /**< True if LED is active low */
} LedConfig_t;

/**
 * @brief LED handle structure
 */
typedef struct {
    LedConfig_t config;     /**< LED configuration */
    LedState_t state;       /**< Current LED state */
    bool blinking;          /**< Blinking enabled flag */
    uint32_t blinkPeriod;   /**< Blink period in milliseconds */
    uint32_t lastToggle;    /**< Last toggle time */
    bool initialized;       /**< Initialization flag */
} LedHandle_t;

/* Exported constants --------------------------------------------------------*/
#define LED_BLINK_FAST      200     /**< Fast blink period (ms) */
#define LED_BLINK_SLOW      1000    /**< Slow blink period (ms) */

/* STM32F429 Discovery Board LED definitions */
#define LED_GREEN_PORT      GPIOG
#define LED_GREEN_PIN       GPIO_PIN_13
#define LED_RED_PORT        GPIOG
#define LED_RED_PIN         GPIO_PIN_14

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize LED with default settings (active low)
 * @param   handle Pointer to LED handle
 * @param   port GPIO port
 * @param   pin GPIO pin number
 * @retval  true if successful, false otherwise
 */
bool Led_Init(LedHandle_t* handle, GPIO_TypeDef* port, uint16_t pin);

/**
 * @brief   Initialize LED with custom configuration
 * @param   handle Pointer to LED handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Led_InitCustom(LedHandle_t* handle, const LedConfig_t* config);

/**
 * @brief   Turn LED on
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_On(LedHandle_t* handle);

/**
 * @brief   Turn LED off
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Off(LedHandle_t* handle);

/**
 * @brief   Toggle LED state
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Toggle(LedHandle_t* handle);

/**
 * @brief   Set LED to specific state
 * @param   handle Pointer to LED handle
 * @param   state LED state to set
 * @retval  true if successful, false otherwise
 */
bool Led_SetState(LedHandle_t* handle, LedState_t state);

/**
 * @brief   Get current LED state
 * @param   handle Pointer to LED handle
 * @retval  Current LED state
 */
LedState_t Led_GetState(LedHandle_t* handle);

/**
 * @brief   Check if LED is currently on
 * @param   handle Pointer to LED handle
 * @retval  true if LED is on, false otherwise
 */
bool Led_IsOn(LedHandle_t* handle);

/**
 * @brief   Start LED blinking
 * @param   handle Pointer to LED handle
 * @param   periodMs Blink period in milliseconds (0 to stop blinking)
 * @retval  true if successful, false otherwise
 */
bool Led_StartBlink(LedHandle_t* handle, uint32_t periodMs);

/**
 * @brief   Stop LED blinking
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_StopBlink(LedHandle_t* handle);

/**
 * @brief   Update LED state (call regularly for blinking)
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Update(LedHandle_t* handle);

/**
 * @brief   Check if LED is blinking
 * @param   handle Pointer to LED handle
 * @retval  true if blinking, false otherwise
 */
bool Led_IsBlinking(LedHandle_t* handle);

/* Convenience functions for STM32F429 Discovery LEDs */

/**
 * @brief   Initialize green LED on STM32F429 Discovery
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_InitGreen(LedHandle_t* handle);

/**
 * @brief   Initialize red LED on STM32F429 Discovery
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_InitRed(LedHandle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* LED_SIMPLE_H */
