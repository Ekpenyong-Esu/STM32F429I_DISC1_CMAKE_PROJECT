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
#include "spi.h"
#include "app_low_power.h"

/* Private variables ---------------------------------------------------------*/
static uint32_t last_update = 0;
static int temp = 25;
static int humidity = 60;
static int sensor_val = 50;
static int demo_counter = 0;

/* Private function prototypes -----------------------------------------------*/
static void UpdateSensors(void);

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
     * STEP 2: Initialize SPI for ILI9488 + XPT2046
     *-----------------------------------------------------------------------*/
    SPI_Init();

    /*-------------------------------------------------------------------------
     * STEP 3: Initialize LVGL and Create GUI
     *-----------------------------------------------------------------------*/
    LVGL_App_Init();

    printf("System initialized successfully\n");
    LVGL_App_UpdateStatus("System Ready");

    /*-------------------------------------------------------------------------
     * STEP 4: Initialize low power management
     * Call AFTER lv_port_indev_init (called inside LVGL_App_Init) so that
     * XPT2046_MspInit has already configured PA15 / EXTI15_10_IRQn.
     *-----------------------------------------------------------------------*/
    APP_LowPowerInit();

    /*-------------------------------------------------------------------------
     * STEP 5: Main Loop
     *-----------------------------------------------------------------------*/
    while(1)
    {
        /* Update LVGL (renders GUI, handles touch, animations) */
        LVGL_App_Tick();

        /* Update demo sensor values periodically */
        UpdateSensors();

        /* Low power check: power off display + enter Stop mode after 1 s idle.
         * Returns immediately if not yet time, or blocks in WFI until a touch
         * on PA15 wakes the MCU. */
        if (APP_ShouldEnterLowPower())
        {
            APP_ClearAutoSleepRequest();
            APP_EnterLowPowerMode();  /* blocks until XPT2046 EXTI wakes MCU */

            /* Mark touch activity so the display wakes and timer resets */
            APP_TouchActivity();
        }

        /* Small delay to prevent tight loop */
        HAL_Delay(5);
    }
}

/**
 * @brief   Update demo sensor values
 * @details Updates temperature, humidity, and chart data
 */
static void UpdateSensors(void)
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
/* NOTE: Touch is handled by XPT2046 in lv_port_indev.c */

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
    printf("Wrong parameters value: file %s on line %lu\r\n", file, (unsigned long)line);
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
