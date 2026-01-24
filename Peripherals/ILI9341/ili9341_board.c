/* ili9341_board.c - Board-specific MSP for ILI9341 on STM32F429I-DISC1
 * Configures GPIOs and SPI5 to the board's preferred settings and exposes
 * the concrete implementation of the weak ILI9341_MspInit/DeInit hooks.
 *
 * Based on ST BSP stm32f429i_discovery.c LCD_IO_Init() and SPIx_MspInit()
 */

#include "ili9341.h"
#include "spi.h"
#include "stm32f4xx_hal.h"

/* Local constants - match ST BSP */
#define SPI_CRC_POLY 7  /* ST BSP uses 7 */

/* LCD_RDX_PIN - used by ST BSP but we don't use it for reads in this driver */
#define LCD_RDX_PIN   GPIO_PIN_12
#define LCD_RDX_PORT  GPIOD

/* Static flag to track if LCD IO is initialized (matches ST BSP pattern) */
static uint8_t Is_LCD_IO_Initialized = 0;

void ILI9341_MspInit(void)
{
    if (Is_LCD_IO_Initialized == 0)
    {
        Is_LCD_IO_Initialized = 1;

        /* Enable clocks */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_SPI5_CLK_ENABLE();

        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* ---- LCD Control pins (per ST BSP stm32f429i_discovery.c LCD_IO_Init) ---- */

        /* WRX/DC pin (PD13) - Output Push-Pull */
        GPIO_InitStruct.Pin = ILI9341_WRX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  /* ST BSP uses FAST */
        HAL_GPIO_Init(ILI9341_WRX_PORT, &GPIO_InitStruct);

        /* RDX pin (PD12) - Output Push-Pull (used by ST BSP for read operations) */
        GPIO_InitStruct.Pin = LCD_RDX_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(LCD_RDX_PORT, &GPIO_InitStruct);

        /* NCS/CS pin (PC2) - Output Push-Pull */
        GPIO_InitStruct.Pin = ILI9341_CS_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(ILI9341_CS_PORT, &GPIO_InitStruct);

        /* ST BSP does a toggle: Set or Reset the control line */
        HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);

        /* ---- SPI5 pins (per ST BSP stm32f429i_discovery.c SPIx_MspInit) ---- */

        /* NOTE: SPI5 SCK/MISO/MOSI (PF7/PF8/PF9) are configured centrally in
         * HAL_SPI_MspInit() (see Core/Src/stm32f4xx_hal_msp.c). To avoid
         * duplicate initialization and potential conflicts, do not reconfigure
         * the AF pins here. The board-specific MSP still enables clocks and
         * configures control lines (CS/WRX/RDX) above.
         */

        /* Configure SPI via central driver using ST BSP settings */
        SPI_ConfigTypeDef cfg = {0};
        cfg.Mode = SPI_MODE_MASTER;
        cfg.Direction = SPI_DIRECTION_2LINES;
        cfg.DataSize = SPI_DATASIZE_8BIT;
        cfg.CLKPolarity = SPI_POLARITY_LOW;     /* CPOL=0 */
        cfg.CLKPhase = SPI_PHASE_1EDGE;         /* CPHA=0 (Mode 0) */
        cfg.NSS = SPI_NSS_SOFT;
        /* ST BSP expects SPI SCLK ≈ 5.6 - 10 MHz. With current clocks (APB2 = 84 MHz)
         * prescaler=8 => 10.5 MHz; prescaler=16 => 5.25 MHz (below 5.6). Use prescaler=8
         * for better compliance with ST BSP recommendations. */
        cfg.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;  /* SPI clock ≈ APB2 / 8 = 10.5 MHz */
        cfg.FirstBit = SPI_FIRSTBIT_MSB;
        cfg.TIMode = SPI_TIMODE_DISABLE;
        cfg.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
        cfg.CRCPolynomial = SPI_CRC_POLY;

        /* Use SPI_Init_Custom when available to apply board-specific config */
        SPI_Init_Custom(&cfg);
    }
}

void ILI9341_MspDeInit(void)
{
    /* Reset pins to default state */
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_RDX_PORT, LCD_RDX_PIN, GPIO_PIN_RESET);

    Is_LCD_IO_Initialized = 0;
    /* Do not de-init SPI here; leave to central SPI DeInit if needed */
}
