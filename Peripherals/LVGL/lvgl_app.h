/*******************************************************************************
 * LVGL Application Header - Simple API for LVGL Usage
 *******************************************************************************
 * Include this header in your main.c to use LVGL with minimal setup.
 *
 * Quick Start:
 * 1. #include "lvgl_app.h" in main.c
 * 2. Call LVGL_App_Init() once after HAL_Init()
 * 3. Call LVGL_App_Tick() every 1-5ms in your main loop
 *
 * Example:
 *   #include "lvgl_app.h"
 *
 *   int main(void) {
 *     HAL_Init();
 *     SystemClock_Config();
 *     // ... other init ...
 *
 *     LVGL_App_Init();  // Initialize LVGL
 *
 *     while(1) {
 *       LVGL_App_Tick();  // Process LVGL
 *       HAL_Delay(5);     // 5ms delay
 *     }
 *   }
 ******************************************************************************/

#ifndef LVGL_APP_H
#define LVGL_APP_H

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * Function: LVGL_App_Init
 * Description: Initialize LVGL and create multi-screen GUI application
 * Parameters: None
 * Returns: None
 * Notes: Call once during startup, after HAL initialization
 *---------------------------------------------------------------------------*/
void LVGL_App_Init(void);

/*-----------------------------------------------------------------------------
 * Function: LVGL_App_Tick
 * Description: Process LVGL timers, rendering, and animations
 * Parameters: None
 * Returns: None
 * Notes: Call periodically (every 1-5ms) in main loop or timer interrupt
 *---------------------------------------------------------------------------*/
void LVGL_App_Tick(void);

/*-----------------------------------------------------------------------------
 * Function: LVGL_App_UpdateTemperature
 * Description: Update the temperature display on home screen
 * Parameters: temp_celsius - Temperature value in Celsius (0-100)
 * Returns: None
 * Notes: Updates the arc gauge and label on the home screen
 *---------------------------------------------------------------------------*/
void LVGL_App_UpdateTemperature(int temp_celsius);

/*-----------------------------------------------------------------------------
 * Function: LVGL_App_UpdateHumidity
 * Description: Update the humidity display on home screen
 * Parameters: humidity_percent - Humidity value (0-100%)
 * Returns: None
 * Notes: Updates the bar and label on the home screen
 *---------------------------------------------------------------------------*/
void LVGL_App_UpdateHumidity(int humidity_percent);

/*-----------------------------------------------------------------------------
 * Function: LVGL_App_UpdateStatus
 * Description: Update system status message
 * Parameters: status - Status message string
 * Returns: None
 * Notes: Updates the status label on the home screen
 *---------------------------------------------------------------------------*/
void LVGL_App_UpdateStatus(const char *status);

/*-----------------------------------------------------------------------------
 * Function: LVGL_App_AddChartData
 * Description: Add a new data point to the sensor chart
 * Parameters: value - Data value to add (0-100)
 * Returns: None
 * Notes: Adds point to the chart on the sensor monitoring screen
 *---------------------------------------------------------------------------*/
void LVGL_App_AddChartData(int value);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_APP_H */
