/**
  ******************************************************************************
  * @file    button_simple.c
  * @brief   Simplified Button implementation for STM32F429
  * @details Streamlined button driver with essential functionality only
  * @version 2.0
  * @date    2025-09-27
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "button.h"
#include "SEGGER_SYSVIEW.h"

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Initialize GPIO for button
 * @param   config Pointer to button configuration
 */
static void Button_GPIO_Init(const ButtonConfig_t* config)
{
    SEGGER_SYSVIEW_OnUserStart(0x200);
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
    }

    /* Configure GPIO pin */
    gpioInit.Pin = config->pin;
    if (config->enableInterrupt) {
        gpioInit.Mode = GPIO_MODE_IT_FALLING; // Interrupt on falling edge (active low)
    } else {
        gpioInit.Mode = GPIO_MODE_INPUT;
    }
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    gpioInit.Pull = config->activeLow ? GPIO_PULLUP : GPIO_PULLDOWN;

    HAL_GPIO_Init(config->port, &gpioInit);

    if (config->enableInterrupt) {
        uint32_t irq = 0;
        if (config->pin == GPIO_PIN_0) irq = EXTI0_IRQn;
        else if (config->pin == GPIO_PIN_1) irq = EXTI1_IRQn;
        else if (config->pin == GPIO_PIN_2) irq = EXTI2_IRQn;
        else if (config->pin == GPIO_PIN_3) irq = EXTI3_IRQn;
        else if (config->pin == GPIO_PIN_4) irq = EXTI4_IRQn;
        else if (config->pin >= GPIO_PIN_5 && config->pin <= GPIO_PIN_9) irq = EXTI9_5_IRQn;
        else if (config->pin >= GPIO_PIN_10 && config->pin <= GPIO_PIN_15) irq = EXTI15_10_IRQn;
        if (irq) {
            HAL_NVIC_SetPriority(irq, 2, 0);
            HAL_NVIC_EnableIRQ(irq);
        }
    }
    SEGGER_SYSVIEW_OnUserStop(0x200);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize button with default settings
 * @param   handle Pointer to button handle
 * @param   port GPIO port
 * @param   pin GPIO pin number
 * @retval  true if successful, false otherwise
 */
bool Button_Init(ButtonHandle_t* handle, GPIO_TypeDef* port, uint16_t pin)
{
    SEGGER_SYSVIEW_OnUserStart(0x201);
    if (handle == NULL || port == NULL) {
        return false;
    }

    ButtonConfig_t config = {
        .port = port,
        .pin = pin,
        .activeLow = false,  // Default: active low (pressed = 0V)
        .debounceMs = BUTTON_DEBOUNCE_DEFAULT,
        .enableInterrupt = true // Enable EXTI interrupt by default
    };
    bool result = Button_InitCustom(handle, &config);
    SEGGER_SYSVIEW_OnUserStop(0x201);
    return result;
}

/**
 * @brief   Initialize button with custom configuration
 * @param   handle Pointer to button handle
 * @param   config Pointer to configuration structure
 * @retval  true if successful, false otherwise
 */
bool Button_InitCustom(ButtonHandle_t* handle, const ButtonConfig_t* config)
{
    SEGGER_SYSVIEW_OnUserStart(0x202);
    if (handle == NULL || config == NULL || config->port == NULL) {
        return false;
    }

    /* Copy configuration */
    handle->config = *config;

    /* Initialize state */
    handle->state = BUTTON_RELEASED;
    handle->lastState = BUTTON_RELEASED;
    handle->lastChangeTime = 0;
    handle->initialized = false;

    /* Initialize GPIO */
    Button_GPIO_Init(&handle->config);

    /* Read initial state */
    handle->state = Button_ReadRaw(handle);
    handle->lastState = handle->state;
    handle->lastChangeTime = HAL_GetTick();
    handle->initialized = true;

    SEGGER_SYSVIEW_OnUserStop(0x202);
    return true;
}

/**
 * @brief   Read current button state (with debouncing)
 * @param   handle Pointer to button handle
 * @retval  Current button state
 */
ButtonState_t Button_Read(ButtonHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x203);
    if (handle == NULL || !handle->initialized) {
        return BUTTON_RELEASED;
    }

    ButtonState_t rawState = Button_ReadRaw(handle);
    uint32_t currentTime = HAL_GetTick();

    /* Check if state has changed */
    if (rawState != handle->lastState) {
        handle->lastChangeTime = currentTime;
        handle->lastState = rawState;
    }

    /* Apply debouncing */
    if ((currentTime - handle->lastChangeTime) >= handle->config.debounceMs) {
        handle->state = rawState;
    }

    ButtonState_t state = handle->state;
    SEGGER_SYSVIEW_OnUserStop(0x203);
    return state;
}

/**
 * @brief   Check if button is currently pressed
 * @param   handle Pointer to button handle
 * @retval  true if pressed, false otherwise
 */
bool Button_IsPressed(ButtonHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x204);
    bool result = (Button_Read(handle) == BUTTON_PRESSED);
    SEGGER_SYSVIEW_OnUserStop(0x204);
    return result;
}

/**
 * @brief   Check if button was just pressed (rising edge)
 * @param   handle Pointer to button handle
 * @retval  true if just pressed, false otherwise
 */
bool Button_WasPressed(ButtonHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x205);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    ButtonState_t oldState = handle->state;
    ButtonState_t newState = Button_Read(handle);

    bool result = (oldState == BUTTON_RELEASED && newState == BUTTON_PRESSED);
    SEGGER_SYSVIEW_OnUserStop(0x205);
    return result;
}

/**
 * @brief   Check if button was just released (falling edge)
 * @param   handle Pointer to button handle
 * @retval  true if just released, false otherwise
 */
bool Button_WasReleased(ButtonHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x206);
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    ButtonState_t oldState = handle->state;
    ButtonState_t newState = Button_Read(handle);

    bool result = (oldState == BUTTON_PRESSED && newState == BUTTON_RELEASED);
    SEGGER_SYSVIEW_OnUserStop(0x206);
    return result;
}

/**
 * @brief   Get raw button state (no debouncing)
 * @param   handle Pointer to button handle
 * @retval  Raw button state
 */
ButtonState_t Button_ReadRaw(ButtonHandle_t* handle)
{
    SEGGER_SYSVIEW_OnUserStart(0x207);
    if (handle == NULL || !handle->initialized) {
        return BUTTON_RELEASED;
    }

    GPIO_PinState pinState = HAL_GPIO_ReadPin(handle->config.port, handle->config.pin);

    /* Determine button state based on active level */
    if (handle->config.activeLow) {
        return (pinState == GPIO_PIN_RESET) ? BUTTON_PRESSED : BUTTON_RELEASED;
    }

    ButtonState_t state = (pinState == GPIO_PIN_SET) ? BUTTON_PRESSED : BUTTON_RELEASED;
    SEGGER_SYSVIEW_OnUserStop(0x207);
    return state;
}
