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

/* Private variables ---------------------------------------------------------*/
static uint32_t last_update = 0;
static int temp = 25;
static int humidity = 60;
static int sensor_val = 50;
static int demo_counter = 0;

/* Private function prototypes -----------------------------------------------*/
static void Demo_UpdateSensors(void);

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
     * STEP 2: Initialize I2C for Touchscreen
     *-----------------------------------------------------------------------*/
    I2C_Init();

    /*-------------------------------------------------------------------------
    * STEP 3: Initialize External SDRAM (for framebuffer) and Touchscreen
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
    * STEP 4: Initialize ILI9341 LCD Controller
     *-----------------------------------------------------------------------*/
    ili9341_Init();  /* Configure LCD via SPI, switch to RGB mode */

    /*-------------------------------------------------------------------------
    * STEP 5: Initialize LTDC Display Controller
     *-----------------------------------------------------------------------*/
    LTDC_HW_Init();

    /*-------------------------------------------------------------------------
    * STEP 6: Initialize LVGL and Create GUI
     *-----------------------------------------------------------------------*/
    LVGL_App_Init();

    printf("System initialized successfully\n");
    LVGL_App_UpdateStatus("System Ready");

    /*-------------------------------------------------------------------------
    * STEP 7: Main Loop
     *-----------------------------------------------------------------------*/
    while(1)
    {
        /* Update LVGL (renders GUI, handles touch, animations) */
        LVGL_App_Tick();

        /* Update demo sensor values periodically */
        Demo_UpdateSensors();

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

        /* NOTE: Don't treat internal demo updates as user activity. */
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
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    printf("Wrong parameters value: file %s on line %lu\r\n", file, (unsigned long)line);
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
