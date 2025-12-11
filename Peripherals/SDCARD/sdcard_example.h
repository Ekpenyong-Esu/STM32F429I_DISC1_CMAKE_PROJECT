/**
 ******************************************************************************
 * @file    sdcard_example.h
 * @author  Mahonri
 * @brief   SD Card example header file for STM32F429I Discovery board
 *          This file contains example function prototypes for SD Card operations
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
#ifndef __SDCARD_EXAMPLE_H
#define __SDCARD_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "sdcard.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief  SD Card basic example - Initialize and display card information
 * @retval None
 */
void SDCARD_BasicExample(void);

/**
 * @brief  SD Card read/write example
 * @retval None
 */
void SDCARD_ReadWriteExample(void);

/**
 * @brief  SD Card DMA operations example
 * @retval None
 */
void SDCARD_DMAExample(void);

/**
 * @brief  SD Card performance test example
 * @retval None
 */
void SDCARD_PerformanceExample(void);

/**
 * @brief  SD Card format example
 * @retval None
 */
void SDCARD_FormatExample(void);

/**
 * @brief  SD Card file operations example (basic file system simulation)
 * @retval None
 */
void SDCARD_FileOperationsExample(void);

/**
 * @brief  SD Card complete example - Demonstrates all features
 * @retval None
 */
void SDCARD_CompleteExample(void);

#ifdef __cplusplus
}
#endif

#endif /* __SDCARD_EXAMPLE_H */
