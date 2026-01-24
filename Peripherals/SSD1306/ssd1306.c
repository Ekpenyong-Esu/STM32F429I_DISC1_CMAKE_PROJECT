/**
  ******************************************************************************
  * @file    ssd1306.c
  * @brief   SSD1306 OLED Display Driver Implementation
  * @details This file provides the implementation of SSD1306 OLED functions
  *          using I2C interface on STM32F429I-DISC1.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ssd1306.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Private variables ---------------------------------------------------------*/

/** @defgroup SSD1306_Private_Variables Private Variables
 * @{
 */

/* Global display buffer - shared across all SSD1306 instances */
uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

/* Static buffer for I2C transmissions - avoids dynamic allocation */
static uint8_t ssd1306_tx_buffer[SSD1306_BUFFER_SIZE + 1];

/* Global SSD1306 handle for interrupt handling */
static SSD1306_Handle_t *g_hssd = NULL;

/** @} */

/* Private defines -----------------------------------------------------------*/

/** @defgroup SSD1306_Private_Defines Private Defines
 * @{
 */

/* SSD1306 Commands */
#define SSD1306_CMD_SET_CONTRAST        0x81
#define SSD1306_CMD_DISPLAY_ALL_ON_RESUME 0xA4
#define SSD1306_CMD_DISPLAY_ALL_ON       0xA5
#define SSD1306_CMD_NORMAL_DISPLAY       0xA6
#define SSD1306_CMD_INVERT_DISPLAY       0xA7
#define SSD1306_CMD_DISPLAY_OFF          0xAE
#define SSD1306_CMD_DISPLAY_ON           0xAF
#define SSD1306_CMD_SET_DISPLAY_OFFSET   0xD3
#define SSD1306_CMD_SET_COM_PINS         0xDA
#define SSD1306_CMD_SET_VCOM_DETECT      0xDB
#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIV 0xD5
#define SSD1306_CMD_SET_PRECHARGE        0xD9
#define SSD1306_CMD_SET_MULTIPLEX        0xA8
#define SSD1306_CMD_SET_LOW_COLUMN       0x00
#define SSD1306_CMD_SET_HIGH_COLUMN      0x10
#define SSD1306_CMD_SET_START_LINE       0x40
#define SSD1306_CMD_MEMORY_MODE          0x20
#define SSD1306_CMD_COLUMN_ADDR          0x21
#define SSD1306_CMD_PAGE_ADDR            0x22
#define SSD1306_CMD_COM_SCAN_INC         0xC0
#define SSD1306_CMD_COM_SCAN_DEC         0xC8
#define SSD1306_CMD_SEG_REMAP            0xA0
#define SSD1306_CMD_CHARGE_PUMP          0x8D

/* Font data */
static const uint8_t font6x8[96][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00}, // "
    // ... (truncated for brevity - full font data would be here)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // DEL
};

/* Private variables ---------------------------------------------------------*/

/** @defgroup SSD1306_Private_Variables Private Variables
 * @{
 */



/** @} */

/* Private function prototypes -----------------------------------------------*/
static SSD1306_StatusTypeDef SSD1306_WriteCommand(SSD1306_Handle_t *hssd, uint8_t command);
static SSD1306_StatusTypeDef SSD1306_WriteData(SSD1306_Handle_t *hssd, uint8_t *data, uint16_t size);
static uint8_t SSD1306_GetFontWidth(SSD1306_FontSize_t fontSize);
static uint8_t SSD1306_GetFontHeight(SSD1306_FontSize_t fontSize);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize SSD1306 OLED display
 * @details Configures I2C and initializes the display
 * @param   hssd Pointer to SSD1306 handle
 * @param   address I2C address (0x3C or 0x3D)
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Init(SSD1306_Handle_t *hssd, uint8_t address)
{
    if (hssd == NULL) {
        log_error("SSD1306: Invalid handle pointer");
        return SSD1306_INVALID_PARAM;
    }

    log_info("SSD1306: Initializing display at I2C address 0x%02X", address);

    /* Initialize structure */
    memset(hssd, 0, sizeof(SSD1306_Handle_t));
    hssd->config.address = address;
    g_hssd = hssd;

    /* Initialize display */
    SSD1306_StatusTypeDef status = SSD1306_OK;

    // Display off
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_DISPLAY_OFF);
    if (status != SSD1306_OK) return status;

    // Set display clock divide ratio/oscillator frequency
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_DISPLAY_CLOCK_DIV);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x80);
    if (status != SSD1306_OK) return status;

    // Set multiplex ratio
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_MULTIPLEX);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x3F);
    if (status != SSD1306_OK) return status;

    // Set display offset
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_DISPLAY_OFFSET);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x00);
    if (status != SSD1306_OK) return status;

    // Set start line
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_START_LINE | 0x00);
    if (status != SSD1306_OK) return status;

    // Set charge pump
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_CHARGE_PUMP);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x14);
    if (status != SSD1306_OK) return status;

    // Set memory mode
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_MEMORY_MODE);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x00);
    if (status != SSD1306_OK) return status;

    // Set segment re-map
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SEG_REMAP | 0x01);
    if (status != SSD1306_OK) return status;

    // Set COM output scan direction
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_COM_SCAN_DEC);
    if (status != SSD1306_OK) return status;

    // Set COM pins hardware configuration
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_COM_PINS);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x12);
    if (status != SSD1306_OK) return status;

    // Set contrast control
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_CONTRAST);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0xCF);
    if (status != SSD1306_OK) return status;

    // Set pre-charge period
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_PRECHARGE);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0xF1);
    if (status != SSD1306_OK) return status;

    // Set VCOMH deselect level
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_VCOM_DETECT);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x40);
    if (status != SSD1306_OK) return status;

    // Set entire display on/off
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_DISPLAY_ALL_ON_RESUME);
    if (status != SSD1306_OK) return status;

    // Set normal display
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_NORMAL_DISPLAY);
    if (status != SSD1306_OK) return status;

    // Clear buffer
    SSD1306_Clear(hssd);

    // Display on
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_DISPLAY_ON);
    if (status != SSD1306_OK) return status;

    hssd->initialized = true;
    hssd->fontSize = SSD1306_FONT_6x8;

    log_info("SSD1306: Display initialized successfully");
    return SSD1306_OK;
}

/**
 * @brief   Update display with buffer content
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_UpdateScreen(SSD1306_Handle_t *hssd)
{
    if (hssd == NULL || !hssd->initialized) {
        log_error("SSD1306: Display not initialized");
        return SSD1306_NOT_INITIALIZED;
    }

    log_debug("SSD1306: Updating screen with buffer data");
    SSD1306_StatusTypeDef status;

    // Set column address
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_COLUMN_ADDR);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, SSD1306_WIDTH - 1);
    if (status != SSD1306_OK) return status;

    // Set page address
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_PAGE_ADDR);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, (SSD1306_HEIGHT / 8) - 1);
    if (status != SSD1306_OK) return status;

    // Write display buffer
    status = SSD1306_WriteData(hssd, SSD1306_Buffer, SSD1306_BUFFER_SIZE);
    if (status == SSD1306_OK) {
        log_debug("SSD1306: Screen updated successfully");
    }

    return status;
}

/**
 * @brief   Clear display buffer
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Clear(SSD1306_Handle_t *hssd)
{
    if (hssd == NULL) {
        log_error("SSD1306: Invalid handle pointer for clear operation");
        return SSD1306_INVALID_PARAM;
    }

    memset(SSD1306_Buffer, 0, SSD1306_BUFFER_SIZE);
    hssd->currentX = 0;
    hssd->currentY = 0;

    log_debug("SSD1306: Display buffer cleared");
    return SSD1306_OK;
}

/**
 * @brief   Fill display buffer with color
 * @param   hssd Pointer to SSD1306 handle
 * @param   color Fill color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Fill(SSD1306_Handle_t *hssd, SSD1306_Color_t color)
{
    if (hssd == NULL) {
        log_error("SSD1306: Invalid handle pointer for fill operation");
        return SSD1306_INVALID_PARAM;
    }

    uint8_t fillValue = (color == SSD1306_COLOR_WHITE) ? 0xFF : 0x00;
    memset(SSD1306_Buffer, fillValue, SSD1306_BUFFER_SIZE);

    log_debug("SSD1306: Display buffer filled with %s", color == SSD1306_COLOR_WHITE ? "white" : "black");
    return SSD1306_OK;
}

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
                                       SSD1306_Color_t color)
{
    if (hssd == NULL || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        log_error("SSD1306: Invalid parameters for DrawPixel - x:%d, y:%d, max:%dx%d", x, y, SSD1306_WIDTH, SSD1306_HEIGHT);
        return SSD1306_INVALID_PARAM;
    }

    if (color == SSD1306_COLOR_WHITE) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }

    return SSD1306_OK;
}

/**
 * @brief   Write character
 * @param   hssd Pointer to SSD1306 handle
 * @param   ch Character to write
 * @param   color Character color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_WriteChar(SSD1306_Handle_t *hssd, char ch, SSD1306_Color_t color)
{
    if (hssd == NULL || !hssd->initialized) {
        return SSD1306_NOT_INITIALIZED;
    }

    uint8_t fontWidth = SSD1306_GetFontWidth(hssd->fontSize);
    uint8_t fontHeight = SSD1306_GetFontHeight(hssd->fontSize);

    if (hssd->currentX + fontWidth > SSD1306_WIDTH) {
        hssd->currentX = 0;
        hssd->currentY += fontHeight;
    }

    if (hssd->currentY + fontHeight > SSD1306_HEIGHT) {
        return SSD1306_INVALID_PARAM;
    }

    // Draw character (simplified - would need full font data)
    for (uint8_t i = 0; i < fontWidth; i++) {
        uint8_t line = 0; // Get from font data
        for (uint8_t j = 0; j < 8; j++) {
            if (line & (1 << j)) {
                SSD1306_DrawPixel(hssd, hssd->currentX + i, hssd->currentY + j, color);
            }
        }
    }

    hssd->currentX += fontWidth;

    return SSD1306_OK;
}

/**
 * @brief   Write string
 * @param   hssd Pointer to SSD1306 handle
 * @param   str String to write
 * @param   color String color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_WriteString(SSD1306_Handle_t *hssd, const char *str, SSD1306_Color_t color)
{
    if (hssd == NULL || str == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    while (*str) {
        if (SSD1306_WriteChar(hssd, *str, color) != SSD1306_OK) {
            return SSD1306_ERROR;
        }
        str++;
    }

    return SSD1306_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Write command to SSD1306
 * @param   hssd Pointer to SSD1306 handle
 * @param   command Command byte
 * @retval  SSD1306_StatusTypeDef Operation status
 */
static SSD1306_StatusTypeDef SSD1306_WriteCommand(SSD1306_Handle_t *hssd, uint8_t command)
{
    uint8_t data[2] = {0x00, command}; // 0x00 = command mode

    if (I2C_Master_Transmit(hssd->config.address, data, 2, I2C_TIMEOUT_DEFAULT) != I2C_OK) {
        log_error("SSD1306: Failed to write command 0x%02X", command);
        return SSD1306_ERROR;
    }

    return SSD1306_OK;
}

/**
 * @brief   Write data to SSD1306
 * @param   hssd Pointer to SSD1306 handle
 * @param   data Data buffer
 * @param   size Data size
 * @retval  SSD1306_StatusTypeDef Operation status
 */
static SSD1306_StatusTypeDef SSD1306_WriteData(SSD1306_Handle_t *hssd, uint8_t *data, uint16_t size)
{
    if (size > SSD1306_BUFFER_SIZE) {
        log_error("SSD1306: Data size %d exceeds buffer size %d", size, SSD1306_BUFFER_SIZE);
        return SSD1306_INVALID_PARAM;
    }

    ssd1306_tx_buffer[0] = 0x40; // Data mode
    memcpy(&ssd1306_tx_buffer[1], data, size);

    if (I2C_Master_Transmit(hssd->config.address, ssd1306_tx_buffer, size + 1, I2C_TIMEOUT_DEFAULT) != I2C_OK) {
        log_error("SSD1306: Failed to write %d bytes of data", size);
        return SSD1306_ERROR;
    }

    return SSD1306_OK;
}

/**
 * @brief   Get font width
 * @param   fontSize Font size
 * @retval  uint8_t Font width
 */
static uint8_t SSD1306_GetFontWidth(SSD1306_FontSize_t fontSize)
{
    switch (fontSize) {
        case SSD1306_FONT_6x8: return 6;
        case SSD1306_FONT_8x16: return 8;
        case SSD1306_FONT_12x16: return 12;
        case SSD1306_FONT_16x24: return 16;
        default: return 6;
    }
}

/**
 * @brief   Get font height
 * @param   fontSize Font size
 * @retval  uint8_t Font height
 */
static uint8_t SSD1306_GetFontHeight(SSD1306_FontSize_t fontSize)
{
    switch (fontSize) {
        case SSD1306_FONT_6x8: return 8;
        case SSD1306_FONT_8x16: return 16;
        case SSD1306_FONT_12x16: return 16;
        case SSD1306_FONT_16x24: return 24;
        default: return 8;
    }
}
