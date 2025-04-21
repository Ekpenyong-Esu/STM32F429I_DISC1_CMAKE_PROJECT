/**
  ******************************************************************************
  * @file    ltdc.h
  * @brief   LTDC module interface
  * @details This file contains all the function prototypes for
  *          the LCD-TFT Display Controller (LTDC) configuration.
  *          It provides APIs to initialize and control the LCD interface
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __LTDC_H__
#define __LTDC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes LTDC peripheral used to control LCD display
 * @details Configures the LTDC with appropriate parameters for resolution,
 *          pixel format, background color, and layer settings
 * @param   None
 * @retval  None
 */
void LTDC_Init(void);

/* Exported variables ---------------------------------------------------------*/
/**
 * @brief   LTDC handle structure
 * @details Used by HAL functions to manage LTDC operations
 *          for LCD display control and framebuffer management
 */
extern LTDC_HandleTypeDef hltdc;

#ifdef __cplusplus
}
#endif

#endif /* __LTDC_H__ */
