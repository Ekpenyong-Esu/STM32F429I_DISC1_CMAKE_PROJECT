/**
  ******************************************************************************
  * @file    keypad.c
  * @brief   4x4 Matrix Keypad implementation for STM32F429
  * @details Configurable row/column GPIO pins with debouncing support
  * @version 1.0
  * @date    2026-01-03
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "keypad.h"

/* Private constants ---------------------------------------------------------*/

/** @brief Default keymap for 4x4 matrix keypad */
static const char DEFAULT_KEYMAP[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Enable GPIO clock for a port
 * @param   port GPIO port
 */
static void Keypad_EnableClock(GPIO_TypeDef* port)
{
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if (port == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if (port == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    } else if (port == GPIOH) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    } else if (port == GPIOI) {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
}

/**
 * @brief   Initialize GPIO pins for keypad
 * @param   config Pointer to keypad configuration
 */
static void Keypad_GPIO_Init(const KeypadConfig_t* config)
{
    GPIO_InitTypeDef gpioInit = {0};

    /* Initialize row pins as outputs (directly drive low for scanning) */
    for (uint8_t i = 0; i < KEYPAD_ROWS; i++) {
        Keypad_EnableClock(config->rows[i].port);

        gpioInit.Pin = config->rows[i].pin;
        gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
        gpioInit.Pull = GPIO_NOPULL;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(config->rows[i].port, &gpioInit);

        /* Set all rows high initially */
        HAL_GPIO_WritePin(config->rows[i].port, config->rows[i].pin, GPIO_PIN_SET);
    }

    /* Initialize column pins as inputs with pull-up resistors */
    for (uint8_t i = 0; i < KEYPAD_COLS; i++) {
        Keypad_EnableClock(config->cols[i].port);

        gpioInit.Pin = config->cols[i].pin;
        gpioInit.Mode = GPIO_MODE_INPUT;
        gpioInit.Pull = GPIO_PULLUP;
        gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(config->cols[i].port, &gpioInit);
    }
}

/**
 * @brief   Set a specific row low for scanning
 * @param   config Pointer to keypad configuration
 * @param   row Row index to set low
 */
static void Keypad_SetRowLow(const KeypadConfig_t* config, uint8_t row)
{
    /* Set all rows high first */
    for (uint8_t i = 0; i < KEYPAD_ROWS; i++) {
        HAL_GPIO_WritePin(config->rows[i].port, config->rows[i].pin, GPIO_PIN_SET);
    }

    /* Set the specified row low */
    HAL_GPIO_WritePin(config->rows[row].port, config->rows[row].pin, GPIO_PIN_RESET);
}

/**
 * @brief   Read column pin state
 * @param   config Pointer to keypad configuration
 * @param   col Column index to read
 * @retval  true if column is low (key pressed), false otherwise
 */
static bool Keypad_ReadColumn(const KeypadConfig_t* config, uint8_t col)
{
    return HAL_GPIO_ReadPin(config->cols[col].port, config->cols[col].pin) == GPIO_PIN_RESET;
}

/**
 * @brief   Scan the keypad matrix
 * @param   handle Pointer to keypad handle
 * @param   row Pointer to store detected row
 * @param   col Pointer to store detected column
 * @retval  true if a key is detected, false otherwise
 */
static bool Keypad_ScanMatrix(KeypadHandle_t* handle, uint8_t* row, uint8_t* col)
{
    for (uint8_t r = 0; r < KEYPAD_ROWS; r++) {
        Keypad_SetRowLow(&handle->config, r);

        /* Small delay for signal settling */
        for (volatile uint32_t i = 0; i < 10; i++) {
            __NOP();
        }

        for (uint8_t c = 0; c < KEYPAD_COLS; c++) {
            if (Keypad_ReadColumn(&handle->config, c)) {
                *row = r;
                *col = c;

                /* Reset all rows high */
                for (uint8_t i = 0; i < KEYPAD_ROWS; i++) {
                    HAL_GPIO_WritePin(handle->config.rows[i].port,
                                      handle->config.rows[i].pin, GPIO_PIN_SET);
                }

                return true;
            }
        }
    }

    /* Reset all rows high */
    for (uint8_t i = 0; i < KEYPAD_ROWS; i++) {
        HAL_GPIO_WritePin(handle->config.rows[i].port,
                          handle->config.rows[i].pin, GPIO_PIN_SET);
    }

    return false;
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize keypad with configuration
 * @param   handle Pointer to keypad handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Keypad_Init(KeypadHandle_t* handle, const KeypadConfig_t* config)
{
    if (handle == NULL || config == NULL) {
        return false;
    }

    /* Validate configuration */
    for (uint8_t i = 0; i < KEYPAD_ROWS; i++) {
        if (config->rows[i].port == NULL) {
            return false;
        }
    }
    for (uint8_t i = 0; i < KEYPAD_COLS; i++) {
        if (config->cols[i].port == NULL) {
            return false;
        }
    }

    /* Copy configuration */
    handle->config = *config;

    /* Set default debounce if not specified */
    if (handle->config.debounceMs == 0) {
        handle->config.debounceMs = KEYPAD_DEBOUNCE_MS;
    }

    /* Set default keymap */
    for (uint8_t r = 0; r < KEYPAD_ROWS; r++) {
        for (uint8_t c = 0; c < KEYPAD_COLS; c++) {
            handle->keyMap[r][c] = DEFAULT_KEYMAP[r][c];
        }
    }

    /* Initialize state */
    handle->lastKey = KEYPAD_NO_KEY;
    handle->currentKey = KEYPAD_NO_KEY;
    handle->lastKeyTime = 0;
    handle->initialized = false;

    /* Initialize GPIO */
    Keypad_GPIO_Init(&handle->config);

    handle->initialized = true;
    return true;
}

/**
 * @brief   Set custom key mapping
 * @param   handle Pointer to keypad handle
 * @param   keyMap 4x4 array of key characters
 * @retval  true if successful, false otherwise
 */
bool Keypad_SetKeyMap(KeypadHandle_t* handle, const char keyMap[KEYPAD_ROWS][KEYPAD_COLS])
{
    if (handle == NULL || keyMap == NULL) {
        return false;
    }

    for (uint8_t r = 0; r < KEYPAD_ROWS; r++) {
        for (uint8_t c = 0; c < KEYPAD_COLS; c++) {
            handle->keyMap[r][c] = keyMap[r][c];
        }
    }

    return true;
}

/**
 * @brief   Scan keypad and get pressed key (blocking with debounce)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key, or KEYPAD_NO_KEY if none
 */
char Keypad_GetKey(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return KEYPAD_NO_KEY;
    }

    uint8_t row, col;
    char key = KEYPAD_NO_KEY;

    /* Scan for pressed key */
    if (Keypad_ScanMatrix(handle, &row, &col)) {
        key = handle->keyMap[row][col];
    }

    uint32_t currentTime = HAL_GetTick();

    /* Debounce logic */
    if (key != handle->currentKey) {
        /* Key state changed, start debounce timer */
        handle->currentKey = key;
        handle->lastKeyTime = currentTime;
        return KEYPAD_NO_KEY;
    }

    /* Check if debounce time has passed */
    if ((currentTime - handle->lastKeyTime) >= handle->config.debounceMs) {
        if (key != KEYPAD_NO_KEY && key != handle->lastKey) {
            /* New key detected after debounce */
            handle->lastKey = key;
            return key;
        }
    }

    /* Key released */
    if (key == KEYPAD_NO_KEY) {
        handle->lastKey = KEYPAD_NO_KEY;
    }

    return KEYPAD_NO_KEY;
}

/**
 * @brief   Scan keypad without debouncing (raw read)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key, or KEYPAD_NO_KEY if none
 */
char Keypad_GetKeyRaw(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return KEYPAD_NO_KEY;
    }

    uint8_t row, col;
    if (Keypad_ScanMatrix(handle, &row, &col)) {
        return handle->keyMap[row][col];
    }

    return KEYPAD_NO_KEY;
}

/**
 * @brief   Check if any key is currently pressed
 * @param   handle Pointer to keypad handle
 * @retval  true if a key is pressed, false otherwise
 */
bool Keypad_IsKeyPressed(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    uint8_t row, col;
    return Keypad_ScanMatrix(handle, &row, &col);
}

/**
 * @brief   Wait for a key press (blocking)
 * @param   handle Pointer to keypad handle
 * @retval  Character of pressed key
 */
char Keypad_WaitForKey(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return KEYPAD_NO_KEY;
    }

    char key = KEYPAD_NO_KEY;

    /* Wait until a key is pressed and debounced */
    while (key == KEYPAD_NO_KEY) {
        key = Keypad_GetKey(handle);
    }

    /* Wait for key release */
    while (Keypad_IsKeyPressed(handle)) {
        HAL_Delay(1);
    }

    return key;
}

/**
 * @brief   Get the row and column of pressed key
 * @param   handle Pointer to keypad handle
 * @param   row Pointer to store row index (0-3)
 * @param   col Pointer to store column index (0-3)
 * @retval  true if a key is pressed, false otherwise
 */
bool Keypad_GetKeyPosition(KeypadHandle_t* handle, uint8_t* row, uint8_t* col)
{
    if (handle == NULL || !handle->initialized || row == NULL || col == NULL) {
        return false;
    }

    return Keypad_ScanMatrix(handle, row, col);
}

/**
 * @brief   Get last pressed key (non-blocking)
 * @param   handle Pointer to keypad handle
 * @retval  Last pressed key character, or KEYPAD_NO_KEY if none
 */
char Keypad_GetLastKey(KeypadHandle_t* handle)
{
    if (handle == NULL || !handle->initialized) {
        return KEYPAD_NO_KEY;
    }

    return handle->lastKey;
}

/**
 * @brief   Clear last key buffer
 * @param   handle Pointer to keypad handle
 */
void Keypad_ClearLastKey(KeypadHandle_t* handle)
{
    if (handle != NULL && handle->initialized) {
        handle->lastKey = KEYPAD_NO_KEY;
    }
}
