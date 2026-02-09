/**
  ******************************************************************************
  * @file    ili9488.h
  * @brief   ILI9488 TFT LCD Display Driver for STM32F429I-DISC1
  * @details This file contains function prototypes and definitions for
  *          ILI9488 4-inch TFT LCD display control using SPI interface.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

#ifndef __ILI9488_H__
#define __ILI9488_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup ILI9488_Display_Specifications Display Specifications
 * @{
 */
#define ILI9488_WIDTH                    320
#define ILI9488_HEIGHT                   480
#define ILI9488_PIXEL_COUNT              (ILI9488_WIDTH * ILI9488_HEIGHT)

/* Color definitions */
#define ILI9488_COLOR_BLACK              0x0000
#define ILI9488_COLOR_WHITE              0xFFFF
#define ILI9488_COLOR_RED                0xF800
#define ILI9488_COLOR_GREEN              0x07E0
#define ILI9488_COLOR_BLUE               0x001F
#define ILI9488_COLOR_YELLOW             0xFFE0
#define ILI9488_COLOR_MAGENTA            0xF81F
#define ILI9488_COLOR_CYAN               0x07FF

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief ILI9488 Status enumeration
 */
typedef enum {
    ILI9488_OK = 0,                 /**< Operation completed successfully */
    ILI9488_ERROR,                  /**< General error occurred */
    ILI9488_BUSY,                   /**< Display is busy */
    ILI9488_TIMEOUT,                /**< Operation timed out */
    ILI9488_INVALID_PARAM,          /**< Invalid parameter provided */
    ILI9488_NOT_INITIALIZED         /**< Driver not initialized */
} ILI9488_StatusTypeDef;

/**
 * @brief ILI9488 Orientation enumeration
 */
typedef enum {
    ILI9488_ORIENTATION_PORTRAIT = 0,     /**< Portrait orientation */
    ILI9488_ORIENTATION_LANDSCAPE,        /**< Landscape orientation */
    ILI9488_ORIENTATION_PORTRAIT_REV,     /**< Portrait reversed */
    ILI9488_ORIENTATION_LANDSCAPE_REV     /**< Landscape reversed */
} ILI9488_Orientation_t;

/**
 * @brief ILI9488 Configuration structure
 */
typedef struct {
    GPIO_TypeDef *cs_port;          /**< Chip select port */
    uint16_t cs_pin;                /**< Chip select pin */
    GPIO_TypeDef *dc_port;          /**< Data/command port */
    uint16_t dc_pin;                /**< Data/command pin */
    GPIO_TypeDef *rst_port;         /**< Reset port */
    uint16_t rst_pin;               /**< Reset pin */
    ILI9488_Orientation_t orientation; /**< Display orientation */
} ILI9488_Config_t;

/**
 * @brief ILI9488 Handle structure
 */
/** Pixel format constants */
#define ILI9488_PIXEL_FMT_RGB565   0x55  /* 16-bit RGB565 */
#define ILI9488_PIXEL_FMT_RGB666   0x66  /* 18-bit RGB666 (3 bytes/pixel) */

typedef struct {
    ILI9488_Config_t config;        /**< Display configuration */
    uint16_t currentX;              /**< Current X position */
    uint16_t currentY;              /**< Current Y position */
    uint16_t width;                 /**< Current display width (adjusted for orientation) */
    uint16_t height;                /**< Current display height (adjusted for orientation) */
    uint16_t base_width;            /**< Base width in portrait mode */
    uint16_t base_height;           /**< Base height in portrait mode */
    uint8_t pixel_format;           /**< Current pixel format (0x55=RGB565,0x66=RGB666) */
    bool initialized;               /**< Initialization status */
} ILI9488_Handle_t;

/**
 * @brief   Board support hooks (weak by default)
 * @details Override these in a board-specific file (e.g., ili9488_board.c)
 *          to configure GPIO clocks and control pins.
 */
void ILI9488_MspInit(void);
void ILI9488_MspDeInit(void);

/* Exported functions -------------------------------------------------------*/

/** @defgroup ILI9488_Init Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize ILI9488 TFT display
 * @details Configures SPI and initializes the display
 * @param   hili Pointer to ILI9488 handle
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   dc_port Data/command port
 * @param   dc_pin Data/command pin
 * @param   rst_port Reset port
 * @param   rst_pin Reset pin
 * @param   width Display width in portrait mode (e.g., 320)
 * @param   height Display height in portrait mode (e.g., 480)
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_Init(ILI9488_Handle_t *hili,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *dc_port, uint16_t dc_pin,
                                  GPIO_TypeDef *rst_port, uint16_t rst_pin,
                                  uint16_t width, uint16_t height);

/**
 * @brief   Deinitialize ILI9488 TFT display
 * @details Turns off display and releases resources
 * @param   hili Pointer to ILI9488 handle
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DeInit(ILI9488_Handle_t *hili);

/**
 * @brief   Configure ILI9488 display parameters
 * @details Sets display configuration options
 * @param   hili Pointer to ILI9488 handle
 * @param   config Pointer to configuration structure
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_Config(ILI9488_Handle_t *hili, ILI9488_Config_t *config);

/**
 * @brief   Set display orientation
 * @param   hili Pointer to ILI9488 handle
 * @param   orientation Display orientation
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_SetOrientation(ILI9488_Handle_t *hili, ILI9488_Orientation_t orientation);

/** @} */

/** @defgroup ILI9488_Display Display Control
 * @{
 */

/**
 * @brief   Turn display on
 * @param   hili Pointer to ILI9488 handle
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DisplayOn(ILI9488_Handle_t *hili);

/**
 * @brief   Turn display off
 * @param   hili Pointer to ILI9488 handle
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DisplayOff(ILI9488_Handle_t *hili);

/**
 * @brief   Clear display
 * @param   hili Pointer to ILI9488 handle
 * @param   color Fill color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_Clear(ILI9488_Handle_t *hili, uint16_t color);

/**
 * @brief   Update display with buffer content
 * @param   hili Pointer to ILI9488 handle
 * @param   buffer Color buffer (RGB565 format)
 * @param   size Buffer size in bytes
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_UpdateScreen(ILI9488_Handle_t *hili, uint16_t *buffer, uint32_t size);
ILI9488_StatusTypeDef ILI9488_WritePixels(ILI9488_Handle_t *hili,
                                         uint16_t x0, uint16_t y0,
                                         uint16_t x1, uint16_t y1,
                                         const uint16_t *data,
                                         uint32_t size);

/* Read device identification (returns first ID byte) */
ILI9488_StatusTypeDef ILI9488_ReadID(ILI9488_Handle_t *hili, uint8_t *id_buf, uint32_t len);

/* Set pixel format manually */
ILI9488_StatusTypeDef ILI9488_SetPixelFormat(ILI9488_Handle_t *hili, uint8_t pixel_fmt);

/** @} */

/** @defgroup ILI9488_Graphics Graphics Functions
 * @{
 */

/**
 * @brief   Draw pixel at specified coordinates
 * @param   hili Pointer to ILI9488 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color Pixel color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DrawPixel(ILI9488_Handle_t *hili,
                                       uint16_t x, uint16_t y,
                                       uint16_t color);

/**
 * @brief   Draw line between two points
 * @param   hili Pointer to ILI9488 handle
 * @param   x1 Start X coordinate
 * @param   y1 Start Y coordinate
 * @param   x2 End X coordinate
 * @param   y2 End Y coordinate
 * @param   color Line color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DrawLine(ILI9488_Handle_t *hili,
                                     uint16_t x1, uint16_t y1,
                                     uint16_t x2, uint16_t y2,
                                     uint16_t color);

/**
 * @brief   Draw rectangle
 * @param   hili Pointer to ILI9488 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   width Rectangle width
 * @param   height Rectangle height
 * @param   color Rectangle color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DrawRectangle(ILI9488_Handle_t *hili,
                                           uint16_t x, uint16_t y,
                                           uint16_t width, uint16_t height,
                                           uint16_t color);

/**
 * @brief   Draw filled rectangle
 * @param   hili Pointer to ILI9488 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   width Rectangle width
 * @param   height Rectangle height
 * @param   color Fill color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DrawFilledRectangle(ILI9488_Handle_t *hili,
                                                 uint16_t x, uint16_t y,
                                                 uint16_t width, uint16_t height,
                                                 uint16_t color);

/**
 * @brief   Draw circle
 * @param   hili Pointer to ILI9488 handle
 * @param   x Center X coordinate
 * @param   y Center Y coordinate
 * @param   radius Circle radius
 * @param   color Circle color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DrawCircle(ILI9488_Handle_t *hili,
                                        uint16_t x, uint16_t y,
                                        uint16_t radius,
                                        uint16_t color);

/**
 * @brief   Draw filled circle
 * @param   hili Pointer to ILI9488 handle
 * @param   x Center X coordinate
 * @param   y Center Y coordinate
 * @param   radius Circle radius
 * @param   color Fill color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DrawFilledCircle(ILI9488_Handle_t *hili,
                                              uint16_t x, uint16_t y,
                                              uint16_t radius,
                                              uint16_t color);

/** @} */

/** @defgroup ILI9488_Text Text Functions
 * @{
 */

/**
 * @brief   Set cursor position
 * @param   hili Pointer to ILI9488 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_SetCursor(ILI9488_Handle_t *hili, uint16_t x, uint16_t y);

/**
 * @brief   Write character
 * @param   hili Pointer to ILI9488 handle
 * @param   ch Character to write
 * @param   color Character color
 * @param   bgcolor Background color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_WriteChar(ILI9488_Handle_t *hili, char ch, uint16_t color, uint16_t bgcolor);

/**
 * @brief   Write string
 * @param   hili Pointer to ILI9488 handle
 * @param   str String to write
 * @param   color String color
 * @param   bgcolor Background color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_WriteString(ILI9488_Handle_t *hili, const char *str, uint16_t color, uint16_t bgcolor);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __ILI9488_H__ */
