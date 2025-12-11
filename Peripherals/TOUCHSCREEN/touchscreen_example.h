/**
  ******************************************************************************
  * @file    touchscreen_example.h
  * @brief   Touchscreen driver example header for STM32F429 Discovery Board
  * @details This file contains example functions and configurations for
  *          using the STMPE811 resistive touchscreen controller.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

#ifndef TOUCHSCREEN_EXAMPLE_H
#define TOUCHSCREEN_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "touchscreen.h"

/* Exported constants --------------------------------------------------------*/
#define TS_EXAMPLE_CALIBRATION_POINTS   5
#define TS_EXAMPLE_TEST_DURATION        30000   /* 30 seconds */
#define TS_EXAMPLE_DRAW_SIZE            5       /* Touch point draw size */

/* Colors for touch visualization */
#define TS_EXAMPLE_COLOR_TOUCH          0xF800  /* Red */
#define TS_EXAMPLE_COLOR_BACKGROUND     0x0000  /* Black */
#define TS_EXAMPLE_COLOR_TEXT           0xFFFF  /* White */
#define TS_EXAMPLE_COLOR_CALIBRATION    0x07E0  /* Green */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Touch drawing structure
 */
typedef struct {
    uint16_t X;                         /**< X coordinate */
    uint16_t Y;                         /**< Y coordinate */
    uint16_t Color;                     /**< Drawing color */
    uint8_t Size;                       /**< Brush size */
    uint32_t Timestamp;                 /**< Drawing timestamp */
} TS_DrawPoint_t;

/**
 * @brief Touch test results structure
 */
typedef struct {
    uint32_t TotalTouches;              /**< Total number of touches */
    uint32_t ValidTouches;              /**< Number of valid touches */
    uint32_t Gestures[8];               /**< Gesture counts */
    float AverageX;                     /**< Average X position */
    float AverageY;                     /**< Average Y position */
    uint16_t MinPressure;               /**< Minimum pressure detected */
    uint16_t MaxPressure;               /**< Maximum pressure detected */
    uint32_t TestDuration;              /**< Test duration in ms */
} TS_TestResults_t;

/* Exported function prototypes ---------------------------------------------*/

/* Basic examples */
TS_StatusTypeDef TS_Example_BasicTouch(void);
TS_StatusTypeDef TS_Example_TouchDrawing(void);
TS_StatusTypeDef TS_Example_Calibration(void);
TS_StatusTypeDef TS_Example_GestureDetection(void);

/* Advanced examples */
TS_StatusTypeDef TS_Example_MultiPointTest(void);
TS_StatusTypeDef TS_Example_PressureSensitivity(void);
TS_StatusTypeDef TS_Example_TouchAccuracy(void);
TS_StatusTypeDef TS_Example_ResponseTime(void);

/* Interactive examples */
TS_StatusTypeDef TS_Example_SimpleMenu(void);
TS_StatusTypeDef TS_Example_VirtualKeyboard(void);
TS_StatusTypeDef TS_Example_TouchPiano(void);
TS_StatusTypeDef TS_Example_DrawingBoard(void);

/* Test and diagnostic functions */
TS_StatusTypeDef TS_Example_DiagnosticTest(void);
TS_StatusTypeDef TS_Example_AccuracyTest(void);
TS_StatusTypeDef TS_Example_PerformanceTest(void);
TS_StatusTypeDef TS_Example_CalibrationTest(void);

/* Utility functions */
TS_StatusTypeDef TS_Example_Init(void);
void TS_Example_DrawTouchPoint(uint16_t xPos, uint16_t yPos, uint16_t color);
void TS_Example_ClearScreen(void);
void TS_Example_DisplayText(uint16_t xPos, uint16_t yPos, const char *text);
void TS_Example_DisplayResults(TS_TestResults_t *results);

/* Callback functions */
void TS_Example_TouchCallback(void);
void TS_Example_ReleaseCallback(void);
void TS_Example_GestureCallback(TS_GestureTypeDef gesture);

#ifdef __cplusplus
}
#endif

#endif /* TOUCHSCREEN_EXAMPLE_H */
