/**
  ******************************************************************************
  * @file    rng.h
  * @brief   Random Number Generator (RNG) module interface
  * @details This file contains all the function prototypes for
  *          the hardware Random Number Generator configuration and control.
  *          It provides APIs to generate true random numbers using
  *          the STM32F429's hardware RNG peripheral.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

#ifndef __RNG_H__
#define __RNG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief RNG Status enumeration
 */
typedef enum {
    RNG_OK = 0,            /**< Operation completed successfully */
    RNG_ERROR,             /**< General error occurred */
    RNG_TIMEOUT,           /**< Operation timed out */
    RNG_CLOCK_ERROR,       /**< Clock configuration error */
    RNG_SEED_ERROR,        /**< Seed error detected */
    RNG_NOT_READY          /**< RNG not ready */
} RNG_StatusTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup RNG_Constants RNG Driver Constants
 * @{
 */

/**
 * @brief RNG timeout value
 */
#define RNG_TIMEOUT_DEFAULT     1000U   /**< Default timeout in milliseconds */

/**
 * @brief RNG generation modes
 */
#define RNG_MODE_BLOCKING       0U      /**< Blocking mode */
#define RNG_MODE_INTERRUPT      1U      /**< Interrupt mode */

/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup RNG_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize the Random Number Generator
 * @details Configures and enables the RNG peripheral
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_Init(void);

/**
 * @brief   Deinitialize the Random Number Generator
 * @details Disables RNG peripheral and releases resources
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_DeInit(void);

/** @} */

/** @defgroup RNG_Generation Random Number Generation
 * @{
 */

/**
 * @brief   Generate a 32-bit random number
 * @details Generates a single 32-bit true random number
 * @param   randomNumber Pointer to store the random number
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_Generate(uint32_t* randomNumber);

/**
 * @brief   Generate a random number with timeout
 * @details Generates random number with specified timeout
 * @param   randomNumber Pointer to store the random number
 * @param   timeout Timeout in milliseconds
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateWithTimeout(uint32_t* randomNumber, uint32_t timeout);

/**
 * @brief   Generate multiple random numbers
 * @details Fills buffer with random 32-bit values
 * @param   buffer Pointer to buffer for random numbers
 * @param   count Number of 32-bit random numbers to generate
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateBuffer(uint32_t* buffer, uint32_t count);

/**
 * @brief   Generate random bytes
 * @details Fills buffer with random bytes
 * @param   buffer Pointer to byte buffer
 * @param   length Number of bytes to generate
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateBytes(uint8_t* buffer, uint32_t length);

/** @} */

/** @defgroup RNG_Range_Functions Range-Limited Generation
 * @{
 */

/**
 * @brief   Generate random number in range [0, max)
 * @details Returns random number less than max
 * @param   max Upper bound (exclusive)
 * @param   result Pointer to store result
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateRange(uint32_t max, uint32_t* result);

/**
 * @brief   Generate random number in range [min, max]
 * @details Returns random number between min and max inclusive
 * @param   min Lower bound (inclusive)
 * @param   max Upper bound (inclusive)
 * @param   result Pointer to store result
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateInRange(uint32_t min, uint32_t max, uint32_t* result);

/**
 * @brief   Generate random boolean
 * @details Returns true or false randomly
 * @param   result Pointer to store boolean result
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateBool(bool* result);

/**
 * @brief   Generate random float [0.0, 1.0)
 * @details Returns random float in range [0, 1)
 * @param   result Pointer to store float result
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateFloat(float* result);

/** @} */

/** @defgroup RNG_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Check if RNG is ready
 * @details Checks if RNG has random data available
 * @param   None
 * @retval  bool True if random number is ready
 */
bool RNG_IsReady(void);

/**
 * @brief   Check for RNG errors
 * @details Checks for seed or clock errors
 * @param   None
 * @retval  bool True if error detected
 */
bool RNG_HasError(void);

/**
 * @brief   Clear RNG errors
 * @details Clears any pending error flags
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_ClearErrors(void);

/**
 * @brief   Get RNG status string
 * @details Converts status code to human-readable string
 * @param   status RNG status code
 * @retval  const char* Status description string
 */
const char* RNG_GetStatusString(RNG_StatusTypeDef status);

/** @} */

/** @defgroup RNG_Interrupt_Functions Interrupt Functions
 * @{
 */

/**
 * @brief   Enable RNG interrupt mode
 * @details Enables interrupt-based random number generation
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_EnableInterrupt(void);

/**
 * @brief   Disable RNG interrupt mode
 * @details Disables interrupt-based generation
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_DisableInterrupt(void);

/** @} */

/* Exported variables ---------------------------------------------------------*/

#ifdef HAL_RNG_MODULE_ENABLED
/**
 * @brief   RNG handle structure
 * @details Used by HAL functions to manage RNG operations
 */
extern RNG_HandleTypeDef hrng;
#endif /* HAL_RNG_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __RNG_H__ */
