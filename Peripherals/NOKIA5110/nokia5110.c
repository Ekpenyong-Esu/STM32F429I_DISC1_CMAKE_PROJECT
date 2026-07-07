/**
  ******************************************************************************
  * @file    nokia5110.c
  * @brief   Nokia 5110 LCD driver implementation
  * @details This file provides the implementation of Nokia 5110 LCD functions
  *          using SPI interface for STM32F429 Discovery board.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "nokia5110.h"
#include "spi.h"
#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup NOKIA5110_Private_Defines Private Defines
 * @{
 */

/* GPIO Pin Definitions for Nokia 5110 */
#define NOKIA5110_RST_PIN        GPIO_PIN_1    /* PB1 - Reset */
#define NOKIA5110_RST_PORT       GPIOB
#define NOKIA5110_CE_PIN         GPIO_PIN_0    /* PB0 - Chip Enable */
#define NOKIA5110_CE_PORT        GPIOB
#define NOKIA5110_DC_PIN         GPIO_PIN_2    /* PB2 - Data/Command */
#define NOKIA5110_DC_PORT        GPIOB

/* SPI Settings */
#define NOKIA5110_SPI_TIMEOUT    1000U         /* SPI timeout in ms */

/* Font dimensions */
#define FONT_WIDTH               5             /* Character width in pixels */
#define FONT_HEIGHT              7             /* Character height in pixels */

/** @} */

/* Private variables ---------------------------------------------------------*/

/** @defgroup NOKIA5110_Private_Variables Private Variables
 * @{
 */

/* 5x7 ASCII font (modified from standard Nokia 5110 font) */
static const uint8_t ASCII[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00} // 20
    ,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
    ,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
    ,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
    ,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
    ,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
    ,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
    ,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
    ,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
    ,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
    ,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
    ,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
    ,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
    ,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
    ,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
    ,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
    ,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
    ,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
    ,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
    ,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
    ,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
    ,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
    ,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
    ,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
    ,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
    ,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
    ,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
    ,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
    ,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
    ,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
    ,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
    ,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
    ,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
    ,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
    ,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
    ,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
    ,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
    ,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
    ,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
    ,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
    ,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
    ,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
    ,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
    ,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
    ,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
    ,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
    ,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
    ,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
    ,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
    ,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
    ,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
    ,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
    ,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
    ,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
    ,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
    ,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
    ,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
    ,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
    ,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
    ,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
    ,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c backslash
    ,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
    ,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
    ,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
    ,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
    ,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
    ,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
    ,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
    ,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
    ,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
    ,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
    ,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
    ,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
    ,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
    ,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j
    ,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
    ,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
    ,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
    ,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
    ,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
    ,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
    ,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
    ,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
    ,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
    ,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
    ,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
    ,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
    ,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
    ,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
    ,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
    ,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
    ,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
    ,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
    ,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
    ,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ~
    ,{0x00, 0x06, 0x09, 0x09, 0x06} // 7f DEL
};

/* Private function prototypes -----------------------------------------------*/
static void NOKIA5110_MspInit(void);
static void NOKIA5110_MspDeInit(void);
static NOKIA5110_StatusTypeDef NOKIA5110_WriteCommand(uint8_t cmd);
static NOKIA5110_StatusTypeDef NOKIA5110_WriteData(uint8_t data);
static NOKIA5110_StatusTypeDef NOKIA5110_WriteDataMulti(const uint8_t *data, uint16_t size);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize Nokia 5110 LCD
 * @details Configures GPIO pins and initializes the display
 * @param   hnok Pointer to Nokia 5110 handle
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Init(NOKIA5110_Handle_t *hnok)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    /* Initialize MSP (GPIO pins) */
    NOKIA5110_MspInit();

    /* Initialize SPI */
    SPI_Init();

    /* Reset the LCD */
    HAL_GPIO_WritePin(NOKIA5110_RST_PORT, NOKIA5110_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(NOKIA5110_RST_PORT, NOKIA5110_RST_PIN, GPIO_PIN_SET);

    /* Send initialization commands */
    NOKIA5110_WriteCommand(0x21);  /* Extended instruction set */
    NOKIA5110_WriteCommand(0xB0);  /* Set Vop (contrast) */
    NOKIA5110_WriteCommand(0x04);  /* Set temperature coefficient */
    NOKIA5110_WriteCommand(0x14);  /* Set bias system */
    NOKIA5110_WriteCommand(0x20);  /* Standard instruction set */
    NOKIA5110_WriteCommand(0x0C);  /* Normal display mode */

    /* Clear the display */
    NOKIA5110_Clear(hnok);

    /* Set default configuration */
    NOKIA5110_Config_t default_config = NOKIA5110_GetDefaultConfig();
    NOKIA5110_Config(hnok, &default_config);

    hnok->IsInitialized = true;

    return NOKIA5110_OK;
}

/**
 * @brief   Deinitialize Nokia 5110 LCD
 * @details Powers down the display and releases resources
 * @param   hnok Pointer to Nokia 5110 handle
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_DeInit(NOKIA5110_Handle_t *hnok)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    /* Clear display */
    NOKIA5110_Clear(hnok);

    /* Power down */
    NOKIA5110_WriteCommand(0x24);  /* Power down */

    /* Deinitialize MSP */
    NOKIA5110_MspDeInit();

    hnok->IsInitialized = false;

    return NOKIA5110_OK;
}

/**
 * @brief   Configure Nokia 5110 LCD parameters
 * @details Sets contrast, temperature coefficient, bias system, and display mode
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   config Pointer to configuration structure
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Config(NOKIA5110_Handle_t *hnok, NOKIA5110_Config_t *config)
{
    if (hnok == NULL || config == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    /* Store configuration */
    hnok->Config = *config;

    /* Apply contrast */
    NOKIA5110_WriteCommand(0x21);  /* Extended instruction set */
    NOKIA5110_WriteCommand(0x80 | (config->Contrast & 0x7F));  /* Set Vop */
    NOKIA5110_WriteCommand(0x04 | (config->TemperatureCoeff & 0x03));  /* Temperature coeff */
    NOKIA5110_WriteCommand(0x10 | (config->BiasSystem & 0x07));  /* Bias system */
    NOKIA5110_WriteCommand(0x20);  /* Standard instruction set */

    /* Apply display mode */
    uint8_t mode_cmd;
    switch (config->Mode) {
        case NOKIA5110_MODE_BLANK:
            mode_cmd = 0x08;
            break;
        case NOKIA5110_MODE_NORMAL:
            mode_cmd = 0x0C;
            break;
        case NOKIA5110_MODE_ALL_ON:
            mode_cmd = 0x09;
            break;
        case NOKIA5110_MODE_INVERSE:
            mode_cmd = 0x0D;
            break;
        default:
            mode_cmd = 0x0C;
            break;
    }
    NOKIA5110_WriteCommand(mode_cmd);

    return NOKIA5110_OK;
}

/**
 * @brief   Clear the display
 * @details Clears the display buffer and updates the screen
 * @param   hnok Pointer to Nokia 5110 handle
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Clear(NOKIA5110_Handle_t *hnok)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    /* Clear buffer */
    memset(hnok->Buffer, 0, sizeof(hnok->Buffer));

    /* Update display */
    return NOKIA5110_Update(hnok);
}

/**
 * @brief   Update the display
 * @details Sends the buffer contents to the LCD
 * @param   hnok Pointer to Nokia 5110 handle
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Update(NOKIA5110_Handle_t *hnok)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    /* Set X and Y addresses */
    NOKIA5110_WriteCommand(0x40);  /* Y address = 0 */
    NOKIA5110_WriteCommand(0x80);  /* X address = 0 */

    /* Send buffer data */
    HAL_GPIO_WritePin(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN, GPIO_PIN_SET);  /* Data mode */
    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_RESET);  /* Chip enable */

    for (uint8_t row = 0; row < NOKIA5110_ROWS; row++) {
        if (SPI_Transmit(hnok->Buffer[row], NOKIA5110_WIDTH, NOKIA5110_SPI_TIMEOUT) != SPI_OK) {
            HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_SET);
            return NOKIA5110_ERROR;
        }
    }

    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_SET);  /* Chip disable */

    return NOKIA5110_OK;
}

/**
 * @brief   Set display contrast
 * @details Adjusts the LCD contrast (0-127)
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   contrast Contrast value
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_SetContrast(NOKIA5110_Handle_t *hnok, uint8_t contrast)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    hnok->Config.Contrast = contrast & 0x7F;

    NOKIA5110_WriteCommand(0x21);  /* Extended instruction set */
    NOKIA5110_WriteCommand(0x80 | hnok->Config.Contrast);  /* Set Vop */
    NOKIA5110_WriteCommand(0x20);  /* Standard instruction set */

    return NOKIA5110_OK;
}

/**
 * @brief   Set display mode
 * @details Changes the display mode (normal, inverse, blank, all on)
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   mode Display mode
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_SetMode(NOKIA5110_Handle_t *hnok, NOKIA5110_DisplayMode_t mode)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    hnok->Config.Mode = mode;

    uint8_t mode_cmd;
    switch (mode) {
        case NOKIA5110_MODE_BLANK:
            mode_cmd = 0x08;
            break;
        case NOKIA5110_MODE_NORMAL:
            mode_cmd = 0x0C;
            break;
        case NOKIA5110_MODE_ALL_ON:
            mode_cmd = 0x09;
            break;
        case NOKIA5110_MODE_INVERSE:
            mode_cmd = 0x0D;
            break;
        default:
            mode_cmd = 0x0C;
            break;
    }
    NOKIA5110_WriteCommand(mode_cmd);

    return NOKIA5110_OK;
}

/**
 * @brief   Draw a pixel
 * @details Sets or clears a pixel at the specified coordinates
 * @param   hnok Pointer to Nokia 5110 handle
 * @param   x X coordinate (0-83)
 * @param   y Y coordinate (0-47)
 * @param   color Pixel color (0 = off, 1 = on)
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_DrawPixel(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y, uint8_t color)
{
    if (hnok == NULL || x >= NOKIA5110_WIDTH || y >= NOKIA5110_HEIGHT) {
        return NOKIA5110_INVALID_PARAM;
    }

    uint8_t row = y / 8;
    uint8_t bit = y % 8;

    if (color) {
        hnok->Buffer[row][x] |= (1 << bit);
    } else {
        hnok->Buffer[row][x] &= ~(1 << bit);
    }

    return NOKIA5110_OK;
}

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
NOKIA5110_StatusTypeDef NOKIA5110_DrawLine(NOKIA5110_Handle_t *hnok, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    while (1) {
        NOKIA5110_DrawPixel(hnok, x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    return NOKIA5110_OK;
}

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
NOKIA5110_StatusTypeDef NOKIA5110_DrawRect(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    NOKIA5110_DrawLine(hnok, x, y, x + width - 1, y, color);
    NOKIA5110_DrawLine(hnok, x, y + height - 1, x + width - 1, y + height - 1, color);
    NOKIA5110_DrawLine(hnok, x, y, x, y + height - 1, color);
    NOKIA5110_DrawLine(hnok, x + width - 1, y, x + width - 1, y + height - 1, color);

    return NOKIA5110_OK;
}

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
NOKIA5110_StatusTypeDef NOKIA5110_FillRect(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    for (uint8_t i = 0; i < height; i++) {
        NOKIA5110_DrawLine(hnok, x, y + i, x + width - 1, y + i, color);
    }

    return NOKIA5110_OK;
}

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
NOKIA5110_StatusTypeDef NOKIA5110_DrawCircle(NOKIA5110_Handle_t *hnok, uint8_t x0, uint8_t y0, uint8_t radius, uint8_t color)
{
    if (hnok == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    int16_t x = radius;
    int16_t y = 0;
    int16_t err = 0;

    while (x >= y) {
        NOKIA5110_DrawPixel(hnok, x0 + x, y0 + y, color);
        NOKIA5110_DrawPixel(hnok, x0 + y, y0 + x, color);
        NOKIA5110_DrawPixel(hnok, x0 - y, y0 + x, color);
        NOKIA5110_DrawPixel(hnok, x0 - x, y0 + y, color);
        NOKIA5110_DrawPixel(hnok, x0 - x, y0 - y, color);
        NOKIA5110_DrawPixel(hnok, x0 - y, y0 - x, color);
        NOKIA5110_DrawPixel(hnok, x0 + y, y0 - x, color);
        NOKIA5110_DrawPixel(hnok, x0 + x, y0 - y, color);

        y += 1;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0) {
            x -= 1;
            err += 1 - 2 * x;
        }
    }

    return NOKIA5110_OK;
}

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
NOKIA5110_StatusTypeDef NOKIA5110_DrawText(NOKIA5110_Handle_t *hnok, uint8_t x, uint8_t y, const char *text, uint8_t color)
{
    if (hnok == NULL || text == NULL) {
        return NOKIA5110_INVALID_PARAM;
    }

    uint8_t cursor_x = x;
    uint8_t cursor_y = y * 8;

    while (*text) {
        if (*text == '\n') {
            cursor_x = x;
            cursor_y += 8;
        } else if (*text >= 0x20 && *text <= 0x7F) {
            const uint8_t *char_data = ASCII[*text - 0x20];

            for (uint8_t i = 0; i < FONT_WIDTH; i++) {
                uint8_t column = char_data[i];
                for (uint8_t j = 0; j < FONT_HEIGHT; j++) {
                    if (column & (1 << j)) {
                        NOKIA5110_DrawPixel(hnok, cursor_x + i, cursor_y + j, color);
                    }
                }
            }
            cursor_x += FONT_WIDTH + 1;  /* Add space between characters */
        }
        text++;
    }

    return NOKIA5110_OK;
}

/**
 * @brief   Get display width
 * @details Returns the display width in pixels
 * @param   None
 * @retval  uint8_t Display width
 */
uint8_t NOKIA5110_GetWidth(void)
{
    return NOKIA5110_WIDTH;
}

/**
 * @brief   Get display height
 * @details Returns the display height in pixels
 * @param   None
 * @retval  uint8_t Display height
 */
uint8_t NOKIA5110_GetHeight(void)
{
    return NOKIA5110_HEIGHT;
}

/**
 * @brief   Get default configuration
 * @details Returns a default configuration structure
 * @param   None
 * @retval  NOKIA5110_Config_t Default configuration
 */
NOKIA5110_Config_t NOKIA5110_GetDefaultConfig(void)
{
    NOKIA5110_Config_t config = {
        .Contrast = 0x40,              /* Default contrast */
        .TemperatureCoeff = 0x00,      /* TC0 */
        .BiasSystem = 0x04,            /* 1:40/1:34 */
        .Mode = NOKIA5110_MODE_NORMAL   /* Normal display mode */
    };

    return config;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Initialize MSP (GPIO pins)
 * @details Configures GPIO pins for Nokia 5110 LCD
 * @param   None
 * @retval  None
 */
static void NOKIA5110_MspInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure RST pin (PB1) */
    GPIO_InitStruct.Pin = NOKIA5110_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(NOKIA5110_RST_PORT, &GPIO_InitStruct);

    /* Configure CE pin (PB0) */
    GPIO_InitStruct.Pin = NOKIA5110_CE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(NOKIA5110_CE_PORT, &GPIO_InitStruct);

    /* Configure DC pin (PB2) */
    GPIO_InitStruct.Pin = NOKIA5110_DC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(NOKIA5110_DC_PORT, &GPIO_InitStruct);

    /* Set default pin states */
    HAL_GPIO_WritePin(NOKIA5110_RST_PORT, NOKIA5110_RST_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN, GPIO_PIN_RESET);
}

/**
 * @brief   Deinitialize MSP
 * @details Resets GPIO pins to default state
 * @param   None
 * @retval  None
 */
static void NOKIA5110_MspDeInit(void)
{
    /* Reset pins to input mode */
    HAL_GPIO_DeInit(NOKIA5110_RST_PORT, NOKIA5110_RST_PIN);
    HAL_GPIO_DeInit(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN);
    HAL_GPIO_DeInit(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN);
}

/**
 * @brief   Write command to LCD
 * @details Sends a command byte to the LCD
 * @param   cmd Command byte
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
static NOKIA5110_StatusTypeDef NOKIA5110_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN, GPIO_PIN_RESET);  /* Command mode */
    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_RESET);  /* Chip enable */

    SPI_StatusTypeDef spi_status = SPI_Transmit(&cmd, 1, NOKIA5110_SPI_TIMEOUT);

    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_SET);  /* Chip disable */

    return (spi_status == SPI_OK) ? NOKIA5110_OK : NOKIA5110_ERROR;
}

/**
 * @brief   Write data to LCD
 * @details Sends a data byte to the LCD
 * @param   data Data byte
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
static NOKIA5110_StatusTypeDef NOKIA5110_WriteData(uint8_t data)
{
    HAL_GPIO_WritePin(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN, GPIO_PIN_SET);  /* Data mode */
    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_RESET);  /* Chip enable */

    SPI_StatusTypeDef spi_status = SPI_Transmit(&data, 1, NOKIA5110_SPI_TIMEOUT);

    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_SET);  /* Chip disable */

    return (spi_status == SPI_OK) ? NOKIA5110_OK : NOKIA5110_ERROR;
}

/**
 * @brief   Write multiple data bytes to LCD
 * @details Sends multiple data bytes to the LCD
 * @param   data Pointer to data buffer
 * @param   size Number of bytes to send
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
static NOKIA5110_StatusTypeDef NOKIA5110_WriteDataMulti(const uint8_t *data, uint16_t size)
{
    HAL_GPIO_WritePin(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN, GPIO_PIN_SET);  /* Data mode */
    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_RESET);  /* Chip enable */

    SPI_StatusTypeDef spi_status = SPI_Transmit((uint8_t*)data, size, NOKIA5110_SPI_TIMEOUT);

    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_SET);  /* Chip disable */

    return (spi_status == SPI_OK) ? NOKIA5110_OK : NOKIA5110_ERROR;
}
