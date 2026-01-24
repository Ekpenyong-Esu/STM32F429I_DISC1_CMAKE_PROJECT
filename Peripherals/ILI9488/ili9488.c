/**
  ******************************************************************************
  * @file    ili9488.c
  * @brief   ILI9488 TFT LCD Display Driver Implementation
  * @details This file provides the implementation of ILI9488 TFT functions
  *          using SPI interface on STM32F429I-DISC1.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ili9488.h"
#include "spi.h"
#include "gpio.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup ILI9488_Private_Defines Private Defines
 * @{
 */

/* ILI9488 Commands */
#define ILI9488_CMD_NOP                 0x00
#define ILI9488_CMD_SOFTWARE_RESET      0x01
#define ILI9488_CMD_READ_DISP_ID        0x04
#define ILI9488_CMD_READ_DISP_STATUS    0x09
#define ILI9488_CMD_READ_DISP_POWER     0x0A
#define ILI9488_CMD_READ_DISP_MADCTL    0x0B
#define ILI9488_CMD_READ_DISP_PIXEL     0x0C
#define ILI9488_CMD_READ_DISP_IMAGE     0x0D
#define ILI9488_CMD_READ_DISP_SIGNAL    0x0E
#define ILI9488_CMD_READ_DISP_DIAG      0x0F
#define ILI9488_CMD_SLEEP_IN            0x10
#define ILI9488_CMD_SLEEP_OUT           0x11
#define ILI9488_CMD_PARTIAL_MODE_ON     0x12
#define ILI9488_CMD_NORMAL_DISP_ON      0x13
#define ILI9488_CMD_DISP_INVERSION_OFF  0x20
#define ILI9488_CMD_DISP_INVERSION_ON   0x21
#define ILI9488_CMD_ALL_PIXEL_OFF       0x22
#define ILI9488_CMD_ALL_PIXEL_ON        0x23
#define ILI9488_CMD_DISPLAY_OFF         0x28
#define ILI9488_CMD_DISPLAY_ON          0x29
#define ILI9488_CMD_COLUMN_ADDR_SET     0x2A
#define ILI9488_CMD_PAGE_ADDR_SET       0x2B
#define ILI9488_CMD_MEMORY_WRITE        0x2C
#define ILI9488_CMD_MEMORY_READ         0x2E
#define ILI9488_CMD_PARTIAL_AREA        0x30
#define ILI9488_CMD_VERT_SCROLL_DEF     0x33
#define ILI9488_CMD_TEARING_OFF         0x34
#define ILI9488_CMD_TEARING_ON          0x35
#define ILI9488_CMD_MEMORY_ACCESS_CTL   0x36
#define ILI9488_CMD_VERT_SCROLL_START   0x37
#define ILI9488_CMD_IDLE_MODE_OFF       0x38
#define ILI9488_CMD_IDLE_MODE_ON        0x39
#define ILI9488_CMD_PIXEL_FORMAT_SET    0x3A
#define ILI9488_CMD_WRITE_MEMORY_CONT   0x3C
#define ILI9488_CMD_READ_MEMORY_CONT    0x3E
#define ILI9488_CMD_SET_TEAR_SCANLINE   0x44
#define ILI9488_CMD_GET_SCANLINE        0x45
#define ILI9488_CMD_WRITE_DISP_BRIGHT   0x51
#define ILI9488_CMD_READ_DISP_BRIGHT    0x52
#define ILI9488_CMD_WRITE_CTRL_DISP     0x53
#define ILI9488_CMD_READ_CTRL_DISP      0x54
#define ILI9488_CMD_WRITE_ADAPT_BRIGHT  0x55
#define ILI9488_CMD_READ_ADAPT_BRIGHT   0x56
#define ILI9488_CMD_WRITE_CABC_MIN      0x5E
#define ILI9488_CMD_READ_CABC_MIN       0x5F
#define ILI9488_CMD_READ_ID1            0xDA
#define ILI9488_CMD_READ_ID2            0xDB
#define ILI9488_CMD_READ_ID3            0xDC

/* Extended commands */
#define ILI9488_CMD_INTERFACE_MODE      0xB0
#define ILI9488_CMD_FRAME_RATE_NORMAL   0xB1
#define ILI9488_CMD_FRAME_RATE_IDLE     0xB2
#define ILI9488_CMD_FRAME_RATE_PARTIAL  0xB3
#define ILI9488_CMD_INVERSION_CONTROL   0xB4
#define ILI9488_CMD_BLANKING_PORCH      0xB5
#define ILI9488_CMD_DISPLAY_FUNCTION    0xB6
#define ILI9488_CMD_ENTRY_MODE_SET      0xB7
#define ILI9488_CMD_BACKLIGHT_CONTROL1  0xB8
#define ILI9488_CMD_BACKLIGHT_CONTROL2  0xB9
#define ILI9488_CMD_BACKLIGHT_CONTROL3  0xBA
#define ILI9488_CMD_BACKLIGHT_CONTROL4  0xBB
#define ILI9488_CMD_BACKLIGHT_CONTROL5  0xBC
#define ILI9488_CMD_BACKLIGHT_CONTROL7  0xBE
#define ILI9488_CMD_BACKLIGHT_CONTROL8  0xBF
#define ILI9488_CMD_POWER_CONTROL1       0xC0
#define ILI9488_CMD_POWER_CONTROL2       0xC1
#define ILI9488_CMD_POWER_CONTROL3       0xC2
#define ILI9488_CMD_POWER_CONTROL4       0xC3
#define ILI9488_CMD_POWER_CONTROL5       0xC4
#define ILI9488_CMD_VCOM_CONTROL1        0xC5
#define ILI9488_CMD_CABC_CONTROL1        0xC6
#define ILI9488_CMD_CABC_CONTROL2        0xC8
#define ILI9488_CMD_CABC_CONTROL3        0xC9
#define ILI9488_CMD_CABC_CONTROL4        0xCA
#define ILI9488_CMD_CABC_CONTROL5        0xCB
#define ILI9488_CMD_CABC_CONTROL6        0xCC
#define ILI9488_CMD_CABC_CONTROL7        0xCD
#define ILI9488_CMD_CABC_CONTROL8        0xCE
#define ILI9488_CMD_CABC_CONTROL9        0xCF
#define ILI9488_CMD_POSITIVE_GAMMA       0xE0
#define ILI9488_CMD_NEGATIVE_GAMMA       0xE1
#define ILI9488_CMD_DIGITAL_GAMMA1       0xE2
#define ILI9488_CMD_DIGITAL_GAMMA2       0xE3
#define ILI9488_CMD_SET_IMAGE_FUNCTION   0xE9
#define ILI9488_CMD_ADJUST_CONTROL1      0xE9
#define ILI9488_CMD_ADJUST_CONTROL2      0xEA
#define ILI9488_CMD_ADJUST_CONTROL3      0xEB
#define ILI9488_CMD_ADJUST_CONTROL4      0xEC
#define ILI9488_CMD_ADJUST_CONTROL5      0xED
#define ILI9488_CMD_SPI_READ_CMD         0xFB
#define ILI9488_CMD_SPI_TIMING1          0xFC
#define ILI9488_CMD_SPI_TIMING2          0xFD

/* Font data (6x8 font) */
static const uint8_t font6x8[96][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62, 0x00}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50, 0x00}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14, 0x00}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08, 0x00}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46, 0x00}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03, 0x00}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14, 0x00}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06, 0x00}, // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E, 0x00}, // @
    {0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01, 0x00}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06, 0x00}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31, 0x00}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01, 0x00}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63, 0x00}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07, 0x00}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43, 0x00}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20, 0x00}, // backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04, 0x00}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78, 0x00}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38, 0x00}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20, 0x00}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F, 0x00}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18, 0x00}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02, 0x00}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78, 0x00}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78, 0x00}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78, 0x00}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38, 0x00}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08, 0x00}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C, 0x00}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08, 0x00}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20, 0x00}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20, 0x00}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44, 0x00}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44, 0x00}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00, 0x00}, // }
    {0x10, 0x08, 0x08, 0x10, 0x08, 0x00}, // ~
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // DEL
};

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static ILI9488_StatusTypeDef ILI9488_WriteCommand(ILI9488_Handle_t *hili, uint8_t command);
static ILI9488_StatusTypeDef ILI9488_WriteData(ILI9488_Handle_t *hili, uint8_t *data, uint16_t size);
static ILI9488_StatusTypeDef ILI9488_WriteData16(ILI9488_Handle_t *hili, uint16_t *data, uint32_t size);
static ILI9488_StatusTypeDef ILI9488_SetAddressWindow(ILI9488_Handle_t *hili, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void ILI9488_Delay(uint32_t delay);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize ILI9488 TFT display
 * @details Configures SPI and initializes the display
 * @param   hili Pointer to ILI9488 handle
 * @param   hspi Pointer to SPI handle
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   dc_port Data/command port
 * @param   dc_pin Data/command pin
 * @param   rst_port Reset port
 * @param   rst_pin Reset pin
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_Init(ILI9488_Handle_t *hili,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *dc_port, uint16_t dc_pin,
                                  GPIO_TypeDef *rst_port, uint16_t rst_pin)
{
    if (hili == NULL) {
        return ILI9488_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hili, 0, sizeof(ILI9488_Handle_t));

    hili->config.cs_port = cs_port;
    hili->config.cs_pin = cs_pin;
    hili->config.dc_port = dc_port;
    hili->config.dc_pin = dc_pin;
    hili->config.rst_port = rst_port;
    hili->config.rst_pin = rst_pin;
    hili->config.orientation = ILI9488_ORIENTATION_PORTRAIT;

    hili->width = ILI9488_WIDTH;
    hili->height = ILI9488_HEIGHT;

    /* Configure GPIO pins */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Chip select pin */
    GPIO_InitStruct.Pin = cs_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(cs_port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET); // Deselect

    /* Data/Command pin */
    GPIO_InitStruct.Pin = dc_pin;
    HAL_GPIO_Init(dc_port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(dc_port, dc_pin, GPIO_PIN_RESET); // Command mode

    /* Reset pin */
    GPIO_InitStruct.Pin = rst_pin;
    HAL_GPIO_Init(rst_port, &GPIO_InitStruct);

    /* Hardware reset */
    HAL_GPIO_WritePin(rst_port, rst_pin, GPIO_PIN_RESET);
    ILI9488_Delay(100);
    HAL_GPIO_WritePin(rst_port, rst_pin, GPIO_PIN_SET);
    ILI9488_Delay(100);

    /* Software reset */
    ILI9488_WriteCommand(hili, ILI9488_CMD_SOFTWARE_RESET);
    ILI9488_Delay(150);

    /* Exit sleep mode */
    ILI9488_WriteCommand(hili, ILI9488_CMD_SLEEP_OUT);
    ILI9488_Delay(150);

    /* Power control settings */
    ILI9488_WriteCommand(hili, ILI9488_CMD_POWER_CONTROL1);
    ILI9488_WriteData(hili, (uint8_t[]){0x17, 0x15}, 2);

    ILI9488_WriteCommand(hili, ILI9488_CMD_POWER_CONTROL2);
    ILI9488_WriteData(hili, (uint8_t[]){0x41}, 1);

    /* VCOM control */
    ILI9488_WriteCommand(hili, ILI9488_CMD_VCOM_CONTROL1);
    ILI9488_WriteData(hili, (uint8_t[]){0x00, 0x12, 0x80}, 3);

    /* Memory access control */
    ILI9488_WriteCommand(hili, ILI9488_CMD_MEMORY_ACCESS_CTL);
    ILI9488_WriteData(hili, (uint8_t[]){0x48}, 1); // Portrait mode

    /* Pixel format */
    ILI9488_WriteCommand(hili, ILI9488_CMD_PIXEL_FORMAT_SET);
    ILI9488_WriteData(hili, (uint8_t[]){0x55}, 1); // 16-bit RGB565

    /* Frame rate */
    ILI9488_WriteCommand(hili, ILI9488_CMD_FRAME_RATE_NORMAL);
    ILI9488_WriteData(hili, (uint8_t[]){0xA0}, 1); // 60Hz

    /* Gamma settings */
    ILI9488_WriteCommand(hili, ILI9488_CMD_POSITIVE_GAMMA);
    ILI9488_WriteData(hili, (uint8_t[]){0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98, 0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00}, 15);

    ILI9488_WriteCommand(hili, ILI9488_CMD_NEGATIVE_GAMMA);
    ILI9488_WriteData(hili, (uint8_t[]){0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75, 0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00}, 15);

    /* Display inversion off */
    ILI9488_WriteCommand(hili, ILI9488_CMD_DISP_INVERSION_OFF);

    /* Turn display on */
    ILI9488_WriteCommand(hili, ILI9488_CMD_DISPLAY_ON);
    ILI9488_Delay(100);

    hili->initialized = true;

    return ILI9488_OK;
}

/**
 * @brief   Set display orientation
 * @param   hili Pointer to ILI9488 handle
 * @param   orientation Display orientation
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_SetOrientation(ILI9488_Handle_t *hili, ILI9488_Orientation_t orientation)
{
    if (hili == NULL || !hili->initialized) {
        return ILI9488_NOT_INITIALIZED;
    }

    uint8_t madctl = 0;

    switch (orientation) {
        case ILI9488_ORIENTATION_PORTRAIT:
            madctl = 0x48;
            hili->width = ILI9488_WIDTH;
            hili->height = ILI9488_HEIGHT;
            break;
        case ILI9488_ORIENTATION_LANDSCAPE:
            madctl = 0x28;
            hili->width = ILI9488_HEIGHT;
            hili->height = ILI9488_WIDTH;
            break;
        case ILI9488_ORIENTATION_PORTRAIT_REV:
            madctl = 0x88;
            hili->width = ILI9488_WIDTH;
            hili->height = ILI9488_HEIGHT;
            break;
        case ILI9488_ORIENTATION_LANDSCAPE_REV:
            madctl = 0xE8;
            hili->width = ILI9488_HEIGHT;
            hili->height = ILI9488_WIDTH;
            break;
    }

    ILI9488_WriteCommand(hili, ILI9488_CMD_MEMORY_ACCESS_CTL);
    ILI9488_WriteData(hili, &madctl, 1);

    hili->config.orientation = orientation;

    return ILI9488_OK;
}

/**
 * @brief   Clear display
 * @param   hili Pointer to ILI9488 handle
 * @param   color Fill color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_Clear(ILI9488_Handle_t *hili, uint16_t color)
{
    if (hili == NULL || !hili->initialized) {
        return ILI9488_NOT_INITIALIZED;
    }

    ILI9488_SetAddressWindow(hili, 0, 0, hili->width - 1, hili->height - 1);

    ILI9488_WriteCommand(hili, ILI9488_CMD_MEMORY_WRITE);

    /* Fill with color */
    uint16_t buffer[ILI9488_WIDTH];
    for (uint16_t i = 0; i < ILI9488_WIDTH; i++) {
        buffer[i] = color;
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_RESET);

    for (uint16_t y = 0; y < hili->height; y++) {
        ILI9488_WriteData16(hili, buffer, ILI9488_WIDTH);
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);

    return ILI9488_OK;
}

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
                                       uint16_t color)
{
    if (hili == NULL || !hili->initialized || x >= hili->width || y >= hili->height) {
        return ILI9488_INVALID_PARAM;
    }

    ILI9488_SetAddressWindow(hili, x, y, x, y);
    ILI9488_WriteCommand(hili, ILI9488_CMD_MEMORY_WRITE);
    ILI9488_WriteData16(hili, &color, 1);

    return ILI9488_OK;
}

/**
 * @brief   Write character
 * @param   hili Pointer to ILI9488 handle
 * @param   ch Character to write
 * @param   color Character color
 * @param   bgcolor Background color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_WriteChar(ILI9488_Handle_t *hili, char ch, uint16_t color, uint16_t bgcolor)
{
    if (hili == NULL || !hili->initialized) {
        return ILI9488_NOT_INITIALIZED;
    }

    if (hili->currentX + 6 > hili->width) {
        hili->currentX = 0;
        hili->currentY += 8;
    }

    if (hili->currentY + 8 > hili->height) {
        return ILI9488_INVALID_PARAM;
    }

    uint8_t charIndex = ch - 32;
    if (charIndex >= 96) charIndex = 0;

    for (uint8_t i = 0; i < 6; i++) {
        uint8_t line = font6x8[charIndex][i];
        for (uint8_t j = 0; j < 8; j++) {
            if (line & (1 << j)) {
                ILI9488_DrawPixel(hili, hili->currentX + i, hili->currentY + j, color);
            } else if (bgcolor != color) {
                ILI9488_DrawPixel(hili, hili->currentX + i, hili->currentY + j, bgcolor);
            }
        }
    }

    hili->currentX += 6;

    return ILI9488_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Write command to ILI9488
 * @param   hili Pointer to ILI9488 handle
 * @param   command Command byte
 * @retval  ILI9488_StatusTypeDef Operation status
 */
static ILI9488_StatusTypeDef ILI9488_WriteCommand(ILI9488_Handle_t *hili, uint8_t command)
{
    HAL_GPIO_WritePin(hili->config.dc_port, hili->config.dc_pin, GPIO_PIN_RESET); // Command mode
    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_RESET);

    if (SPI_Transmit(&command, 1, SPI_TIMEOUT_SHORT) != SPI_OK) {
        HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
        return ILI9488_ERROR;
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
    return ILI9488_OK;
}

/**
 * @brief   Write data to ILI9488
 * @param   hili Pointer to ILI9488 handle
 * @param   data Data buffer
 * @param   size Data size
 * @retval  ILI9488_StatusTypeDef Operation status
 */
static ILI9488_StatusTypeDef ILI9488_WriteData(ILI9488_Handle_t *hili, uint8_t *data, uint16_t size)
{
    HAL_GPIO_WritePin(hili->config.dc_port, hili->config.dc_pin, GPIO_PIN_SET); // Data mode
    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_RESET);

    if (SPI_Transmit(data, size, SPI_TIMEOUT_LONG) != SPI_OK) {
        HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
        return ILI9488_ERROR;
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
    return ILI9488_OK;
}

/**
 * @brief   Write 16-bit data to ILI9488
 * @param   hili Pointer to ILI9488 handle
 * @param   data 16-bit data buffer
 * @param   size Data size in 16-bit words
 * @retval  ILI9488_StatusTypeDef Operation status
 */
static ILI9488_StatusTypeDef ILI9488_WriteData16(ILI9488_Handle_t *hili, uint16_t *data, uint32_t size)
{
    HAL_GPIO_WritePin(hili->config.dc_port, hili->config.dc_pin, GPIO_PIN_SET); // Data mode
    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_RESET);

    /* Convert 16-bit to 8-bit for SPI transmission */
    for (uint32_t i = 0; i < size; i++) {
        uint8_t bytes[2] = {(uint8_t)(data[i] >> 8), (uint8_t)(data[i] & 0xFF)};
        if (SPI_Transmit(bytes, 2, SPI_TIMEOUT_SHORT) != SPI_OK) {
            HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
            return ILI9488_ERROR;
        }
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
    return ILI9488_OK;
}

/**
 * @brief   Set address window
 * @param   hili Pointer to ILI9488 handle
 * @param   x0 Start X coordinate
 * @param   y0 Start Y coordinate
 * @param   x1 End X coordinate
 * @param   y1 End Y coordinate
 * @retval  ILI9488_StatusTypeDef Operation status
 */
static ILI9488_StatusTypeDef ILI9488_SetAddressWindow(ILI9488_Handle_t *hili, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    /* Column address set */
    ILI9488_WriteCommand(hili, ILI9488_CMD_COLUMN_ADDR_SET);
    uint8_t colData[] = {(uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF), (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF)};
    ILI9488_WriteData(hili, colData, 4);

    /* Row address set */
    ILI9488_WriteCommand(hili, ILI9488_CMD_PAGE_ADDR_SET);
    uint8_t rowData[] = {(uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF), (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF)};
    ILI9488_WriteData(hili, rowData, 4);

    return ILI9488_OK;
}

/**
 * @brief   Delay function
 * @param   delay Delay in milliseconds
 */
static void ILI9488_Delay(uint32_t delay)
{
    HAL_Delay(delay);
}
