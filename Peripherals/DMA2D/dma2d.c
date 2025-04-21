/**
  ******************************************************************************
  * @file    dma2d.c
  * @brief   DMA2D module implementation
  * @details This file provides code for the configuration
  *          and initialization of the DMA2D (Chrom-Art Accelerator)
  *          for hardware-accelerated 2D graphics operations.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "dma2d.h"
#include "../SYS/sys.h"

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   DMA2D handle structure
 * @details Used by HAL functions for DMA2D peripheral operations
 */
DMA2D_HandleTypeDef hdma2d;

/**
  * @brief  DMA2D Initialization Function
  * @details Configures the DMA2D peripheral with the following settings:
  *          - Mode: Register-to-Memory transfer (R2M)
  *            This mode is used for filling a rectangular area with a specific color
  *          - Color mode: 32-bit ARGB8888 format
  *          - Output offset: 0 (no gap between lines)
  *
  * @note   The DMA2D peripheral can be reconfigured at runtime for different
  *         operations such as Memory-to-Memory (M2M), Memory-to-Memory with
  *         Pixel Format Conversion (M2M_PFC), or Memory-to-Memory with Blending (M2M_Blending)
  * @param  None
  * @retval None
  */
void DMA2D_Init(void)
{
  hdma2d.Instance = DMA2D;                          /* Select DMA2D peripheral */
  hdma2d.Init.Mode = DMA2D_R2M;                     /* Register to memory mode (for area filling) */
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;    /* 32-bit ARGB color format */
  hdma2d.Init.OutputOffset = 0;                     /* No offset between lines */

  /* Initialize the DMA2D peripheral with the specified parameters */
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }
}
