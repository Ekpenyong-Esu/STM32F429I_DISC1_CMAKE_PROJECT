/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body for STM32F429I-DISC1 with LVGL
 * @version        : 2.0 - Corrected with proper low power support
 * @date           : 2025-01-25
 ******************************************************************************
 */
/* USER CODE END Header */

#include "SEGGER_SYSVIEW.h"
#include "main.h"
#include "log.h"
#include "stm32f4xx_hal_def.h"
#include "sys.h"
#include <stdint.h>
#include "lvgl.h"
#include "lvgl_app.h"
#include "ltdc.h"
#include "fmc.h"
#include "ili9341.h"
#include "i2c.h"
#include "touchscreen.h"
#include "app_low_power.h"
#include "pwr.h"

/* Private variables ---------------------------------------------------------*/
static uint32_t last_update = 0;
static int temp = 25;
static int humidity = 60;
static int sensor_val = 50;
static int demo_counter = 0;

/* Private function prototypes -----------------------------------------------*/
static void Demo_UpdateSensors(void);
static void Demo_CheckLowPower(void);

int main(void)
{
    /*-------------------------------------------------------------------------
     * STEP 1: Initialize System Hardware
     *-----------------------------------------------------------------------*/
    SYS_Init();

#ifdef USE_SEGGER_SYSVIEW
    SEGGER_SYSVIEW_Conf();
    SEGGER_SYSVIEW_Start();
#endif

    /*-------------------------------------------------------------------------
     * STEP 2: Initialize Power Management (EARLY!)
     *-----------------------------------------------------------------------*/
    /* Initialize power management BEFORE other peripherals */
    /* This configures debug support and power controller */
    if (PWR_InitDefault() != HAL_OK) {
        log_error("Pwr Init failed");
    }

    /*-------------------------------------------------------------------------
     * STEP 3: Initialize I2C for Touchscreen
     *-----------------------------------------------------------------------*/
    I2C_Init();

    /*-------------------------------------------------------------------------
     * STEP 4: Initialize External SDRAM (for framebuffer)
     *-----------------------------------------------------------------------*/
    FMC_Driver_Handle_t fmcHandle;
    FMC_Driver_SDRAM_Config_t sdramConfig = {
        .bank = FMC_SDRAM_BANK2,
        .columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8,
        .rowBits = FMC_SDRAM_ROW_BITS_NUM_12,
        .dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16,
        .internalBanks = FMC_SDRAM_INTERN_BANKS_NUM_4,
        .casLatency = FMC_SDRAM_CAS_LATENCY_3,
        .clockPeriod = FMC_SDRAM_CLOCK_PERIOD_3,
        .readBurst = FMC_SDRAM_RBURST_DISABLE,
        .readPipeDelay = FMC_SDRAM_RPIPE_DELAY_1,
        .writeProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
        .loadToActiveDelay = 2,
        .exitSelfRefreshDelay = 7,
        .selfRefreshTime = 4,
        .rowCycleDelay = 7,
        .writeRecoveryTime = 2,
        .rpDelay = 2,
        .rcdDelay = 2
    };

    /* Initialize SDRAM and verify return status */
    HAL_StatusTypeDef sdram_status = FMC_Driver_SDRAM_Init(&fmcHandle, &sdramConfig);
    if (sdram_status != HAL_OK)
    {
        log_error("SRAM Init Failed");
    }

    /*-------------------------------------------------------------------------
     * STEP 5: Initialize ILI9341 LCD Controller
     *-----------------------------------------------------------------------*/
    ili9341_Init();  /* Configure LCD via SPI, switch to RGB mode */

    /*-------------------------------------------------------------------------
     * STEP 6: Initialize LTDC Display Controller
     *-----------------------------------------------------------------------*/
    LTDC_HW_Init();

    /*-------------------------------------------------------------------------
     * STEP 7: Initialize LVGL and Create GUI
     *-----------------------------------------------------------------------*/
    LVGL_App_Init();

    /*-------------------------------------------------------------------------
     * STEP 8: Initialize Application Low Power Management
     *-----------------------------------------------------------------------*/
    APP_LowPowerInit();

    printf("System initialized successfully\n");
    LVGL_App_UpdateStatus("System Ready");

    /*-------------------------------------------------------------------------
     * STEP 9: Main Loop
     *-----------------------------------------------------------------------*/
    while(1)
    {
        /* Update LVGL (renders GUI, handles touch, animations) */
        LVGL_App_Tick();

        /* Update demo sensor values periodically */
        Demo_UpdateSensors();

        /* Check if should enter low power mode */
        Demo_CheckLowPower();

        /* Small delay to prevent tight loop */
        HAL_Delay(5);
    }
}

/**
 * @brief   Update demo sensor values
 * @details Updates temperature, humidity, and chart data
 */
static void Demo_UpdateSensors(void)
{
    uint32_t current_time = HAL_GetTick();

    /* Update sensor values every 200ms */
    if(current_time - last_update >= 200)
    {
        /* Animate temperature 25-40°C */
        temp = 25 + (demo_counter % 15);
        LVGL_App_UpdateTemperature(temp);

        /* Animate humidity 50-80% */
        humidity = 50 + (demo_counter % 30);
        LVGL_App_UpdateHumidity(humidity);

        /* Add chart data point */
        sensor_val = 30 + (demo_counter % 40);
        LVGL_App_AddChartData(sensor_val);

        /* Update status every 2 seconds */
        if(demo_counter % 10 == 0)
        {
            LVGL_App_UpdateStatus("System Running");
        }

        demo_counter++;
        last_update = current_time;

        /* NOTE: Don't treat internal demo updates as user activity;
           User interaction (touch) sets activity via APP_TouchActivity(). */
    }
}

/**
 * @brief   Check and handle low power mode entry
 * @details Checks every second if system should enter low power
 */
static void Demo_CheckLowPower(void)
{
    static uint32_t last_low_power_check = 0;
    uint32_t current_time = HAL_GetTick();

    /* Check for low power mode every 1 second */
    if (current_time - last_low_power_check >= 1000)
    {
        /* Check if auto-sleep was requested (from dim animation) */
        if (APP_IsAutoSleepRequested())
        {
            printf("Auto-sleep requested from animation\n");
            APP_ClearAutoSleepRequest();

            /* Enter low power mode */
            LVGL_App_UpdateStatus("Auto Sleep...");
            HAL_Delay(100); /* Give time for status update to render */

            PWR_StatusTypeDef status = APP_EnterLowPowerMode();

            if (status == PWR_OK)
            {
                /* System woke up from low power mode */
                printf("Woke up from low power mode\n");
                LVGL_App_UpdateStatus("Woke Up!");
                APP_UpdateActivity(); /* Reset activity timer */
            }
            else
            {
                printf("Low power mode failed: %d\n", status);
                LVGL_App_UpdateStatus("Low Power Failed");
            }
        }
        /* Or check if should enter low power based on inactivity */
        else if (APP_ShouldEnterLowPower())
        {
            printf("Entering low power mode due to inactivity\n");

            /* Update status before entering low power */
            LVGL_App_UpdateStatus("Entering Low Power...");
            HAL_Delay(100); /* Give time for status update to render */

            PWR_StatusTypeDef status = APP_EnterLowPowerMode();

            if (status == PWR_OK)
            {
                /* System woke up from low power mode */
                printf("Woke up from low power mode\n");
                LVGL_App_UpdateStatus("System Resumed");
                APP_UpdateActivity(); /* Reset activity timer */
            }
            else
            {
                printf("Low power mode failed: %d\n", status);
                LVGL_App_UpdateStatus("Low Power Failed");
            }
        }

        last_low_power_check = current_time;
    }
}

/* NOTE: HAL_GPIO_EXTI_Callback is already implemented in touchscreen.c
 * The touchscreen driver handles calling APP_TouchActivity() for wake-up
 * No need to duplicate the callback here!
 */

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    printf("Wrong parameters value: file %s on line %d\r\n", file, line);
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
