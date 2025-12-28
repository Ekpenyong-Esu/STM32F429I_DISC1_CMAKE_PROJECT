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
#include "stm32f4xx.h"
#include "main.h"

#include "sys.h"
#include <stdint.h>
#include "lvgl.h"       /* LVGL symbols and types */
#include "lvgl_app.h"   /* Application GUI functions */
#include "ltdc.h"       /* Display controller */
#include "fmc.h"        /* External SDRAM controller */


/*******************************************************************************
 * Main Function - Program Entry Point
 *******************************************************************************
 * This is where your program starts!
 *
 * For Beginners:
 * 1. Initialize hardware (SYS_Init)
 * 2. Create GUI screens (LVGL_App_Init)
 * 3. Loop forever updating the display
 ******************************************************************************/
int main(void)
{
    SYS_Init();              // Step 1: Initialize all system hardware
    /*-------------------------------------------------------------------------
     * STEP 1: Initialize Hardware
     *-----------------------------------------------------------------------*/
    SEGGER_SYSVIEW_Conf();   // Optional: System tracing for debugging
    SEGGER_SYSVIEW_Start();

    /*-------------------------------------------------------------------------
     * STEP 2: Initialize External SDRAM (for framebuffer)
     *-----------------------------------------------------------------------*/
    FMC_Driver_Handle_t fmcHandle;
    FMC_Driver_SDRAM_Config_t sdramConfig = {
        .bank = FMC_SDRAM_BANK2, /* STM32F429I-DISCO uses Bank 2 (SDNE1) */
        .columnBits = FMC_SDRAM_COLUMN_BITS_NUM_8,
        .rowBits = FMC_SDRAM_ROW_BITS_NUM_12,
        .dataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16,
        .internalBanks = FMC_SDRAM_INTERN_BANKS_NUM_4,
        .casLatency = FMC_SDRAM_CAS_LATENCY_3,
        .clockPeriod = FMC_SDRAM_CLOCK_PERIOD_2,
        .readBurst = FMC_SDRAM_RBURST_ENABLE,
        .readPipeDelay = FMC_SDRAM_RPIPE_DELAY_0,
        .writeProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE,
        .loadToActiveDelay = 2,
        .exitSelfRefreshDelay = 7,
        .selfRefreshTime = 4,
        .rowCycleDelay = 7,
        .writeRecoveryTime = 2,
        .rpDelay = 2,
        .rcdDelay = 2
    };

    FMC_Driver_SDRAM_Init(&fmcHandle, &sdramConfig);

    /*-------------------------------------------------------------------------
     * STEP 2.1: Clear SDRAM (Black Screen)
     *-----------------------------------------------------------------------*/
    /* Clear framebuffer at Bank 2 Address (0xD0000000) */
    volatile uint16_t *fb = (volatile uint16_t *)0xD0000000;
    for(int i = 0; i < (240 * 320); i++) {
        fb[i] = 0x0000; /* Black in RGB565 */
    }
    if (fb[0] != 0x0000  || fb[1] != 0x0000 || fb[320] != 0x0000) {
        // Error: SDRAM not working
        while (1) {
            // Optionally blink an LED here
        }
    }

    /*-------------------------------------------------------------------------
     * STEP 3: Initialize Display Hardware (LTDC)
     *-----------------------------------------------------------------------*/
    LTDC_HW_Init();          // Initialize LTDC display controller hardware

    /*-------------------------------------------------------------------------
     * STEP 4: Create GUI Screens
     *-----------------------------------------------------------------------*/
    LVGL_App_Init();  // Creates 4 screens: Home, Sensors, Settings, Info

    /*-------------------------------------------------------------------------
     * STEP 5: Demo Mode Variables
     *-----------------------------------------------------------------------*/
    /* These create animated demo values - replace with real sensors! */
    uint32_t last_update = 0;    // Tracks time for updates
    int temp = 25;               // Simulated temperature (°C)
    int humidity = 60;           // Simulated humidity (%)
    int sensor_val = 50;         // Simulated sensor reading
    int demo_counter = 0;        // Counter for animation

    /*-------------------------------------------------------------------------
     * STEP 6: Main Loop (Runs Forever)
     *-----------------------------------------------------------------------*/
    while(1) {
        /*---------------------------------------------------------------------
         * Update GUI (Call this every 5ms)
         *-------------------------------------------------------------------*/
        LVGL_App_Tick();  // Process animations, touch input, redraws

        /*---------------------------------------------------------------------
         * Update Display Values Every 500ms (0.5 seconds)
         *-------------------------------------------------------------------*/
         uint32_t current_update = HAL_GetTick();
         if(current_update - last_update >= 200) {

            /* Temperature: Animate between 25-40°C */
            temp = 25 + (demo_counter % 15);
            LVGL_App_UpdateTemperature(temp);

            /* Humidity: Animate between 50-80% */
            humidity = 50 + (demo_counter % 30);
            LVGL_App_UpdateHumidity(humidity);

            /* Chart: Add new data point (30-70 range) */
            sensor_val = 30 + (demo_counter % 40);
            LVGL_App_AddChartData(sensor_val);

            /* Status: Update message every 5 seconds */
            if(demo_counter % 10 == 0) {
                LVGL_App_UpdateStatus(LV_SYMBOL_OK " System Running");
            }

            /* Increment counter and save time */
            demo_counter++;
            last_update = HAL_GetTick();
        }

        /*---------------------------------------------------------------------
         * Small Delay (Prevents CPU Overload)
         *-------------------------------------------------------------------*/
        HAL_Delay(5);  // Sleep 5ms, then loop again
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
