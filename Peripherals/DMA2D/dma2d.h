/**
  ******************************************************************************
  * @file    dma2d.h
  * @brief   DMA2D module interface
  * @details This file contains all the function prototypes for
  *          the DMA2D (Chrom-Art Accelerator) configuration.
  *          It provides APIs to initialize and control 2D graphics
  *          acceleration on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __DMA2D_H
#define __DMA2D_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes DMA2D peripheral used for graphics acceleration
 * @details Configures the DMA2D with appropriate parameters for
 *          graphic acceleration operations like color conversion and blending
 * @param   None
 * @retval  None
 */
void DMA2D_Init(void);

/* External variables --------------------------------------------------------*/
/**
 * @brief   DMA2D handle structure
 * @details Used by HAL functions to manage DMA2D operations
 *          for 2D graphics acceleration, pixel format conversion, and blending
 */
extern DMA2D_HandleTypeDef hdma2d;

#ifdef __cplusplus
}
#endif

#endif /* __DMA2D_H */
