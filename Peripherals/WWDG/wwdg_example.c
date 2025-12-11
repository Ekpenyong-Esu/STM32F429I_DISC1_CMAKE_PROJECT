/**
  ******************************************************************************
  * @file    wwdg_example.c
  * @brief   Window Watchdog (WWDG) usage examples
  * @details This file provides example code demonstrating how to use
  *          the WWDG driver for precise timing watchdog requirements.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "wwdg.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t ewi_counter = 0;

/* Private function prototypes -----------------------------------------------*/
static void WWDG_EWI_UserCallback(void);

/* Example 1: Basic WWDG usage */
void WWDG_Example_Basic(void)
{
    printf("=== WWDG Basic Example ===\r\n");

    /* Check if previous reset was caused by WWDG */
    if (WWDG_WasResetSource())
    {
        printf("WARNING: Previous reset was caused by WWDG!\r\n");
        WWDG_ClearResetFlag();
    }

    /* Initialize WWDG with default settings */
    WWDG_StatusTypeDef status = WWDG_Init();
    if (status != WWDG_OK)
    {
        printf("WWDG initialization failed: %s\r\n", WWDG_GetStatusString(status));
        return;
    }

    printf("WWDG initialized successfully\r\n");
    printf("Counter starts at 0x7F, window at 0x50\r\n");

    /* Main loop - refresh within window */
    for (int i = 0; i < 10; i++)
    {
        /* Wait for counter to enter valid window */
        while (!WWDG_IsInWindow())
        {
            /* Wait... */
        }

        /* Refresh when in window */
        status = WWDG_Refresh();
        if (status == WWDG_OK)
        {
            printf("WWDG refreshed (iteration %d)\r\n", i + 1);
        }
        else
        {
            printf("Refresh failed: %s\r\n", WWDG_GetStatusString(status));
        }
    }

    printf("Basic example complete\r\n");
}

/* Example 2: Custom configuration */
void WWDG_Example_CustomConfig(void)
{
    printf("=== WWDG Custom Configuration Example ===\r\n");

    WWDG_ConfigTypeDef config = {
        .Prescaler = WWDG_PRESCALER_8,
        .Window = 0x50,     /* Window value */
        .Counter = 0x7F,    /* Max counter */
        .EWIMode = WWDG_EWI_DISABLE
    };

    /* Calculate and display timeouts */
    uint32_t minTimeout = 0;
    uint32_t maxTimeout = 0;
    WWDG_CalculateTimeout(config.Prescaler, config.Counter, config.Window,
                          &minTimeout, &maxTimeout);

    printf("Configuration:\r\n");
    printf("  Prescaler: 8\r\n");
    printf("  Counter: 0x%02X\r\n", (unsigned int)config.Counter);
    printf("  Window: 0x%02X\r\n", (unsigned int)config.Window);
    printf("  Window opens after: %u us\r\n", (unsigned int)minTimeout);
    printf("  Reset occurs after: %u us\r\n", (unsigned int)maxTimeout);

    WWDG_StatusTypeDef status = WWDG_Init_Custom(&config);
    if (status != WWDG_OK)
    {
        printf("WWDG initialization failed: %s\r\n", WWDG_GetStatusString(status));
        return;
    }

    printf("WWDG initialized with custom config\r\n");

    /* Refresh a few times */
    for (int i = 0; i < 5; i++)
    {
        while (!WWDG_IsInWindow()) { }
        WWDG_Refresh();
        printf("Refresh %d - Counter: 0x%02X\r\n", i + 1, (unsigned int)WWDG_GetCounter());
    }
}

/* EWI callback function */
static void WWDG_EWI_UserCallback(void)
{
    ewi_counter++;
    /* This is called when counter reaches 0x40 */
    /* WWDG is automatically refreshed in the HAL callback */
}

/* Example 3: Early Wakeup Interrupt */
void WWDG_Example_EWI(void)
{
    printf("=== WWDG Early Wakeup Interrupt Example ===\r\n");

    WWDG_ConfigTypeDef config = {
        .Prescaler = WWDG_PRESCALER_8,
        .Window = 0x50,
        .Counter = 0x7F,
        .EWIMode = WWDG_EWI_ENABLE
    };

    WWDG_StatusTypeDef status = WWDG_Init_Custom(&config);
    if (status != WWDG_OK)
    {
        printf("WWDG initialization failed\r\n");
        return;
    }

    /* Register callback */
    WWDG_RegisterEWICallback(WWDG_EWI_UserCallback);
    WWDG_EnableEWI();

    printf("EWI enabled - callback will refresh WWDG automatically\r\n");
    ewi_counter = 0;

    /* Let EWI handle refreshing for a while */
    HAL_Delay(1000);

    printf("EWI callback was called %u times\r\n", (unsigned int)ewi_counter);
}

/* Example 4: Window timing demonstration */
void WWDG_Example_WindowTiming(void)
{
    printf("=== WWDG Window Timing Example ===\r\n");

    /* Initialize with default settings */
    WWDG_Init();

    printf("Demonstrating window behavior:\r\n");
    printf("- Counter: 0x7F (starts here)\r\n");
    printf("- Window: 0x50 (refresh allowed below this)\r\n");
    printf("- Reset: 0x3F (reset if counter reaches here)\r\n\r\n");

    for (int i = 0; i < 3; i++)
    {
        uint32_t counter = WWDG_GetCounter();
        printf("Iteration %d:\r\n", i + 1);

        /* Wait and show counter decrementing */
        printf("  Waiting for window...\r\n");
        while (!WWDG_IsInWindow())
        {
            counter = WWDG_GetCounter();
            if ((counter & 0x0F) == 0)  /* Print every 16 counts */
            {
                printf("  Counter: 0x%02X %s\r\n", (unsigned int)counter,
                       WWDG_IsInWindow() ? "(IN WINDOW)" : "(OUTSIDE)");
            }
        }

        printf("  >>> Window open! Counter: 0x%02X\r\n", (unsigned int)WWDG_GetCounter());

        /* Refresh */
        WWDG_Refresh();
        printf("  Refreshed! Counter reset to 0x%02X\r\n\r\n", (unsigned int)WWDG_GetCounter());
    }
}

/* Example 5: Reset demonstration (CAUTION!) */
void WWDG_Example_ResetDemo(void)
{
    printf("=== WWDG Reset Demo (WARNING: System will reset!) ===\r\n");

    if (WWDG_WasResetSource())
    {
        printf("SUCCESS: System was reset by WWDG!\r\n");
        WWDG_ClearResetFlag();
        return;
    }

    /* Initialize WWDG */
    WWDG_Init();

    printf("WWDG initialized - NOT refreshing!\r\n");
    printf("System will reset when counter reaches 0x3F...\r\n");

    /* Show countdown */
    uint32_t lastCounter = WWDG_GetCounter();
    while (1)
    {
        uint32_t counter = WWDG_GetCounter();
        if (counter != lastCounter)
        {
            printf("Counter: 0x%02X\r\n", (unsigned int)counter);
            lastCounter = counter;
        }
        /* Reset will occur automatically - this loop will be interrupted */
    }
}

/**
 * @brief   Run WWDG examples
 */
void WWDG_RunExamples(void)
{
    printf("\r\n========================================\r\n");
    printf("      WWDG Driver Examples\r\n");
    printf("========================================\r\n\r\n");

    WWDG_Example_Basic();
    printf("\r\n");

    /* Other examples - uncomment to run */
    /* WWDG_Example_CustomConfig(); */
    /* WWDG_Example_EWI(); */
    /* WWDG_Example_WindowTiming(); */
    /* WWDG_Example_ResetDemo(); */  /* Caution! */
}
