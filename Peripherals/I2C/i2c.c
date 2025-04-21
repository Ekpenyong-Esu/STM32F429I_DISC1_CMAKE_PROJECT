/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   I2C module implementation
  * @details This file provides code for the configuration
  *          and initialization of the I2C peripheral.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include "../SYS/sys.h"

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   I2C3 handle structure
 * @details Used by HAL functions for I2C3 peripheral operations
 */
I2C_HandleTypeDef hi2c3;

/**
  * @brief  I2C Initialization Function
  * @details Configures the I2C3 peripheral with the following settings:
  *          - Clock speed: 100 kHz (standard mode)
  *          - Duty cycle: 50% (2:1 ratio)
  *          - 7-bit addressing mode
  *          - Own address: 0x00 (acts as master only)
  *          - Dual addressing mode: Disabled
  *          - General call mode: Disabled
  *          - Clock stretching: Enabled (NOSTRETCH disabled)
  *
  * @note   I2C3 is commonly used for communication with sensors,
  *         audio codec, or EEPROM on the STM32F429 board
  * @param  None
  * @retval None
  */
void I2C_Init(void)
{
  hi2c3.Instance = I2C3;                               /* Select I2C3 peripheral */
  hi2c3.Init.ClockSpeed = 100000;                      /* 100 kHz clock (standard mode) */
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;              /* 50% duty cycle */
  hi2c3.Init.OwnAddress1 = 0;                          /* Own address when in slave mode (not used) */
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT; /* 7-bit addressing mode */
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE; /* Dual addressing disabled */
  hi2c3.Init.OwnAddress2 = 0;                          /* Second own address (not used) */
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE; /* General call mode disabled */
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;    /* Clock stretching enabled */

  /* Initialize the I2C peripheral with the specified parameters */
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }
}
