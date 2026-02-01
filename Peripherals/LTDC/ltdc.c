
/**
 * @file ltdc.c
 * @brief Minimal LTDC (RGB interface) driver for STM32F429I Discovery
 * @details Pure LTDC (RGB) logic: display init, layer management, framebuffer, minimal pixel drawing. No SPI/ILI9341 or non-LTDC code.
 */

/* Includes ------------------------------------------------------------------*/
#include "ltdc.h"
#include <string.h>
#include "stm32f4xx_hal.h"
#include "log.h"



/* Private defines -----------------------------------------------------------*/
#define LTDC_TIMEOUT                5000    /*!< Timeout for LTDC operations */
#define LTDC_MIN_ALPHA              0       /*!< Minimum alpha value */
#define LTDC_MAX_ALPHA              255     /*!< Maximum alpha value */
#define LTDC_DEFAULT_BRIGHTNESS     100     /*!< Default brightness percentage */


LTDC_HandleTypeDef hltdc;                   /*!< LTDC HAL handle */

static volatile uint16_t *fb_ptr = (volatile uint16_t *)(SDRAM_DEVICE_ADDR); /*!< Framebuffer pointer in SDRAM */


/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef LTDC_ValidateDriver(LTDC_Driver_t *driver);
static HAL_StatusTypeDef LTDC_ValidateLayer(uint8_t layer);
static HAL_StatusTypeDef LTDC_ValidateCoordinates(uint16_t xCoord, uint16_t yCoord);
static HAL_StatusTypeDef LTDC_ValidateRect(LTDC_Rect_t *rect);
static uint32_t LTDC_GetPixelFormatHAL(LTDC_PixelFormat_t format);

/* Global instance used for forwarding HAL callbacks to driver-level state */
static LTDC_Driver_t *g_ltdc_driver = NULL;

/* Minimal helper: set a pixel by index in framebuffer (row-major) */
static void LTDC_SetPixelByIndex(uint8_t *fbBase, uint32_t index, uint32_t color, LTDC_PixelFormat_t format);


/* Public Functions ----------------------------------------------------------*/

/**
 * @brief Initialize LTDC driver
 * @param driver: Pointer to LTDC driver structure
 * @param hltdc: Pointer to HAL LTDC handle
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_Driver_Init(LTDC_Driver_t *driver, LTDC_HandleTypeDef *hltdc_handle) {
    log_debug("LTDC: Initializing driver");
    if (driver == NULL || hltdc_handle == NULL) {
        log_error("LTDC: Invalid parameters for init");
        return HAL_ERROR;
    }

    /* Initialize driver structure */
    memset(driver, 0, sizeof(LTDC_Driver_t));
    driver->hltdc = hltdc_handle;
    driver->errorCode = LTDC_ERROR_NONE;
    driver->reloadFlag = 0;

    /* Register global instance for HAL callbacks (single-LTDC system assumption) */
    g_ltdc_driver = driver;

    /* Set default display configuration */
    driver->displayConfig.width = LTDC_DISPLAY_WIDTH;
    driver->displayConfig.height = LTDC_DISPLAY_HEIGHT;
    driver->displayConfig.backgroundColor = LTDC_COLOR_BLACK;
    driver->displayConfig.hsyncActiveLow = true;
    driver->displayConfig.vsyncActiveLow = true;
    /* Panel uses DE active LOW according to CubeMX / panel datasheet */
    driver->displayConfig.dataEnableActiveLow = true;
    /* Pixel clock: normal input (not inverted) */
    driver->displayConfig.pixelClockInverted = false;

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

    log_debug("LTDC: Driver initialized successfully");
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

    /* Clear global instance */
    if (g_ltdc_driver == driver) {
        g_ltdc_driver = NULL;
    }

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
        log_error("LTDC: Invalid driver or config for display setup");
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }
    log_debug("LTDC: Configuring display %dx%d", config->width, config->height);

    /* Configure LTDC timing parameters */
    driver->hltdc->Instance = LTDC;

    /* Set signal polarities from provided config (must match panel datasheet) */
    driver->hltdc->Init.HSPolarity = config->hsyncActiveLow ? LTDC_HSPOLARITY_AL : LTDC_HSPOLARITY_AH;
    driver->hltdc->Init.VSPolarity = config->vsyncActiveLow ? LTDC_VSPOLARITY_AL : LTDC_VSPOLARITY_AH;
    driver->hltdc->Init.DEPolarity  = config->dataEnableActiveLow ? LTDC_DEPOLARITY_AL : LTDC_DEPOLARITY_AH;
    driver->hltdc->Init.PCPolarity  = config->pixelClockInverted ? LTDC_PCPOLARITY_IIPC : LTDC_PCPOLARITY_IPC;

    // Set timing parameters for STM32F429I-DISC1 onboard LCD (ILI9341)
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

    /* Initialize LTDC only if not already initialized to avoid double-initialization */
    HAL_StatusTypeDef status = HAL_OK;
    if (driver->hltdc->State == HAL_LTDC_STATE_RESET) {
        status = HAL_LTDC_Init(driver->hltdc);
        if (status != HAL_OK) {
            driver->errorCode = LTDC_ERROR_INIT_FAILED;
            return status;
        }
    }

    /* Store configuration */
    driver->displayConfig = *config;

    log_debug("LTDC: Display configured successfully");
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
    log_debug("LTDC: Configuring layer %d", layer);
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK || config == NULL) {
        log_error("LTDC: Invalid parameters for layer %d config", layer);
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Note: RGB888 (24-bit packed) is discouraged on STM32 LTDC because DMA and
     * LTDC fetches are optimized for 32-bit aligned bursts. Packed 3-bytes-per-pixel
     * layouts can cause misaligned DMA transfers, performance penalties, or require
     * padding/stride. Prefer RGB565 (16-bit) or ARGB8888 (32-bit) for best performance.
     * Explicitly reject RGB888 here to avoid subtle bugs — applications should use
     * ARGB8888 if 24-bit color is required.
     */
    if (config->pixelFormat == LTDC_PIXEL_FORMAT_RGB888_ENUM) {
        driver->errorCode = LTDC_ERROR_UNSUPPORTED_FORMAT;
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

    log_debug("LTDC: Layer %d configured successfully", layer);
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

    /* Request a reload at next VSync so the change is applied without tearing */
    HAL_StatusTypeDef status = LTDC_RequestReload(driver, LTDC_SRCR_VBR);
    if (status != HAL_OK) {
        driver->errorCode = LTDC_ERROR_LAYER_CONFIG;
        return status;
    }

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
    __HAL_LTDC_RELOAD_CONFIG(driver->hltdc);

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

    /* Calculate width/height (inclusive coordinates stored in windowX1/windowY1) */
    uint16_t width = (driver->layers[layer].windowX1 - driver->layers[layer].windowX0) + 1;
    uint16_t height = (driver->layers[layer].windowY1 - driver->layers[layer].windowY0) + 1;

    /* Use HAL helper to set window position without immediate reload and then force a reload */
    HAL_StatusTypeDef status = HAL_LTDC_SetWindowPosition_NoReload(driver->hltdc, x, y, layer);
    if (status != HAL_OK) {
        driver->errorCode = LTDC_ERROR_LAYER_CONFIG;
        return status;
    }

    /* Update the cached geometry (applied after reload) */
    driver->layers[layer].windowX0 = x;
    driver->layers[layer].windowY0 = y;
    driver->layers[layer].windowX1 = x + width - 1;
    driver->layers[layer].windowY1 = y + height - 1;

    /* Request reload at next VSync */
    LTDC_RequestReload(driver, LTDC_SRCR_VBR);

    return HAL_OK;
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

    /* Build layer config and apply */
    LTDC_LayerCfgTypeDef layerCfg = {0};
    layerCfg.WindowX0 = window->x;
    layerCfg.WindowX1 = window->x + window->width - 1; /* inclusive */
    layerCfg.WindowY0 = window->y;
    layerCfg.WindowY1 = window->y + window->height - 1; /* inclusive */
    layerCfg.PixelFormat = LTDC_GetPixelFormatHAL(driver->layers[layer].pixelFormat);
    layerCfg.Alpha = driver->layers[layer].alpha;
    layerCfg.Alpha0 = driver->layers[layer].alpha0;

    switch (driver->layers[layer].blendMode) {
        case LTDC_BLEND_CONSTANT_ALPHA:
            layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
            layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
            break;
        case LTDC_BLEND_PIXEL_ALPHA:
            layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
            layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
            break;
        default:
            layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
            layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
            break;
    }

    layerCfg.FBStartAdress = driver->layers[layer].framebufferAddress;
    layerCfg.ImageWidth = window->width;
    layerCfg.ImageHeight = window->height;

    layerCfg.Backcolor.Blue = (driver->layers[layer].backgroundColor) & 0xFF;
    layerCfg.Backcolor.Green = (driver->layers[layer].backgroundColor >> 8) & 0xFF;
    layerCfg.Backcolor.Red = (driver->layers[layer].backgroundColor >> 16) & 0xFF;

    HAL_StatusTypeDef status = HAL_LTDC_ConfigLayer(driver->hltdc, &layerCfg, layer);
    if (status == HAL_OK) {
        driver->layers[layer].windowX0 = layerCfg.WindowX0;
        driver->layers[layer].windowY0 = layerCfg.WindowY0;
        driver->layers[layer].windowX1 = layerCfg.WindowX1;
        driver->layers[layer].windowY1 = layerCfg.WindowY1;
        driver->layers[layer].imageWidth = layerCfg.ImageWidth;
        driver->layers[layer].imageHeight = layerCfg.ImageHeight;
    } else {
        driver->errorCode = LTDC_ERROR_LAYER_CONFIG;
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

    /* Set framebuffer address (HAL will stage the change) */
    HAL_StatusTypeDef status = HAL_LTDC_SetAddress(driver->hltdc, address, layer);
    if (status == HAL_OK) {
        /* Request register reload during next vertical blanking to avoid tearing */
        LTDC_RequestReload(driver, LTDC_SRCR_VBR);

        /* Wait for reload flag (using driver-level reload flag) */
        if (LTDC_WaitForReload(driver, LTDC_TIMEOUT) != HAL_OK) {
            driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
            return HAL_TIMEOUT;
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

    /* After clearing the backbuffer, request reload so the displayed buffer updates at VSYNC */
    HAL_StatusTypeDef status = HAL_OK;
    /* Perform clearing */

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

    /* Request reload to apply buffer changes if using swap at VSYNC */
    LTDC_RequestReload(driver, LTDC_SRCR_VBR);

    return status;
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
    LTDC_LayerConfig_t *l = &driver->layers[layer];
    if (l->framebufferAddress == 0) {
        driver->errorCode = LTDC_ERROR_FRAMEBUFFER;
        return HAL_ERROR;
    }

    /* Ensure pixel is inside layer window */
    if (x < l->windowX0 || x > l->windowX1 || y < l->windowY0 || y > l->windowY1) {
        driver->errorCode = LTDC_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    /* Compute index within the image buffer (row-major) */
    uint32_t ix = x - l->windowX0;
    uint32_t iy = y - l->windowY0;
    uint32_t index = iy * l->imageWidth + ix;

    uint8_t *fbBase = (uint8_t *)(uintptr_t)l->framebufferAddress;
    LTDC_SetPixelByIndex(fbBase, index, color, l->pixelFormat);

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

    uint32_t blue = (color) & 0xFF;
    uint32_t green = (color >> 8) & 0xFF;
    uint32_t red = (color >> 16) & 0xFF;

    /* Write directly to LTDC background color register (BCCR) */
    driver->hltdc->Instance->BCCR = (red << 16) | (green << 8) | blue;
    __HAL_LTDC_RELOAD_CONFIG(driver->hltdc);

    driver->displayConfig.backgroundColor = color;
    return HAL_OK;
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

/**
 * @brief Set display brightness (platform dependent; default stub)
 * @param driver: Pointer to LTDC driver structure
 * @param brightness: Brightness percent (0-100)
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_SetDisplayBrightness(LTDC_Driver_t *driver, uint8_t brightness) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) {
        return HAL_ERROR;
    }

    if (brightness > 100) {
        return HAL_ERROR;
    }

    /* Platform-specific: hook up to PWM/backlight if available. For now, store nothing and return OK. */
    (void)driver;
    (void)brightness;

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
    uint32_t rgb888 = 0U; /* initialize to silence static analysis */

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
 * @brief  Request a register reload (e.g., LTDC_SRCR_VBR) to apply staged changes at VSYNC
 */
HAL_StatusTypeDef LTDC_RequestReload(LTDC_Driver_t *driver, uint32_t reload) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) return HAL_ERROR;

    /* Clear previous flag and request reload */
    driver->reloadFlag = 0;
    HAL_LTDC_Reload(driver->hltdc, reload);
    return HAL_OK;
}

/**
 * @brief Wait for reload event (VSYNC) - interrupt-driven
 * This implementation uses ARM WFE/SEV to wait efficiently for the HAL reload
 * event callback to set the driver's reloadFlag and wake this task.
 */
HAL_StatusTypeDef LTDC_WaitForReload(LTDC_Driver_t *driver, uint32_t timeout_ms) {
    if (LTDC_ValidateDriver(driver) != HAL_OK) return HAL_ERROR;
    uint32_t start = HAL_GetTick();
    /* Wait efficiently using WFE (wait for event). The HAL_LTDC_ReloadEventCallback
       will set reloadFlag and call __SEV() to wake up this waiter. */
    while (driver->reloadFlag == 0) {
        if (HAL_GetTick() - start > timeout_ms) return HAL_TIMEOUT;
        __WFE(); /* low-power wait until an event occurs */
    }
    driver->reloadFlag = 0;
    return HAL_OK;
}

/**
 * @brief Set window position without triggering a reload immediately
 */
HAL_StatusTypeDef LTDC_SetWindowPosition_NoReload(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint8_t layer) {
    if (LTDC_ValidateDriver(driver) != HAL_OK || LTDC_ValidateLayer(layer) != HAL_OK) return HAL_ERROR;
    if (x >= LTDC_DISPLAY_WIDTH || y >= LTDC_DISPLAY_HEIGHT) return HAL_ERROR;

    /* Use HAL's NoReload API and update cached window origin to be applied after reload */
    HAL_StatusTypeDef status = HAL_LTDC_SetWindowPosition_NoReload(driver->hltdc, x, y, layer);
    if (status == HAL_OK) {
        /* Update cache - x/y origin; end coordinates kept consistent with width/height */
        uint16_t width = (driver->layers[layer].windowX1 - driver->layers[layer].windowX0) + 1;
        uint16_t height = (driver->layers[layer].windowY1 - driver->layers[layer].windowY0) + 1;
        driver->layers[layer].windowX0 = x;
        driver->layers[layer].windowY0 = y;
        driver->layers[layer].windowX1 = x + width - 1;
        driver->layers[layer].windowY1 = y + height - 1;
    }
    return status;
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



/**
 * @brief Hardware initialization
 * @return HAL_StatusTypeDef: HAL status
 */
HAL_StatusTypeDef LTDC_HW_Init(void) {

    /* Initialize LTDC Peripheral (RGB interface only) */

    /* LTDC clocks and GPIOs are configured in HAL_LTDC_MspInit() to centralize board init.
     * Do not enable the LTDC clock here to avoid duplicate clock setup. */
    /* __HAL_RCC_LTDC_CLK_ENABLE(); intentionally omitted */

    /* Configure LTDC peripheral */
    hltdc.Instance = LTDC;

    /* Signal polarities - MUST MATCH ILI9341 RGB interface settings! */
    hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;   // HSYNC active low
    hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;   // VSYNC active low
    hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;   // DE active high (ILI9341 RGB)
    hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;  // Pixel clock not inverted (input pixel clock)


    /* Timing parameters for STM32F429I-DISC1 LCD (RGB interface)
     * Matches ST BSP values: HSYNC=10, HBP=20, HFP=10, VSYNC=2, VBP=2, VFP=4, active=240x320.
     */
    hltdc.Init.HorizontalSync = 9;   // HSYNC width - 1 (10)
    hltdc.Init.VerticalSync = 1;     // VSYNC height - 1 (2)
    hltdc.Init.AccumulatedHBP = 29;  // HSYNC + HBP - 1 (10+20-1)
    hltdc.Init.AccumulatedVBP = 3;   // VSYNC + VBP - 1 (2+2-1)
    hltdc.Init.AccumulatedActiveW = 269;  // HSYNC + HBP + Width - 1 (10+20+240-1)
    hltdc.Init.AccumulatedActiveH = 323;  // VSYNC + VBP + Height - 1 (2+2+320-1)
    hltdc.Init.TotalWidth = 279;     // Total horizontal - 1 (add HFP=10)
    hltdc.Init.TotalHeigh = 327;     // Total vertical - 1 (add VFP=4)

    /* Background color (black) */
    hltdc.Init.Backcolor.Blue = 0;
    hltdc.Init.Backcolor.Green = 0;
    hltdc.Init.Backcolor.Red = 0;

    /* Initialize LTDC */
    if (HAL_LTDC_Init(&hltdc) != HAL_OK) {
        log_debug("LTDC: LTDC init failed");  // TODO: Set error code (LTDC)
        return HAL_ERROR;
    }

    memset((void *)fb_ptr, (unsigned char)0x1234, LTDC_FB_SIZE_RGB565);

    /* Configure Layer 1 */
    LTDC_LayerCfgTypeDef layerCfg = {0};
    layerCfg.WindowX0 = 0;
    layerCfg.WindowX1 = 240;  // 240 pixels wide (inclusive)
    layerCfg.WindowY0 = 0;
    layerCfg.WindowY1 = 320;  // 320 pixels tall (inclusive)
    layerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
    layerCfg.Alpha = 255;
    layerCfg.Alpha0 = 0;
    layerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    layerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    layerCfg.FBStartAdress = (uint32_t)fb_ptr;  // SDRAM framebuffer
    layerCfg.ImageWidth = LTDC_DISPLAY_WIDTH;
    layerCfg.ImageHeight = LTDC_DISPLAY_HEIGHT;
    layerCfg.Backcolor.Blue = 0;
    layerCfg.Backcolor.Green = 0;
    layerCfg.Backcolor.Red = 0;

    if (HAL_LTDC_ConfigLayer(&hltdc, &layerCfg, 0) != HAL_OK) {
        log_debug("LTDC Layer config error\n");

        return HAL_ERROR;
    }

    log_debug("LTDC: Driver initialized successfully");
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

/* HAL reload event callback - forward to driver instance if available */
void HAL_LTDC_ReloadEventCallback(LTDC_HandleTypeDef *hltdc) {
    (void)hltdc;
    if (g_ltdc_driver != NULL) {
        g_ltdc_driver->reloadFlag = 1;
        /* Wake any waiter blocked in __WFE() */
        __SEV();
    }
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


/* New helper: set pixel by linear index (row-major) where framebuffer base is a byte pointer */
/* NOTE: RGB888 (3 bytes per pixel) is discouraged due to 32-bit DMA/LTDC fetch alignment.
 * Use RGB565 or ARGB8888 for best performance and correct DMA behavior. RGB888 support
 * is intentionally rejected at layer configuration time, but this helper keeps the
 * conversion logic for completeness.
 */
static void LTDC_SetPixelByIndex(uint8_t *fbBase, uint32_t index, uint32_t color, LTDC_PixelFormat_t format) {
    switch (format) {
        case LTDC_PIXEL_FORMAT_RGB565_ENUM: {
            uint16_t *fb16 = (uint16_t *)fbBase;
            fb16[index] = (uint16_t)color;
            break;
        }
        case LTDC_PIXEL_FORMAT_RGB888_ENUM: {
            /* Packed 24-bit; kept for completeness but discouraged. */
            uint8_t *p = fbBase + index * 3;
            p[0] = (color) & 0xFF;        /* Blue */
            p[1] = (color >> 8) & 0xFF;  /* Green */
            p[2] = (color >> 16) & 0xFF; /* Red */
            break;
        }
        case LTDC_PIXEL_FORMAT_ARGB8888_ENUM:
        default: {
            uint32_t *fb32 = (uint32_t *)fbBase;
            fb32[index] = color;
            break;
        }
    }
}

