/**
  ******************************************************************************
  * @file    button_simple_example.c
  * @brief   Simple button usage examples
  * @details Demonstrates how to use the simplified button driver
  * @version 1.0
  * @date    2025-09-27
  ******************************************************************************
  */

#include "button.h"
#include <stdio.h>

/* Example button handle */
static ButtonHandle_t userButton;

/**
 * @brief   Example 1: Basic button initialization and reading
 */
void Button_Example_Basic(void)
{
    /* Initialize button on PA0 (STM32F429 Discovery user button) */
    if (!Button_Init(&userButton, GPIOA, GPIO_PIN_0)) {
        printf("Button initialization failed!\n");
        return;
    }

    printf("Button initialized successfully!\n");

    /* Main loop example */
    while (1) {
        /* Check if button is pressed */
        if (Button_IsPressed(&userButton)) {
            printf("Button is pressed\n");
        }

        /* Check for button press event */
        if (Button_WasPressed(&userButton)) {
            printf("Button was just pressed!\n");
        }

        /* Check for button release event */
        if (Button_WasReleased(&userButton)) {
            printf("Button was just released!\n");
        }

        HAL_Delay(10); /* Small delay for debouncing */
    }
}

/**
 * @brief   Example 2: Custom button configuration
 */
void Button_Example_Custom(void)
{
    /* Custom button configuration */
    ButtonConfig_t config = {
        .port = GPIOB,           /* Use GPIOB */
        .pin = GPIO_PIN_1,       /* Pin 1 */
        .activeLow = false,      /* Active high button */
        .debounceMs = 30         /* 30ms debounce time */
    };

    /* Initialize with custom configuration */
    if (!Button_InitCustom(&userButton, &config)) {
        printf("Custom button initialization failed!\n");
        return;
    }

    printf("Custom button initialized successfully!\n");

    /* Usage is the same as basic example */
    while (1) {
        if (Button_WasPressed(&userButton)) {
            printf("Custom button pressed!\n");
        }
        HAL_Delay(10);
    }
}

/**
 * @brief   Example 3: Toggle LED on button press
 */
void Button_Example_LED_Toggle(void)
{
    /* Initialize button */
    Button_Init(&userButton, GPIOA, GPIO_PIN_0);

    /* Initialize LED (assuming PG13 for STM32F429 Discovery green LED) */
    __HAL_RCC_GPIOG_CLK_ENABLE();
    GPIO_InitTypeDef ledInit = {
        .Pin = GPIO_PIN_13,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Pull = GPIO_NOPULL
    };
    HAL_GPIO_Init(GPIOG, &ledInit);

    printf("Button LED toggle example started\n");

    while (1) {
        /* Toggle LED on button press */
        if (Button_WasPressed(&userButton)) {
            HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_13);
            printf("LED toggled!\n");
        }
        HAL_Delay(10);
    }
}

/**
 * @brief   Example 4: Button state monitoring
 */
void Button_Example_Monitor(void)
{
    Button_Init(&userButton, GPIOA, GPIO_PIN_0);

    ButtonState_t lastState = BUTTON_RELEASED;
    uint32_t pressCount = 0;

    printf("Button monitoring started\n");

    while (1) {
        ButtonState_t currentState = Button_Read(&userButton);

        /* Print state changes */
        if (currentState != lastState) {
            if (currentState == BUTTON_PRESSED) {
                pressCount++;
                printf("Button pressed (count: %u)\n", (unsigned int)pressCount);
            } else {
                printf("Button released\n");
            }
            lastState = currentState;
        }

        HAL_Delay(10);
    }
}
