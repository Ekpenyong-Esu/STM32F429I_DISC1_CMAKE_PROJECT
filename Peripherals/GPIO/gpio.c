/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO module implementation
  * @details This file provides code for the configuration and control
  *          of the GPIO pins used in the application.
  * @version 1.0
  * @date    2025-09-03
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "main.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t button_press_count = 0;

/**
  * @brief  GPIO Initialization Function
  * @details This function configures the GPIO pins as follows:
  *          - Enables the peripheral clocks for all used GPIO ports
  *          - Configures pins for LEDs, buttons, and peripherals
  *          - Sets initial output levels for output pins
  *
  * @note   The specific pins configured include:
  *          - User LEDs (LD3, LD4) for status indication
  *          - Push buttons for user input
  *          - Boot pin
  *
  * @param  None
  * @retval None
  */
void GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();  /* Enable GPIOA peripheral clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();  /* Enable GPIOB peripheral clock */
  __HAL_RCC_GPIOG_CLK_ENABLE();  /* Enable GPIOG peripheral clock */

  /* Set initial output levels for output pins to ensure known state at startup */
  HAL_GPIO_WritePin(GPIOG, LD3_Pin|LD4_Pin, GPIO_PIN_RESET);

  /* Configure bootloader pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /* Configure LED pins */
  GPIO_InitStruct.Pin = LD3_Pin|LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/**
 * @brief   Set the state of the LD3 LED
 * @details Controls the LD3 LED on the STM32F429 board
 * @param   state: Desired state (GPIO_PIN_SET or GPIO_PIN_RESET)
 * @retval  None
 */
void GPIO_LED_LD3_Set(GPIO_PinState state)
{
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, state);
}

/**
 * @brief   Set the state of the LD4 LED
 * @details Controls the LD4 LED on the STM32F429 board
 * @param   state: Desired state (GPIO_PIN_SET or GPIO_PIN_RESET)
 * @retval  None
 */
void GPIO_LED_LD4_Set(GPIO_PinState state)
{
    HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, state);
}

/**
 * @brief   Toggle the LD3 LED
 * @details Toggles the current state of LD3 LED
 * @param   None
 * @retval  None
 */
void GPIO_LED_LD3_Toggle(void)
{
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
}

/**
 * @brief   Toggle the LD4 LED
 * @details Toggles the current state of LD4 LED
 * @param   None
 * @retval  None
 */
void GPIO_LED_LD4_Toggle(void)
{
    HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin);
}

/**
 * @brief   Get the state of the user button (B1/PA0)
 * @details Reads the current state of the user button
 * @param   None
 * @retval  GPIO_PinState: Current button state
 */
GPIO_PinState GPIO_Button_B1_GetState(void)
{
    return HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
}

/**
 * @brief   Check if user button is pressed (with debouncing)
 * @details Reads button state with software debouncing
 * @param   debounce_ms: Debounce time in milliseconds
 * @retval  uint8_t: 1 if button is pressed, 0 otherwise
 */
uint8_t GPIO_Button_B1_IsPressed(uint32_t debounce_ms)
{
    static uint32_t last_press_time = 0;
    GPIO_PinState current_state = GPIO_Button_B1_GetState();

    if (current_state == GPIO_PIN_SET) {
        if ((HAL_GetTick() - last_press_time) > debounce_ms) {
            last_press_time = HAL_GetTick();
            return 1;
        }
    }

    return 0;
}

/**
  * @brief  Initialize PA0 button interrupt
  * @details Configures PA0 (B1) button for interrupt operation:
  *          - Configures pin as input with pull-down
  *          - Enables rising edge interrupt
  *          - Sets interrupt priority
  * @param  mode: GPIO_Mode_t (INTERRUPT_MODE or POLLING_MODE)
  * @retval None
  */
void GPIO_PA0_Button_Init(GPIO_Mode_t mode) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    switch(mode)
    {
        case INTERRUPT_MODE:
        {
            /* Configure PA0 (B1) as input with pull-down resistor */
            GPIO_InitStruct.Pin = B1_Pin;
            GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

            /* Set interrupt priority and enable it */
            HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
            HAL_NVIC_EnableIRQ(EXTI0_IRQn);
            break;
        }
        case POLLING_MODE:
        {
            /* Configure PA0 (B1) as input with pull-down resistor */
            GPIO_InitStruct.Pin = B1_Pin;
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_PULLDOWN;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
            break;
        }
        default:
            /* Invalid mode, do nothing */
            return;
    }
}

/**
  * @brief  EXTI line interrupt callback
  * @details This function is called when an EXTI line interrupt occurs.
  *          It checks the pin and calls the appropriate callback function.
  * @param  GPIO_Pin: Specifies the pin connected to the EXTI line
  * @retval None
  */
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//     if(GPIO_Pin == B1_Pin)
//     {
//         GPIO_Button_Callback();
//     }
//     else if(GPIO_Pin == TS_INT_PIN && g_hts != NULL)
//     {
//         TS_IRQHandler(g_hts);
//     }
// }

/**
  * @brief  Button interrupt callback handler
  * @details This function is called when the user button (PA0/B1) is pressed.
  *          It toggles LED3 to provide visual feedback and prints a message.
  * @param  None
  * @retval None
  */
void GPIO_Button_Callback(void)
{
    /* Increment button press count */
    button_press_count++;

    /* Toggle LED3 for visual feedback */
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

    /* Print debug message */
    printf("Button pressed! Count: %u\n", (unsigned int)button_press_count);
}

/**
  * @brief  Get button press count
  * @details Returns the total number of button presses since last reset
  * @param  None
  * @retval uint32_t: Button press count
  */
uint32_t GPIO_Button_GetPressCount(void) {
    return button_press_count;
}

/**
  * @brief  Reset button press count
  * @details Resets the button press counter to zero
  * @param  None
  * @retval None
  */
void GPIO_Button_ResetPressCount(void) {
    button_press_count = 0;
}

/**
  * @brief  Initialize GPIO interrupts
  * @details Initializes GPIO and configures PA0 button for interrupt operation
  * @param  None
  * @retval None
  */
void initialize_gpio_interrupts(void) {
    GPIO_Init();
    /* Configure user button pin for interrupt mode */
    GPIO_PA0_Button_Init(INTERRUPT_MODE);
}
