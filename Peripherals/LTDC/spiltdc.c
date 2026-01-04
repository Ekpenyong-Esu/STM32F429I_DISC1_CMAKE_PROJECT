/**
 * @file spiltdc.c
 * @brief ILI9341 (SPI) LCD Controller driver implementation (extracted from ltdc.c)
 * @details This file provides the implementation for controlling the ILI9341 LCD controller via SPI.
 *          All SPI/ILI9341 logic is separated from the LTDC (RGB) driver.
 */

#include "spiltdc.h"
#include "stm32f4xx_hal.h"

/* Deprecated: wrapper functions that forward to the new Peripherals/ILI9341 API.
   Keep wrappers for backwards compatibility; prefer using ili9341_* directly. */

#include "spiltdc.h"
#include "../ILI9341/ili9341.h"

void ILI9341_SPI_Init(void)
{
    /* Delegate to the new driver's MSP init which configures pins and SPI */
    ILI9341_MspInit();
}

void ILI9341_WriteCommand(uint8_t cmd)
{
    ili9341_WriteReg(cmd);
}

void ILI9341_WriteData(uint8_t data)
{
    /* Convert 8-bit to 16-bit write (driver uses 16-bit write API) */
    ili9341_WriteData((uint16_t)data);
}

void ILI9341_Init(void)
{
    /* Call into new driver init (keeps existing behavior) */
    ili9341_Init();
}
