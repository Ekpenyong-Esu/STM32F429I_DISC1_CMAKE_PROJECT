/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body for STM32F429I-DISC1 with LVGL
 ******************************************************************************
 */
/* USER CODE END Header */

#include "SEGGER_SYSVIEW.h"
#include "stm32f4xx.h"
#include "main.h"
#include "stm32f4xx_hal_def.h"
#include "sys.h"
#include <stdint.h>
#include "lvgl.h"
#include "lvgl_app.h"
#include "ltdc.h"
#include "fmc.h"
#include "ili9341.h"  /* ILI9341 driver */

int main(void)
{
    /*-------------------------------------------------------------------------
     * STEP 1: Initialize System Hardware
     *-----------------------------------------------------------------------*/
    SYS_Init();
    SEGGER_SYSVIEW_Conf();
    SEGGER_SYSVIEW_Start();

    /*-------------------------------------------------------------------------
     * STEP 2: Initialize External SDRAM (for framebuffer)
     *-----------------------------------------------------------------------*/
    FMC_Driver_Handle_t fmcHandle;
    FMC_Driver_SDRAM_Config_t sdramConfig = {
        .bank = FMC_SDRAM_BANK2,
        .columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8,
        .rowBits = FMC_SDRAM_ROW_BITS_NUM_12,
        .dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16,
        .internalBanks = FMC_SDRAM_INTERN_BANKS_NUM_4,
        .casLatency = FMC_SDRAM_CAS_LATENCY_3,
        .clockPeriod = FMC_SDRAM_CLOCK_PERIOD_2,
        .readBurst = FMC_SDRAM_RBURST_DISABLE,   /* ST BSP: DISABLE */
        .readPipeDelay = FMC_SDRAM_RPIPE_DELAY_1, /* ST BSP: DELAY_1 */
        .writeProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
        .loadToActiveDelay = 2,
        .exitSelfRefreshDelay = 7,
        .selfRefreshTime = 4,
        .rowCycleDelay = 7,
        .writeRecoveryTime = 3,
        .rpDelay = 2,
        .rcdDelay = 2
    };

    /* Initialize SDRAM and verify return status */
    HAL_StatusTypeDef sdram_status = FMC_Driver_SDRAM_Init(&fmcHandle, &sdramConfig);
    if (sdram_status != HAL_OK) {
        printf("ERROR: SDRAM init failed\n");
        Error_Handler();
    }

    if(FMC_Driver_SDRAM_Test(&fmcHandle, LTDC_FB_BASE_ADDR, LTDC_FB_SIZE_RGB565 ) != true) {
        printf("ERROR: SDRAM test failed\n");
        Error_Handler();
    }


    /*-------------------------------------------------------------------------
     * STEP 3: Initialize ILI9341 LCD Controller (CRITICAL!)
     *-----------------------------------------------------------------------*/
    ili9341_Init();  /* Configure LCD via SPI, switch to RGB mode */

    /* Ensure panel ENABLE is driven */
    HAL_Delay(10);
    HAL_GPIO_WritePin(ENABLE_GPIO_Port, ENABLE_Pin, GPIO_PIN_SET);

    /*-------------------------------------------------------------------------
     * STEP 4: Initialize LTDC Display Controller
     *-----------------------------------------------------------------------*/
    LTDC_HW_Init();

    /* Simple smoke test: set LTDC to show framebuffer 0 and log for diagnostics */
    HAL_LTDC_SetAddress(&hltdc, LTDC_FB_BASE_ADDR, 0);
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
    HAL_Delay(50);
    printf("INFO: SDRAM smoke test - LTDC address set to 0x%08lX\n", (unsigned long)LTDC_FB_BASE_ADDR);

    /*-------------------------------------------------------------------------
     * STEP 5: Initialize LVGL and Create GUI
     *-----------------------------------------------------------------------*/
    LVGL_App_Init();

    /*-------------------------------------------------------------------------
     * STEP 6: Demo Variables
     *-----------------------------------------------------------------------*/
    uint32_t last_update = 0;
    int temp = 25;
    int humidity = 60;
    int sensor_val = 50;
    int demo_counter = 0;

    /*-------------------------------------------------------------------------
     * STEP 7: Main Loop
     *-----------------------------------------------------------------------*/
    while(1) {
        /* Update LVGL (renders GUI, handles touch, animations) */
        LVGL_App_Tick();

        /* Update sensor values every 200ms */
        uint32_t current_time = HAL_GetTick();
        if(current_time - last_update >= 200) {
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
            if(demo_counter % 10 == 0) {
                LVGL_App_UpdateStatus(LV_SYMBOL_OK " System Running");
            }

            demo_counter++;
            last_update = current_time;
        }

        /* Small delay to prevent CPU overload */
        HAL_Delay(5);
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
