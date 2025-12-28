/**
 * @file ltdc.c
 * @brief LTDC (LCD-TFT Display Controller) driver implementation for STM32F429 Discovery board
 * @details This file provides the implementation for controlling the LCD-TFT
 *          Display Controller on the STM32F429 Discovery board. Features include
 *          display initialization, layer management, framebuffer control, and
 *          various drawing functions.
 * @version 1.0
 * @date 2025-09-03
 * @author STM32 Team
 */

/* Includes ------------------------------------------------------------------*/
#include "ltdc.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stm32f4xx_hal.h"

/*=============================================================================
 * ILI9341 LCD Controller Definitions (SPI Interface)
 *===========================================================================*/
/* ILI9341 Commands */
#define ILI9341_RESET               0x01
#define ILI9341_SLEEP_OUT           0x11
#define ILI9341_GAMMA               0x26
#define ILI9341_DISPLAY_OFF         0x28
#define ILI9341_DISPLAY_ON          0x29
#define ILI9341_COLUMN_ADDR         0x2A
#define ILI9341_PAGE_ADDR           0x2B
#define ILI9341_GRAM                0x2C
#define ILI9341_MAC                 0x36
#define ILI9341_PIXEL_FORMAT        0x3A
#define ILI9341_WDB                 0x51
#define ILI9341_WCD                 0x53
#define ILI9341_RGB_INTERFACE       0xB0
#define ILI9341_FRC                 0xB1
#define ILI9341_BPC                 0xB5
#define ILI9341_DFC                 0xB6
#define ILI9341_POWER1              0xC0
#define ILI9341_POWER2              0xC1
#define ILI9341_VCOM1               0xC5
#define ILI9341_VCOM2               0xC7
#define ILI9341_POWERA              0xCB
#define ILI9341_POWERB              0xCF
#define ILI9341_PGAMMA              0xE0
#define ILI9341_NGAMMA              0xE1
#define ILI9341_DTCA                0xE8
#define ILI9341_DTCB                0xEA
#define ILI9341_POWER_SEQ           0xED
#define ILI9341_3GAMMA_EN           0xF2
#define ILI9341_INTERFACE           0xF6
#define ILI9341_PRC                 0xF7

/* ILI9341 SPI GPIO Pins (STM32F429I-DISC1) */
#define ILI9341_WRX_PIN             GPIO_PIN_13
#define ILI9341_WRX_PORT            GPIOD
#define ILI9341_CS_PIN              GPIO_PIN_2
#define ILI9341_CS_PORT             GPIOC

/* Private defines -----------------------------------------------------------*/
#define LTDC_TIMEOUT                5000    /*!< Timeout for LTDC operations */
#define LTDC_MIN_ALPHA              0       /*!< Minimum alpha value */
#define LTDC_MAX_ALPHA              255     /*!< Maximum alpha value */
#define LTDC_DEFAULT_BRIGHTNESS     100     /*!< Default brightness percentage */

/* Private variables ---------------------------------------------------------*/
LTDC_HandleTypeDef hltdc;                   /*!< LTDC HAL handle */
static SPI_HandleTypeDef hspi_lcd;          /*!< SPI handle for ILI9341 */

/* ILI9341 SPI Helper Functions - Forward Declarations */
static void ILI9341_SPI_Init(void);
static void ILI9341_WriteCommand(uint8_t cmd);
static void ILI9341_WriteData(uint8_t data);
static void ILI9341_Init(void);

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef LTDC_ValidateDriver(LTDC_Driver_t *driver);
static HAL_StatusTypeDef LTDC_ValidateLayer(uint8_t layer);
static HAL_StatusTypeDef LTDC_ValidateCoordinates(uint16_t x, uint16_t y);
static HAL_StatusTypeDef LTDC_ValidateRect(LTDC_Rect_t *rect);
static uint32_t LTDC_GetPixelFormatHAL(LTDC_PixelFormat_t format);
static LTDC_PixelFormat_t LTDC_GetPixelFormatDriver(uint32_t halFormat);
static void LTDC_SetPixel(uint32_t *framebuffer, uint16_t x, uint16_t y, uint32_t color, LTDC_PixelFormat_t format);
static uint32_t LTDC_GetPixel(uint32_t *framebuffer, uint16_t x, uint16_t y, LTDC_PixelFormat_t format);
static void LTDC_DrawHorizontalLine(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint16_t length, uint32_t color);
static void LTDC_DrawVerticalLine(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint16_t length, uint32_t color);

/* Public Functions ----------------------------------------------------------*/

/**
 * @brief Initialize LTDC driver
 * @param driver: Pointer to LTDC driver structure
 * @param hltdc: Pointer to HAL LTDC handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_Driver_Init(LTDC_Driver_t *driver, LTDC_HandleTypeDef *hltdc_handle) {
    if (driver == NULL || hltdc_handle == NULL) {
        return HAL_ERROR;
    }

    /* Initialize driver structure */
    memset(driver, 0, sizeof(LTDC_Driver_t));
    driver->hltdc = hltdc_handle;
    driver->errorCode = LTDC_ERROR_NONE;

    /* Set default display configuration */
    driver->displayConfig.width = LTDC_DISPLAY_WIDTH;
    driver->displayConfig.height = LTDC_DISPLAY_HEIGHT;
    driver->displayConfig.backgroundColor = LTDC_COLOR_BLACK;
    driver->displayConfig.hsyncActiveLow = true;
    driver->displayConfig.vsyncActiveLow = true;
    driver->displayConfig.dataEnableActiveLow = false;
    driver->displayConfig.pixelClockInverted = true;

    /* Initialize layer configurations */
    for (uint8_t i = 0; i < LTDC_MAX_LAYERS; i++) {
        driver->layers[i].windowX0 = 0;
        driver->layers[i].windowY0 = 0;
        /* WindowX1/Y1 are inclusive end coordinates: use width-1/height-1 */
        driver->layers[i].windowX1 = LTDC_DISPLAY_WIDTH - 1;
        driver->layers[i].windowY1 = LTDC_DISPLAY_HEIGHT - 1;
        driver->layers[i].imageWidth = LTDC_DISPLAY_WIDTH;
        driver->layers[i].imageHeight = LTDC_DISPLAY_HEIGHT;
        driver->layers[i].pixelFormat = LTDC_PIXEL_FORMAT_RGB565_ENUM;
        driver->layers[i].alpha = LTDC_MAX_ALPHA;
        driver->layers[i].alpha0 = 0;
        driver->layers[i].blendMode = LTDC_BLEND_CONSTANT_ALPHA;
        driver->layers[i].backgroundColor = LTDC_COLOR_BLACK;
        driver->layers[i].enabled = false;
        driver->layers[i].framebufferAddress = 0;
    }

    driver->activeLayer = 0;
    driver->initialized = true;

    return HAL_OK;
}

/**
 * @brief Deinitialize LTDC driver
 * @param driver: Pointer to LTDC driver structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_Driver_DeInit(LTDC_Driver_t *driver) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Disable all layers */
    for (uint8_t i = 0; i < LTDC_MAX_LAYERS; i++) {
        LTDC_DisableLayer(driver, i);
    }

    /* Deinitialize HAL LTDC */
    HAL_StatusTypeDef status = HAL_LTDC_DeInit(driver->hltdc);

    /* Reset driver structure */
    driver->initialized = false;
    driver->errorCode = LTDC_ERROR_NONE;

    return status;
}

/**
 * @brief Configure display parameters
 * @param driver: Pointer to LTDC driver structure
 * @param config: Pointer to display configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ConfigureDisplay(LTDC_Driver_t *driver, LTDC_DisplayConfig_t *config) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || config == NULL) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Configure LTDC timing parameters */
    driver->hltdc->Instance = LTDC;

    /* Set signal polarities */
    driver->hltdc->Init.HSPolarity = config->hsyncActiveLow ? LTDC_HSPOLARITY_AL : LTDC_HSPOLARITY_AH;
    driver->hltdc->Init.VSPolarity = config->vsyncActiveLow ? LTDC_VSPOLARITY_AL : LTDC_VSPOLARITY_AH;
    driver->hltdc->Init.DEPolarity = config->dataEnableActiveLow ? LTDC_DEPOLARITY_AL : LTDC_DEPOLARITY_AH;
    driver->hltdc->Init.PCPolarity = config->pixelClockInverted ? LTDC_PCPOLARITY_IPC : LTDC_PCPOLARITY_IIPC;

    /* Set timing parameters for STM32F429I-DISC1 */
    driver->hltdc->Init.HorizontalSync = LTDC_HSYNC_WIDTH - 1;
    driver->hltdc->Init.VerticalSync = LTDC_VSYNC_HEIGHT - 1;
    driver->hltdc->Init.AccumulatedHBP = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH - 1;
    driver->hltdc->Init.AccumulatedVBP = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT - 1;
    driver->hltdc->Init.AccumulatedActiveW = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH + config->width - 1;
    driver->hltdc->Init.AccumulatedActiveH = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT + config->height - 1;
    driver->hltdc->Init.TotalWidth = LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH + config->width + LTDC_HFP_WIDTH - 1;
    driver->hltdc->Init.TotalHeigh = LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT + config->height + LTDC_VFP_HEIGHT - 1;

    /* Set background color */
    driver->hltdc->Init.Backcolor.Blue = (config->backgroundColor) & 0xFF;
    driver->hltdc->Init.Backcolor.Green = (config->backgroundColor >> 8) & 0xFF;
    driver->hltdc->Init.Backcolor.Red = (config->backgroundColor >> 16) & 0xFF;

    /* Initialize LTDC */
    HAL_StatusTypeDef status = HAL_LTDC_Init(driver->hltdc);
    if (status != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INIT_FAILED;
        return status;
    }

    /* Store configuration */
    driver->displayConfig = *config;

    return HAL_OK;
}

/**
 * @brief Configure layer parameters
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @param config: Pointer to layer configuration
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ConfigureLayer(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *config) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK || config == NULL) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    LTDC_LayerCfgTypeDef layerCfg = {0};

    /* Configure layer parameters */
    layerCfg.WindowX0 = config->windowX0;
    layerCfg.WindowX1 = config->windowX1;
    layerCfg.WindowY0 = config->windowY0;
    layerCfg.WindowY1 = config->windowY1;
    layerCfg.PixelFormat = LTDC_GetPixelFormatHAL(config->pixelFormat);
    layerCfg.Alpha = config->alpha;
    layerCfg.Alpha0 = config->alpha0;

    /* Set blending factors based on blend mode */
    switch (config->blendMode) {
        case LTDC_BLEND_CONSTANT_ALPHA:
            layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
            layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
            break;
        case LTDC_BLEND_PIXEL_ALPHA:
            layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
            layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
            break;
        case LTDC_BLEND_NO_BLENDING:
        default:
            layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
            layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
            break;
    }

    layerCfg.FBStartAdress = config->framebufferAddress;
    layerCfg.ImageWidth = config->imageWidth;
    layerCfg.ImageHeight = config->imageHeight;

    /* Set background color */
    layerCfg.Backcolor.Blue = (config->backgroundColor) & 0xFF;
    layerCfg.Backcolor.Green = (config->backgroundColor >> 8) & 0xFF;
    layerCfg.Backcolor.Red = (config->backgroundColor >> 16) & 0xFF;

    /* Configure HAL layer */
    HAL_StatusTypeDef status = HAL_LTDC_ConfigLayer(driver->hltdc, &layerCfg, layer);
    if (status != HAL_OK) {
        driver->errorCode = LTDC_ERROR_LAYER_CONFIG;
        return status;
    }

    /* Store configuration */
    driver->layers[layer] = *config;

    return HAL_OK;
}

/**
 * @brief Enable layer
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_EnableLayer(LTDC_Driver_t *driver, uint8_t layer) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Enable layer */
    __HAL_LTDC_LAYER_ENABLE(driver->hltdc, layer);
    __HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(driver->hltdc);

    driver->layers[layer].enabled = true;

    return HAL_OK;
}

/**
 * @brief Disable layer
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_DisableLayer(LTDC_Driver_t *driver, uint8_t layer) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Disable layer */
    __HAL_LTDC_LAYER_DISABLE(driver->hltdc, layer);
    __HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(driver->hltdc);

    driver->layers[layer].enabled = false;

    return HAL_OK;
}

/**
 * @brief Set active layer for drawing operations
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_SetActiveLayer(LTDC_Driver_t *driver, uint8_t layer) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    driver->activeLayer = layer;
    return HAL_OK;
}

/**
 * @brief Set layer alpha (transparency)
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @param alpha: Alpha value (0-255)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_SetLayerAlpha(LTDC_Driver_t *driver, uint8_t layer, uint8_t alpha) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Set layer alpha */
    HAL_StatusTypeDef status = HAL_LTDC_SetAlpha(driver->hltdc, alpha, layer);
    if (status == HAL_OK) {
        driver->layers[layer].alpha = alpha;
    }

    return status;
}

/**
 * @brief Set layer position
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @param x: X position
 * @param y: Y position
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_SetLayerPosition(LTDC_Driver_t *driver, uint8_t layer, uint16_t x, uint16_t y) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    if (x >= LTDC_DISPLAY_WIDTH || y >= LTDC_DISPLAY_HEIGHT) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Calculate new window coordinates */
    uint16_t width = driver->layers[layer].windowX1 - driver->layers[layer].windowX0;
    uint16_t height = driver->layers[layer].windowY1 - driver->layers[layer].windowY0;

    /* Set layer window position */
    HAL_StatusTypeDef status = HAL_LTDC_SetWindowPosition(driver->hltdc, x, y, layer);
    if (status == HAL_OK) {
        driver->layers[layer].windowX0 = x;
        driver->layers[layer].windowY0 = y;
        driver->layers[layer].windowX1 = x + width;
        driver->layers[layer].windowY1 = y + height;
    }

    return status;
}

/**
 * @brief Set layer window
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @param window: Pointer to window rectangle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_SetLayerWindow(LTDC_Driver_t *driver, uint8_t layer, LTDC_Rect_t *window) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK ||
        LTDC_ValidateRect(window) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Set layer window size */
    HAL_StatusTypeDef status = HAL_LTDC_SetWindowSize(driver->hltdc, window->width, window->height, layer);
    if (status == HAL_OK) {
        /* Update position */
        status = HAL_LTDC_SetWindowPosition(driver->hltdc, window->x, window->y, layer);
        if (status == HAL_OK) {
            driver->layers[layer].windowX0 = window->x;
            driver->layers[layer].windowY0 = window->y;
            driver->layers[layer].windowX1 = window->x + window->width;
            driver->layers[layer].windowY1 = window->y + window->height;
        }
    }

    return status;
}

/**
 * @brief Set framebuffer address for layer
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @param address: Framebuffer start address
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_SetFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t address) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK || address == 0) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Set framebuffer address */
    HAL_StatusTypeDef status = HAL_LTDC_SetAddress(driver->hltdc, address, layer);
    if (status == HAL_OK) {
        /* Request register reload during next vertical blanking to avoid tearing */
        __HAL_LTDC_VERTICAL_BLANKING_RELOAD_CONFIG(driver->hltdc);

        /* Wait for register reload flag (RR) with timeout */
        uint32_t timeout = LTDC_TIMEOUT;
        while((__HAL_LTDC_GET_FLAG(driver->hltdc, LTDC_FLAG_RR) == RESET) && timeout--) {
            /* simple busy-wait; in real app consider using IRQ */
        }

        /* Clear the RR flag if set */
        if (__HAL_LTDC_GET_FLAG(driver->hltdc, LTDC_FLAG_RR) != RESET) {
            __HAL_LTDC_CLEAR_FLAG(driver->hltdc, LTDC_FLAG_RR);
        }

        driver->layers[layer].framebufferAddress = address;
    }

    return status;
}

/**
 * @brief Swap framebuffer safely at next VSYNC
 * @param driver: Pointer to LTDC driver
 * @param layer: Layer index
 * @param address: New framebuffer address
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef LTDC_SwapFramebufferAtVSync(LTDC_Driver_t *driver, uint8_t layer, uint32_t address) {
    return LTDC_SetFramebuffer(driver, layer, address);
}

/**
 * @brief Clear framebuffer with specified color
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @param color: Clear color
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ClearFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t color) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    if (driver->layers[layer].framebufferAddress == 0) {
        driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
        return HAL_ERROR;
    }

    uint32_t *framebuffer = (uint32_t *)driver->layers[layer].framebufferAddress;
    uint32_t pixelCount = driver->layers[layer].imageWidth * driver->layers[layer].imageHeight;

    /* Clear framebuffer based on pixel format */
    switch (driver->layers[layer].pixelFormat) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            {
                uint16_t *fb16 = (uint16_t *)framebuffer;
                uint16_t color16 = (uint16_t)color;
                for (uint32_t i = 0; i < pixelCount; i++) {
                    fb16[i] = color16;
                }
            }
            break;

        case LTDC_PIXEL_FORMAT_RGB888_ENUM:
            {
                uint8_t *fb8 = (uint8_t *)framebuffer;
                for (uint32_t i = 0; i < pixelCount; i++) {
                    fb8[i * 3] = (color) & 0xFF;        /* Blue */
                    fb8[i * 3 + 1] = (color >> 8) & 0xFF;  /* Green */
                    fb8[i * 3 + 2] = (color >> 16) & 0xFF; /* Red */
                }
            }
            break;

        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
        default:
            for (uint32_t i = 0; i < pixelCount; i++) {
                framebuffer[i] = color;
            }
            break;
    }

    return HAL_OK;
}

/**
 * @brief Fill framebuffer with specified color
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number (0 or 1)
 * @param color: Fill color
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_FillFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t color) {
    return LTDC_ClearFramebuffer(driver, layer, color);
}

/**
 * @brief Copy framebuffer from source to destination layer
 * @param driver: Pointer to LTDC driver structure
 * @param srcLayer: Source layer number
 * @param dstLayer: Destination layer number
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_CopyFramebuffer(LTDC_Driver_t *driver, uint8_t srcLayer, uint8_t dstLayer) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(srcLayer) != HAL_OK ||
        LTDC_ValidateLayer(dstLayer) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    if (driver->layers[srcLayer].framebufferAddress == 0 || driver->layers[dstLayer].framebufferAddress == 0) {
        driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
        return HAL_ERROR;
    }

    /* Calculate size based on pixel format */
    uint32_t srcSize = driver->layers[srcLayer].imageWidth * driver->layers[srcLayer].imageHeight;
    uint32_t dstSize = driver->layers[dstLayer].imageWidth * driver->layers[dstLayer].imageHeight;

    /* Use smaller size for safety */
    uint32_t copySize = (srcSize < dstSize) ? srcSize : dstSize;

    /* Calculate bytes per pixel */
    uint32_t bytesPerPixel = 4; /* Default to 32-bit */
    if (driver->layers[srcLayer].pixelFormat == LTDC_PIXEL_FORMAT_RGB565_ENUM) {
        bytesPerPixel = 2;
    } else if (driver->layers[srcLayer].pixelFormat == LTDC_PIXEL_FORMAT_RGB888_ENUM) {
        bytesPerPixel = 3;
    }

    /* Copy framebuffer */
    memcpy((void *)driver->layers[dstLayer].framebufferAddress,
           (void *)driver->layers[srcLayer].framebufferAddress,
           copySize * bytesPerPixel);

    return HAL_OK;
}

/**
 * @brief Draw pixel at specified coordinates
 * @param driver: Pointer to LTDC driver structure
 * @param x: X coordinate
 * @param y: Y coordinate
 * @param color: Pixel color
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_DrawPixel(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint32_t color) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateCoordinates(x, y) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    uint8_t layer = driver->activeLayer;
    if (driver->layers[layer].framebufferAddress == 0) {
        driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
        return HAL_ERROR;
    }

    uint32_t *framebuffer = (uint32_t *)driver->layers[layer].framebufferAddress;
    LTDC_SetPixel(framebuffer, x, y, color, driver->layers[layer].pixelFormat);

    return HAL_OK;
}

/**
 * @brief Draw line between two points
 * @param driver: Pointer to LTDC driver structure
 * @param start: Start point
 * @param end: End point
 * @param color: Line color
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_DrawLine(LTDC_Driver_t *driver, LTDC_Point_t start, LTDC_Point_t end, uint32_t color) {
    if (LTDC_ValidateDriver(driver) != HAL_OK ||
        LTDC_ValidateCoordinates(start.x, start.y) != HAL_OK ||
        LTDC_ValidateCoordinates(end.x, end.y) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Bresenham's line algorithm */
    int16_t dx = abs(end.x - start.x);
    int16_t dy = abs(end.y - start.y);
    int16_t sx = (start.x < end.x) ? 1 : -1;
    int16_t sy = (start.y < end.y) ? 1 : -1;
    int16_t err = dx - dy;

    int16_t x = start.x;
    int16_t y = start.y;

    while (true) {
        LTDC_DrawPixel(driver, x, y, color);

        if (x == end.x && y == end.y) {
            break;
        }

        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }

    return HAL_OK;
}

/**
 * @brief Draw rectangle
 * @param driver: Pointer to LTDC driver structure
 * @param rect: Pointer to rectangle structure
 * @param color: Rectangle color
 * @param filled: true for filled rectangle, false for outline
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_DrawRectangle(LTDC_Driver_t *driver, LTDC_Rect_t *rect, uint32_t color, bool filled) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateRect(rect) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    if (filled) {
        /* Fill rectangle */
        for (uint16_t y = rect->y; y < (rect->y + rect->height); y++) {
            LTDC_DrawHorizontalLine(driver, rect->x, y, rect->width, color);
        }
    } else {
        /* Draw rectangle outline */
        LTDC_DrawHorizontalLine(driver, rect->x, rect->y, rect->width, color);
        LTDC_DrawHorizontalLine(driver, rect->x, rect->y + rect->height - 1, rect->width, color);
        LTDC_DrawVerticalLine(driver, rect->x, rect->y, rect->height, color);
        LTDC_DrawVerticalLine(driver, rect->x + rect->width - 1, rect->y, rect->height, color);
    }

    return HAL_OK;
}

/**
 * @brief Draw circle
 * @param driver: Pointer to LTDC driver structure
 * @param center: Circle center point
 * @param radius: Circle radius
 * @param color: Circle color
 * @param filled: true for filled circle, false for outline
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_DrawCircle(LTDC_Driver_t *driver, LTDC_Point_t center, uint16_t radius, uint32_t color, bool filled) {
    if (LTDC_ValidateDriver(driver) != HAL_OK ||
        LTDC_ValidateCoordinates(center.x, center.y) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Midpoint circle algorithm */
    int16_t x = 0;
    int16_t y = radius;
    int16_t d = 1 - radius;

    while (x <= y) {
        if (filled) {
            /* Draw horizontal lines for filled circle */
            LTDC_DrawHorizontalLine(driver, center.x - x, center.y + y, 2 * x + 1, color);
            LTDC_DrawHorizontalLine(driver, center.x - x, center.y - y, 2 * x + 1, color);
            LTDC_DrawHorizontalLine(driver, center.x - y, center.y + x, 2 * y + 1, color);
            LTDC_DrawHorizontalLine(driver, center.x - y, center.y - x, 2 * y + 1, color);
        } else {
            /* Draw circle outline */
            LTDC_DrawPixel(driver, center.x + x, center.y + y, color);
            LTDC_DrawPixel(driver, center.x - x, center.y + y, color);
            LTDC_DrawPixel(driver, center.x + x, center.y - y, color);
            LTDC_DrawPixel(driver, center.x - x, center.y - y, color);
            LTDC_DrawPixel(driver, center.x + y, center.y + x, color);
            LTDC_DrawPixel(driver, center.x - y, center.y + x, color);
            LTDC_DrawPixel(driver, center.x + y, center.y - x, color);
            LTDC_DrawPixel(driver, center.x - y, center.y - x, color);
        }

        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }

    return HAL_OK;
}

/**
 * @brief Set background color
 * @param driver: Pointer to LTDC driver structure
 * @param color: Background color
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_SetBackgroundColor(LTDC_Driver_t *driver, uint32_t color) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Set background color */
    LTDC_ColorTypeDef bgColor;
    bgColor.Blue = (color) & 0xFF;
    bgColor.Green = (color >> 8) & 0xFF;
    bgColor.Red = (color >> 16) & 0xFF;

    HAL_StatusTypeDef status = HAL_LTDC_ConfigCLUT(driver->hltdc, &bgColor, 1, 0);
    if (status == HAL_OK) {
        driver->displayConfig.backgroundColor = color;
    }

    return status;
}

/**
 * @brief Display on
 * @param driver: Pointer to LTDC driver structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_DisplayOn(LTDC_Driver_t *driver) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Enable LTDC */
    __HAL_LTDC_ENABLE(driver->hltdc);

    return HAL_OK;
}

/**
 * @brief Display off
 * @param driver: Pointer to LTDC driver structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_DisplayOff(LTDC_Driver_t *driver) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Disable LTDC */
    __HAL_LTDC_DISABLE(driver->hltdc);

    return HAL_OK;
}

/* Utility Functions ---------------------------------------------------------*/

/**
 * @brief Convert color between different pixel formats
 * @param color: Input color
 * @param fromFormat: Source pixel format
 * @param toFormat: Target pixel format
 * @return uint32_t: Converted color
 */
uint32_t LTDC_ConvertColor(uint32_t color, LTDC_PixelFormat_t fromFormat, LTDC_PixelFormat_t toFormat) {
    if (fromFormat == toFormat) {
        return color;
    }

    /* Convert to RGB888 as intermediate format */
    uint32_t rgb888;

    switch (fromFormat) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            rgb888 = LTDC_RGB565_To_RGB888((uint16_t)color);
            break;
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
            rgb888 = color & 0x00FFFFFF; /* Remove alpha */
            break;
        default:
            rgb888 = color;
            break;
    }

    /* Convert from RGB888 to target format */
    switch (toFormat) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            return LTDC_RGB888_To_RGB565(rgb888);
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
            return rgb888 | 0xFF000000; /* Add full alpha */
        default:
            return rgb888;
    }
}

/**
 * @brief Convert RGB888 to RGB565
 * @param rgb888: 24-bit RGB color
 * @return uint32_t: 16-bit RGB565 color
 */
uint32_t LTDC_RGB888_To_RGB565(uint32_t rgb888) {
    uint32_t r = (rgb888 >> 16) & 0xFF;
    uint32_t g = (rgb888 >> 8) & 0xFF;
    uint32_t b = rgb888 & 0xFF;

    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

/**
 * @brief Convert RGB565 to RGB888
 * @param rgb565: 16-bit RGB565 color
 * @return uint32_t: 24-bit RGB888 color
 */
uint32_t LTDC_RGB565_To_RGB888(uint16_t rgb565) {
    uint32_t r = (rgb565 >> 11) & 0x1F;
    uint32_t g = (rgb565 >> 5) & 0x3F;
    uint32_t b = rgb565 & 0x1F;

    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    return (r << 16) | (g << 8) | b;
}

/**
 * @brief Convert ARGB8888 to RGB565
 * @param argb8888: 32-bit ARGB color
 * @return uint32_t: 16-bit RGB565 color
 */
uint32_t LTDC_ARGB8888_To_RGB565(uint32_t argb8888) {
    return LTDC_RGB888_To_RGB565(argb8888 & 0x00FFFFFF);
}

/**
 * @brief Get layer information
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number
 * @param info: Pointer to layer info structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_GetLayerInfo(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *info) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK || info == NULL) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    *info = driver->layers[layer];
    return HAL_OK;
}

/**
 * @brief Check if layer is enabled
 * @param driver: Pointer to LTDC driver structure
 * @param layer: Layer number
 * @return bool: true if enabled, false otherwise
 */
bool LTDC_IsLayerEnabled(LTDC_Driver_t *driver, uint8_t layer) {
    if (driver == NULL || layer >= LTDC_MAX_LAYERS) {
        return false;
    }
    return driver->layers[layer].enabled;
}

/**
 * @brief Get last error code
 * @param driver: Pointer to LTDC driver structure
 * @return uint32_t: Error code
 */
uint32_t LTDC_GetError(LTDC_Driver_t *driver) {
    if (driver == NULL) {
        return LTDC_ERROR_INVALID_PARAM;
    }
    return driver->errorCode;
}

/**
 * @brief Clear error code
 * @param driver: Pointer to LTDC driver structure
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_ClearError(LTDC_Driver_t *driver) {
    if (driver == NULL) {
        return HAL_ERROR;
    }
    driver->errorCode = LTDC_ERROR_NONE;
    return HAL_OK;
}

/*=============================================================================
 * ILI9341 LCD Controller SPI Initialization (CRITICAL FOR DISPLAY!)
 *===========================================================================*/

/**
 * @brief Initialize SPI5 for ILI9341 LCD controller communication
 * @note  STM32F429I-DISC1 uses SPI5 for LCD communication
 */
static void ILI9341_SPI_Init(void)
{
    /* Enable GPIO clocks for SPI5 and control pins */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_SPI5_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure SPI5 pins: SCK (PF7), MISO (PF8), MOSI (PF9) */
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* Configure LCD CS pin (PC2) */
    GPIO_InitStruct.Pin = ILI9341_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ILI9341_CS_PORT, &GPIO_InitStruct);

    /* Configure LCD WRX/DCX pin (PD13) - Data/Command selection */
    GPIO_InitStruct.Pin = ILI9341_WRX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ILI9341_WRX_PORT, &GPIO_InitStruct);

    /* Set CS high (deselect) */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);

    /* Configure SPI5 */
    hspi_lcd.Instance = SPI5;
    hspi_lcd.Init.Mode = SPI_MODE_MASTER;
    hspi_lcd.Init.Direction = SPI_DIRECTION_2LINES;
    hspi_lcd.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi_lcd.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi_lcd.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi_lcd.Init.NSS = SPI_NSS_SOFT;
    hspi_lcd.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi_lcd.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi_lcd.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi_lcd.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi_lcd.Init.CRCPolynomial = 10;
    HAL_SPI_Init(&hspi_lcd);
}

/**
 * @brief Send command to ILI9341 LCD controller
 * @param cmd: Command byte to send
 */
static void ILI9341_WriteCommand(uint8_t cmd)
{
    /* WRX low = command */
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_RESET);
    /* CS low (select) */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
    /* Send command */
    HAL_SPI_Transmit(&hspi_lcd, &cmd, 1, HAL_MAX_DELAY);
    /* CS high (deselect) */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief Send data to ILI9341 LCD controller
 * @param data: Data byte to send
 */
static void ILI9341_WriteData(uint8_t data)
{
    /* WRX high = data */
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_SET);
    /* CS low (select) */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
    /* Send data */
    HAL_SPI_Transmit(&hspi_lcd, &data, 1, HAL_MAX_DELAY);
    /* CS high (deselect) */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
}

/**
 * @brief Initialize ILI9341 LCD controller for RGB interface mode
 * @note  This configures the ILI9341 to receive pixel data from LTDC via RGB interface
 */
static void ILI9341_Init(void)
{
    /* Initialize SPI for communication */
    ILI9341_SPI_Init();

    /* Hardware reset delay */
    HAL_Delay(10);

    /* Software Reset */
    ILI9341_WriteCommand(ILI9341_RESET);
    HAL_Delay(20);

    /* Power Control A */
    ILI9341_WriteCommand(ILI9341_POWERA);
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);

    /* Power Control B */
    ILI9341_WriteCommand(ILI9341_POWERB);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x30);

    /* Driver Timing Control A */
    ILI9341_WriteCommand(ILI9341_DTCA);
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);

    /* Driver Timing Control B */
    ILI9341_WriteCommand(ILI9341_DTCB);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);

    /* Power On Sequence Control */
    ILI9341_WriteCommand(ILI9341_POWER_SEQ);
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x12);
    ILI9341_WriteData(0x81);

    /* Pump Ratio Control */
    ILI9341_WriteCommand(ILI9341_PRC);
    ILI9341_WriteData(0x20);

    /* Power Control 1 */
    ILI9341_WriteCommand(ILI9341_POWER1);
    ILI9341_WriteData(0x23);

    /* Power Control 2 */
    ILI9341_WriteCommand(ILI9341_POWER2);
    ILI9341_WriteData(0x10);

    /* VCOM Control 1 */
    ILI9341_WriteCommand(ILI9341_VCOM1);
    ILI9341_WriteData(0x3E);
    ILI9341_WriteData(0x28);

    /* VCOM Control 2 */
    ILI9341_WriteCommand(ILI9341_VCOM2);
    ILI9341_WriteData(0x86);

    /* Memory Access Control - Set orientation */
    ILI9341_WriteCommand(ILI9341_MAC);
    ILI9341_WriteData(0x08);  /* Portrait mode,z RGB order */

    /* Pixel Format Set - 16 bits/pixel (RGB565) */
    ILI9341_WriteCommand(ILI9341_PIXEL_FORMAT);
    ILI9341_WriteData(0x55);  /* 16-bit for RGB and MCU interfaces */

    /* Frame Rate Control */
    ILI9341_WriteCommand(ILI9341_FRC);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x18);  /* 79Hz frame rate */

    /* Display Function Control */
    ILI9341_WriteCommand(ILI9341_DFC);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x82);
    ILI9341_WriteData(0x27);

    /* 3Gamma Function Disable */
    ILI9341_WriteCommand(ILI9341_3GAMMA_EN);
    ILI9341_WriteData(0x00);

    /* Gamma Set */
    ILI9341_WriteCommand(ILI9341_GAMMA);
    ILI9341_WriteData(0x01);

    /* Positive Gamma Correction */
    ILI9341_WriteCommand(ILI9341_PGAMMA);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0xF1);
    ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x10);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x00);

    /* Negative Gamma Correction */
    ILI9341_WriteCommand(ILI9341_NGAMMA);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x36);
    ILI9341_WriteData(0x0F);

    /* Column Address Set (full width: 0-239) */
    ILI9341_WriteCommand(ILI9341_COLUMN_ADDR);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xEF);  /* 239 */

    /* Page Address Set (full height: 0-319) */
    ILI9341_WriteCommand(ILI9341_PAGE_ADDR);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x01);
    ILI9341_WriteData(0x3F);  /* 319 */

    /* Interface Control - Enable RGB interface */
    ILI9341_WriteCommand(ILI9341_INTERFACE);
    ILI9341_WriteData(0x01);  /* System interface */
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x06);  /* RGB interface, DE polarity */

    /* RGB Interface Control */
    ILI9341_WriteCommand(ILI9341_RGB_INTERFACE);
    ILI9341_WriteData(0xC2);  /* RGB interface mode, DE polarity high */

    /* Exit Sleep Mode */
    ILI9341_WriteCommand(ILI9341_SLEEP_OUT);
    HAL_Delay(120);  /* Wait for sleep out (required by datasheet) */

    /* Display ON */
    ILI9341_WriteCommand(ILI9341_DISPLAY_ON);
    HAL_Delay(20);

    /* Memory Write - Start receiving pixel data */
    ILI9341_WriteCommand(ILI9341_GRAM);
}

/**
 * @brief Hardware initialization
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_HW_Init(void) {
    /*=========================================================================
     * STEP 1: Initialize ILI9341 LCD Controller via SPI
     * This is CRITICAL! The LCD won't display anything without this step.
     *=======================================================================*/
    ILI9341_Init();

    /* Enable LTDC clock */
    __HAL_RCC_LTDC_CLK_ENABLE();

    /* Configure LTDC peripheral */
    hltdc.Instance = LTDC;

    /* Configure synchronous timings: signal polarities and timing parameters */
    hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

    /* Timing parameters for ILI9341 LCD controller */
    hltdc.Init.HorizontalSync = (LTDC_HSYNC_WIDTH - 1);
    hltdc.Init.VerticalSync = (LTDC_VSYNC_HEIGHT - 1);
    hltdc.Init.AccumulatedHBP = (LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH - 1);
    hltdc.Init.AccumulatedVBP = (LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT - 1);
    hltdc.Init.AccumulatedActiveW = (LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH + LTDC_DISPLAY_WIDTH - 1);
    hltdc.Init.AccumulatedActiveH = (LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT + LTDC_DISPLAY_HEIGHT - 1);
    hltdc.Init.TotalWidth = (LTDC_HSYNC_WIDTH + LTDC_HBP_WIDTH + LTDC_DISPLAY_WIDTH + LTDC_HFP_WIDTH - 1);
    hltdc.Init.TotalHeigh = (LTDC_VSYNC_HEIGHT + LTDC_VBP_HEIGHT + LTDC_DISPLAY_HEIGHT + LTDC_VFP_HEIGHT - 1);

    /* Background color */
    hltdc.Init.Backcolor.Blue = 0;
    hltdc.Init.Backcolor.Green = 0;
    hltdc.Init.Backcolor.Red = 0;

    /* Initialize LTDC peripheral */
    if (HAL_LTDC_Init(&hltdc) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Configure Layer 1 (background layer) for LVGL */
    LTDC_LayerCfgTypeDef layerCfg = {0};
    layerCfg.WindowX0 = 0;
    /* HAL expects inclusive end positions: use width - 1 */
    layerCfg.WindowX1 = LTDC_DISPLAY_WIDTH - 1;
    layerCfg.WindowY0 = 0;
    layerCfg.WindowY1 = LTDC_DISPLAY_HEIGHT - 1;
    layerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    layerCfg.Alpha = 255;
    layerCfg.Alpha0 = 0;
    layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
    layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
    layerCfg.FBStartAdress = 0xD0000000; /* SDRAM Bank 2 framebuffer address */
    layerCfg.ImageWidth = LTDC_DISPLAY_WIDTH;
    layerCfg.ImageHeight = LTDC_DISPLAY_HEIGHT;
    layerCfg.Backcolor.Blue = 0;
    layerCfg.Backcolor.Green = 0;
    layerCfg.Backcolor.Red = 0;

    if (HAL_LTDC_ConfigLayer(&hltdc, &layerCfg, 0) != HAL_OK) {
        return HAL_ERROR;
    }

    /* CRITICAL FIX: Enable Layer 1 - without this, the LCD stays black! */
    __HAL_LTDC_LAYER_ENABLE(&hltdc, 0);
    //__HAL_LTDC_RELOAD_IMMEDIATE_CONFIG(&hltdc);
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);  /* Reload at VSYNC, not immediately */

    return HAL_OK;
}

/**
 * @brief Initialize LTDC MSP (wrapper for HAL_LTDC_MspInit)
 * @param hltdc: Pointer to LTDC handle
 */
void LTDC_MspInit(LTDC_HandleTypeDef *hltdc) {
    HAL_LTDC_MspInit(hltdc);
}

/**
 * @brief De-initialize LTDC MSP (wrapper for HAL_LTDC_MspDeInit)
 * @param hltdc: Pointer to LTDC handle
 */
void LTDC_MspDeInit(LTDC_HandleTypeDef *hltdc) {
    HAL_LTDC_MspDeInit(hltdc);
}

/* Private Functions ---------------------------------------------------------*/

/**
 * @brief Validate driver handle
 * @param driver: Pointer to LTDC driver structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef LTDC_ValidateDriver(LTDC_Driver_t *driver) {
    if (driver == NULL || !driver->initialized || driver->hltdc == NULL) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @brief Validate layer number
 * @param layer: Layer number
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef LTDC_ValidateLayer(uint8_t layer) {
    if (layer >= LTDC_MAX_LAYERS) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @brief Validate coordinates
 * @param x: X coordinate
 * @param y: Y coordinate
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef LTDC_ValidateCoordinates(uint16_t x, uint16_t y) {
    if (x >= LTDC_DISPLAY_WIDTH || y >= LTDC_DISPLAY_HEIGHT) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @brief Validate rectangle
 * @param rect: Pointer to rectangle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef LTDC_ValidateRect(LTDC_Rect_t *rect) {
    if (rect == NULL ||
        rect->x >= LTDC_DISPLAY_WIDTH ||
        rect->y >= LTDC_DISPLAY_HEIGHT ||
        (rect->x + rect->width) > LTDC_DISPLAY_WIDTH ||
        (rect->y + rect->height) > LTDC_DISPLAY_HEIGHT) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

/**
 * @brief Get HAL pixel format from driver format
 * @param format: Driver pixel format
 * @return uint32_t: HAL pixel format
 */
static uint32_t LTDC_GetPixelFormatHAL(LTDC_PixelFormat_t format) {
    switch (format) {
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM: return LTDC_PIXEL_FORMAT_ARGB8888;
        case LTDC_PIXEL_FORMAT_RGB888_ENUM: return LTDC_PIXEL_FORMAT_RGB888;
        case LTDC_PIXEL_FORMAT_RGB565_ENUM: return LTDC_PIXEL_FORMAT_RGB565;
        case LTDC_PIXEL_FORMAT_ARGB1555_ENUM: return LTDC_PIXEL_FORMAT_ARGB1555;
        case LTDC_PIXEL_FORMAT_ARGB4444_ENUM: return LTDC_PIXEL_FORMAT_ARGB4444;
        case LTDC_PIXEL_FORMAT_L8_ENUM: return LTDC_PIXEL_FORMAT_L8;
        case LTDC_PIXEL_FORMAT_AL44_ENUM: return LTDC_PIXEL_FORMAT_AL44;
        case LTDC_PIXEL_FORMAT_AL88_ENUM: return LTDC_PIXEL_FORMAT_AL88;
        default: return LTDC_PIXEL_FORMAT_ARGB8888;
    }
}

/**
 * @brief Get driver pixel format from HAL format
 * @param halFormat: HAL pixel format
 * @return LTDC_PixelFormat_t: Driver pixel format
 */
static LTDC_PixelFormat_t LTDC_GetPixelFormatDriver(uint32_t halFormat) {
    switch (halFormat) {
        case LTDC_PIXEL_FORMAT_ARGB8888: return LTDC_PIXEL_FORMAT_ARGB8888_ENUM;
        case LTDC_PIXEL_FORMAT_RGB888: return LTDC_PIXEL_FORMAT_RGB888_ENUM;
        case LTDC_PIXEL_FORMAT_RGB565: return LTDC_PIXEL_FORMAT_RGB565_ENUM;
        case LTDC_PIXEL_FORMAT_ARGB1555: return LTDC_PIXEL_FORMAT_ARGB1555_ENUM;
        case LTDC_PIXEL_FORMAT_ARGB4444: return LTDC_PIXEL_FORMAT_ARGB4444_ENUM;
        case LTDC_PIXEL_FORMAT_L8: return LTDC_PIXEL_FORMAT_L8_ENUM;
        case LTDC_PIXEL_FORMAT_AL44: return LTDC_PIXEL_FORMAT_AL44_ENUM;
        case LTDC_PIXEL_FORMAT_AL88: return LTDC_PIXEL_FORMAT_AL88_ENUM;
        default: return LTDC_PIXEL_FORMAT_ARGB8888_ENUM;
    }
}

/**
 * @brief Set pixel in framebuffer
 * @param framebuffer: Framebuffer pointer
 * @param x: X coordinate
 * @param y: Y coordinate
 * @param color: Pixel color
 * @param format: Pixel format
 */
static void LTDC_SetPixel(uint32_t *framebuffer, uint16_t x, uint16_t y, uint32_t color, LTDC_PixelFormat_t format) {
    uint32_t index = y * LTDC_DISPLAY_WIDTH + x;

    switch (format) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            {
                uint16_t *fb16 = (uint16_t *)framebuffer;
                fb16[index] = (uint16_t)color;
            }
            break;

        case LTDC_PIXEL_FORMAT_RGB888_ENUM:
            {
                uint8_t *fb8 = (uint8_t *)framebuffer;
                fb8[index * 3] = (color) & 0xFF;        /* Blue */
                fb8[index * 3 + 1] = (color >> 8) & 0xFF;  /* Green */
                fb8[index * 3 + 2] = (color >> 16) & 0xFF; /* Red */
            }
            break;

        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
        default:
            framebuffer[index] = color;
            break;
    }
}

/**
 * @brief Get pixel from framebuffer
 * @param framebuffer: Framebuffer pointer
 * @param x: X coordinate
 * @param y: Y coordinate
 * @param format: Pixel format
 * @return uint32_t: Pixel color
 */
static uint32_t LTDC_GetPixel(uint32_t *framebuffer, uint16_t x, uint16_t y, LTDC_PixelFormat_t format) {
    uint32_t index = y * LTDC_DISPLAY_WIDTH + x;

    switch (format) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM:
            {
                uint16_t *fb16 = (uint16_t *)framebuffer;
                return fb16[index];
            }

        case LTDC_PIXEL_FORMAT_RGB888_ENUM:
            {
                uint8_t *fb8 = (uint8_t *)framebuffer;
                return (fb8[index * 3 + 2] << 16) | (fb8[index * 3 + 1] << 8) | fb8[index * 3];
            }

        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
        default:
            return framebuffer[index];
    }
}

/**
 * @brief Draw horizontal line
 * @param driver: Pointer to LTDC driver structure
 * @param x: Start X coordinate
 * @param y: Y coordinate
 * @param length: Line length
 * @param color: Line color
 */
static void LTDC_DrawHorizontalLine(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint16_t length, uint32_t color) {
    for (uint16_t i = 0; i < length; i++) {
        if ((x + i) < LTDC_DISPLAY_WIDTH) {
            LTDC_DrawPixel(driver, x + i, y, color);
        }
    }
}

/**
 * @brief Draw vertical line
 * @param driver: Pointer to LTDC driver structure
 * @param x: X coordinate
 * @param y: Start Y coordinate
 * @param length: Line length
 * @param color: Line color
 */
static void LTDC_DrawVerticalLine(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint16_t length, uint32_t color) {
    for (uint16_t i = 0; i < length; i++) {
        if ((y + i) < LTDC_DISPLAY_HEIGHT) {
            LTDC_DrawPixel(driver, x, y + i, color);
        }
    }
}
