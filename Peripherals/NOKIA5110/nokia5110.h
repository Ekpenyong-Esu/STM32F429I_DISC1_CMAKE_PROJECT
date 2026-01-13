/**
  ******************************************************************************
  * @file    nokia5110.h
  * @brief   Nokia 5110 LCD driver interface
  * @details This file contains function prototypes and definitions for
  *          the Nokia 5110 LCD display (84x48 pixels) using SPI interface.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

#ifndef __NOKIA5110_H__
#define __NOKIA5110_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup NOKIA5110_Display_Specifications Display Specifications
 * @{
 */
#define NOKIA5110_WIDTH          84      /*!< Display width in pixels */
#define NOKIA5110_HEIGHT         48      /*!< Display height in pixels */
#define NOKIA5110_ROWS           6       /*!< Number of rows (8 pixels each) */
/** @} */

/** @defgroup NOKIA5110_Commands LCD Commands
 * @{
 */
#define NOKIA5110_CMD_FUNCTION_SET     0x20  /*!< Function set */
#define NOKIA5110_CMD_DISPLAY_CONTROL  0x08  /*!< Display control */
#define NOKIA5110_CMD_SET_Y_ADDR       0x40  /*!< Set Y address */
#define NOKIA5110_CMD_SET_X_ADDR       0x80  /*!< Set X address */
#define NOKIA5110_CMD_TEMP_CONTROL     0x04  /*!< Temperature control */
#define NOKIA5110_CMD_BIAS_SYSTEM      0x10  /*!< Bias system */
#define NOKIA5110_CMD_VOP              0x80  /*!< Vop (contrast) */

/* Function set options */
#define NOKIA5110_FUNCTION_H            0x01  /*!< Extended instruction set */
#define NOKIA5110_FUNCTION_V            0x02  /*!< Vertical addressing */
#define NOKIA5110_FUNCTION_PD           0x04  /*!< Power down mode */

/* Display control options */
#define NOKIA5110_DISPLAY_BLANK         0x00  /*!< Display blank */
#define NOKIA5110_DISPLAY_NORMAL        0x04  /*!< Normal mode */
#define NOKIA5110_DISPLAY_ALL_ON        0x01  /*!< All pixels on */
#define NOKIA5110_DISPLAY_INVERSE       0x05  /*!< Inverse video mode */

/* Temperature control coefficients */
#define NOKIA5110_TEMP_COEFF_0          0x00  /*!< TC0 */
#define NOKIA5110_TEMP_COEFF_1          0x01  /*!< TC1 */
#define NOKIA5110_TEMP_COEFF_2          0x02  /*!< TC2 */
#define NOKIA5110_TEMP_COEFF_3          0x03  /*!< TC3 */

/* Bias system values */
#define NOKIA5110_BIAS_1_100            0x00  /*!< 1:100 bias */
#define NOKIA5110_BIAS_1_80             0x01  /*!< 1:80 bias */
#define NOKIA5110_BIAS_1_65             0x02  /*!< 1:65 bias */
#define NOKIA5110_BIAS_1_48             0x03  /*!< 1:48 bias */
#define NOKIA5110_BIAS_1_40_1           0x04  /*!< 1:40/1:34 */
#define NOKIA5110_BIAS_1_24             0x05  /*!< 1:24 bias */
#define NOKIA5110_BIAS_1_18_1           0x06  /*!< 1:18/1:16 */
#define NOKIA5110_BIAS_1_10_1           0x07  /*!< 1:10/1:9 */

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Nokia 5110 status enumeration
 */
typedef enum {
    NOKIA5110_OK = 0,              /*!< Operation completed successfully */
    NOKIA5110_ERROR,               /*!< General error occurred */
    NOKIA5110_TIMEOUT,             /*!< Operation timed out */
    NOKIA5110_INVALID_PARAM,       /*!< Invalid parameter provided */
    NOKIA5110_NOT_INITIALIZED      /*!< Device not initialized */
} NOKIA5110_StatusTypeDef;

/**
 * @brief Nokia 5110 display mode enumeration
 */
typedef enum {
    NOKIA5110_MODE_BLANK = 0,      /*!< Display blank */
    NOKIA5110_MODE_NORMAL,         /*!< Normal display mode */
    NOKIA5110_MODE_ALL_ON,         /*!< All pixels on */
    NOKIA5110_MODE_INVERSE         /*!< Inverse video mode */
} NOKIA5110_DisplayMode_t;

/**
 * @brief Nokia 5110 configuration structure
 */
typedef struct {
    uint8_t Contrast;              /*!< Display contrast (0-127) */
    uint8_t TemperatureCoeff;      /*!< Temperature coefficient (0-3) */
    uint8_t BiasSystem;            /*!< Bias system (0-7) */
    NOKIA5110_DisplayMode_t Mode;  /*!< Display mode */
} NOKIA5110_Config_t;

/**
 * @brief Nokia 5110 handle structure
 */
typedef struct {
    NOKIA5110_Config_t Config;     /*!< Configuration */
    bool IsInitialized;            /*!< Initialization status */
    uint8_t Buffer[NOKIA5110_ROWS][NOKIA5110_WIDTH]; /*!< Display buffer */
} NOKIA5110_Handle_t;

/* Exported functions -------------------------------------------------------*/

/** @defgroup NOKIA5110_Init Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize Nokia 5110 LCD
 * @details Configures GPIO pins and initializes the display
 * @param   hnok Pointer to Nokia 5110 handle
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Init(NOKIA5110_Handle_t *hnok);

/**
 * @brief   Deinitialize Nokia 5110 LCD
 * @details Powers down the display and releases resources
 * @param   hnok Pointer to Nokia 5110 handle
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_DeInit(NOKIA5110_Handle_t *hnok);

/**
 * @brief   Configure Nokia 5110 LCD parameters
 * @details Sets contrast, temperature coefficient, bias system, and display mode
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   config Pointer to configuration structure
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Config(NOKIA5110_Handle_t *hnok, NOKIA5110_Config_t *config);

/** @} */

/** @defgroup NOKIA5110_Display Display Control
 * @{
 */

/**
 * @brief   Clear the display
 * @details Clears the display buffer and updates the screen
 * @param   hnok Pointer to Nokia 5110 handle
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Clear(NOKIA5110_Handle_t *hnok);

/**
 * @brief   Update the display
 * @details Sends the buffer contents to the LCD
 * @param   hnok Pointer to Nokia 5110 handle
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Update(NOKIA5110_Handle_t *hnok);

/**
 * @brief   Set display contrast
 * @details Adjusts the LCD contrast (0-127)
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   contrast Contrast value
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_SetContrast(NOKIA5110_Handle_t *hnok, uint8_t contrast);

/**
 * @brief   Set display mode
 * @details Changes the display mode (normal, inverse, blank, all on)
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   mode Display mode
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_SetMode(NOKIA5110_Handle_t *hnok, NOKIA5110_DisplayMode_t mode);

/** @} */

/** @defgroup NOKIA5110_Drawing Drawing Functions
 * @{
 */

/**
 * @brief   Draw a pixel
 * @details Sets or clears a pixel at the specified coordinates
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   x X coordinate (0-83)
 * @param   y Y coordinate (0-47)
 * @param   color Pixel color (0 = off, 1 = on)
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawPixel(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief   Draw a line
 * @details Draws a line between two points using Bresenham's algorithm
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   x0 Starting X coordinate
 * @param   y0 Starting Y coordinate
 * @param   x1 Ending X coordinate
 * @param   y1 Ending Y coordinate
 * @param   color Line color (0 = off, 1 = on)
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawLine(NOKIA5110_Handle_t *hnok, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);

/**
 * @brief   Draw a rectangle
 * @details Draws a rectangle outline
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   x X coordinate of top-left corner
 * @param   y Y coordinate of top-left corner
 * @param   width Rectangle width
 * @param   height Rectangle height
 * @param   color Rectangle color (0 = off, 1 = on)
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawRect(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

/**
 * @brief   Fill a rectangle
 * @details Draws a filled rectangle
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   x X coordinate of top-left corner
 * @param   y Y coordinate of top-left corner
 * @param   width Rectangle width
 * @param   height Rectangle height
 * @param   color Fill color (0 = off, 1 = on)
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_FillRect(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);

/**
 * @brief   Draw a circle
 * @details Draws a circle outline using Bresenham's algorithm
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   x0 Center X coordinate
 * @param   y0 Center Y coordinate
 * @param   radius Circle radius
 * @param   color Circle color (0 = off, 1 = on)
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawCircle(NOKIA5110_Handle_t *hnok, uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color);

/**
 * @brief   Draw text
 * @details Draws ASCII text at specified coordinates
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   x X coordinate
 * @param   y Y coordinate (row number, 0-5)
 * @param   text Null-terminated string to draw
 * @param   color Text color (0 = off, 1 = on)
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawText(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y, const char *text, uint8_t color);

/** @} */

/** @defgroup NOKIA5110_Utility Utility Functions
 * @{
 */

/**
 * @brief   Get display width
 * @details Returns the display width in pixels
 * @param   None
 * @retval  uint8_t Display width
 */
uint8_t NOKIA5110_GetWidth(void);

/**
 * @brief   Get display height
 * @details Returns the display height in pixels
 * @param   None
 * @retval  uint8_t Display height
 */
uint8_t NOKIA5110_GetHeight(void);

/**
 * @brief   Get default configuration
 * @details Returns a default configuration structure
 * @param   None
 * @retval  NOKIA5110_Config_t Default configuration
 */
NOKIA5110_Config_t NOKIA5110_GetDefaultConfig(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __NOKIA5110_H__ */
