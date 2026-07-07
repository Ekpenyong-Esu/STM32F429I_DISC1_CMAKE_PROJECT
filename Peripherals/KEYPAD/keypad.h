/**
  ******************************************************************************
  * @file    keypad.h
  * @brief   4x4 Matrix Keypad driver for STM32F429
  * @details Configurable row/column GPIO pins with debouncing support
  * @version 1.0
  * @date    2026-01-03
  ******************************************************************************
  */

#ifndef KEYPAD_H
#define KEYPAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/
#define KEYPAD_ROWS             4       /**< Number of rows */
#define KEYPAD_COLS             4       /**< Number of columns */
#define KEYPAD_DEBOUNCE_MS      20      /**< Debounce time in milliseconds */
#define KEYPAD_NO_KEY           '\0'    /**< No key pressed */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Keypad GPIO pin definition
 */
typedef struct {
    GPIO_TypeDef* port;     /**< GPIO port */
    uint16_t pin;           /**< GPIO pin */
} KeypadPin_t;

/**
 * @brief Keypad configuration structure
 */
typedef struct {
    KeypadPin_t rows[KEYPAD_ROWS];  /**< Row pins (directly connected) */
    KeypadPin_t cols[KEYPAD_COLS];  /**< Column pins (directly connected) */
    uint32_t debounceMs;            /**< Debounce time in milliseconds */
} KeypadConfig_t;

/**
 * @brief Keypad handle structure
 */
typedef struct {
    KeypadConfig_t config;                          /**< Keypad configuration */
    char keyMap[KEYPAD_ROWS][KEYPAD_COLS];          /**< Key mapping matrix */
    char lastKey;                                   /**< Last detected key */
    char currentKey;                                /**< Current key */
    uint32_t lastKeyTime;                           /**< Time of last key press */
    bool initialized;                               /**< Initialization flag */
} KeypadHandle_t;

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize keypad with configuration
 * @param   handle Pointer to keypad handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Keypad_Init(KeypadHandle_t* handle, const KeypadConfig_t* config);

/**
 * @brief   Set custom key mapping
 * @param   handle Pointer to keypad handle
 * @param   keyMap 4x4 array of key characters
 * @retval  true if successful, false otherwise
 * @note    Default mapping: 1-9, 0, A-D, *, #
 */
bool Keypad_SetKeyMap(KeypadHandle_t* handle, const char keyMap[KEYPAD_ROWS][KEYPAD_COLS]);

/**
 * @brief   Scan keypad and get pressed key (blocking with debounce)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key, or KEYPAD_NO_KEY if none
 */
char Keypad_GetKey(KeypadHandle_t* handle);

/**
 * @brief   Scan keypad without debouncing (raw read)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key, or KEYPAD_NO_KEY if none
 */
char Keypad_GetKeyRaw(KeypadHandle_t* handle);

/**
 * @brief   Check if any key is currently pressed
 * @param   handle Pointer to keypad handle
 * @retval  true if a key is pressed, false otherwise
 */
bool Keypad_IsKeyPressed(KeypadHandle_t* handle);

/**
 * @brief   Wait for a key press (blocking)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key
 */
char Keypad_WaitForKey(KeypadHandle_t* handle);

/**
 * @brief   Get the row and column of pressed key
 * @param   handle Pointer to keypad handle
 * @param   row Pointer to store row index (0-3)
 * @param   col Pointer to store column index (0-3)
 * @retval  true if a key is pressed, false otherwise
 */
bool Keypad_GetKeyPosition(KeypadHandle_t* handle, uint8_t* row, uint8_t* col);

/**
 * @brief   Get last pressed key (non-blocking)
 * @param   handle Pointer to keypad handle
 * @retval  Last pressed key character, or KEYPAD_NO_KEY if none
 */
char Keypad_GetLastKey(KeypadHandle_t* handle);

/**
 * @brief   Clear last key buffer
 * @param   handle Pointer to keypad handle
 */
void Keypad_ClearLastKey(KeypadHandle_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* KEYPAD_H */
