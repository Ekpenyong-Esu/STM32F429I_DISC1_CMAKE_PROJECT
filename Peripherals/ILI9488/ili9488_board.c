/* ili9488_board.c - Board-specific MSP for ILI9488 on STM32F429I-DISC1
 * Configures GPIO clocks and control pins used by the ILI9488 SPI display.
 *
 * This file overrides the weak ILI9488_MspInit/DeInit hooks.
 */

#include "ili9488.h"
#include "main.h"


/* Static flag to track if LCD IO is initialized */
static uint8_t Is_LCD_IO_Initialized = 0;

void ILI9488_MspInit(void)
{
    if (Is_LCD_IO_Initialized == 0)
    {
        Is_LCD_IO_Initialized = 1;

        /* Enable GPIO clocks for control pins */
        if (CSX_GPIO_Port == GPIOA || WRX_DCX_GPIO_Port == GPIOA || RDX_GPIO_Port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
        if (CSX_GPIO_Port == GPIOB || WRX_DCX_GPIO_Port == GPIOB || RDX_GPIO_Port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
        if (CSX_GPIO_Port == GPIOC || WRX_DCX_GPIO_Port == GPIOC || RDX_GPIO_Port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
        if (CSX_GPIO_Port == GPIOD || WRX_DCX_GPIO_Port == GPIOD || RDX_GPIO_Port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
        if (CSX_GPIO_Port == GPIOE || WRX_DCX_GPIO_Port == GPIOE || RDX_GPIO_Port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();

        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        /* CS pin */
        GPIO_InitStruct.Pin = CSX_Pin;
        HAL_GPIO_Init(CSX_GPIO_Port, &GPIO_InitStruct);

        /* DC pin */
        GPIO_InitStruct.Pin = WRX_DCX_Pin;
        HAL_GPIO_Init(WRX_DCX_GPIO_Port, &GPIO_InitStruct);

        /* RST pin */
        GPIO_InitStruct.Pin = RDX_Pin;
        HAL_GPIO_Init(RDX_GPIO_Port, &GPIO_InitStruct);

        /* Default pin states */
        HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(WRX_DCX_GPIO_Port, WRX_DCX_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(RDX_GPIO_Port, RDX_Pin, GPIO_PIN_SET);
    }
}

void ILI9488_MspDeInit(void)
{
    HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(WRX_DCX_GPIO_Port, WRX_DCX_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(RDX_GPIO_Port, RDX_Pin, GPIO_PIN_RESET);

    Is_LCD_IO_Initialized = 0;
}
