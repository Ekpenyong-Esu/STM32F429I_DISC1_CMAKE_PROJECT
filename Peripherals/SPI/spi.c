/**
  ******************************************************************************
  * @file    spi.c
  * @brief   SPI module implementation
  * @details This file provides code for the configuration
  *          and initialization of the SPI peripheral.
  * @version 1.0
  * @date    2025-04-15
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "../SYS/sys.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define SPI_CRC_POLYNOMIAL_DEFAULT    10U     /**< Default CRC polynomial value */

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   SPI5 handle structure
 * @details Used by HAL functions for SPI5 peripheral operations
 */
SPI_HandleTypeDef hspi5;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  SPI Initialization Function
  * @details Configures the SPI5 peripheral with the following settings:
  *          - Master mode operation
  *          - Full-duplex (2 lines) communication
  *          - 8-bit data size
  *          - High clock polarity (CPOL=1)
  *          - Second clock transition is the data capture edge (CPHA=1)
  *          - Software NSS management
  *          - Baud rate = fPCLK/8
  *          - MSB transmitted/received first
  *          - TI mode disabled
  *          - CRC calculation disabled
  *
  * @note   SPI5 is commonly used for display or external sensor communication
  * @param  None
  * @retval None
  */
void SPI_Init(void)
{
  /* SPI5 parameter configuration*/
  hspi5.Instance = SPI5;                                /* Select SPI5 peripheral */
  hspi5.Init.Mode = SPI_MODE_MASTER;                    /* Configure as master */
  hspi5.Init.Direction = SPI_DIRECTION_2LINES;          /* Full-duplex mode */
  hspi5.Init.DataSize = SPI_DATASIZE_8BIT;              /* 8-bit data size */
  hspi5.Init.CLKPolarity = SPI_POLARITY_HIGH;           /* Clock polarity high (CPOL=1) for L3GD20 */
  hspi5.Init.CLKPhase = SPI_PHASE_2EDGE;                /* Clock phase 2nd edge (CPHA=1) for L3GD20 */
  hspi5.Init.NSS = SPI_NSS_SOFT;                        /* Software NSS management */
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8; /* SPI clock = APB2 clock / 8 - faster for L3GD20 */
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;               /* MSB transmitted first */
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;               /* TI mode disabled */
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; /* CRC calculation disabled */
  hspi5.Init.CRCPolynomial = SPI_CRC_POLYNOMIAL_DEFAULT;                        /* CRC polynomial (not used) */

  /* Initialize the SPI peripheral with the specified parameters */
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }
}

/**
 * @brief   Initializes SPI peripheral with custom configuration
 * @details Allows custom configuration of SPI parameters
 * @param   config Pointer to SPI configuration structure
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_Init_Custom(const SPI_ConfigTypeDef* config)
{
  if (config == NULL)
  {
    return SPI_INVALID_PARAM;
  }

  /* Configure SPI with custom parameters */
  hspi5.Instance = SPI5;
  hspi5.Init.Mode = config->Mode;
  hspi5.Init.Direction = config->Direction;
  hspi5.Init.DataSize = config->DataSize;
  hspi5.Init.CLKPolarity = config->CLKPolarity;
  hspi5.Init.CLKPhase = config->CLKPhase;
  hspi5.Init.NSS = config->NSS;
  hspi5.Init.BaudRatePrescaler = config->BaudRatePrescaler;
  hspi5.Init.FirstBit = config->FirstBit;
  hspi5.Init.TIMode = config->TIMode;
  hspi5.Init.CRCCalculation = config->CRCCalculation;
  hspi5.Init.CRCPolynomial = config->CRCPolynomial;

  /* Initialize the SPI peripheral with the specified parameters */
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    return SPI_ERROR;
  }

  return SPI_OK;
}

/**
 * @brief   Deinitializes SPI peripheral
 * @details Disables SPI peripheral and releases resources
 * @param   None
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_DeInit(void)
{
  if (HAL_SPI_DeInit(&hspi5) != HAL_OK)
  {
    return SPI_ERROR;
  }

  return SPI_OK;
}

/**
 * @brief   Transmit data via SPI
 * @details Sends data buffer via SPI
 * @param   pData Pointer to data buffer to transmit
 * @param   Size Number of bytes to transmit
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_Transmit(uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
  if (pData == NULL || Size == 0)
  {
    return SPI_INVALID_PARAM;
  }

  if (HAL_SPI_Transmit(&hspi5, pData, Size, Timeout) != HAL_OK)
  {
    return SPI_ERROR;
  }

  return SPI_OK;
}

/**
 * @brief   Receive data via SPI
 * @details Receives data via SPI
 * @param   pData Pointer to data buffer to receive
 * @param   Size Number of bytes to receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_Receive(uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
  if (pData == NULL || Size == 0)
  {
    return SPI_INVALID_PARAM;
  }

  if (HAL_SPI_Receive(&hspi5, pData, Size, Timeout) != HAL_OK)
  {
    return SPI_ERROR;
  }

  return SPI_OK;
}

/**
 * @brief   Transmit and receive data simultaneously
 * @details Performs full-duplex SPI communication
 * @param   pTxData Pointer to transmit data buffer
 * @param   pRxData Pointer to receive data buffer
 * @param   Size Number of bytes to transmit/receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_TransmitReceive(uint8_t* pTxData, uint8_t* pRxData, uint16_t Size, uint32_t Timeout)
{
  if (pTxData == NULL || pRxData == NULL || Size == 0)
  {
    return SPI_INVALID_PARAM;
  }

  if (HAL_SPI_TransmitReceive(&hspi5, pTxData, pRxData, Size, Timeout) != HAL_OK)
  {
    return SPI_ERROR;
  }

  return SPI_OK;
}

/**
 * @brief   Get current SPI error status
 * @details Returns detailed error information from SPI peripheral
 * @param   None
 * @retval  uint32_t Error code (HAL_SPI_ERROR_* values)
 */
uint32_t SPI_GetError(void)
{
  return HAL_SPI_GetError(&hspi5);
}

/**
 * @brief   Get SPI status string
 * @details Converts SPI status code to human-readable string
 * @param   status SPI status code
 * @retval  const char* Status description string
 */
const char* SPI_GetStatusString(SPI_StatusTypeDef status)
{
  switch (status)
  {
    case SPI_OK:
      return "SPI_OK";
    case SPI_ERROR:
      return "SPI_ERROR";
    case SPI_BUSY:
      return "SPI_BUSY";
    case SPI_TIMEOUT:
      return "SPI_TIMEOUT";
    case SPI_INVALID_PARAM:
      return "SPI_INVALID_PARAM";
    default:
      return "UNKNOWN_STATUS";
  }
}
