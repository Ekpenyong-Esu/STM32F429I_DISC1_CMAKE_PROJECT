/**
  ******************************************************************************
  * @file    iwdg_example.c
  * @brief   Independent Watchdog (IWDG) usage examples
  * @details This file provides example code demonstrating how to use
  *          the IWDG driver for system reliability.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "iwdg.h"
#include <stdio.h>

/* Example 1: Basic IWDG usage with default 1 second timeout */
void IWDG_Example_Basic(void)
{
    printf("=== IWDG Basic Example ===\r\n");

    /* Check if previous reset was caused by watchdog */
    if (IWDG_WasResetSource())
    {
        printf("WARNING: Previous reset was caused by IWDG timeout!\r\n");
        IWDG_ClearResetFlag();
    }

    /* Initialize IWDG with default settings (~1 second timeout) */
    IWDG_StatusTypeDef status = IWDG_Init();
    if (status != IWDG_OK)
    {
        printf("IWDG initialization failed: %s\r\n", IWDG_GetStatusString(status));
        return;
    }

    printf("IWDG initialized with ~1 second timeout\r\n");
    printf("Watchdog is now running - must refresh periodically!\r\n");

    /* Main loop - must refresh watchdog to prevent reset */
    uint32_t counter = 0;
    while (1)
    {
        /* Simulate main application work */
        HAL_Delay(500);  /* Do some work */

        /* Refresh watchdog to prevent reset */
        status = IWDG_Refresh();
        if (status == IWDG_OK)
        {
            printf("Watchdog refreshed (count: %u)\r\n", (unsigned int)counter++);
        }

        /* Exit after 10 iterations for demo */
        if (counter >= 10)
        {
            printf("Demo complete\r\n");
            break;
        }
    }
}

/* Example 2: Custom timeout configuration */
void IWDG_Example_CustomTimeout(void)
{
    printf("=== IWDG Custom Timeout Example ===\r\n");

    /* Initialize IWDG with 2 second timeout */
    IWDG_StatusTypeDef status = IWDG_Init_TimeoutMs(IWDG_TIMEOUT_2S);
    if (status != IWDG_OK)
    {
        printf("IWDG initialization failed: %s\r\n", IWDG_GetStatusString(status));
        return;
    }

    printf("IWDG initialized with 2 second timeout\r\n");

    /* Refresh every 1 second (well within 2 second window) */
    for (int i = 0; i < 5; i++)
    {
        HAL_Delay(1000);
        IWDG_Refresh();
        printf("Refreshed at %d seconds\r\n", i + 1);
    }
}

/* Example 3: Advanced configuration with manual prescaler/reload */
void IWDG_Example_AdvancedConfig(void)
{
    printf("=== IWDG Advanced Configuration Example ===\r\n");

    IWDG_ConfigTypeDef config = {
        .Prescaler = IWDG_PRESCALER_64,
        .Reload = 2000  /* With prescaler 64: timeout = (2000 * 64) / 32000 = 4 seconds */
    };

    /* Calculate and display the actual timeout */
    uint32_t timeout = IWDG_CalculateTimeout(config.Prescaler, config.Reload);
    printf("Calculated timeout: %u ms\r\n", (unsigned int)timeout);

    IWDG_StatusTypeDef status = IWDG_Init_Custom(&config);
    if (status != IWDG_OK)
    {
        printf("IWDG initialization failed: %s\r\n", IWDG_GetStatusString(status));
        return;
    }

    printf("IWDG initialized with custom configuration\r\n");

    /* Refresh periodically */
    for (int i = 0; i < 3; i++)
    {
        HAL_Delay(2000);  /* 2 second delay (within 4 second timeout) */
        IWDG_Refresh();
        printf("Refreshed after 2 seconds\r\n");
    }
}

/* Example 4: Watchdog reset demonstration (CAUTION: Will reset system!) */
void IWDG_Example_ResetDemo(void)
{
    printf("=== IWDG Reset Demo (WARNING: System will reset!) ===\r\n");

    /* Check if this is a restart after watchdog reset */
    if (IWDG_WasResetSource())
    {
        printf("SUCCESS: System was reset by IWDG as expected!\r\n");
        IWDG_ClearResetFlag();
        return;  /* Exit demo after successful reset detection */
    }

    /* Initialize with short 500ms timeout */
    IWDG_StatusTypeDef status = IWDG_Init_TimeoutMs(500);
    if (status != IWDG_OK)
    {
        printf("IWDG initialization failed\r\n");
        return;
    }

    printf("IWDG initialized with 500ms timeout\r\n");
    printf("NOT refreshing watchdog - system will reset in ~500ms...\r\n");

    /* Intentionally NOT refreshing - will cause reset */
    HAL_Delay(1000);  /* Wait for watchdog to trigger reset */

    /* This line should never be reached */
    printf("ERROR: This message should not appear!\r\n");
}

/* Example 5: Integration with FreeRTOS task */
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"

void IWDG_WatchdogTask(void *pvParameters)
{
    (void)pvParameters;

    /* Initialize IWDG with 2 second timeout */
    IWDG_Init_TimeoutMs(IWDG_TIMEOUT_2S);

    while (1)
    {
        /* Refresh watchdog every 1 second */
        IWDG_Refresh();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void IWDG_Example_FreeRTOS(void)
{
    printf("=== IWDG FreeRTOS Example ===\r\n");

    /* Create watchdog refresh task */
    xTaskCreate(IWDG_WatchdogTask, "WDG", 128, NULL, tskIDLE_PRIORITY + 1, NULL);

    printf("Watchdog task created\r\n");
}
#endif /* USE_FREERTOS */

/**
 * @brief   Run all IWDG examples (except reset demo)
 * @note    Call individual examples as needed
 */
void IWDG_RunExamples(void)
{
    printf("\r\n========================================\r\n");
    printf("      IWDG Driver Examples\r\n");
    printf("========================================\r\n\r\n");

    /* Run safe examples */
    IWDG_Example_Basic();

    printf("\r\n");

    /* Note: Only run one IWDG init per session since it can't be stopped */
    /* IWDG_Example_CustomTimeout(); */
    /* IWDG_Example_AdvancedConfig(); */
    /* IWDG_Example_ResetDemo(); */  /* Caution: causes reset */
}
