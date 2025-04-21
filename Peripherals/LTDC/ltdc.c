/**
  ******************************************************************************
  * @file    ltdc.c
  * @brief   LTDC module implementation
  * @details This file provides code for the configuration
  *          and initialization of the LCD-TFT Display Controller
  *          for graphic display operations.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ltdc.h"
#include "../SYS/sys.h"

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   LTDC handle structure
 * @details Used by HAL functions for LTDC peripheral operations
 */
LTDC_HandleTypeDef hltdc;

/**
  * @brief  LTDC Initialization Function
  * @details Configures the LCD-TFT Display Controller with the following settings:
  *          - Synchronization signals polarity: Active low
  *          - Pixel clock polarity: Inverted
  *          - Timing parameters for 240x320 display
  *          - Black background color
  *
  *          Also configures two display layers with:
  *          - ARGB8888 pixel format (32-bit per pixel)
  *          - Full 255 opacity
  *          - Alpha blending mode
  *          - Black background color
  *          - 240x320 resolution
  *
  * @note   The framebuffer address (FBStartAdress) needs to be
  *         set properly before using the display
  * @param  None
  * @retval None
  */
void LTDC_Init(void)
{
  LTDC_LayerCfgTypeDef pLayerCfg = {0};   /* Layer 0 configuration structure */
  LTDC_LayerCfgTypeDef pLayerCfg1 = {0};  /* Layer 1 configuration structure */

  /* LTDC controller basic configuration */
  hltdc.Instance = LTDC;                   /* Select LTDC peripheral */

  /* Set signal polarities */
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;    /* Horizontal sync active low */
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;    /* Vertical sync active low */
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;    /* Data enable active low */
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;   /* Pixel clock inverted */

  /* Set timing parameters */
  hltdc.Init.HorizontalSync = 7;                 /* Horizontal sync width */
  hltdc.Init.VerticalSync = 3;                   /* Vertical sync height */
  hltdc.Init.AccumulatedHBP = 14;                /* Accumulated horizontal back porch */
  hltdc.Init.AccumulatedVBP = 5;                 /* Accumulated vertical back porch */
  hltdc.Init.AccumulatedActiveW = 654;           /* Width including porches */
  hltdc.Init.AccumulatedActiveH = 485;           /* Height including porches */
  hltdc.Init.TotalWidth = 660;                   /* Total width */
  hltdc.Init.TotalHeigh = 487;                   /* Total height */

  /* Set background color (black) */
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;

  /* Initialize the LTDC peripheral */
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }

  /* Configure Layer 0 */
  pLayerCfg.WindowX0 = 0;                           /* Window left position */
  pLayerCfg.WindowX1 = 240;                         /* Window right position */
  pLayerCfg.WindowY0 = 0;                           /* Window top position */
  pLayerCfg.WindowY1 = 320;                         /* Window bottom position */
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888; /* 32-bit ARGB format */
  pLayerCfg.Alpha = 255;                            /* Layer opacity (fully opaque) */
  pLayerCfg.Alpha0 = 0;                             /* Transparency for pixel value 0 */
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA; /* Constant alpha blending */
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA; /* Constant alpha blending */
  pLayerCfg.FBStartAdress = 0;                      /* Framebuffer address (to be set later) */
  pLayerCfg.ImageWidth = 240;                       /* Image width in pixels */
  pLayerCfg.ImageHeight = 320;                      /* Image height in pixels */

  /* Set layer background color (black) */
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;

  /* Configure layer 0 */
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if configuration fails */
  }

  /* Configure Layer 1 with same settings */
  pLayerCfg1.WindowX0 = 0;                          /* Window left position */
  pLayerCfg1.WindowX1 = 240;                        /* Window right position */
  pLayerCfg1.WindowY0 = 0;                          /* Window top position */
  pLayerCfg1.WindowY1 = 320;                        /* Window bottom position */
  pLayerCfg1.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888; /* 32-bit ARGB format */
  pLayerCfg1.Alpha = 255;                           /* Layer opacity (fully opaque) */
  pLayerCfg1.Alpha0 = 0;                            /* Transparency for pixel value 0 */
  pLayerCfg1.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA; /* Constant alpha blending */
  pLayerCfg1.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA; /* Constant alpha blending */
  pLayerCfg1.FBStartAdress = 0;                     /* Framebuffer address (to be set later) */
  pLayerCfg1.ImageWidth = 240;                      /* Image width in pixels */
  pLayerCfg1.ImageHeight = 320;                      /* Image height in pixels */

  /* Set layer background color (black) */
  pLayerCfg1.Backcolor.Blue = 0;
  pLayerCfg1.Backcolor.Green = 0;
  pLayerCfg1.Backcolor.Red = 0;

  /* Configure layer 1 */
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if configuration fails */
  }
}
