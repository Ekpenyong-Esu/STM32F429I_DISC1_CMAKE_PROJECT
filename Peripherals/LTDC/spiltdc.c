/**
 * @file spiltdc.c
 * @brief ILI9341 (SPI) LCD Controller driver implementation (extracted from ltdc.c)
 * @details This file provides the implementation for controlling the ILI9341 LCD controller via SPI.
 *          All SPI/ILI9341 logic is separated from the LTDC (RGB) driver.
 */

#include "spiltdc.h"
#include "stm32f4xx_hal.h"

static SPI_HandleTypeDef hspi_lcd;

void ILI9341_SPI_Init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_SPI5_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure SPI5 pins: SCK (PF7), MISO (PF8), MOSI (PF9) */
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* Configure LCD CS pin (PC2) */
    GPIO_InitStruct.Pin = ILI9341_CS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ILI9341_CS_PORT, &GPIO_InitStruct);

    /* Configure LCD WRX/DCX pin (PD13) - Data/Command selection */
    GPIO_InitStruct.Pin = ILI9341_WRX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ILI9341_WRX_PORT, &GPIO_InitStruct);

    /* Set CS high (deselect) */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);

    /* Configure SPI5 */
    hspi_lcd.Instance = SPI5;
    hspi_lcd.Init.Mode = SPI_MODE_MASTER;
    hspi_lcd.Init.Direction = SPI_DIRECTION_2LINES;
    hspi_lcd.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi_lcd.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi_lcd.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi_lcd.Init.NSS = SPI_NSS_SOFT;
    hspi_lcd.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi_lcd.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi_lcd.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi_lcd.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi_lcd.Init.CRCPolynomial = 10;
    HAL_SPI_Init(&hspi_lcd);
}

void ILI9341_WriteCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi_lcd, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
}

void ILI9341_WriteData(uint8_t data)
{
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi_lcd, &data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
}

void ILI9341_Init(void)
{
    ILI9341_SPI_Init();
    HAL_Delay(10);
    ILI9341_WriteCommand(ILI9341_RESET);
    HAL_Delay(120);
    ILI9341_WriteCommand(ILI9341_POWERA);
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);
    ILI9341_WriteCommand(ILI9341_POWERB);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x30);
    ILI9341_WriteCommand(ILI9341_DTCA);
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);
    ILI9341_WriteCommand(ILI9341_DTCB);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);
    ILI9341_WriteCommand(ILI9341_POWER_SEQ);
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x12);
    ILI9341_WriteData(0x81);
    ILI9341_WriteCommand(ILI9341_PRC);
    ILI9341_WriteData(0x20);
    ILI9341_WriteCommand(ILI9341_POWER1);
    ILI9341_WriteData(0x23);
    ILI9341_WriteCommand(ILI9341_POWER2);
    ILI9341_WriteData(0x10);
    ILI9341_WriteCommand(ILI9341_VCOM1);
    ILI9341_WriteData(0x3E);
    ILI9341_WriteData(0x28);
    ILI9341_WriteCommand(ILI9341_VCOM2);
    ILI9341_WriteData(0x86);
    ILI9341_WriteCommand(ILI9341_MAC);
    ILI9341_WriteData(0x40);
    ILI9341_WriteCommand(ILI9341_PIXEL_FORMAT);
    ILI9341_WriteData(0x55);
    ILI9341_WriteCommand(ILI9341_FRC);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x1B);
    ILI9341_WriteCommand(ILI9341_DFC);
    ILI9341_WriteData(0x0A);
    ILI9341_WriteData(0xA2);
    ILI9341_WriteData(0x27);
    ILI9341_WriteData(0x04);
    ILI9341_WriteCommand(ILI9341_3GAMMA_EN);
    ILI9341_WriteData(0x00);
    ILI9341_WriteCommand(ILI9341_GAMMA);
    ILI9341_WriteData(0x01);
    ILI9341_WriteCommand(ILI9341_PGAMMA);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0xF1);
    ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x10);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x00);
    ILI9341_WriteCommand(ILI9341_NGAMMA);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x36);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteCommand(ILI9341_INTERFACE);
    ILI9341_WriteData(0x01);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x06);
    ILI9341_WriteCommand(ILI9341_RGB_INTERFACE);
    ILI9341_WriteData(0xC2);
    ILI9341_WriteCommand(ILI9341_SLEEP_OUT);
    HAL_Delay(120);
    ILI9341_WriteCommand(ILI9341_DISPLAY_ON);
    HAL_Delay(20);
}
