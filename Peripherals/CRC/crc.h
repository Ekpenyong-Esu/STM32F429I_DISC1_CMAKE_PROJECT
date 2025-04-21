/**
  ******************************************************************************
  * @file    crc.h
  * @brief   CRC module interface
  * @details This file contains all the function prototypes for
  *          the Cyclic Redundancy Check (CRC) calculation unit.
  *          It provides APIs to initialize and use the CRC hardware
  *          accelerator on the STM32F429 board.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief   Initializes CRC peripheral
 * @details Configures and enables the CRC calculation unit
 * @param   None
 * @retval  None
 */
void CRC_Init(void);

/**
 * @brief   Calculates CRC value for a block of data
 * @details Uses the hardware CRC unit to compute a 32-bit CRC value
 *          using the standard CRC-32 polynomial (0x04C11DB7)
 * @param   data  Pointer to the input data buffer
 * @param   size  Size of the data buffer in words (32-bit)
 * @retval  uint32_t  The calculated CRC value
 */
uint32_t CRC_Calculate(uint32_t *data, uint32_t size);

/* Exported variables ---------------------------------------------------------*/
/**
 * @brief   CRC handle structure
 * @details Used by HAL functions to manage CRC operations
 */
extern CRC_HandleTypeDef hcrc;

#ifdef __cplusplus
}
#endif

#endif /* __CRC_H__ */
