/* xpt2046_board.c - Board-specific MSP for XPT2046 on STM32F429I-DISC1
 * Configures GPIO clocks and pins used by the XPT2046 touch controller.
 *
 * This file overrides the weak XPT2046_MspInit/DeInit hooks.
 */

#include "xpt2046.h"


/* Static flag to track if Touch IO is initialized */
static uint8_t Is_TP_IO_Initialized = 0;

/**
 * @brief   Initialize MSP (GPIO pins)
 * @details Configures GPIO pins for XPT2046 touch controller
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port
 * @param   irq_pin Interrupt pin
 * @retval  None
 */
void XPT2046_MspInit(GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    if (Is_TP_IO_Initialized == 0)
    {
        Is_TP_IO_Initialized = 1;

        /* Enable GPIO clocks for CS pin */
        if (cs_port == GPIOA) __HAL_RCC_GPIOA_CLK_ENABLE();
        if (cs_port == GPIOB) __HAL_RCC_GPIOB_CLK_ENABLE();
        if (cs_port == GPIOC) __HAL_RCC_GPIOC_CLK_ENABLE();
        if (cs_port == GPIOD) __HAL_RCC_GPIOD_CLK_ENABLE();
        if (cs_port == GPIOE) __HAL_RCC_GPIOE_CLK_ENABLE();

        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* CS pin - output push-pull */
        GPIO_InitStruct.Pin = cs_pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(cs_port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_SET);
    }
}

void XPT2046_MspDeInit(GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET);
    HAL_GPIO_DeInit(cs_port, cs_pin);

    Is_TP_IO_Initialized = 0;
}
