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
#include "sys.h"
#include <stdint.h>
#include "lvgl.h"
#include "lvgl_app.h"
#include "ltdc.h"
#include "fmc.h"
#include "spiltdc.h"  /* ILI9341 SPI initialization */

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
     * STEP 2.1: Clear BOTH SDRAM Framebuffers
     *-----------------------------------------------------------------------*/
    #define FB_SIZE (240 * 320)
    #define FB_BYTES (FB_SIZE * 2)

    volatile uint16_t *fb1 = (volatile uint16_t *)0xD0000000;
    volatile uint16_t *fb2 = (volatile uint16_t *)(0xD0000000 + FB_BYTES);

    /* Clear buffer 0 */
    for(int i = 0; i < FB_SIZE; i++) {
        fb1[i] = 0x0000;
    }

    /* Clear buffer 1 */
    for(int i = 0; i < FB_SIZE; i++) {
        fb2[i] = 0x0000;
    }

    /* Verify SDRAM works */
    if (fb1[0] != 0x0000 || fb1[100] != 0x0000 || fb2[0] != 0x0000) {
        while (1) { /* SDRAM error - hang here */ }
    }

    /*-------------------------------------------------------------------------
     * STEP 3: Initialize ILI9341 LCD Controller (CRITICAL!)
     *-----------------------------------------------------------------------*/
    ILI9341_Init();  /* Configure LCD via SPI, switch to RGB mode */

    /*-------------------------------------------------------------------------
     * STEP 4: Initialize LTDC Display Controller
     *-----------------------------------------------------------------------*/
    LTDC_HW_Init();

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
