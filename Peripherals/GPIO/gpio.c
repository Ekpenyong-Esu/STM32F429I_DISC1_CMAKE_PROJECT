/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO module implementation
  * @details This file provides code for the configuration and control
  *          of the GPIO pins used in the application.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "main.h"
#include "stm32f429xx.h"
#include "stm32f4xx_hal_gpio.h"

/**
  * @brief  GPIO Initialization Function
  * @details This function configures the GPIO pins as follows:
  *          - Enables the peripheral clocks for all used GPIO ports
  *          - Configures pins for LEDs, buttons, and peripherals
  *          - Sets initial output levels for output pins
  *
  * @note   The specific pins configured include:
  *          - User LEDs (LD3, LD4) for status indication
  *          - Push buttons for user input
  *          - SPI chip select for MEMS
  *          - LCD control signals (CSX, RDX, WRX_DCX, TE)
  *          - OTG_FS power and overcurrent pins
  *
  * @param  None
  * @retval None
  */
void GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();  /* Enable GPIOC peripheral clock */
  __HAL_RCC_GPIOF_CLK_ENABLE();  /* Enable GPIOF peripheral clock */
  __HAL_RCC_GPIOH_CLK_ENABLE();  /* Enable GPIOH peripheral clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();  /* Enable GPIOA peripheral clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();  /* Enable GPIOB peripheral clock */
  __HAL_RCC_GPIOG_CLK_ENABLE();  /* Enable GPIOG peripheral clock */
  __HAL_RCC_GPIOE_CLK_ENABLE();  /* Enable GPIOE peripheral clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();  /* Enable GPIOD peripheral clock */

  /* Configure GPIO pin Output Levels */
  /* Set initial output levels for output pins to ensure known state at startup */
  HAL_GPIO_WritePin(GPIOC, NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(ACP_RST_GPIO_Port, ACP_RST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, RDX_Pin|WRX_DCX_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOG, LD3_Pin|LD4_Pin, GPIO_PIN_RESET);

  /* Configure MEMS SPI chip select and LCD control pins */
  GPIO_InitStruct.Pin = NCS_MEMS_SPI_Pin|CSX_Pin|OTG_FS_PSO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* Configure user button and sensor interrupt pins */
  GPIO_InitStruct.Pin = B1_Pin|MEMS_INT1_Pin|MEMS_INT2_Pin|TP_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure audio codec reset pin */
  GPIO_InitStruct.Pin = ACP_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ACP_RST_GPIO_Port, &GPIO_InitStruct);

  /* Configure USB OTG overcurrent pin */
  GPIO_InitStruct.Pin = OTG_FS_OC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OC_GPIO_Port, &GPIO_InitStruct);

  /* Configure bootloader pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /* Configure LCD timing enable pin */
  GPIO_InitStruct.Pin = TE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TE_GPIO_Port, &GPIO_InitStruct);

  /* Configure LCD control pins */
  GPIO_InitStruct.Pin = RDX_Pin|WRX_DCX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* Configure LED pins */
  GPIO_InitStruct.Pin = LD3_Pin|LD4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);


    GPIO_InitStruct.Pin = LD3_Pin|LD4_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOG, LD3_Pin|LD4_Pin, GPIO_PIN_RESET);

}
