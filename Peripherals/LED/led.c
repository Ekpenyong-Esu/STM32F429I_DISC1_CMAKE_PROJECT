/**
 * @file led_simple.c
 * @brief Simplified LED driver implementation for STM32F429
 * @details Streamlined LED driver with essential functionality only
 * @version 2.0
 * @date 2025-09-27
 */

/* Includes ------------------------------------------------------------------*/
#include "led.h"
#include "SEGGER_SYSVIEW.h"

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Initialize GPIO for LED
 * @param   config Pointer to LED configuration
 */
static void Led_GPIO_Init(const LedConfig_t* config)
{
    SEGGER_SYSVIEW_OnUserStart(0x100);
    GPIO_InitTypeDef gpioInit = {0};

    /* Enable GPIO clock based on port */
    if (config->port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (config->port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (config->port == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (config->port == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (config->port == GPIOE) {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    } else if (config->port == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else if (config->port == GPIOG) {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    } else if (config->port == GPIOH) {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }

    /* Configure GPIO pin */
    gpioInit.Pin = config->pin;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    gpioInit.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(config->port, &gpioInit);
    SEGGER_SYSVIEW_OnUserStop(0x100);
}

/**
 * @brief   Set physical LED state
 * @param   handle Pointer to LED handle
 * @param   state LED state to set
 */
static void Led_SetPhysical(LedHandle_t* handle, LedState_t state)
{
    SEGGER_SYSVIEW_OnUserStart(0x101);
    GPIO_PinState pinState = GPIO_PIN_RESET;  // Initialize to default value

    /* Determine pin state based on active level and desired LED state */
    if (handle->config.activeLow) {
        pinState = (state == LED_ON) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    } else {
        pinState = (state == LED_ON) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    }

    HAL_GPIO_WritePin(handle->config.port, handle->config.pin, pinState);
    handle->state = state;
    SEGGER_SYSVIEW_OnUserStop(0x101);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize LED with default settings (active low)
 * @param   handle Pointer to LED handle
 * @param   port GPIO port
 * @param   pin GPIO pin number
 * @retval  true if successful, false otherwise
 */
bool Led_Init(LedHandle_t* handle, GPIO_TypeDef* port, uint16_t pin)
{
    SEGGER_SYSVIEW_OnUserStart(0x102);
    if (handle == NULL || port == NULL) {
        return false;
    }

    LedConfig_t config = {
        .port = port,
        .pin = pin,
        .activeLow = false  // Default: active low (common for STM32 Discovery LEDs)
    };

    bool result = Led_InitCustom(handle, &config);
    SEGGER_SYSVIEW_OnUserStop(0x102);
    return result;
}

/**
 * @brief   Initialize LED with custom configuration
 * @param   handle Pointer to LED handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Led_InitCustom(LedHandle_t* handle, const LedConfig_t* config)
{
    SEGGER_SYSVIEW_OnUserStart(0x103);
    if (handle == NULL || config == NULL || config->port == NULL) {
        return false;
    }

    /* Copy configuration */
    handle->config = *config;

    /* Initialize state */
    handle->state = LED_OFF;
    handle->blinking = false;
    handle->blinkPeriod = 0;
    handle->lastToggle = 0;
    handle->initialized = false;

    /* Initialize GPIO */
    Led_GPIO_Init(&handle->config);

    /* Set initial state (off) */
    Led_SetPhysical(handle, LED_OFF);

    handle->initialized = true;
    SEGGER_SYSVIEW_OnUserStop(0x103);
    return true;
}

/**
 * @brief   Turn LED on
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_On(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x104);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    handle->blinking = false;  // Stop any blinking
    Led_SetPhysical(handle, LED_ON);
    SEGGER_SYSVIEW_OnUserStop(0x104);
    return true;
}

/**
 * @brief   Turn LED off
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Off(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x105);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    handle->blinking = false;  // Stop any blinking
    Led_SetPhysical(handle, LED_OFF);
    SEGGER_SYSVIEW_OnUserStop(0x105);
    return true;
}

/**
 * @brief   Toggle LED state
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Toggle(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x106);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    LedState_t newState = (handle->state == LED_ON) ? LED_OFF : LED_ON;
    Led_SetPhysical(handle, newState);
    SEGGER_SYSVIEW_OnUserStop(0x106);
    return true;
}

/**
 * @brief   Set LED to specific state
 * @param   handle Pointer to LED handle
 * @param   state LED state to set
 * @retval  true if successful, false otherwise
 */
bool Led_SetState(LedHandle_t* handle, LedState_t state)
{
    SEGGER_SYSVIEW_OnUserStart(0x107);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    if (state == LED_ON) {
        return Led_On(handle);
    }

    bool result = Led_Off(handle);
    SEGGER_SYSVIEW_OnUserStop(0x107);
    return result;
}

/**
 * @brief   Get current LED state
 * @param   handle Pointer to LED handle
 * @retval  Current LED state
 */
LedState_t Led_GetState(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x108);
    if (handle == NULL || !handle->initialized) {
        return LED_OFF;
    }

    LedState_t state = handle->state;
    SEGGER_SYSVIEW_OnUserStop(0x108);
    return state;
}

/**
 * @brief   Check if LED is currently on
 * @param   handle Pointer to LED handle
 * @retval  true if LED is on, false otherwise
 */
bool Led_IsOn(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x109);
    bool result = (Led_GetState(handle) == LED_ON);
    SEGGER_SYSVIEW_OnUserStop(0x109);
    return result;
}

/**
 * @brief   Start LED blinking
 * @param   handle Pointer to LED handle
 * @param   periodMs Blink period in milliseconds (0 to stop blinking)
 * @retval  true if successful, false otherwise
 */
bool Led_StartBlink(LedHandle_t* handle, uint32_t periodMs)
{
    SEGGER_SYSVIEW_OnUserStart(0x10A);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    if (periodMs == 0) {
        return Led_StopBlink(handle);
    }

    handle->blinking = true;
    handle->blinkPeriod = periodMs;
    handle->lastToggle = HAL_GetTick();
    SEGGER_SYSVIEW_OnUserStop(0x10A);
    return true;
}

/**
 * @brief   Stop LED blinking
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_StopBlink(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x10B);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    handle->blinking = false;
    SEGGER_SYSVIEW_OnUserStop(0x10B);
    return true;
}

/**
 * @brief   Update LED state (call regularly for blinking)
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_Update(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x10C);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    if (!handle->blinking) {
        return true;  // Nothing to update
    }

    uint32_t currentTime = HAL_GetTick();
    uint32_t halfPeriod = handle->blinkPeriod / 2;

    if ((currentTime - handle->lastToggle) >= halfPeriod) {
        Led_Toggle(handle);
        handle->lastToggle = currentTime;
    }

    SEGGER_SYSVIEW_OnUserStop(0x10C);
    return true;
}

/**
 * @brief   Check if LED is blinking
 * @param   handle Pointer to LED handle
 * @retval  true if blinking, false otherwise
 */
bool Led_IsBlinking(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x10D);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    bool result = handle->blinking;
    SEGGER_SYSVIEW_OnUserStop(0x10D);
    return result;
}

/* Convenience functions for STM32F429 Discovery LEDs */

/**
 * @brief   Initialize green LED on STM32F429 Discovery
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_InitGreen(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x10E);
    bool result = Led_Init(handle, LED_GREEN_PORT, LED_GREEN_PIN);
    SEGGER_SYSVIEW_OnUserStop(0x10E);
    return result;
}

/**
 * @brief   Initialize red LED on STM32F429 Discovery
 * @param   handle Pointer to LED handle
 * @retval  true if successful, false otherwise
 */
bool Led_InitRed(LedHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x10F);
    bool result = Led_Init(handle, LED_RED_PORT, LED_RED_PIN);
    SEGGER_SYSVIEW_OnUserStop(0x10F);
    return result;
}
