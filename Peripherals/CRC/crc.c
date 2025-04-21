/**
  ******************************************************************************
  * @file    crc.c
  * @brief   CRC module implementation
  * @details This file provides code for the configuration
  *          and initialization of the CRC calculation unit
  *          for data integrity verification.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "crc.h"
#include "../SYS/sys.h"

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   CRC handle structure
 * @details Used by HAL functions for CRC peripheral operations
 */
CRC_HandleTypeDef hcrc;

/**
  * @brief  CRC Initialization Function
  * @details Initializes the CRC calculation unit with the following:
  *          - Default polynomial: 0x04C11DB7 (standard CRC-32)
  *          - Default initialization value: 0xFFFFFFFF
  *          - Input data format: 32-bit words, not reversed
  *          - Output data format: 32-bit word, not reversed
  *
  * @note   The STM32F4 hardware CRC unit uses a fixed polynomial and cannot
  *         be changed without software implementation for other CRC standards
  * @param  None
  * @retval None
  */
void CRC_Init(void)
{
  hcrc.Instance = CRC;  /* Select CRC peripheral */

  /* Initialize the CRC peripheral */
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }
}

/**
  * @brief  Calculate CRC value for a block of data
  * @details Uses the hardware CRC unit to compute a CRC-32 value
  *          according to the standard polynomial 0x04C11DB7
  *
  * @note   The input data must be 32-bit aligned. For non-aligned data,
  *         a software padding and transformation would be required.
  * @note   For optimal performance, the DMA can be used for large data blocks.
  *
  * @param  data: Pointer to input data buffer (must be 32-bit aligned)
  * @param  size: Size of data buffer in 32-bit words
  * @retval uint32_t: The calculated CRC value
  */
uint32_t CRC_Calculate(uint32_t *data, uint32_t size)
{
  /* Use HAL function to calculate CRC through the CRC peripheral */
  return HAL_CRC_Calculate(&hcrc, data, size);
}
