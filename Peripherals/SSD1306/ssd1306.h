/**
  ******************************************************************************
  * @file    ssd1306.h
  * @brief   SSD1306 OLED Display Driver for STM32F429I-DISC1
  * @details This file contains function prototypes and definitions for
  *          SSD1306 128x64 OLED display control using I2C interface.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "i2c.h"
#include <stdint.h>
#include <stdbool.h>
#include "log.h"

/* Exported constants --------------------------------------------------------*/

/** @defgroup SSD1306_Display_Specifications Display Specifications
 * @{
 */
#define SSD1306_WIDTH                    128
#define SSD1306_HEIGHT                   64
#define SSD1306_BUFFER_SIZE              (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

#define SSD1306_I2C_ADDR                 0x3C    // Default I2C address
#define SSD1306_I2C_ADDR_ALT             0x3D    // Alternative I2C address

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief SSD1306 Status enumeration
 */
typedef enum {
    SSD1306_OK = 0,                 /**< Operation completed successfully */
    SSD1306_ERROR,                  /**< General error occurred */
    SSD1306_BUSY,                   /**< Display is busy */
    SSD1306_TIMEOUT,                /**< Operation timed out */
    SSD1306_INVALID_PARAM,          /**< Invalid parameter provided */
    SSD1306_NOT_INITIALIZED         /**< Driver not initialized */
} SSD1306_StatusTypeDef;

/**
 * @brief SSD1306 Color enumeration
 */
typedef enum {
    SSD1306_COLOR_BLACK = 0,        /**< Black color */
    SSD1306_COLOR_WHITE = 1         /**< White color */
} SSD1306_Color_t;

/**
 * @brief SSD1306 Font size enumeration
 */
typedef enum {
    SSD1306_FONT_6x8 = 0,           /**< 6x8 font */
    SSD1306_FONT_8x16,              /**< 8x16 font */
    SSD1306_FONT_12x16,             /**< 12x16 font */
    SSD1306_FONT_16x24              /**< 16x24 font */
} SSD1306_FontSize_t;

/**
 * @brief SSD1306 Configuration structure
 */
typedef struct {
    uint8_t address;                /**< I2C address */
    bool flipVertically;            /**< Flip display vertically */
    bool flipHorizontally;          /**< Flip display horizontally */
} SSD1306_Config_t;

/**
 * @brief SSD1306 Handle structure
 */
typedef struct {
    SSD1306_Config_t config;        /**< Display configuration */
    uint16_t currentX;              /**< Current X position */
    uint16_t currentY;              /**< Current Y position */
    SSD1306_FontSize_t fontSize;    /**< Current font size */
    bool initialized;               /**< Initialization status */
} SSD1306_Handle_t;

/* Global display buffer - shared across all instances */
extern uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

/* Exported functions -------------------------------------------------------*/

/** @defgroup SSD1306_Init Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize SSD1306 OLED display
 * @details Configures I2C and initializes the display
 * @param   hssd Pointer to SSD1306 handle
 * @param   address I2C address (0x3C or 0x3D)
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Init(SSD1306_Handle_t *hssd, uint8_t address);

/**
 * @brief   Deinitialize SSD1306 OLED display
 * @details Turns off display and releases resources
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DeInit(SSD1306_Handle_t *hssd);

/**
 * @brief   Configure SSD1306 display parameters
 * @details Sets display configuration options
 * @param   hssd Pointer to SSD1306 handle
 * @param   config Pointer to configuration structure
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Config(SSD1306_Handle_t *hssd, SSD1306_Config_t *config);

/** @} */

/** @defgroup SSD1306_Display Display Control
 * @{
 */

/**
 * @brief   Turn display on
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DisplayOn(SSD1306_Handle_t *hssd);

/**
 * @brief   Turn display off
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DisplayOff(SSD1306_Handle_t *hssd);

/**
 * @brief   Update display with buffer content
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_UpdateScreen(SSD1306_Handle_t *hssd);

/**
 * @brief   Clear display buffer
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Clear(SSD1306_Handle_t *hssd);

/**
 * @brief   Fill display buffer with color
 * @param   hssd Pointer to SSD1306 handle
 * @param   color Fill color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Fill(SSD1306_Handle_t *hssd, SSD1306_Color_t color);

/** @} */

/** @defgroup SSD1306_Graphics Graphics Functions
 * @{
 */

/**
 * @brief   Draw pixel at specified coordinates
 * @param   hssd Pointer to SSD1306 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color Pixel color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DrawPixel(SSD1306_Handle_t *hssd,
                                       uint16_t x, uint16_t y,
                                       SSD1306_Color_t color);

/**
 * @brief   Draw line between two points
 * @param   hssd Pointer to SSD1306 handle
 * @param   x1 Start X coordinate
 * @param   y1 Start Y coordinate
 * @param   x2 End X coordinate
 * @param   y2 End Y coordinate
 * @param   color Line color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DrawLine(SSD1306_Handle_t *hssd,
                                     uint16_t x1, uint16_t y1,
                                     uint16_t x2, uint16_t y2,
                                     SSD1306_Color_t color);

/**
 * @brief   Draw rectangle
 * @param   hssd Pointer to SSD1306 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   width Rectangle width
 * @param   height Rectangle height
 * @param   color Rectangle color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DrawRectangle(SSD1306_Handle_t *hssd,
                                           uint16_t x, uint16_t y,
                                           uint16_t width, uint16_t height,
                                           SSD1306_Color_t color);

/**
 * @brief   Draw filled rectangle
 * @param   hssd Pointer to SSD1306 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   width Rectangle width
 * @param   height Rectangle height
 * @param   color Fill color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DrawFilledRectangle(SSD1306_Handle_t *hssd,
                                                 uint16_t x, uint16_t y,
                                                 uint16_t width, uint16_t height,
                                                 SSD1306_Color_t color);

/**
 * @brief   Draw circle
 * @param   hssd Pointer to SSD1306 handle
 * @param   x Center X coordinate
 * @param   y Center Y coordinate
 * @param   radius Circle radius
 * @param   color Circle color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DrawCircle(SSD1306_Handle_t *hssd,
                                        uint16_t x, uint16_t y,
                                        uint16_t radius,
                                        SSD1306_Color_t color);

/**
 * @brief   Draw filled circle
 * @param   hssd Pointer to SSD1306 handle
 * @param   x Center X coordinate
 * @param   y Center Y coordinate
 * @param   radius Circle radius
 * @param   color Fill color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DrawFilledCircle(SSD1306_Handle_t *hssd,
                                              uint16_t x, uint16_t y,
                                              uint16_t radius,
                                              SSD1306_Color_t color);

/** @} */

/** @defgroup SSD1306_Text Text Functions
 * @{
 */

/**
 * @brief   Set cursor position
 * @param   hssd Pointer to SSD1306 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_SetCursor(SSD1306_Handle_t *hssd, uint16_t x, uint16_t y);

/**
 * @brief   Set font size
 * @param   hssd Pointer to SSD1306 handle
 * @param   fontSize Font size
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_SetFontSize(SSD1306_Handle_t *hssd, SSD1306_FontSize_t fontSize);

/**
 * @brief   Write character
 * @param   hssd Pointer to SSD1306 handle
 * @param   ch Character to write
 * @param   color Character color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_WriteChar(SSD1306_Handle_t *hssd, char ch, SSD1306_Color_t color);

/**
 * @brief   Write string
 * @param   hssd Pointer to SSD1306 handle
 * @param   str String to write
 * @param   color String color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_WriteString(SSD1306_Handle_t *hssd, const char *str, SSD1306_Color_t color);

/**
 * @brief   Write formatted string
 * @param   hssd Pointer to SSD1306 handle
 * @param   color String color
 * @param   format Format string
 * @param   ... Additional arguments
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Printf(SSD1306_Handle_t *hssd, SSD1306_Color_t color, const char *format, ...);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __SSD1306_H__ */
