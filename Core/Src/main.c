/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

#include "SEGGER_SYSVIEW.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "led.h"
#include "button.h"
#include "sys.h"
#include <stdint.h>

/* Private defines -----------------------------------------------------------*/
#define LED_CONTROL_PERIOD_MS   2000  // LED control loop period in milliseconds
#define LED_ON_TIME_MS          500   // Time to keep LED on
#define LED_OFF_TIME_MS         500   // Time to keep LED off
#define LED_FAST_TOGGLE_MS      100   // Fast LED toggle time for manual mode
#define BUTTON_POLL_DELAY_MS    50    // Button polling delay

// Define SystemView events

#define SYSVIEW_EVT_MAIN_LOOP_START   0x85

#define LED_PIN_11 GPIO_PIN_11
#define LED_PIN_12 GPIO_PIN_12
#define LED_PIN_15 GPIO_PIN_15

static volatile uint8_t loopCounter = 0;
static volatile bool ledPressed = false;

int main(void){
  /* USER CODE BEGIN 2 */
    SYS_Init();
    SEGGER_SYSVIEW_Conf();
    SEGGER_SYSVIEW_Start();

    // Initialize User Button (PA0) with interrupt
    ButtonHandle_t userButton;
    ButtonConfig_t buttonConfig = {
        .port = GPIOA,
        .pin = GPIO_PIN_0,
        .activeLow = false,
        .debounceMs = BUTTON_DEBOUNCE_DEFAULT,
        .enableInterrupt = true
    };

    LedHandle_t greenLed;
    LedHandle_t redLed;
    LedHandle_t led11;
    LedHandle_t led12;
    LedHandle_t led15;

    // LED initializations
    if (!Led_Init(&greenLed, LED_GREEN_PORT, LED_GREEN_PIN)) Error_Handler();
    if (!Led_Init(&redLed, LED_RED_PORT, LED_RED_PIN)) Error_Handler();
    if (!Led_Init(&led11, LED_GREEN_PORT, LED_PIN_11)) Error_Handler();
    if (!Led_Init(&led12, LED_GREEN_PORT, LED_PIN_12)) Error_Handler();
    if (!Led_Init(&led15, LED_GREEN_PORT, LED_PIN_15)) Error_Handler();

    // Button initialization
    if (!Button_InitCustom(&userButton, &buttonConfig)) Error_Handler();

    // Main application loop - no OS
    while (1) {
        SEGGER_SYSVIEW_RecordU32(SYSVIEW_EVT_MAIN_LOOP_START, loopCounter);

        if (Button_WasPressed(&userButton)) {
            SEGGER_SYSVIEW_PrintfHost(0, "Button pressed!\n");
            Led_Off(&led11);
            Led_Off(&led12);
            Led_Off(&led15);
        }

        if (Button_Read(&userButton) == BUTTON_RELEASED) {
            // Button released event
        }

        if (ledPressed) {
            Led_Toggle(&led11);
            HAL_Delay(LED_FAST_TOGGLE_MS);
            Led_Toggle(&led12);
            HAL_Delay(LED_FAST_TOGGLE_MS);
            Led_Toggle(&led15);
            HAL_Delay(LED_FAST_TOGGLE_MS);
            ledPressed = false;
        } else {
            Led_On(&redLed);
            HAL_Delay(LED_ON_TIME_MS);
            Led_Off(&redLed);
            HAL_Delay(LED_OFF_TIME_MS);

            Led_On(&greenLed);
            HAL_Delay(LED_ON_TIME_MS);
            Led_Off(&greenLed);
            HAL_Delay(LED_OFF_TIME_MS);

            Led_On(&led15);
            HAL_Delay(LED_ON_TIME_MS);
            Led_Off(&led15);
            HAL_Delay(LED_OFF_TIME_MS);
        }
        loopCounter++;
        SEGGER_SYSVIEW_OnIdle();
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == GPIO_PIN_0) {
     ledPressed = true;
    // Optionally process button event here, e.g. set a flag or call BUTTON_Process
  }
}


#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,*/
    printf("Wrong parameters value: file %s on line %d\r\n", file, line);
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
