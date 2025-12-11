/**
 ******************************************************************************
 * @file    rtc_example.h
 * @author  Mahonri
 * @brief   RTC example header file for STM32F429I Discovery board
 *          This file contains example function prototypes for RTC operations
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_EXAMPLE_H
#define __RTC_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  RTC basic example - Initialize and display current time
 * @retval None
 */
void RTC_BasicExample(void);

/**
 * @brief  RTC set time example
 * @retval None
 */
void RTC_SetTimeExample(void);

/**
 * @brief  RTC alarm example
 * @retval None
 */
void RTC_AlarmExample(void);

/**
 * @brief  RTC timestamp example
 * @retval None
 */
void RTC_TimestampExample(void);

/**
 * @brief  RTC format example - Display formatted date and time
 * @retval None
 */
void RTC_FormatExample(void);

/**
 * @brief  RTC complete example - Demonstrates all features
 * @retval None
 */
void RTC_CompleteExample(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_EXAMPLE_H */
