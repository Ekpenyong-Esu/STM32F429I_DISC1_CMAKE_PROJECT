/**
  ******************************************************************************
  * @file    iwdg.h
  * @brief   Independent Watchdog (IWDG) module interface
  * @details This file contains all the function prototypes for
  *          the Independent Watchdog configuration and control.
  *          It provides APIs to initialize and manage the IWDG
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

#ifndef __IWDG_H__
#define __IWDG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief IWDG Status enumeration
 */
typedef enum {
    IWDG_OK = 0,           /**< Operation completed successfully */
    IWDG_ERROR,            /**< General error occurred */
    IWDG_TIMEOUT,          /**< Operation timed out */
    IWDG_INVALID_PARAM     /**< Invalid parameter provided */
} IWDG_StatusTypeDef;

/**
 * @brief IWDG Prescaler values
 * @note  Use HAL-defined IWDG_PRESCALER_x values directly:
 *        - IWDG_PRESCALER_4   - Divide by 4
 *        - IWDG_PRESCALER_8   - Divide by 8
 *        - IWDG_PRESCALER_16  - Divide by 16
 *        - IWDG_PRESCALER_32  - Divide by 32
 *        - IWDG_PRESCALER_64  - Divide by 64
 *        - IWDG_PRESCALER_128 - Divide by 128
 *        - IWDG_PRESCALER_256 - Divide by 256
 */

/**
 * @brief IWDG Configuration structure
 */
typedef struct {
    uint32_t Prescaler;    /**< IWDG prescaler value */
    uint32_t Reload;       /**< IWDG reload value (0-4095) */
} IWDG_ConfigTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup IWDG_Constants IWDG Driver Constants
 * @{
 */

/**
 * @brief IWDG timing constants
 * @note  LSI clock is approximately 32kHz
 */
#define IWDG_LSI_FREQ               32000U      /**< LSI oscillator frequency (Hz) */
#define IWDG_RELOAD_MAX             4095U       /**< Maximum reload value */
#define IWDG_RELOAD_MIN             0U          /**< Minimum reload value */

/**
 * @brief Common timeout values (in milliseconds)
 */
#define IWDG_TIMEOUT_100MS          100U        /**< 100ms timeout */
#define IWDG_TIMEOUT_500MS          500U        /**< 500ms timeout */
#define IWDG_TIMEOUT_1S             1000U       /**< 1 second timeout */
#define IWDG_TIMEOUT_2S             2000U       /**< 2 seconds timeout */
#define IWDG_TIMEOUT_4S             4000U       /**< 4 seconds timeout */
#define IWDG_TIMEOUT_8S             8000U       /**< 8 seconds timeout */
#define IWDG_TIMEOUT_16S            16000U      /**< 16 seconds timeout */
#define IWDG_TIMEOUT_32S            32000U      /**< 32 seconds timeout (max) */

/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup IWDG_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize the Independent Watchdog
 * @details Configures IWDG with default 1 second timeout
 * @param   None
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Init(void);

/**
 * @brief   Initialize IWDG with custom configuration
 * @details Allows custom configuration of IWDG parameters
 * @param   config Pointer to IWDG configuration structure
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Init_Custom(const IWDG_ConfigTypeDef* config);

/**
 * @brief   Initialize IWDG with timeout in milliseconds
 * @details Automatically calculates prescaler and reload for desired timeout
 * @param   timeout_ms Desired timeout in milliseconds (max ~32000ms)
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Init_TimeoutMs(uint32_t timeout_ms);

/** @} */

/** @defgroup IWDG_Operations Watchdog Operations
 * @{
 */

/**
 * @brief   Refresh the watchdog counter
 * @details Must be called periodically to prevent system reset
 * @param   None
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Refresh(void);

/**
 * @brief   Start the watchdog (cannot be stopped once started)
 * @details Once started, IWDG cannot be stopped except by reset
 * @param   None
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Start(void);

/** @} */

/** @defgroup IWDG_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Check if last reset was caused by IWDG
 * @details Reads RCC reset flags to determine reset source
 * @param   None
 * @retval  bool True if IWDG caused the reset
 */
bool IWDG_WasResetSource(void);

/**
 * @brief   Clear IWDG reset flag
 * @details Clears the IWDG reset flag in RCC
 * @param   None
 * @retval  None
 */
void IWDG_ClearResetFlag(void);

/**
 * @brief   Calculate timeout from prescaler and reload
 * @details Calculates actual timeout in milliseconds
 * @param   prescaler IWDG prescaler value
 * @param   reload IWDG reload value
 * @retval  uint32_t Timeout in milliseconds
 */
uint32_t IWDG_CalculateTimeout(uint32_t prescaler, uint32_t reload);

/**
 * @brief   Get IWDG status string
 * @details Converts status code to human-readable string
 * @param   status IWDG status code
 * @retval  const char* Status description string
 */
const char* IWDG_GetStatusString(IWDG_StatusTypeDef status);

/** @} */

/* Exported variables ---------------------------------------------------------*/

#ifdef HAL_IWDG_MODULE_ENABLED
/**
 * @brief   IWDG handle structure
 * @details Used by HAL functions to manage IWDG operations
 */
extern IWDG_HandleTypeDef hiwdg;
#endif /* HAL_IWDG_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __IWDG_H__ */
