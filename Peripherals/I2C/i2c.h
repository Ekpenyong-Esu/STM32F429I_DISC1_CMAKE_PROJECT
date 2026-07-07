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
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief I2C Status enumeration
 */
typedef enum {
    I2C_OK = 0,           /**< Operation completed successfully */
    I2C_ERROR,            /**< General error occurred */
    I2C_BUSY,             /**< I2C bus is busy */
    I2C_TIMEOUT,          /**< Operation timed out */
    I2C_NACK,             /**< No acknowledge received */
    I2C_INVALID_PARAM     /**< Invalid parameter provided */
} I2C_StatusTypeDef;

/**
 * @brief I2C Configuration structure
 */
typedef struct {
    uint32_t ClockSpeed;      /**< I2C clock speed (100kHz or 400kHz) */
    uint32_t DutyCycle;       /**< I2C duty cycle (2:1 or 16:9) */
    uint32_t AddressingMode;  /**< 7-bit or 10-bit addressing */
    uint32_t OwnAddress1;     /**< Primary device address */
    uint32_t DualAddressMode; /**< Dual addressing mode */
    uint32_t OwnAddress2;     /**< Secondary device address */
    uint32_t GeneralCallMode; /**< General call mode */
    uint32_t NoStretchMode;   /**< Clock stretching mode */
} I2C_ConfigTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup I2C_Constants I2C Driver Constants
 * @{
 */

/**
 * @brief I2C timeout values
 */
#define I2C_TIMEOUT_DEFAULT     1000U   /**< Default timeout in milliseconds */
#define I2C_TIMEOUT_SHORT       100U    /**< Short timeout for quick operations */
#define I2C_TIMEOUT_LONG        5000U   /**< Long timeout for memory operations */

/**
 * @brief I2C device address range
 */
#define I2C_ADDR_MIN            0x08U   /**< Minimum valid I2C address */
#define I2C_ADDR_MAX            0x77U   /**< Maximum valid I2C address */

/**
 * @brief Common I2C result values
 */
#define I2C_SUCCESS             0       /**< Operation successful */
#define I2C_FAILURE             1       /**< Operation failed */

/** @} */ /* End of I2C_Constants */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup I2C_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initializes I2C peripheral used in the application
 * @details Configures the I2C with appropriate parameters for clock speed,
 *          addressing mode, and enables the peripheral
 * @param   None
 * @retval  None
 */
void I2C_Init(void);

/**
 * @brief   Initializes I2C peripheral with custom configuration
 * @details Allows custom configuration of I2C parameters
 * @param   config Pointer to I2C configuration structure
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_Init_Custom(const I2C_ConfigTypeDef* config);

/**
 * @brief   Deinitializes I2C peripheral
 * @details Disables I2C peripheral and releases resources
 * @param   None
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_DeInit(void);

/** @} */ /* End of I2C_Init_Config */

/** @defgroup I2C_Master_Operations Master Operations
 * @{
 */

/**
 * @brief   Transmit data to I2C slave device
 * @details Sends data buffer to specified I2C slave address
 * @param   DevAddress Target device address (7-bit or 10-bit)
 * @param   pData Pointer to data buffer to transmit
 * @param   Size Number of bytes to transmit
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_Master_Transmit(uint16_t DevAddress, uint8_t* pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief   Receive data from I2C slave device
 * @details Receives data from specified I2C slave address
 * @param   DevAddress Target device address (7-bit or 10-bit)
 * @param   pData Pointer to data buffer to receive
 * @param   Size Number of bytes to receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_Master_Receive(uint16_t DevAddress, uint8_t* pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief   Transmit and receive data in single transaction
 * @details Performs write followed by read operation
 * @param   DevAddress Target device address (7-bit or 10-bit)
 * @param   pTxData Pointer to transmit data buffer
 * @param   TxSize Number of bytes to transmit
 * @param   pRxData Pointer to receive data buffer
 * @param   RxSize Number of bytes to receive
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_Master_TransmitReceive(uint16_t DevAddress,
                                           uint8_t* pTxData, uint16_t TxSize,
                                           uint8_t* pRxData, uint16_t RxSize,
                                           uint32_t Timeout);

/** @} */ /* End of I2C_Master_Operations */

/** @defgroup I2C_Memory_Operations Memory Operations
 * @{
 */

/**
 * @brief   Write data to I2C memory device
 * @details Writes data to specified memory address in I2C EEPROM/Memory device
 * @param   DevAddress Target device address
 * @param   MemAddress Memory address to write to
 * @param   MemAddSize Size of memory address (1 or 2 bytes)
 * @param   pData Pointer to data buffer to write
 * @param   Size Number of bytes to write
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_Mem_Write(uint16_t DevAddress, uint16_t MemAddress,
                               uint16_t MemAddSize, uint8_t* pData,
                               uint16_t Size, uint32_t Timeout);

/**
 * @brief   Read data from I2C memory device
 * @details Reads data from specified memory address in I2C EEPROM/Memory device
 * @param   DevAddress Target device address
 * @param   MemAddress Memory address to read from
 * @param   MemAddSize Size of memory address (1 or 2 bytes)
 * @param   pData Pointer to data buffer to read into
 * @param   Size Number of bytes to read
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_Mem_Read(uint16_t DevAddress, uint16_t MemAddress,
                              uint16_t MemAddSize, uint8_t* pData,
                              uint16_t Size, uint32_t Timeout);


I2C_StatusTypeDef I2C_Mem_Read_Multi(uint16_t DevAddress, uint16_t MemAddress,
                              uint16_t MemAddSize, uint8_t* pData,
                              uint16_t Size, uint32_t Timeout);

/** @} */ /* End of I2C_Memory_Operations */

/** @defgroup I2C_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Check if I2C device is ready/responding
 * @details Tests if target device acknowledges its address
 * @param   DevAddress Target device address
 * @param   Trials Number of trials to attempt
 * @param   Timeout Timeout duration in milliseconds
 * @retval  I2C_StatusTypeDef Operation status
 */
I2C_StatusTypeDef I2C_IsDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

/**
 * @brief   Scan I2C bus for connected devices
 * @details Scans all valid I2C addresses and reports responding devices
 * @param   pDevices Pointer to array to store found device addresses
 * @param   MaxDevices Maximum number of devices to find
 * @param   Timeout Timeout per device check in milliseconds
 * @retval  uint8_t Number of devices found
 */
uint8_t I2C_ScanBus(uint8_t* pDevices, uint8_t MaxDevices, uint32_t Timeout);

/**
 * @brief   Get current I2C error status
 * @details Returns detailed error information from I2C peripheral
 * @param   None
 * @retval  uint32_t Error code (HAL_I2C_ERROR_* values)
 */
uint32_t I2C_GetError(void);

/**
 * @brief   Get I2C status string
 * @details Converts I2C status code to human-readable string
 * @param   status I2C status code
 * @retval  const char* Status description string
 */
const char* I2C_GetStatusString(I2C_StatusTypeDef status);

/** @} */ /* End of I2C_Utility_Functions */

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
