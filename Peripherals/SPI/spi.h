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
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief SPI Status enumeration
 */
typedef enum {
    SPI_OK = 0,           /**< Operation completed successfully */
    SPI_ERROR,            /**< General error occurred */
    SPI_BUSY,             /**< SPI bus is busy */
    SPI_TIMEOUT,          /**< Operation timed out */
    SPI_INVALID_PARAM     /**< Invalid parameter provided */
} SPI_StatusTypeDef;

/**
 * @brief SPI Configuration structure
 */
typedef struct {
    uint32_t Mode;              /**< Master/Slave mode */
    uint32_t Direction;         /**< Communication direction */
    uint32_t DataSize;          /**< Data size (8-bit or 16-bit) */
    uint32_t CLKPolarity;       /**< Clock polarity */
    uint32_t CLKPhase;          /**< Clock phase */
    uint32_t NSS;               /**< NSS management */
    uint32_t BaudRatePrescaler; /**< Baud rate prescaler */
    uint32_t FirstBit;          /**< MSB/LSB first */
    uint32_t TIMode;            /**< TI mode */
    uint32_t CRCCalculation;    /**< CRC calculation */
    uint32_t CRCPolynomial;     /**< CRC polynomial */
} SPI_ConfigTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup SPI_Constants SPI Driver Constants
 * @{
 */

/**
 * @brief SPI timeout values
 */
#define SPI_TIMEOUT_DEFAULT     1000U   /**< Default timeout in milliseconds */
#define SPI_TIMEOUT_SHORT       100U    /**< Short timeout for quick operations */
#define SPI_TIMEOUT_LONG        5000U   /**< Long timeout for memory operations */

/**
 * @brief Common SPI result values
 */
#define SPI_SUCCESS             0       /**< Operation successful */
#define SPI_FAILURE             1       /**< Operation failed */

/** @} */ /* End of SPI_Constants */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup SPI_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initializes SPI peripheral used in the application
 * @details Configures the SPI with appropriate parameters for clock speed,
 *          data format, mode and enables the peripheral
 * @param   None
 * @retval  None
 */
void SPI_Init(void);

/**
 * @brief   Initializes SPI peripheral with custom configuration
 * @details Allows custom configuration of SPI parameters
 * @param   config Pointer to SPI configuration structure
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_Init_Custom(const SPI_ConfigTypeDef* config);

/**
 * @brief   Change SPI baud-rate prescaler at runtime
 * @details Allows switching SPI clock speed without changing other settings.
 *          Useful for boards where LCD needs high speed but shared touch
 *          controller (XPT2046) requires a lower SPI clock.
 * @param   BaudRatePrescaler One of HAL SPI_BAUDRATEPRESCALER_2.._256 constants
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_SetBaudRatePrescaler(uint32_t BaudRatePrescaler);

/**
 * @brief   Deinitializes SPI peripheral
 * @details Disables SPI peripheral and releases resources
 * @param   None
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_DeInit(void);

/** @} */ /* End of SPI_Init_Config */

/** @defgroup SPI_Data_Operations Data Operations
 * @{
 */

/**
 * @brief   Transmit data via SPI
 * @details Sends data buffer via SPI
 * @param   pData Pointer to data buffer to transmit
 * @param   Size Number of bytes to transmit
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_Transmit(uint8_t* pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief   Receive data via SPI
 * @details Receives data via SPI
 * @param   pData Pointer to data buffer to receive
 * @param   Size Number of bytes to receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_Receive(uint8_t* pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief   Transmit and receive data simultaneously
 * @details Performs full-duplex SPI communication
 * @param   pTxData Pointer to transmit data buffer
 * @param   pRxData Pointer to receive data buffer
 * @param   Size Number of bytes to transmit/receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  SPI_StatusTypeDef Operation status
 */
SPI_StatusTypeDef SPI_TransmitReceive(uint8_t* pTxData, uint8_t* pRxData, uint16_t Size, uint32_t Timeout);

/** DMA-based transmit (blocking until complete). */
SPI_StatusTypeDef SPI_Transmit_DMA(uint8_t* pData, uint16_t Size);

/** Wait until SPI/DMA transfers are complete (returns SPI_TIMEOUT on timeout). */
SPI_StatusTypeDef SPI_WaitReady(uint32_t Timeout);

/** @} */ /* End of SPI_Data_Operations */

/** @defgroup SPI_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Get current SPI error status
 * @details Returns detailed error information from SPI peripheral
 * @param   None
 * @retval  uint32_t Error code (HAL_SPI_ERROR_* values)
 */
uint32_t SPI_GetError(void);

/**
 * @brief   Get SPI status string
 * @details Converts SPI status code to human-readable string
 * @param   status SPI status code
 * @retval  const char* Status description string
 */
const char* SPI_GetStatusString(SPI_StatusTypeDef status);

/** @} */ /* End of SPI_Utility_Functions */

/* Exported variables ---------------------------------------------------------*/

/**
 * @brief   SPI handle structure
 * @details Used by HAL functions to manage SPI4 operations
 *          typically used for display or external sensor communication
 */
extern SPI_HandleTypeDef hspi4;

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */
