/**
 * @file led_simple_example.c
 * @brief Simple LED usage examples
 * @details Demonstrates how to use the simplified LED driver
 * @version 1.0
 * @date 2025-09-27
 */

#include "led.h"
#include <stdio.h>

/* Constants for cleaner code */
#define DELAY_SHORT     100
#define DELAY_MEDIUM    500
#define DELAY_LONG      1000

/* Example LED handles */
static LedHandle_t greenLed;
static LedHandle_t redLed;

/**
 * @brief   Example 1: Basic LED control
 */
void Led_Example_Basic(void)
{
    /* Initialize green LED on STM32F429 Discovery */
    if (!Led_InitGreen(&greenLed)) {
        printf("Green LED initialization failed!\n");
        return;
    }

    printf("Basic LED control example started\n");

    while (1) {
        /* Turn LED on */
        Led_On(&greenLed);
        printf("LED ON\n");
        HAL_Delay(DELAY_LONG);

        /* Turn LED off */
        Led_Off(&greenLed);
        printf("LED OFF\n");
        HAL_Delay(DELAY_LONG);

        /* Toggle LED a few times */
        for (int i = 0; i < 5; i++) {
            Led_Toggle(&greenLed);
            printf("LED toggled (state: %s)\n",
                   Led_IsOn(&greenLed) ? "ON" : "OFF");
            HAL_Delay(DELAY_MEDIUM);
        }
    }
}

/**
 * @brief   Example 2: LED blinking
 */
void Led_Example_Blinking(void)
{
    /* Initialize both LEDs */
    Led_InitGreen(&greenLed);
    Led_InitRed(&redLed);

    printf("LED blinking example started\n");

    /* Start green LED blinking slowly */
    Led_StartBlink(&greenLed, LED_BLINK_SLOW);
    printf("Green LED blinking slowly\n");

    HAL_Delay(5000);  /* Let it blink for 5 seconds */

    /* Start red LED blinking fast */
    Led_StartBlink(&redLed, LED_BLINK_FAST);
    printf("Red LED blinking fast\n");

    HAL_Delay(3000);  /* Both blinking for 3 seconds */

    /* Stop green LED blinking and turn it on solid */
    Led_StopBlink(&greenLed);
    Led_On(&greenLed);
    printf("Green LED now solid ON\n");

    HAL_Delay(2000);

    /* Stop all blinking */
    Led_StopBlink(&redLed);
    Led_Off(&greenLed);
    Led_Off(&redLed);
    printf("All LEDs OFF\n");

    /* Main loop to keep blinking active */
    while (1) {
        Led_Update(&greenLed);
        Led_Update(&redLed);
        HAL_Delay(10);  /* Small delay for update loop */
    }
}

/**
 * @brief   Example 3: Custom LED configuration
 */
void Led_Example_Custom(void)
{
    /* Custom LED configuration for active high LED */
    LedConfig_t customConfig = {
        .port = GPIOD,
        .pin = GPIO_PIN_0,
        .activeLow = false  /* Active high LED */
    };

    LedHandle_t customLed;
    if (!Led_InitCustom(&customLed, &customConfig)) {
        printf("Custom LED initialization failed!\n");
        return;
    }

    printf("Custom LED example started\n");

    while (1) {
        /* Test custom LED */
        Led_On(&customLed);
        HAL_Delay(DELAY_MEDIUM);

        Led_Off(&customLed);
        HAL_Delay(DELAY_MEDIUM);
    }
}

/**
 * @brief   Example 4: LED state monitoring and control
 */
void Led_Example_StateMonitoring(void)
{
    Led_InitGreen(&greenLed);
    Led_InitRed(&redLed);

    printf("LED state monitoring example started\n");

    uint32_t loopCount = 0;

    while (1) {
        loopCount++;

        /* Set LED states based on loop count */
        if (loopCount % 4 == 0) {
            Led_SetState(&greenLed, LED_ON);
            Led_SetState(&redLed, LED_OFF);
        } else if (loopCount % 4 == 1) {
            Led_SetState(&greenLed, LED_OFF);
            Led_SetState(&redLed, LED_ON);
        } else if (loopCount % 4 == 2) {
            Led_SetState(&greenLed, LED_ON);
            Led_SetState(&redLed, LED_ON);
        } else {
            Led_SetState(&greenLed, LED_OFF);
            Led_SetState(&redLed, LED_OFF);
        }

        /* Print current states */
        printf("Loop %lu: Green=%s, Red=%s\n",
               (unsigned long)loopCount,
               Led_IsOn(&greenLed) ? "ON" : "OFF",
               Led_IsOn(&redLed) ? "ON" : "OFF");

        HAL_Delay(DELAY_LONG);

        /* After 16 loops, start blinking demonstration */
        if (loopCount == 16) {
            printf("Starting alternating blink pattern\n");
            Led_StartBlink(&greenLed, LED_BLINK_FAST);

            /* Wait a bit, then start red LED blinking offset */
            HAL_Delay(DELAY_SHORT);
            Led_StartBlink(&redLed, LED_BLINK_FAST);

            /* Update loop for blinking */
            for (int i = 0; i < 100; i++) {
                Led_Update(&greenLed);
                Led_Update(&redLed);
                HAL_Delay(10);
            }

            /* Reset for next cycle */
            Led_StopBlink(&greenLed);
            Led_StopBlink(&redLed);
            Led_Off(&greenLed);
            Led_Off(&redLed);
            loopCount = 0;
        }
    }
}

/**
 * @brief   Example 5: LED heartbeat pattern
 */
void Led_Example_Heartbeat(void)
{
    Led_InitGreen(&greenLed);

    printf("LED heartbeat pattern example\n");

    while (1) {
        /* Double blink pattern (heartbeat) */
        Led_On(&greenLed);
        HAL_Delay(150);
        Led_Off(&greenLed);
        HAL_Delay(100);

        Led_On(&greenLed);
        HAL_Delay(150);
        Led_Off(&greenLed);
        HAL_Delay(700);  /* Long pause */
    }
}
