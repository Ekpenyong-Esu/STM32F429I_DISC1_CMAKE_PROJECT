/**
  ******************************************************************************
  * @file    ili9488.c
  * @brief   ILI9488 TFT LCD Display Driver Implementation
  * @details This file provides the implementation of ILI9488 TFT functions
  *          using SPI interface on STM32F429I-DISC1.
  * @version 1.1
  * @date    2025-02-08
  * @note    Fixed for proper LVGL integration with RGB666 support
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ili9488.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Default weak MSP implementations (board can override) */
__weak void ILI9488_MspInit(void)
{
    /* Default no-op: board-specific file should configure GPIO clocks/pins */
}

__weak void ILI9488_MspDeInit(void)
{
    /* Default no-op */
}

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
#define ILI9488_CMD_POWER_CONTROL1      0xC0
#define ILI9488_CMD_POWER_CONTROL2      0xC1
#define ILI9488_CMD_POWER_CONTROL3      0xC2
#define ILI9488_CMD_POWER_CONTROL4      0xC3
#define ILI9488_CMD_POWER_CONTROL5      0xC4
#define ILI9488_CMD_VCOM_CONTROL1       0xC5
#define ILI9488_CMD_CABC_CONTROL1       0xC6
#define ILI9488_CMD_CABC_CONTROL2       0xC8
#define ILI9488_CMD_CABC_CONTROL3       0xC9
#define ILI9488_CMD_CABC_CONTROL4       0xCA
#define ILI9488_CMD_CABC_CONTROL5       0xCB
#define ILI9488_CMD_CABC_CONTROL6       0xCC
#define ILI9488_CMD_CABC_CONTROL7       0xCD
#define ILI9488_CMD_CABC_CONTROL8       0xCE
#define ILI9488_CMD_CABC_CONTROL9       0xCF
#define ILI9488_CMD_POSITIVE_GAMMA      0xE0
#define ILI9488_CMD_NEGATIVE_GAMMA      0xE1
#define ILI9488_CMD_DIGITAL_GAMMA1      0xE2
#define ILI9488_CMD_DIGITAL_GAMMA2      0xE3
#define ILI9488_CMD_SET_IMAGE_FUNCTION  0xE9
#define ILI9488_CMD_ADJUST_CONTROL1     0xE9
#define ILI9488_CMD_ADJUST_CONTROL2     0xEA
#define ILI9488_CMD_ADJUST_CONTROL3     0xEB
#define ILI9488_CMD_ADJUST_CONTROL4     0xEC
#define ILI9488_CMD_ADJUST_CONTROL5     0xED
#define ILI9488_CMD_ADJUST_CONTROL3_F7  0xF7
#define ILI9488_CMD_SPI_READ_CMD        0xFB
#define ILI9488_CMD_SPI_TIMING1         0xFC
#define ILI9488_CMD_SPI_TIMING2         0xFD

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
 * @details Configures SPI and initializes the display with improved init sequence
 * @param   hili Pointer to ILI9488 handle
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
                                  GPIO_TypeDef *rst_port, uint16_t rst_pin,
                                  uint16_t width, uint16_t height)
{
    if (hili == NULL) {
        return ILI9488_INVALID_PARAM;
    }

    /* Board-specific MSP hook (GPIO clocks/pins) */
    ILI9488_MspInit();

    /* Initialize structure */
    memset(hili, 0, sizeof(ILI9488_Handle_t));

    hili->config.cs_port = cs_port;
    hili->config.cs_pin = cs_pin;
    hili->config.dc_port = dc_port;
    hili->config.dc_pin = dc_pin;
    hili->config.rst_port = rst_port;
    hili->config.rst_pin = rst_pin;
    hili->config.orientation = ILI9488_ORIENTATION_PORTRAIT; // Default to portrait, will switch to landscape in init

    /* Store base dimensions (portrait mode) */
    hili->base_width = width;
    hili->base_height = height;
    hili->width = width;
    hili->height = height;

    /* Hardware reset */
    HAL_GPIO_WritePin(rst_port, rst_pin, GPIO_PIN_RESET);
    ILI9488_Delay(10);
    HAL_GPIO_WritePin(rst_port, rst_pin, GPIO_PIN_SET);
    ILI9488_Delay(120);

    /* Software reset */
    ILI9488_WriteCommand(hili, ILI9488_CMD_SOFTWARE_RESET);
    ILI9488_Delay(150);

    /* Read display ID to verify communication */
    uint8_t id_buf[4] = {0};
    if (ILI9488_ReadID(hili, id_buf, 4) == ILI9488_OK) {
        log_debug("ILI9488: Display ID = 0x%02X 0x%02X 0x%02X 0x%02X",
                  id_buf[0], id_buf[1], id_buf[2], id_buf[3]);
    }

    /* Exit sleep mode */
    ILI9488_WriteCommand(hili, ILI9488_CMD_SLEEP_OUT);
    ILI9488_Delay(120);

    /* Positive Gamma Control */
    ILI9488_WriteCommand(hili, ILI9488_CMD_POSITIVE_GAMMA);
    ILI9488_WriteData(hili, (uint8_t[]){0x00, 0x03, 0x09, 0x08, 0x16, 0x0A,
                                        0x3F, 0x78, 0x4C, 0x09, 0x0A, 0x08,
                                        0x16, 0x1A, 0x0F}, 15);

    /* Negative Gamma Control */
    ILI9488_WriteCommand(hili, ILI9488_CMD_NEGATIVE_GAMMA);
    ILI9488_WriteData(hili, (uint8_t[]){0x00, 0x16, 0x19, 0x03, 0x0F, 0x05,
                                        0x32, 0x45, 0x46, 0x04, 0x0E, 0x0D,
                                        0x35, 0x37, 0x0F}, 15);

    /* Power Control 1 (VRH[5:0]) */
    ILI9488_WriteCommand(hili, ILI9488_CMD_POWER_CONTROL1);
    ILI9488_WriteData(hili, (uint8_t[]){0x17, 0x15}, 2);

    /* Power Control 2 (SAP[2:0];BT[3:0]) */
    ILI9488_WriteCommand(hili, ILI9488_CMD_POWER_CONTROL2);
    ILI9488_WriteData(hili, (uint8_t[]){0x41}, 1);

    /* VCOM Control */
    ILI9488_WriteCommand(hili, ILI9488_CMD_VCOM_CONTROL1);
    ILI9488_WriteData(hili, (uint8_t[]){0x00, 0x12, 0x80}, 3);

    /* Memory Access Control - Use portrait mode to match LVGL (0x48) */
    ILI9488_WriteCommand(hili, ILI9488_CMD_MEMORY_ACCESS_CTL);
    ILI9488_WriteData(hili, (uint8_t[]){0x48}, 1); // Portrait: MY=0, MX=1, MV=0, ML=0, BGR=1

    /* Ensure driver dimensions match portrait orientation */
    hili->width = hili->base_width;
    hili->height = hili->base_height;
    hili->config.orientation = ILI9488_ORIENTATION_PORTRAIT;

    log_debug("ILI9488: Orientation set to PORTRAIT (width=%u height=%u)", hili->width, hili->height);

    /* Pixel Format Set - RGB666 (18-bit/pixel) */
    ILI9488_WriteCommand(hili, ILI9488_CMD_PIXEL_FORMAT_SET);
    ILI9488_WriteData(hili, (uint8_t[]){0x66}, 1); // 0x66 = 18-bit RGB666
    hili->pixel_format = ILI9488_PIXEL_FMT_RGB666;

    /* Interface Mode Control */
    ILI9488_WriteCommand(hili, ILI9488_CMD_INTERFACE_MODE);
    ILI9488_WriteData(hili, (uint8_t[]){0x00}, 1); // SDO not used (SPI mode)

    /* Frame Rate Control (In Normal Mode) */
    ILI9488_WriteCommand(hili, ILI9488_CMD_FRAME_RATE_NORMAL);
    ILI9488_WriteData(hili, (uint8_t[]){0xA0}, 1); // 60Hz

    /* Display Inversion Control */
    ILI9488_WriteCommand(hili, ILI9488_CMD_INVERSION_CONTROL);
    ILI9488_WriteData(hili, (uint8_t[]){0x02}, 1); // 2-dot inversion

    /* Display Function Control */
    ILI9488_WriteCommand(hili, ILI9488_CMD_DISPLAY_FUNCTION);
    ILI9488_WriteData(hili, (uint8_t[]){0x02, 0x02}, 2); // Non-display area: white, SS=0, GS=0

    /* Set Image Function */
    ILI9488_WriteCommand(hili, ILI9488_CMD_SET_IMAGE_FUNCTION);
    ILI9488_WriteData(hili, (uint8_t[]){0x00}, 1); // Disable 24-bit data bus

    /* Adjust Control 3 */
    ILI9488_WriteCommand(hili, ILI9488_CMD_ADJUST_CONTROL3_F7);
    ILI9488_WriteData(hili, (uint8_t[]){0xA9, 0x51, 0x2C, 0x82}, 4);

    /* Entry Mode Set */
    ILI9488_WriteCommand(hili, ILI9488_CMD_ENTRY_MODE_SET);
    ILI9488_WriteData(hili, (uint8_t[]){0xC6}, 1); // Normal mode, low voltage detection enabled

    /* Sleep Out (already sent above, but ensure) */
    ILI9488_WriteCommand(hili, ILI9488_CMD_SLEEP_OUT);
    ILI9488_Delay(120);

    /* Display ON */
    ILI9488_WriteCommand(hili, ILI9488_CMD_DISPLAY_ON);
    ILI9488_Delay(25);

    hili->initialized = true;
    log_debug("ILI9488: Initialized successfully (width=%u height=%u, pixel_fmt=0x%02X)",
              hili->width, hili->height, hili->pixel_format);

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
            madctl = 0x48; // MY=0, MX=1, MV=0, ML=0, BGR=1
            hili->width = hili->base_width;
            hili->height = hili->base_height;
            break;
        case ILI9488_ORIENTATION_LANDSCAPE:
            madctl = 0x28; // MY=0, MX=0, MV=1, ML=0, BGR=1
            hili->width = hili->base_height;
            hili->height = hili->base_width;
            break;
        case ILI9488_ORIENTATION_PORTRAIT_REV:
            madctl = 0x88; // MY=1, MX=0, MV=0, ML=0, BGR=1
            hili->width = hili->base_width;
            hili->height = hili->base_height;
            break;
        case ILI9488_ORIENTATION_LANDSCAPE_REV:
            madctl = 0xE8; // MY=1, MX=1, MV=1, ML=0, BGR=1
            hili->width = hili->base_height;
            hili->height = hili->base_width;
            break;
    }

    ILI9488_WriteCommand(hili, ILI9488_CMD_MEMORY_ACCESS_CTL);
    ILI9488_WriteData(hili, &madctl, 1);

    hili->config.orientation = orientation;

    log_debug("ILI9488: Orientation changed to %d (MADCTL=0x%02X, width=%u, height=%u)",
              orientation, madctl, hili->width, hili->height);

    return ILI9488_OK;
}

/**
 * @brief   Clear display with solid color
 * @param   hili Pointer to ILI9488 handle
 * @param   color Fill color (RGB565)
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_Clear(ILI9488_Handle_t *hili, uint16_t color)
{
    if (hili == NULL || !hili->initialized) {
        return ILI9488_NOT_INITIALIZED;
    }

    ILI9488_SetAddressWindow(hili, 0, 0, hili->width - 1, hili->height - 1);

    ILI9488_WriteCommand(hili, ILI9488_CMD_MEMORY_WRITE);

    /* Create a line buffer with the fill color */
    const uint16_t line_width = (hili->width > hili->height) ? hili->width : hili->height;
    uint16_t *line_buffer = (uint16_t *)malloc(line_width * sizeof(uint16_t));

    if (line_buffer == NULL) {
        log_error("ILI9488: Failed to allocate line buffer for clear");
        return ILI9488_ERROR;
    }

    for (uint16_t i = 0; i < line_width; i++) {
        line_buffer[i] = color;
    }

    /* Fill display line by line */
    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_RESET);

    for (uint16_t y = 0; y < hili->height; y++) {
        ILI9488_WriteData16(hili, line_buffer, hili->width);
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);

    free(line_buffer);

    return ILI9488_OK;
}

/**
 * @brief   Draw pixel at specified coordinates
 * @param   hili Pointer to ILI9488 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color Pixel color (RGB565)
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
 * @brief   Write a block of pixels to the display
 * @param   hili Pointer to ILI9488 handle
 * @param   x0 Start X coordinate
 * @param   y0 Start Y coordinate
 * @param   x1 End X coordinate
 * @param   y1 End Y coordinate
 * @param   data Pixel buffer (RGB565)
 * @param   size Number of pixels in buffer
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_WritePixels(ILI9488_Handle_t *hili,
                                         uint16_t x0, uint16_t y0,
                                         uint16_t x1, uint16_t y1,
                                         const uint16_t *data,
                                         uint32_t size)
{
    if (hili == NULL || data == NULL) {
        return ILI9488_INVALID_PARAM;
    }

    if (!hili->initialized) {
        return ILI9488_NOT_INITIALIZED;
    }

    if (x1 < x0 || y1 < y0 || x1 >= hili->width || y1 >= hili->height) {
        return ILI9488_INVALID_PARAM;
    }

    ILI9488_SetAddressWindow(hili, x0, y0, x1, y1);
    ILI9488_WriteCommand(hili, ILI9488_CMD_MEMORY_WRITE);
    return ILI9488_WriteData16(hili, (uint16_t *)data, size);
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
        log_error("ILI9488: Command 0x%02X transmit failed (SPI error)", command);
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
        log_error("ILI9488: Data transmit failed (size=%u)", size);
        return ILI9488_ERROR;
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
    return ILI9488_OK;
}

/**
 * @brief   Write 16-bit RGB565 data to ILI9488 (converts to RGB666 if needed)
 * @param   hili Pointer to ILI9488 handle
 * @param   data 16-bit RGB565 data buffer
 * @param   size Data size in 16-bit words (number of pixels)
 * @retval  ILI9488_StatusTypeDef Operation status
 */
static ILI9488_StatusTypeDef ILI9488_WriteData16(ILI9488_Handle_t *hili, uint16_t *data, uint32_t size)
{
    HAL_GPIO_WritePin(hili->config.dc_port, hili->config.dc_pin, GPIO_PIN_SET); // Data mode
    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_RESET);

    /* Send in chunks using DMA for efficiency. When the display is configured
     * for RGB666 (18-bit) we convert each 16-bit RGB565 pixel into a 3-byte
     * RGB666 stream (R6,G6,B6) left-aligned into the top bits of each byte.
     */
    const uint32_t max_chunk_pixels = 2048; /* Reduced chunk size for stability */
    static uint8_t txbuf_rgb565[2048 * 2]; /* 4 KB for RGB565 path */
    static uint8_t txbuf_rgb666[2048 * 3]; /* 6 KB for RGB666 path */

    uint32_t remaining = size;
    uint16_t const *p = data;

    if (hili->pixel_format == ILI9488_PIXEL_FMT_RGB666) {
        /* RGB666 mode: Convert RGB565 to RGB666 (3 bytes per pixel) */
        while (remaining > 0) {
            uint32_t chunk = (remaining > max_chunk_pixels) ? max_chunk_pixels : remaining;

            /* Convert each RGB565 pixel to RGB666 triple (6 bits/channel) */
            for (uint32_t i = 0; i < chunk; i++) {
                uint16_t pix = p[i];

                /* Extract RGB565 components */
                uint8_t r5 = (pix >> 11) & 0x1F;  // 5 bits red
                uint8_t g6 = (pix >> 5) & 0x3F;   // 6 bits green
                uint8_t b5 = pix & 0x1F;          // 5 bits blue

                /* Expand 5-bit to 6-bit by left shift and OR with MSB replicated */
                uint8_t r6 = (r5 << 1) | (r5 >> 4);
                uint8_t b6 = (b5 << 1) | (b5 >> 4);

                /* Pack into RGB666 format: 6 bits per channel, left-aligned in byte
                 * ILI9488 expects: [R5:R0,xx][G5:G0,xx][B5:B0,xx] */
                txbuf_rgb666[3*i]     = (r6 << 2); // Red in upper 6 bits
                txbuf_rgb666[3*i + 1] = (g6 << 2); // Green in upper 6 bits
                txbuf_rgb666[3*i + 2] = (b6 << 2); // Blue in upper 6 bits
            }

            /* Transmit chunk via DMA (blocking until complete) */
            if (SPI_Transmit_DMA(txbuf_rgb666, (uint16_t)(chunk * 3)) != SPI_OK) {
                HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
                log_error("ILI9488: DMA transmit failed (RGB666, chunk_pixels=%lu)", (unsigned long)chunk);
                return ILI9488_ERROR;
            }

            p += chunk;
            remaining -= chunk;
        }
    } else {
        /* RGB565 mode: Direct transmission with byte swapping */
        while (remaining > 0) {
            uint32_t chunk = (remaining > max_chunk_pixels) ? max_chunk_pixels : remaining;

            /* Convert to big-endian byte order for transmission */
            for (uint32_t i = 0; i < chunk; i++) {
                txbuf_rgb565[2*i]     = (uint8_t)(p[i] >> 8);   // High byte
                txbuf_rgb565[2*i + 1] = (uint8_t)(p[i] & 0xFF); // Low byte
            }

            if (SPI_Transmit_DMA(txbuf_rgb565, (uint16_t)(chunk * 2)) != SPI_OK) {
                HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
                log_error("ILI9488: DMA transmit failed (RGB565, chunk_pixels=%lu)", (unsigned long)chunk);
                return ILI9488_ERROR;
            }

            p += chunk;
            remaining -= chunk;
        }
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
    return ILI9488_OK;
}

/**
 * @brief   Set address window for pixel writing
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
 * @brief   Read device ID bytes
 * @param   hili Pointer to ILI9488 handle
 * @param   id_buf Buffer to fill with read bytes
 * @param   len Number of bytes to read
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_ReadID(ILI9488_Handle_t *hili, uint8_t *id_buf, uint32_t len)
{
    if (hili == NULL || id_buf == NULL || len == 0) {
        return ILI9488_INVALID_PARAM;
    }

    HAL_GPIO_WritePin(hili->config.dc_port, hili->config.dc_pin, GPIO_PIN_RESET); // Command
    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_RESET);

    uint8_t cmd = ILI9488_CMD_READ_DISP_ID;
    if (SPI_Transmit(&cmd, 1, SPI_TIMEOUT_SHORT) != SPI_OK) {
        HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
        return ILI9488_ERROR;
    }

    HAL_GPIO_WritePin(hili->config.dc_port, hili->config.dc_pin, GPIO_PIN_SET); // Data mode

    /* Clock out the bytes by sending dummy 0x00 bytes and reading MISO */
    uint8_t dummy[8] = {0};
    if (len > 8) len = 8; // Safety limit

    if (SPI_TransmitReceive(dummy, id_buf, (uint16_t)len, SPI_TIMEOUT_LONG) != SPI_OK) {
        HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
        return ILI9488_ERROR;
    }

    HAL_GPIO_WritePin(hili->config.cs_port, hili->config.cs_pin, GPIO_PIN_SET);
    return ILI9488_OK;
}

/**
 * @brief   Set pixel format on controller and update local state
 * @param   hili Pointer to ILI9488 handle
 * @param   pixel_fmt Pixel format (0x55 for RGB565, 0x66 for RGB666)
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_SetPixelFormat(ILI9488_Handle_t *hili, uint8_t pixel_fmt)
{
    if (hili == NULL) {
        return ILI9488_INVALID_PARAM;
    }

    ILI9488_WriteCommand(hili, ILI9488_CMD_PIXEL_FORMAT_SET);
    ILI9488_WriteData(hili, &pixel_fmt, 1);
    hili->pixel_format = pixel_fmt;

    log_debug("ILI9488: PixelFormat set to 0x%02X", pixel_fmt);
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
