/**
 * @file spiltdc.h
 * @brief ILI9341 (SPI) LCD Controller driver header (extracted from ltdc.c)
 * @details This file provides the interface for controlling the ILI9341 LCD controller via SPI.
 *          All SPI/ILI9341 logic is separated from the LTDC (RGB) driver.
 */

#ifndef __SPILTDCH__
#define __SPILTDCH__

#include "stm32f4xx_hal.h"

/* ILI9341 Commands */
#define ILI9341_RESET               0x01
#define ILI9341_SLEEP_OUT           0x11
#define ILI9341_GAMMA               0x26
#define ILI9341_DISPLAY_OFF         0x28
#define ILI9341_DISPLAY_ON          0x29
#define ILI9341_COLUMN_ADDR         0x2A
#define ILI9341_PAGE_ADDR           0x2B
#define ILI9341_GRAM                0x2C
#define ILI9341_MAC                 0x36
#define ILI9341_PIXEL_FORMAT        0x3A
#define ILI9341_WDB                 0x51
#define ILI9341_WCD                 0x53
#define ILI9341_RGB_INTERFACE       0xB0
#define ILI9341_FRC                 0xB1
#define ILI9341_BPC                 0xB5
#define ILI9341_DFC                 0xB6
#define ILI9341_POWER1              0xC0
#define ILI9341_POWER2              0xC1
#define ILI9341_VCOM1               0xC5
#define ILI9341_VCOM2               0xC7
#define ILI9341_POWERA              0xCB
#define ILI9341_POWERB              0xCF
#define ILI9341_PGAMMA              0xE0
#define ILI9341_NGAMMA              0xE1
#define ILI9341_DTCA                0xE8
#define ILI9341_DTCB                0xEA
#define ILI9341_POWER_SEQ           0xED
#define ILI9341_3GAMMA_EN           0xF2
#define ILI9341_INTERFACE           0xF6
#define ILI9341_PRC                 0xF7

/* ILI9341 SPI GPIO Pins (STM32F429I-DISC1) */
#define ILI9341_WRX_PIN             GPIO_PIN_13
#define ILI9341_WRX_PORT            GPIOD
#define ILI9341_CS_PIN              GPIO_PIN_2
#define ILI9341_CS_PORT             GPIOC

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: This header provided legacy SPI-based access to the ILI9341. The
   functionality has been replaced by `Peripherals/ILI9341/ili9341.{h,c}`. Use
   that API instead. These wrappers remain for backward compatibility. */

#include "../ILI9341/ili9341.h"

void ILI9341_SPI_Init(void);   /* Deprecated wrapper - calls ILI9341_MspInit() */
void ILI9341_WriteCommand(uint8_t cmd); /* Deprecated wrapper - calls ili9341_WriteReg() */
void ILI9341_WriteData(uint8_t data);   /* Deprecated wrapper - calls ili9341_WriteData() */
void ILI9341_Init(void);       /* Deprecated wrapper - calls ili9341_Init() */

#ifdef __cplusplus
}
#endif

#endif /* __SPILTDCH__ */
