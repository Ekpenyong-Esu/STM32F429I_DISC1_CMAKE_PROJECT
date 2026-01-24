/* ili9341.h - Minimal ILI9341 driver adapted from ST but following project driver style
 * - Uses central Peripherals/SPI APIs (calls `SPI_Init()` and `SPI_Transmit`/`SPI_TransmitReceive`)
 * - Keeps ST-like API (ili9341_Init, ili9341_ReadID, DisplayOn/Off, GetLcdPixelWidth/Height)
 * - Provides weak `ILI9341_MspInit()` hook for board pin configuration
 */

#ifndef __ILI9341_H__
#define __ILI9341_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
/* Keep HAL headers out of the public header to limit transitive includes */

/* Display size */
#define ILI9341_LCD_PIXEL_WIDTH    ((uint16_t)240)
#define ILI9341_LCD_PIXEL_HEIGHT   ((uint16_t)320)

/* Default I/O pins (can be overridden in board-specific headers) */
#ifndef ILI9341_WRX_PIN
#define ILI9341_WRX_PIN             GPIO_PIN_13
#define ILI9341_WRX_PORT            GPIOD
#endif

#ifndef ILI9341_CS_PIN
#define ILI9341_CS_PIN              GPIO_PIN_2
#define ILI9341_CS_PORT             GPIOC
#endif

/* ILI9341 Commands (subset used by init sequence) */
#define ILI9341_SWRESET             0x01
#define ILI9341_SLEEP_IN            0x10
#define ILI9341_SLEEP_OUT           0x11
#define ILI9341_DISPLAY_ON          0x29
#define ILI9341_DISPLAY_OFF         0x28
#define ILI9341_COLUMN_ADDR         0x2A
#define ILI9341_PAGE_ADDR           0x2B
#define ILI9341_GRAM                0x2C
#define ILI9341_MAC                 0x36
#define ILI9341_PIXEL_FORMAT        0x3A
#define ILI9341_GAMMA               0x26
#define ILI9341_PGAMMA              0xE0
#define ILI9341_NGAMMA              0xE1
#define ILI9341_POWERA              0xCB
#define ILI9341_POWERB              0xCF
#define ILI9341_DTCA                0xE8
#define ILI9341_DTCB                0xEA
#define ILI9341_POWER_SEQ           0xED
#define ILI9341_3GAMMA_EN           0xF2
#define ILI9341_INTERFACE           0xF6
#define ILI9341_PRC                 0xF7
#define ILI9341_FRC                 0xB1
#define ILI9341_DFC                 0xB6
#define ILI9341_RGB_INTERFACE       0xB0
#define ILI9341_POWER1              0xC0
#define ILI9341_POWER2              0xC1
#define ILI9341_VCOM1               0xC5
#define ILI9341_VCOM2               0xC7

/* Read size used by original ST driver for ID */
#define ILI9341_READ_ID4           0xD3
#define ILI9341_READ_ID4_SIZE      3

/* Useful masks/delays */
#define ILI9341_BYTE_MASK          0xFFU
#define ILI9341_WORD_MASK          0xFFFFU
#define ILI9341_INIT_DELAY_MS      200U
#define ILI9341_WAKE_DELAY_MS      200U

/* Column / Page end markers (ST constants used during init sequence) */
#define ILI9341_COL_END            0xEF
#define ILI9341_PAGE_END           0x3F

/* Public API (ST-like signatures kept) */
void     ili9341_Init(void);
uint16_t ili9341_ReadID(void);
void     ili9341_WriteReg(uint8_t LCD_Reg);
void     ili9341_WriteData(uint8_t RegValue);
uint32_t ili9341_ReadData(uint16_t RegValue, uint8_t ReadSize);
void     ili9341_DisplayOn(void);
void     ili9341_DisplayOff(void);
uint16_t ili9341_GetLcdPixelWidth(void);
uint16_t ili9341_GetLcdPixelHeight(void);

/* Weak hook for board-specific GPIO/SPI configuration. Implement in board files to override. */
void ILI9341_MspInit(void);
void ILI9341_MspDeInit(void);

void ili9341_SleepIn(void);
void ili9341_SleepOut(void);


#ifdef __cplusplus
}
#endif

#endif /* __ILI9341_H__ */
