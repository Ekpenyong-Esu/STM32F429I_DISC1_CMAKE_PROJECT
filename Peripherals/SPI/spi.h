/**
  ******************************************************************************
  * @file    spi.h
  * @brief   SPI module interface
  * @details This file contains all the function prototypes for
  *          the Serial Peripheral Interface (SPI) configuration.
  *          It provides APIs to initialize and control SPI communication
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes SPI peripheral used in the application
 * @details Configures the SPI with appropriate parameters for clock,
 *          data format, mode and enables the peripheral
 * @param   None
 * @retval  None
 */
void SPI_Init(void);

/* Exported variables ---------------------------------------------------------*/
/**
 * @brief   SPI5 handle structure
 * @details Used by HAL functions to manage SPI5 operations
 */
extern SPI_HandleTypeDef hspi5;

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */
