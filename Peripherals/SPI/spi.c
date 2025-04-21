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

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   SPI5 handle structure
 * @details Used by HAL functions for SPI5 peripheral operations
 */
SPI_HandleTypeDef hspi5;

/**
  * @brief  SPI Initialization Function
  * @details Configures the SPI5 peripheral with the following settings:
  *          - Master mode operation
  *          - Full-duplex (2 lines) communication
  *          - 8-bit data size
  *          - Low clock polarity (CPOL=0)
  *          - First clock transition is the first data capture edge (CPHA=0)
  *          - Software NSS management
  *          - Baud rate = fPCLK/16
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
  hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;            /* Clock polarity low (CPOL=0) */
  hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;                /* Clock phase 1st edge (CPHA=0) */
  hspi5.Init.NSS = SPI_NSS_SOFT;                        /* Software NSS management */
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; /* SPI clock = APB2 clock / 16 */
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;               /* MSB transmitted first */
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;               /* TI mode disabled */
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; /* CRC calculation disabled */
  hspi5.Init.CRCPolynomial = 10;                        /* CRC polynomial (not used) */

  /* Initialize the SPI peripheral with the specified parameters */
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    Error_Handler();  /* Call error handler if initialization fails */
  }
}
