/**
 * @file gpio_example.c
 * @brief GPIO driver usage examples
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 *
 * This file contains example functions demonstrating GPIO usage
 * on STM32F429 Discovery board following HAL best practices.
 */

#include "gpio.h"
#include "gpio_example.h"
#include "main.h"
#include <stdio.h>

/* Private defines */
#define LED_BLINK_DELAY_MS    500
#define BUTTON_DEBOUNCE_MS    50
#define EXAMPLE_DURATION_MS   5000
#define POLLING_DELAY_MS      10
#define PRINT_INTERVAL_MS     1000
#define LED_TOGGLE_PERIOD_MS  2000
#define INTERRUPT_DELAY_MS    50
#define EXAMPLE_PAUSE_MS      1000

/* Private variables */
static volatile uint8_t led_state = 0;

/**
 * @brief LED example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_LED_Example(void) {
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t start_time = HAL_GetTick();

    printf("Starting GPIO LED Example...\n");

    /* Initialize GPIO */
    GPIO_Init();

    /* LED blink pattern */
    while ((HAL_GetTick() - start_time) < EXAMPLE_DURATION_MS) {
        /* Toggle LD3 */
        HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
        HAL_Delay(LED_BLINK_DELAY_MS);

        /* Toggle LD4 */
        HAL_GPIO_TogglePin(LD4_GPIO_Port, LD4_Pin);
        HAL_Delay(LED_BLINK_DELAY_MS);

        printf("LEDs toggled - LD3: %s, LD4: %s\n",
               HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin) ? "ON" : "OFF",
               HAL_GPIO_ReadPin(LD4_GPIO_Port, LD4_Pin) ? "ON" : "OFF");
    }

    /* Turn off LEDs */
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);

    printf("GPIO LED Example completed!\n");
    return status;
}

/**
 * @brief Button polling example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_Polling_Example(void) {
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t start_time = HAL_GetTick();
    GPIO_PinState last_button_state = GPIO_PIN_RESET;
    uint32_t last_debounce_time = 0;
    uint32_t local_button_count = 0;

    printf("Starting GPIO Polling Example...\n");

    /* Initialize GPIO */
    GPIO_Init();

    /* Configure button for polling mode */
    GPIO_PA0_Button_Init(POLLING_MODE);

    while ((HAL_GetTick() - start_time) < EXAMPLE_DURATION_MS) {
        GPIO_PinState current_button_state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);

        /* Check for button press with debouncing */
        if ((current_button_state == GPIO_PIN_SET) &&
            (last_button_state == GPIO_PIN_RESET) &&
            ((HAL_GetTick() - last_debounce_time) > BUTTON_DEBOUNCE_MS)) {

            local_button_count++;
            last_debounce_time = HAL_GetTick();

            /* Toggle LED on button press */
            led_state = !led_state;
            HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin,
                            led_state ? GPIO_PIN_SET : GPIO_PIN_RESET);

            printf("Button pressed! Count: %u, LED: %s\n",
                   (unsigned int)local_button_count,
                   led_state ? "ON" : "OFF");
        }

        last_button_state = current_button_state;
        HAL_Delay(POLLING_DELAY_MS); /* Small delay to prevent excessive polling */
    }

    printf("GPIO Polling Example completed! Total presses: %u\n", (unsigned int)local_button_count);
    return status;
}

/**
 * @brief Button interrupt example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_Interrupt_Example(void) {
    HAL_StatusTypeDef status = HAL_OK;
    uint32_t start_time = HAL_GetTick();

    printf("Starting GPIO Interrupt Example...\n");

    /* Initialize GPIO interrupts */
    initialize_gpio_interrupts();

    /* Reset counters */
    GPIO_Button_ResetPressCount();

    /* Wait for interrupts */
    while ((HAL_GetTick() - start_time) < EXAMPLE_DURATION_MS) {
        /* Print status every second */
        static uint32_t last_print_time = 0;
        if ((HAL_GetTick() - last_print_time) > PRINT_INTERVAL_MS) {
            printf("Interrupt Example running... Presses: %u\n", (unsigned int)GPIO_Button_GetPressCount());
            last_print_time = HAL_GetTick();
        }

        /* Toggle LD4 periodically to show system is running */
        if ((HAL_GetTick() % LED_TOGGLE_PERIOD_MS) < (LED_TOGGLE_PERIOD_MS / 2)) {
            HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, GPIO_PIN_RESET);
        }

        HAL_Delay(INTERRUPT_DELAY_MS);
    }

    printf("GPIO Interrupt Example completed! Total interrupts: %u\n", (unsigned int)GPIO_Button_GetPressCount());
    return status;
}

/**
 * @brief GPIO Button example function (combines polling and interrupt)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_Button_Example(void) {
    HAL_StatusTypeDef status = HAL_OK;

    printf("Starting GPIO Button Example...\n");

    /* First demonstrate polling mode */
    printf("\n--- Polling Mode ---\n");
    status = GPIO_Polling_Example();
    if (status != HAL_OK) {
        printf("Polling example failed!\n");
        return status;
    }

    HAL_Delay(EXAMPLE_PAUSE_MS); /* Pause between examples */

    /* Then demonstrate interrupt mode */
    printf("\n--- Interrupt Mode ---\n");
    status = GPIO_Interrupt_Example();
    if (status != HAL_OK) {
        printf("Interrupt example failed!\n");
        return status;
    }

    printf("\nGPIO Button Example completed!\n");
    return status;
}

/**
 * @brief Main GPIO example function
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef GPIO_Example(void) {
    HAL_StatusTypeDef status = HAL_OK;

    printf("Starting GPIO Driver Examples...\n");

    /* LED Example */
    printf("\n=== LED Example ===\n");
    status = GPIO_LED_Example();
    if (status != HAL_OK) {
        printf("LED example failed!\n");
        return status;
    }

    HAL_Delay(EXAMPLE_PAUSE_MS); /* Pause between examples */

    /* Button Example */
    printf("\n=== Button Example ===\n");
    status = GPIO_Button_Example();
    if (status != HAL_OK) {
        printf("Button example failed!\n");
        return status;
    }

    printf("\nAll GPIO examples completed successfully!\n");
    return status;
}
