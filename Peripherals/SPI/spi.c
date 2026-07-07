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
#include "log.h"
#include "stm32f4xx_hal_spi.h"

/* Private defines -----------------------------------------------------------*/
#define SPI_CRC_POLYNOMIAL_DEFAULT    10U     /**< Default CRC polynomial value */

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   SPI handle structure
 * @details Used by HAL functions for SPI4 peripheral operations
 */
SPI_HandleTypeDef hspi4;

/* Private function prototypes -----------------------------------------------*/
static SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus);
static void SPIx_Error(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Convert HAL status to SPI status
 * @param   halStatus HAL status code
 * @retval  SPI_StatusTypeDef Converted status
 */
static SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus)
    {
        case HAL_OK:
            return SPI_OK;
        case HAL_ERROR:
            return SPI_ERROR;
        case HAL_BUSY:
            return SPI_BUSY;
        case HAL_TIMEOUT:
            return SPI_TIMEOUT;
        default:
            return SPI_ERROR;
    }
}

/**
 * @brief   SPI error recovery function
 * @details Deinitializes and reinitializes SPI peripheral on error
 * @param   None
 * @retval  None
 */
static void SPIx_Error(void)
{
    /* De-initialize the SPI communication BUS */
    SPI_DeInit();

    /* Re-Initialize the SPI communication BUS */
    SPI_Init();
}

/**
  * @brief  SPI Initialization Function
  * @details Configures the SPI4 peripheral with the following settings:
  *          - Master mode operation
  *          - Full-duplex (2 lines) communication
  *          - 8-bit data size
  *          - Low clock polarity (CPOL=0) - ILI9341 requirement
  *          - First clock transition is the data capture edge (CPHA=0) - ILI9341 requirement
  *          - Software NSS management
  *          - Baud rate = fPCLK/16 (conservative for LCD stability)
  *          - MSB transmitted/received first
  *          - TI mode disabled
  *          - CRC calculation disabled
  *
  * @note   SPI4 is used for LCD/touch SPI communication (SPI Mode 0)
  *         ILI9341 requires CPOL=0, CPHA=0 (clock idle LOW, sample on rising edge)
  * @param  None
  * @retval None
  */
void SPI_Init(void)
{
    log_debug("SPI: Initializing SPI4");
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;                                /* Select SPI4 peripheral */
  hspi4.Init.Mode = SPI_MODE_MASTER;                    /* Configure as master */
  hspi4.Init.Direction = SPI_DIRECTION_2LINES;          /* Full-duplex mode */
  hspi4.Init.DataSize = SPI_DATASIZE_8BIT;              /* 8-bit data size */
  hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;            /* Clock polarity low (CPOL=0) for ILI9341 */
  hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;                /* Clock phase 1st edge (CPHA=0) for ILI9341 */
  hspi4.Init.NSS = SPI_NSS_SOFT;                        /* Software NSS management */
  /* Choose prescaler to reach ~40 MHz SPI SCLK for display/touch.
   * With SystemClock = 168 MHz, APB2 runs at 84 MHz; using prescaler 2 yields SCLK = 84/2 = 42 MHz (~40 MHz target).
   * Use the smallest prescaler (2) that keeps SCLK within tolerable range for the display.
   */
  /* Use a safer default SPI speed for troubleshooting. Lowering from prescaler 2 (~42 MHz)
   * to prescaler 32 (~2.6 MHz) so the XPT2046 touch controller runs reliably at the same
   * SPI bus used for the LCD. If you prefer full LCD throughput, switch SPI speed only
   * for touch reads instead of changing the global prescaler.
   */
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; /* SPI clock = APB2 clock / 8 (≈10.5 MHz) */
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;               /* MSB transmitted first */
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;               /* TI mode disabled */
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; /* CRC calculation disabled */
  hspi4.Init.CRCPolynomial = SPI_CRC_POLYNOMIAL_DEFAULT;                        /* CRC polynomial (not used) */

  /* Initialize the SPI peripheral with the specified parameters */
   HAL_SPI_Init(&hspi4);
  log_debug("SPI: SPI4 initialized successfully");
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
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = config->Mode;
  hspi4.Init.Direction = config->Direction;
  hspi4.Init.DataSize = config->DataSize;
  hspi4.Init.CLKPolarity = config->CLKPolarity;
  hspi4.Init.CLKPhase = config->CLKPhase;
  hspi4.Init.NSS = config->NSS;
  hspi4.Init.BaudRatePrescaler = config->BaudRatePrescaler;
  hspi4.Init.FirstBit = config->FirstBit;
  hspi4.Init.TIMode = config->TIMode;
  hspi4.Init.CRCCalculation = config->CRCCalculation;
  hspi4.Init.CRCPolynomial = config->CRCPolynomial;

  /* Initialize the SPI peripheral with the specified parameters */
   HAL_SPI_Init(&hspi4);
   return SPI_OK;
}

/**
 * @brief   Change SPI baud-rate prescaler at runtime
 * @details De-initializes and re-initializes SPI peripheral with the new
 *          BaudRatePrescaler. Validates prescaler value before applying.
 * @param   BaudRatePrescaler One of SPI_BAUDRATEPRESCALER_2 .. _256
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_SetBaudRatePrescaler(uint32_t BaudRatePrescaler)
{
    /* Validate supported prescaler constants */
    switch (BaudRatePrescaler) {
        case SPI_BAUDRATEPRESCALER_2:
        case SPI_BAUDRATEPRESCALER_4:
        case SPI_BAUDRATEPRESCALER_8:
        case SPI_BAUDRATEPRESCALER_16:
        case SPI_BAUDRATEPRESCALER_32:
        case SPI_BAUDRATEPRESCALER_64:
        case SPI_BAUDRATEPRESCALER_128:
        case SPI_BAUDRATEPRESCALER_256:
            break;
        default:
            return SPI_INVALID_PARAM;
    }

    /* Nothing to do if already at requested speed */
    if (hspi4.Init.BaudRatePrescaler == BaudRatePrescaler) {
        return SPI_OK;
    }

    /* De-initialize and re-initialize the peripheral with new prescaler */
    HAL_StatusTypeDef hal = HAL_SPI_DeInit(&hspi4);
    if (hal != HAL_OK) {
        return SPI_ConvertHALStatus(hal);
    }

    hspi4.Init.BaudRatePrescaler = BaudRatePrescaler;
    hal = HAL_SPI_Init(&hspi4);
    return SPI_ConvertHALStatus(hal);
}


/**
 * @brief   Deinitializes SPI peripheral
 * @details Disables SPI peripheral and releases resources
 * @param   None
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_DeInit(void)
{
  if (HAL_SPI_DeInit(&hspi4) != HAL_OK)
  {
    return SPI_ERROR;
  }

  return SPI_OK;
}

/**
 * @brief   Transmit data via SPI
 * @details Sends data buffer via SPI with automatic error recovery
 * @param   pData Pointer to data buffer to transmit
 * @param   Size Number of bytes to transmit
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls SPIx_Error() to reset the SPI bus
 */
SPI_StatusTypeDef SPI_Transmit(uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
  if (pData == NULL || Size == 0)
  {
    return SPI_INVALID_PARAM;
  }

  HAL_StatusTypeDef halStatus = HAL_SPI_Transmit(&hspi4, pData, Size, Timeout);
  if (halStatus != HAL_OK)
  {
    /* Re-Initialize the BUS on error for automatic recovery */
    SPIx_Error();
    return SPI_ConvertHALStatus(halStatus);
  }

  return SPI_OK;
}

/**
 * @brief   Receive data via SPI
 * @details Receives data via SPI with automatic error recovery
 * @param   pData Pointer to data buffer to receive
 * @param   Size Number of bytes to receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls SPIx_Error() to reset the SPI bus
 */
SPI_StatusTypeDef SPI_Receive(uint8_t* pData, uint16_t Size, uint32_t Timeout)
{
  if (pData == NULL || Size == 0)
  {
    return SPI_INVALID_PARAM;
  }

  HAL_StatusTypeDef halStatus = HAL_SPI_Receive(&hspi4, pData, Size, Timeout);
  if (halStatus != HAL_OK)
  {
    /* Re-Initialize the BUS on error for automatic recovery */
    SPIx_Error();
    return SPI_ConvertHALStatus(halStatus);
  }

  return SPI_OK;
}

/**
 * @brief   Transmit and receive data simultaneously
 * @details Performs full-duplex SPI communication with automatic error recovery
 * @param   pTxData Pointer to transmit data buffer
 * @param   pRxData Pointer to receive data buffer
 * @param   Size Number of bytes to transmit/receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 * @note    On HAL error, automatically calls SPIx_Error() to reset the SPI bus
 */
SPI_StatusTypeDef SPI_TransmitReceive(uint8_t* pTxData, uint8_t* pRxData, uint16_t Size, uint32_t Timeout)
{
  if (pTxData == NULL || pRxData == NULL || Size == 0)
  {
    return SPI_INVALID_PARAM;
  }

  HAL_StatusTypeDef halStatus = HAL_SPI_TransmitReceive(&hspi4, pTxData, pRxData, Size, Timeout);
  if (halStatus != HAL_OK)
  {
    /* Re-Initialize the BUS on error for automatic recovery */
    SPIx_Error();
    return SPI_ConvertHALStatus(halStatus);
  }

  return SPI_OK;
}

/* --- DMA-based helper functions --- */
static volatile uint8_t spi_dma_tx_done = 0;

SPI_StatusTypeDef SPI_Transmit_DMA(uint8_t* pData, uint16_t Size)
{
  if (pData == NULL || Size == 0)
  {
    return SPI_INVALID_PARAM;
  }

  spi_dma_tx_done = 0;
  if (HAL_SPI_Transmit_DMA(&hspi4, pData, Size) != HAL_OK)
  {
    /* Re-Initialize the BUS on error for automatic recovery */
    SPIx_Error();
    return SPI_ERROR;
  }

  /* Wait for DMA completion (blocking). Caller can use SPI_WaitReady for non-blocking behavior) */
  if (SPI_WaitReady(SPI_TIMEOUT_LONG) != SPI_OK) {
      return SPI_TIMEOUT;
  }

  return SPI_OK;
}

SPI_StatusTypeDef SPI_WaitReady(uint32_t Timeout)
{
  uint32_t start = HAL_GetTick();
  while (!spi_dma_tx_done)
  {
    if ((HAL_GetTick() - start) > Timeout)
    {
      return SPI_TIMEOUT;
    }
  }
  return SPI_OK;
}

/* HAL callbacks to set DMA completion flag and handle errors */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi == &hspi4) {
    spi_dma_tx_done = 1;
  }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi == &hspi4) {
    /* Attempt recovery on error */
    SPIx_Error();
    spi_dma_tx_done = 1;
  }
}

/**
 * @brief   Get current SPI error status
 * @details Returns detailed error information from SPI peripheral
 * @param   None
 * @retval  uint32_t Error code (HAL_SPI_ERROR_* values)
 */
uint32_t SPI_GetError(void)
{
  return HAL_SPI_GetError(&hspi4);
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
