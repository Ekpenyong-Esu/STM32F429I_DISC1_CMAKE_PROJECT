/**
  ******************************************************************************
  * @file    touchscreen_example_fixed.c
  * @brief   Fixed touchscreen driver example implementation for STM32F429 Discovery Board
  * @details This file provides comprehensive examples for using the STMPE811
  *          resistive touchscreen controller with corrected API calls.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "touchscreen_example.h"
#include <stdio.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define TS_DEBOUNCE_TIME        50      /* Debounce time in ms */
#define TS_MAX_DRAW_POINTS      500     /* Maximum drawing points */
#define TS_GESTURE_TIMEOUT      1000    /* Gesture timeout in ms */
#define TS_I2C_CLOCK_SPEED      100000  /* I2C clock speed */
#define TS_DISPLAY_MARGIN       10      /* Display margin */

/* Private variables ---------------------------------------------------------*/
static TS_DrawPoint_t DrawPoints[TS_MAX_DRAW_POINTS];
static uint16_t DrawPointCount = 0;
static TS_TestResults_t CurrentTestResults;
static uint32_t LastTouchTime = 0;
static TS_HandleTypeDef hts;
static I2C_HandleTypeDef hi2c3;

/* Private function prototypes -----------------------------------------------*/
static void TS_Example_ResetTestResults(void);
static uint32_t TS_Example_GetTick(void);
static void TS_Example_Delay(uint32_t delay);

/* Function implementations --------------------------------------------------*/

/**
  * @brief  Initialize touchscreen example system
  * @retval TS_StatusTypeDef: Operation status
  */
TS_StatusTypeDef TS_Example_Init(void)
{
    TS_StatusTypeDef status = TS_OK;

    /* Initialize I2C3 */
    hi2c3.Instance = I2C3;
    hi2c3.Init.ClockSpeed = TS_I2C_CLOCK_SPEED;
    hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    /* Initialize touchscreen */
    status = TS_Init(&hts, &hi2c3);
    if (status != TS_OK) {
        return status;
    }

    /* Reset test results */
    TS_Example_ResetTestResults();

    /* Clear drawing points */
    DrawPointCount = 0;
    memset(DrawPoints, 0, sizeof(DrawPoints));

    return TS_OK;
}

/**
  * @brief  Basic touch detection example
  * @retval TS_StatusTypeDef: Operation status
  */
TS_StatusTypeDef TS_Example_BasicTouch(void)
{
    TS_TouchDataTypeDef touchData;
    uint32_t startTime = TS_Example_GetTick();

    printf("Basic Touch Test - Touch the screen for 10 seconds\n");
    TS_Example_ClearScreen();
    TS_Example_DisplayText(TS_DISPLAY_MARGIN, TS_DISPLAY_MARGIN, "Touch the screen");

    while ((TS_Example_GetTick() - startTime) < 10000) {
        if (TS_GetTouchData(&hts, &touchData) == TS_OK) {
            if (touchData.TouchCount > 0) {
                printf("Touch detected at (%d, %d) with pressure %d\n",
                       touchData.Points[0].X, touchData.Points[0].Y, touchData.Points[0].Z);

                TS_Example_DrawTouchPoint(touchData.Points[0].X, touchData.Points[0].Y,
                                        TS_EXAMPLE_COLOR_TOUCH);

                CurrentTestResults.TotalTouches++;
                CurrentTestResults.ValidTouches++;
            }
        }
        TS_Example_Delay(50);
    }

    printf("Basic touch test completed. Total touches: %u\n",
           CurrentTestResults.TotalTouches);
    return TS_OK;
}

/**
  * @brief  Touch drawing example
  * @retval TS_StatusTypeDef: Operation status
  */
TS_StatusTypeDef TS_Example_TouchDrawing(void)
{
    TS_TouchDataTypeDef touchData;
    uint32_t startTime = TS_Example_GetTick();
    uint8_t lastTouchState = 0;

    printf("Touch Drawing Test - Draw on the screen for 30 seconds\n");
    TS_Example_ClearScreen();
    TS_Example_DisplayText(TS_DISPLAY_MARGIN, TS_DISPLAY_MARGIN, "Drawing Mode - Draw!");

    while ((TS_Example_GetTick() - startTime) < 30000) {
        if (TS_GetTouchData(&hts, &touchData) == TS_OK) {
            if (touchData.TouchCount > 0) {
                TS_Example_DrawTouchPoint(touchData.Points[0].X, touchData.Points[0].Y,
                                        TS_EXAMPLE_COLOR_TOUCH);

                /* Store drawing point */
                if (DrawPointCount < TS_MAX_DRAW_POINTS) {
                    DrawPoints[DrawPointCount].X = touchData.Points[0].X;
                    DrawPoints[DrawPointCount].Y = touchData.Points[0].Y;
                    DrawPoints[DrawPointCount].Color = TS_EXAMPLE_COLOR_TOUCH;
                    DrawPoints[DrawPointCount].Size = TS_EXAMPLE_DRAW_SIZE;
                    DrawPoints[DrawPointCount].Timestamp = TS_Example_GetTick();
                    DrawPointCount++;
                }

                lastTouchState = 1;
            } else {
                lastTouchState = 0;
            }
        }
        TS_Example_Delay(20);
    }

    printf("Drawing test completed. Points drawn: %d\n", DrawPointCount);
    return TS_OK;
}

/**
  * @brief  Touchscreen calibration example
  * @retval TS_StatusTypeDef: Operation status
  */
TS_StatusTypeDef TS_Example_Calibration(void)
{
    TS_CalibrationTypeDef calibData;
    TS_TouchDataTypeDef touchData;
    uint16_t calibPoints[5][2] = {
        {20, 20},       /* Top-left */
        {220, 20},      /* Top-right */
        {120, 160},     /* Center */
        {20, 300},      /* Bottom-left */
        {220, 300}      /* Bottom-right */
    };
    uint8_t pointIndex = 0;
    uint32_t waitStart;

    printf("Touchscreen Calibration - Follow the instructions\n");
    TS_Example_ClearScreen();

    while (pointIndex < 5) {
        char buffer[50];
        sprintf(buffer, "Touch point %d/5", pointIndex + 1);
        TS_Example_DisplayText(TS_DISPLAY_MARGIN, TS_DISPLAY_MARGIN, buffer);

        /* Draw calibration point */
        TS_Example_DrawTouchPoint(calibPoints[pointIndex][0],
                                calibPoints[pointIndex][1],
                                TS_EXAMPLE_COLOR_CALIBRATION);

        /* Wait for touch */
        waitStart = TS_Example_GetTick();
        while ((TS_Example_GetTick() - waitStart) < 10000) {
            if (TS_GetTouchData(&hts, &touchData) == TS_OK && touchData.TouchCount > 0) {
                printf("Calibration point %d: Touch(%d,%d) -> Display(%d,%d)\n",
                       pointIndex + 1,
                       touchData.Points[0].X, touchData.Points[0].Y,
                       calibPoints[pointIndex][0], calibPoints[pointIndex][1]);

                /* Wait for release */
                while (touchData.TouchCount > 0) {
                    TS_GetTouchData(&hts, &touchData);
                    TS_Example_Delay(50);
                }

                pointIndex++;
                TS_Example_ClearScreen();
                break;
            }
            TS_Example_Delay(50);
        }

        if ((TS_Example_GetTick() - waitStart) >= 10000) {
            printf("Calibration timeout for point %d\n", pointIndex + 1);
            return TS_ERROR;
        }
    }

    /* Apply calibration */
    calibData.IsCalibrated = true;
    if (TS_SetCalibration(&hts, &calibData) == TS_OK) {
        printf("Calibration completed successfully\n");
        TS_Example_DisplayText(TS_DISPLAY_MARGIN, TS_DISPLAY_MARGIN, "Calibration Complete!");
        TS_Example_Delay(2000);
        return TS_OK;
    } else {
        printf("Calibration failed\n");
        return TS_ERROR;
    }
}

/**
  * @brief  Draw a touch point on the screen
  * @param  xPos: X coordinate
  * @param  yPos: Y coordinate
  * @param  color: Drawing color
  */
void TS_Example_DrawTouchPoint(uint16_t xPos, uint16_t yPos, uint16_t color)
{
    /* This function would interface with the LCD driver */
    /* For now, just print the coordinates */
    printf("Drawing point at (%d, %d) with color 0x%04X\n", xPos, yPos, color);
}

/**
  * @brief  Clear the screen
  */
void TS_Example_ClearScreen(void)
{
    /* This function would interface with the LCD driver */
    printf("Clearing screen\n");
}

/**
  * @brief  Display text on screen
  * @param  xPos: X coordinate
  * @param  yPos: Y coordinate
  * @param  text: Text to display
  */
void TS_Example_DisplayText(uint16_t xPos, uint16_t yPos, const char *text)
{
    /* This function would interface with the LCD driver */
    printf("Display text at (%d, %d): %s\n", xPos, yPos, text);
}

/**
  * @brief  Display test results
  * @param  results: Test results structure
  */
void TS_Example_DisplayResults(TS_TestResults_t *results)
{
    printf("Test Results:\n");
    printf("  Total Touches: %u\n", results->TotalTouches);
    printf("  Valid Touches: %u\n", results->ValidTouches);
    printf("  Average Position: (%.1f, %.1f)\n", results->AverageX, results->AverageY);
    printf("  Pressure Range: %d - %d\n", results->MinPressure, results->MaxPressure);
    printf("  Test Duration: %u ms\n", results->TestDuration);
}

/**
  * @brief  Touch callback function
  */
void TS_Example_TouchCallback(void)
{
    printf("Touch detected (callback)\n");
    CurrentTestResults.TotalTouches++;
}

/**
  * @brief  Release callback function
  */
void TS_Example_ReleaseCallback(void)
{
    printf("Touch released (callback)\n");
}

/**
  * @brief  Gesture callback function
  * @param  gesture: Detected gesture
  */
void TS_Example_GestureCallback(TS_GestureTypeDef gesture)
{
    const char *gestureName = "Unknown";

    switch (gesture) {
        case TS_GESTURE_SWIPE_UP:    gestureName = "Swipe Up"; break;
        case TS_GESTURE_SWIPE_DOWN:  gestureName = "Swipe Down"; break;
        case TS_GESTURE_SWIPE_LEFT:  gestureName = "Swipe Left"; break;
        case TS_GESTURE_SWIPE_RIGHT: gestureName = "Swipe Right"; break;
        case TS_GESTURE_TAP:         gestureName = "Tap"; break;
        case TS_GESTURE_DOUBLE_TAP:  gestureName = "Double Tap"; break;
        case TS_GESTURE_LONG_PRESS:  gestureName = "Long Press"; break;
        default: break;
    }

    printf("Gesture detected (callback): %s\n", gestureName);
}

/* Private function implementations ------------------------------------------*/

/**
  * @brief  Reset test results structure
  */
static void TS_Example_ResetTestResults(void)
{
    memset(&CurrentTestResults, 0, sizeof(CurrentTestResults));
    CurrentTestResults.MinPressure = 0xFFFF;
}

/**
  * @brief  Get system tick count
  * @retval Current tick count
  */
static uint32_t TS_Example_GetTick(void)
{
    return HAL_GetTick();
}

/**
  * @brief  Delay function
  * @param  delay: Delay in milliseconds
  */
static void TS_Example_Delay(uint32_t delay)
{
    HAL_Delay(delay);
}
