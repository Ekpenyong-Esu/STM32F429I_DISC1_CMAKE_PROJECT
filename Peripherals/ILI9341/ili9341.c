/* ili9341.c - Minimal ILI9341 driver adapted to repository style
 * - Uses Peripherals/SPI for bus access
 * - Keeps ST-like higher-level API
 * - Provides weak MSP hooks for pin setup
 */

#include "ili9341.h"
#include "spi.h"
#include "log.h"

/* Helper: select/deselect and set data/command mode */
static inline void ILI9341_Select(void)
{
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
}
static inline void ILI9341_Deselect(void)
{
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
}
static inline void ILI9341_SetCmdMode(void)
{
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_RESET);
}
static inline void ILI9341_SetDataMode(void)
{
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_SET);
}

/* Default weak MSP implementations: configure CS and WRX pins and call central SPI init.
   Boards can override these by providing their own ILI9341_MspInit/DeInit. */
__weak void ILI9341_MspInit(void)
{
    /* Enable required GPIO clocks */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    /* WRX/DC pin (PD13) - Output Push-Pull, per ST BSP */
    gpio.Pin = ILI9341_WRX_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ILI9341_WRX_PORT, &gpio);

    /* CS pin (PC2) - Output Push-Pull */
    gpio.Pin = ILI9341_CS_PIN;
    HAL_GPIO_Init(ILI9341_CS_PORT, &gpio);

    /* ST BSP pattern: toggle CS low then high */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);

    /* Initialize central SPI driver */
    SPI_Init();
}

__weak void ILI9341_MspDeInit(void)
{
    /* Default no-op; boards can implement to deinit pins or SPI if desired */
}

void ili9341_WriteReg(uint8_t LCD_Reg)
{
    ILI9341_SetCmdMode();
    ILI9341_Select();
    SPI_Transmit((uint8_t*)&LCD_Reg, 1, SPI_TIMEOUT_LONG);
    ILI9341_Deselect();
}

void ili9341_WriteData(uint8_t RegValue)
{
    uint8_t byte = RegValue & ILI9341_BYTE_MASK;

    ILI9341_SetDataMode();
    ILI9341_Select();
    SPI_Transmit(&byte, 1, SPI_TIMEOUT_LONG);
    ILI9341_Deselect();
}

uint32_t ili9341_ReadData(uint16_t RegValue, uint8_t ReadSize)
{
    /* Send command then read ReadSize bytes (max 4) */
    uint8_t cmd = (uint8_t)RegValue;
    uint8_t responseBuffer[4] = {0};
    uint8_t transmitBuffer[4] = {0};

    if (ReadSize > (uint8_t)sizeof(responseBuffer)) {
        ReadSize = (uint8_t)sizeof(responseBuffer);
    }

    ILI9341_SetCmdMode();
    ILI9341_Select();
    SPI_Transmit(&cmd, 1, SPI_TIMEOUT_LONG);

    ILI9341_SetDataMode();
    SPI_TransmitReceive(transmitBuffer, responseBuffer, ReadSize, SPI_TIMEOUT_LONG);

    ILI9341_Deselect();

    uint32_t value = 0;
    for (uint8_t i = 0; i < ReadSize; ++i) {
        value = (value << 8) | responseBuffer[i];
    }
    return value;
}

uint16_t ili9341_GetLcdPixelWidth(void)
{
    return ILI9341_LCD_PIXEL_WIDTH;
}

uint16_t ili9341_GetLcdPixelHeight(void)
{
    return ILI9341_LCD_PIXEL_HEIGHT;
}

uint16_t ili9341_ReadID(void)
{
    /* ST uses a read of 3 bytes for ID — we implement same behavior */
    uint32_t devId = ili9341_ReadData(ILI9341_READ_ID4, ILI9341_READ_ID4_SIZE);
    return (uint16_t)(devId & ILI9341_WORD_MASK);
}

void ili9341_DisplayOn(void)
{
    log_debug("ILI9341: Turning display on");
    ili9341_WriteReg(ILI9341_DISPLAY_ON);
}

void ili9341_DisplayOff(void)
{
    log_debug("ILI9341: Turning display off");
    ili9341_WriteReg(ILI9341_DISPLAY_OFF);
}

void ili9341_SleepIn(void)
{
    log_debug("ILI9341: Entering sleep mode");
    ili9341_WriteReg(ILI9341_SLEEP_IN);
    HAL_Delay(5);  /* Small delay after sleep in */
}

void ili9341_SleepOut(void)
{
    log_debug("ILI9341: Exiting sleep mode");
    ili9341_WriteReg(ILI9341_SLEEP_OUT);
    HAL_Delay(ILI9341_WAKE_DELAY_MS);  /* Wait for wake up */
}

void ili9341_Init(void)
{
    log_debug("ILI9341: Initializing display");
    /* Allow board to setup pins and SPI */
    ILI9341_MspInit();

    /* Software Reset - CRITICAL for proper initialization */
    ili9341_WriteReg(ILI9341_SWRESET);
    /* Wait for reset to complete. Use the standard init delay to be safe */
    HAL_Delay(ILI9341_INIT_DELAY_MS);

    /* Follow ST sequence (trimmed) to configure display */
    ili9341_WriteReg(0xCA);
    ili9341_WriteData(0xC3);
    ili9341_WriteData(0x08);
    ili9341_WriteData(0x50);
    ili9341_WriteReg(ILI9341_POWERB);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0xC1);
    ili9341_WriteData(0x30);
    ili9341_WriteReg(ILI9341_POWER_SEQ);
    ili9341_WriteData(0x64);
    ili9341_WriteData(0x03);
    ili9341_WriteData(0x12);
    ili9341_WriteData(0x81);
    ili9341_WriteReg(ILI9341_DTCA);
    ili9341_WriteData(0x85);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x78);
    ili9341_WriteReg(ILI9341_POWERA);
    ili9341_WriteData(0x39);
    ili9341_WriteData(0x2C);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x34);
    ili9341_WriteData(0x02);
    ili9341_WriteReg(ILI9341_PRC);
    ili9341_WriteData(0x20);
    ili9341_WriteReg(ILI9341_DTCB);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x00);
    ili9341_WriteReg(ILI9341_FRC);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x1B);
    ili9341_WriteReg(ILI9341_DFC);
    ili9341_WriteData(0x0A);
    ili9341_WriteData(0xA2);
    ili9341_WriteReg(ILI9341_POWER1);
    ili9341_WriteData(0x10);
    ili9341_WriteReg(ILI9341_POWER2);
    ili9341_WriteData(0x10);
    ili9341_WriteReg(ILI9341_VCOM1);
    ili9341_WriteData(0x45);
    ili9341_WriteData(0x15);
    ili9341_WriteReg(ILI9341_VCOM2);
    ili9341_WriteData(0x90);
    ili9341_WriteReg(ILI9341_MAC);
    ili9341_WriteData(0xC8);
    ili9341_WriteReg(ILI9341_3GAMMA_EN);
    ili9341_WriteData(0x00);
    ili9341_WriteReg(ILI9341_RGB_INTERFACE);
    ili9341_WriteData(0xC2);  /* Enable RGB interface: RCM=11, bypass memory, RGB through DE/HSYNC/VSYNC */
    ili9341_WriteReg(ILI9341_DFC);
    ili9341_WriteData(0x0A);
    ili9341_WriteData(0xA7);
    ili9341_WriteData(0x27);
    ili9341_WriteData(0x04);

    /* Column address */
    ili9341_WriteReg(ILI9341_COLUMN_ADDR);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x00);
    ili9341_WriteData(ILI9341_COL_END);

    /* Page address */
    ili9341_WriteReg(ILI9341_PAGE_ADDR);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x01);
    ili9341_WriteData(ILI9341_PAGE_END);

    ili9341_WriteReg(ILI9341_INTERFACE);
    ili9341_WriteData(0x01);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x06);

    // /* in ili9341_Init(), after selecting RGB interface and before GRAM */
    // ili9341_WriteReg(ILI9341_PIXEL_FORMAT);
    // ili9341_WriteData(0x55); /* 0x55 = 16-bit/pixel (RGB565) on ILI9341 */

    ili9341_WriteReg(ILI9341_GRAM);
    HAL_Delay(ILI9341_INIT_DELAY_MS);

    /* Gamma selection */
    ili9341_WriteReg(ILI9341_GAMMA);
    ili9341_WriteData(0x01);

    /* Positive Gamma Correction (15 parameters) - CRITICAL for proper display */
    ili9341_WriteReg(ILI9341_PGAMMA);
    ili9341_WriteData(0x0F);
    ili9341_WriteData(0x29);
    ili9341_WriteData(0x24);
    ili9341_WriteData(0x0C);
    ili9341_WriteData(0x0E);
    ili9341_WriteData(0x09);
    ili9341_WriteData(0x4E);
    ili9341_WriteData(0x78);
    ili9341_WriteData(0x3C);
    ili9341_WriteData(0x09);
    ili9341_WriteData(0x13);
    ili9341_WriteData(0x05);
    ili9341_WriteData(0x17);
    ili9341_WriteData(0x11);
    ili9341_WriteData(0x00);

    /* Negative Gamma Correction (15 parameters) - CRITICAL for proper display */
    ili9341_WriteReg(ILI9341_NGAMMA);
    ili9341_WriteData(0x00);
    ili9341_WriteData(0x16);
    ili9341_WriteData(0x1B);
    ili9341_WriteData(0x04);
    ili9341_WriteData(0x11);
    ili9341_WriteData(0x07);
    ili9341_WriteData(0x31);
    ili9341_WriteData(0x33);
    ili9341_WriteData(0x42);
    ili9341_WriteData(0x05);
    ili9341_WriteData(0x0C);
    ili9341_WriteData(0x0A);
    ili9341_WriteData(0x28);
    ili9341_WriteData(0x2F);
    ili9341_WriteData(0x0F);

    /* Exit sleep mode */
    ili9341_WriteReg(ILI9341_SLEEP_OUT);
    HAL_Delay(ILI9341_WAKE_DELAY_MS);

    /* Turn on display */
    ili9341_WriteReg(ILI9341_DISPLAY_ON);

    /* Start GRAM write mode */
    ili9341_WriteReg(ILI9341_GRAM);
    log_debug("ILI9341: Initialization complete");
}
