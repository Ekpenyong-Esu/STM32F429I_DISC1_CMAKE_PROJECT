/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   I2C module interface
  * @details This file contains all the function prototypes for
  *          the Inter-Integrated Circuit (I2C) configuration.
  *          It provides APIs to initialize and control I2C communication
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes I2C peripheral used in the application
 * @details Configures the I2C with appropriate parameters for clock speed,
 *          addressing mode, and enables the peripheral
 * @param   None
 * @retval  None
 */
void I2C_Init(void);

/* Exported variables ---------------------------------------------------------*/
/**
 * @brief   I2C3 handle structure
 * @details Used by HAL functions to manage I2C3 operations
 *          typically used for audio codec, sensors or EEPROM communication
 */
extern I2C_HandleTypeDef hi2c3;

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */
