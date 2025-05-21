/**
  ******************************************************************************
  * @file    gpio_base.c
  * @brief   PA0 Button interrupt handling implementation
  * @details This file contains the implementation for handling the PA0 (B1)
  *          button interrupt on the STM32F429 Discovery board.
  * @version 1.0
  * @date    2025-05-13
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "main.h"
#include "stm32f429xx.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdio.h>

/**
  * @brief  Initialize PA0 button interrupt
  * @details Configures PA0 (B1) button for interrupt operation:
  *          - Configures pin as input with pull-down
  *          - Enables rising edge interrupt
  *          - Sets interrupt priority
  * @param  None
  * @retval None
  */
void GPIO_PA0_Button_Init(Status status) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    switch(status)
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
            /* Invalid status, do nothing */
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
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == B1_Pin)
  {
    GPIO_Button_Callback();
  }
}

/**
  * @brief  Button interrupt callback handler
  * @details This function is called when the user button (PA0/B1) is pressed.
  *          It toggles LED3 to provide visual feedback and prints a message.
  * @param  None
  * @retval None
  */
void GPIO_Button_Callback(void)
{
    /* Toggle LED3 for visual feedback */
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

    /* Print debug message */
    printf("Button pressed!\n");
}
