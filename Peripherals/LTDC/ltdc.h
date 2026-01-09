/**
 * @file ltdc.h
 * @brief Minimal LTDC (RGB interface) driver for STM32F429 Discovery board
 * @details Pure LTDC (RGB) logic: display init, layer management, framebuffer, minimal pixel drawing. No SPI/ILI9341 or non-LTDC code.
 */

#ifndef LTDC_H
#define LTDC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>
#include "fmc.h"

/* Display specifications for STM32F429I-DISC1 ------------------------------*/
#define LTDC_DISPLAY_WIDTH          240     /*!< Display width in pixels */
#define LTDC_DISPLAY_HEIGHT         320     /*!< Display height in pixels */
#define LTDC_MAX_LAYERS             2       /*!< Maximum number of display layers */

#if 0 /* (Legacy) ILI9341 timing parameters - not used */
#endif
#define LTDC_HSYNC_WIDTH            9        /*!< Horizontal sync width (from doc) */
#define LTDC_VSYNC_HEIGHT           1        /*!< Vertical sync height (from doc) */
#define LTDC_HBP_WIDTH              29       /*!< Horizontal back porch (from doc) */
#define LTDC_VBP_HEIGHT             3        /*!< Vertical back porch (from doc) */
#define LTDC_HFP_WIDTH              2        /*!< Horizontal front porch (from doc) */
#define LTDC_VFP_HEIGHT             2        /*!< Vertical front porch (from doc) */

/* Memory allocation constants -----------------------------------------------*/
#define LTDC_BYTES_PER_PIXEL_RGB565 2       /*!< Bytes per pixel for RGB565 format */
#define LTDC_BYTES_PER_PIXEL_RGB888 3       /*!< Bytes per pixel for RGB888 format */
#define LTDC_BYTES_PER_PIXEL_ARGB8888 4     /*!< Bytes per pixel for ARGB8888 format */

/* Framebuffer size calculations */
#define LTDC_FB_SIZE_RGB565         (LTDC_DISPLAY_WIDTH * LTDC_DISPLAY_HEIGHT * LTDC_BYTES_PER_PIXEL_RGB565)
#define LTDC_FB_SIZE_RGB888         (LTDC_DISPLAY_WIDTH * LTDC_DISPLAY_HEIGHT * LTDC_BYTES_PER_PIXEL_RGB888)
#define LTDC_FB_SIZE_ARGB8888       (LTDC_DISPLAY_WIDTH * LTDC_DISPLAY_HEIGHT * LTDC_BYTES_PER_PIXEL_ARGB8888)

  /* second buffer for double buffering */

/* Default colors in RGB565 format ------------------------------------------*/
#define LTDC_COLOR_BLACK            0x0000  /*!< Black color */
#define LTDC_COLOR_WHITE            0xFFFF  /*!< White color */
#define LTDC_COLOR_RED              0xF800  /*!< Red color */
#define LTDC_COLOR_GREEN            0x07E0  /*!< Green color */
#define LTDC_COLOR_BLUE             0x001F  /*!< Blue color */
#define LTDC_COLOR_YELLOW           0xFFE0  /*!< Yellow color */
#define LTDC_COLOR_CYAN             0x07FF  /*!< Cyan color */
#define LTDC_COLOR_MAGENTA          0xF81F  /*!< Magenta color */
#define LTDC_COLOR_GRAY             0x7BEF  /*!< Gray color */
#define LTDC_COLOR_DARKGRAY         0x39E7  /*!< Dark gray color */
#define LTDC_COLOR_LIGHTGRAY        0xBDF7  /*!< Light gray color */

/* Error codes ---------------------------------------------------------------*/
#define LTDC_ERROR_NONE                  0x00    /*!< No error */
#define LTDC_ERROR_INVALID_PARAM         0x01    /*!< Invalid parameter */
#define LTDC_ERROR_INIT_FAILED           0x02    /*!< Initialization failed */
#define LTDC_ERROR_LAYER_CONFIG          0x03    /*!< Layer configuration failed */
#define LTDC_ERROR_MEMORY_ALLOC          0x04    /*!< Memory allocation failed */
#define LTDC_ERROR_INVALID_LAYER         0x05    /*!< Invalid layer number */
#define LTDC_ERROR_FRAMEBUFFER           0x06    /*!< Framebuffer error */
#define LTDC_ERROR_UNSUPPORTED_FORMAT    0x07    /*!< Unsupported pixel format (avoid RGB888) */


/* Data Types ----------------------------------------------------------------*/

/**
 * @brief LTDC pixel format enumeration
 */
typedef enum {
    LTDC_PIXEL_FORMAT_ARGB8888_ENUM = 0,    /*!< 32-bit ARGB8888 format */
    LTDC_PIXEL_FORMAT_RGB888_ENUM,          /*!< 24-bit RGB888 format */
    LTDC_PIXEL_FORMAT_RGB565_ENUM,          /*!< 16-bit RGB565 format */
    LTDC_PIXEL_FORMAT_ARGB1555_ENUM,        /*!< 16-bit ARGB1555 format */
    LTDC_PIXEL_FORMAT_ARGB4444_ENUM,        /*!< 16-bit ARGB4444 format */
    LTDC_PIXEL_FORMAT_L8_ENUM,              /*!< 8-bit luminance format */
    LTDC_PIXEL_FORMAT_AL44_ENUM,            /*!< 8-bit alpha-luminance format */
    LTDC_PIXEL_FORMAT_AL88_ENUM             /*!< 16-bit alpha-luminance format */
} LTDC_PixelFormat_t;

/**
 * @brief LTDC blending mode enumeration
 */
typedef enum {
    LTDC_BLEND_CONSTANT_ALPHA = 0,          /*!< Constant alpha blending */
    LTDC_BLEND_PIXEL_ALPHA,                 /*!< Pixel alpha blending */
    LTDC_BLEND_NO_BLENDING                  /*!< No blending (opaque) */
} LTDC_BlendMode_t;

/**
 * @brief LTDC layer configuration structure
 */
typedef struct {
    uint32_t framebufferAddress;            /*!< Framebuffer start address */
    uint16_t windowX0;                      /*!< Window left position */
    uint16_t windowY0;                      /*!< Window top position */
    uint16_t windowX1;                      /*!< Window right position */
    uint16_t windowY1;                      /*!< Window bottom position */
    uint16_t imageWidth;                    /*!< Image width in pixels */
    uint16_t imageHeight;                   /*!< Image height in pixels */
    LTDC_PixelFormat_t pixelFormat;         /*!< Pixel format */
    uint8_t alpha;                          /*!< Layer alpha value (0-255) */
    uint8_t alpha0;                         /*!< Transparent pixel alpha */
    LTDC_BlendMode_t blendMode;             /*!< Blending mode */
    uint32_t backgroundColor;               /*!< Layer background color */
    bool enabled;                           /*!< Layer enable status */
} LTDC_LayerConfig_t;

/**
 * @brief LTDC display configuration structure
 */
typedef struct {
    uint16_t width;                         /*!< Display width */
    uint16_t height;                        /*!< Display height */
    uint32_t backgroundColor;               /*!< Display background color */
    bool hsyncActiveLow;                    /*!< Horizontal sync polarity */
    bool vsyncActiveLow;                    /*!< Vertical sync polarity */
    bool dataEnableActiveLow;               /*!< Data enable polarity */
    bool pixelClockInverted;                /*!< Pixel clock polarity */
} LTDC_DisplayConfig_t;

/**
 * @brief LTDC driver handle structure
 */
typedef struct {
    LTDC_HandleTypeDef *hltdc;              /*!< HAL LTDC handle */
    LTDC_DisplayConfig_t displayConfig;     /*!< Display configuration */
    LTDC_LayerConfig_t layers[LTDC_MAX_LAYERS]; /*!< Layer configurations */
    uint8_t activeLayer;                    /*!< Currently active layer */
    bool initialized;                       /*!< Initialization status */
    uint32_t errorCode;                     /*!< Last error code */
    volatile uint8_t reloadFlag;           /*!< Set when a reload event occurs (VSYNC) */
} LTDC_Driver_t;



/**
 * @brief Rectangle structure
 */
typedef struct {
    uint16_t x;                             /*!< X coordinate */
    uint16_t y;                             /*!< Y coordinate */
    uint16_t width;                         /*!< Rectangle width */
    uint16_t height;                        /*!< Rectangle height */
} LTDC_Rect_t;

/**
 * @brief Point structure
 */
typedef struct {
    uint16_t x;                             /*!< X coordinate */
    uint16_t y;                             /*!< Y coordinate */
} LTDC_Point_t;

/* Function prototypes -------------------------------------------------------*/

/* Initialization and configuration functions */
HAL_StatusTypeDef LTDC_Driver_Init(LTDC_Driver_t *driver, LTDC_HandleTypeDef *hltdc);
HAL_StatusTypeDef LTDC_Driver_DeInit(LTDC_Driver_t *driver);
HAL_StatusTypeDef LTDC_ConfigureDisplay(LTDC_Driver_t *driver, LTDC_DisplayConfig_t *config);
HAL_StatusTypeDef LTDC_ConfigureLayer(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *config);

/* Layer management functions */
HAL_StatusTypeDef LTDC_EnableLayer(LTDC_Driver_t *driver, uint8_t layer);
HAL_StatusTypeDef LTDC_DisableLayer(LTDC_Driver_t *driver, uint8_t layer);
HAL_StatusTypeDef LTDC_SetActiveLayer(LTDC_Driver_t *driver, uint8_t layer);
HAL_StatusTypeDef LTDC_SetLayerAlpha(LTDC_Driver_t *driver, uint8_t layer, uint8_t alpha);
HAL_StatusTypeDef LTDC_SetLayerPosition(LTDC_Driver_t *driver, uint8_t layer, uint16_t x, uint16_t y);
HAL_StatusTypeDef LTDC_SetLayerWindow(LTDC_Driver_t *driver, uint8_t layer, LTDC_Rect_t *window);

/* Non-blocking window position update: change window position without triggering reload
   The application should call LTDC_RequestReload() to apply changes at next VSYNC. */
HAL_StatusTypeDef LTDC_SetWindowPosition_NoReload(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint8_t layer);

/* Request hardware register reload (example: LTDC_SRCR_VBR) */
HAL_StatusTypeDef LTDC_RequestReload(LTDC_Driver_t *driver, uint32_t reload);

/* Wait for a reload event (VSYNC) with timeout in ms. Returns HAL_OK on event, HAL_TIMEOUT on timeout. */
HAL_StatusTypeDef LTDC_WaitForReload(LTDC_Driver_t *driver, uint32_t timeout_ms);

/* Framebuffer functions */
HAL_StatusTypeDef LTDC_SetFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t address);
HAL_StatusTypeDef LTDC_ClearFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t color);
HAL_StatusTypeDef LTDC_FillFramebuffer(LTDC_Driver_t *driver, uint8_t layer, uint32_t color);
HAL_StatusTypeDef LTDC_CopyFramebuffer(LTDC_Driver_t *driver, uint8_t srcLayer, uint8_t dstLayer);

/* Drawing functions (simplified)
 * Note: Higher level drawing (lines, shapes, fonts) should be done in
 * application code or by a GUI library (LVGL). Keeping only a minimal
 * pixel write helper keeps this driver simple and correct.
 */
HAL_StatusTypeDef LTDC_DrawPixel(LTDC_Driver_t *driver, uint16_t x, uint16_t y, uint32_t color);

/* Display control functions */
HAL_StatusTypeDef LTDC_SetBackgroundColor(LTDC_Driver_t *driver, uint32_t color);
HAL_StatusTypeDef LTDC_SetDisplayBrightness(LTDC_Driver_t *driver, uint8_t brightness);
HAL_StatusTypeDef LTDC_DisplayOn(LTDC_Driver_t *driver);
HAL_StatusTypeDef LTDC_DisplayOff(LTDC_Driver_t *driver);

/* Utility functions */
uint32_t LTDC_ConvertColor(uint32_t color, LTDC_PixelFormat_t fromFormat, LTDC_PixelFormat_t toFormat);
uint32_t LTDC_RGB888_To_RGB565(uint32_t rgb888);
uint32_t LTDC_RGB565_To_RGB888(uint16_t rgb565);
uint32_t LTDC_ARGB8888_To_RGB565(uint32_t argb8888);
HAL_StatusTypeDef LTDC_GetLayerInfo(LTDC_Driver_t *driver, uint8_t layer, LTDC_LayerConfig_t *info);
bool LTDC_IsLayerEnabled(LTDC_Driver_t *driver, uint8_t layer);

/* Error handling functions */
uint32_t LTDC_GetError(LTDC_Driver_t *driver);
HAL_StatusTypeDef LTDC_ClearError(LTDC_Driver_t *driver);

/* Hardware initialization functions */
HAL_StatusTypeDef LTDC_HW_Init(void);
void LTDC_MspInit(LTDC_HandleTypeDef *hltdc);
void LTDC_MspDeInit(LTDC_HandleTypeDef *hltdc);

/* Global variables ----------------------------------------------------------*/
extern LTDC_HandleTypeDef hltdc;

#ifdef __cplusplus
}
#endif

#endif /* LTDC_H */
