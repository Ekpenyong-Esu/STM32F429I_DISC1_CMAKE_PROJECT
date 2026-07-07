/**
  ******************************************************************************
  * @file    wwdg.h
  * @brief   Window Watchdog (WWDG) module interface
  * @details This file contains all the function prototypes for
  *          the Window Watchdog configuration and control.
  *          It provides APIs to initialize and manage the WWDG
  *          on the STM32F429 board.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

#ifndef __WWDG_H__
#define __WWDG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief WWDG Status enumeration
 */
typedef enum {
    WWDG_OK = 0,           /**< Operation completed successfully */
    WWDG_ERROR,            /**< General error occurred */
    WWDG_TIMEOUT,          /**< Operation timed out */
    WWDG_INVALID_PARAM,    /**< Invalid parameter provided */
    WWDG_WINDOW_ERROR      /**< Refresh outside valid window */
} WWDG_StatusTypeDef;

/**
 * @brief WWDG Configuration structure
 */
typedef struct {
    uint32_t Prescaler;    /**< WWDG prescaler (WWDG_PRESCALER_1/2/4/8) */
    uint32_t Window;       /**< Window value (must be < Counter and >= 0x40) */
    uint32_t Counter;      /**< Counter value (0x40 - 0x7F) */
    uint32_t EWIMode;      /**< Early wakeup interrupt mode */
} WWDG_ConfigTypeDef;

/**
 * @brief WWDG Callback function pointer type
 */
typedef void (*WWDG_EWI_Callback_t)(void);

/* Exported constants --------------------------------------------------------*/

/** @defgroup WWDG_Constants WWDG Driver Constants
 * @{
 */

/**
 * @brief WWDG counter limits
 */
#define WWDG_COUNTER_MIN            0x40U       /**< Minimum counter value */
#define WWDG_COUNTER_MAX            0x7FU       /**< Maximum counter value */
#define WWDG_WINDOW_MIN             0x40U       /**< Minimum window value */
#define WWDG_WINDOW_MAX             0x7FU       /**< Maximum window value */

/**
 * @brief WWDG timing constants
 * @note  PCLK1 = 45MHz on STM32F429 at 180MHz
 */
#define WWDG_PCLK1_FREQ             45000000U   /**< APB1 clock frequency */

/**
 * @brief Common timeout configurations
 */
#define WWDG_TIMEOUT_MIN_US         113U        /**< Minimum timeout (~113µs) */
#define WWDG_TIMEOUT_MAX_MS         58U         /**< Maximum timeout (~58ms) */

/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup WWDG_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize the Window Watchdog
 * @details Configures WWDG with default settings
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Init(void);

/**
 * @brief   Initialize WWDG with custom configuration
 * @details Allows custom configuration of WWDG parameters
 * @param   config Pointer to WWDG configuration structure
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Init_Custom(const WWDG_ConfigTypeDef* config);

/**
 * @brief   Deinitialize the Window Watchdog
 * @details Disables WWDG (only possible before first refresh after reset)
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_DeInit(void);

/** @} */

/** @defgroup WWDG_Operations Watchdog Operations
 * @{
 */

/**
 * @brief   Refresh the watchdog counter
 * @details Must be called within the valid window to prevent reset
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Refresh(void);

/**
 * @brief   Refresh with specific counter value
 * @details Allows setting a new counter value during refresh
 * @param   counter New counter value (0x40 - 0x7F)
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_RefreshWithCounter(uint32_t counter);

/**
 * @brief   Start the watchdog
 * @details Enables WWDG countdown
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Start(void);

/** @} */

/** @defgroup WWDG_Interrupt_Functions Interrupt Functions
 * @{
 */

/**
 * @brief   Register Early Wakeup Interrupt callback
 * @details Callback is called when counter reaches 0x40
 * @param   callback Function pointer to callback
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_RegisterEWICallback(WWDG_EWI_Callback_t callback);

/**
 * @brief   Enable Early Wakeup Interrupt
 * @details Enables interrupt when counter reaches 0x40
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_EnableEWI(void);

/** @} */

/** @defgroup WWDG_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Check if last reset was caused by WWDG
 * @details Reads RCC reset flags to determine reset source
 * @param   None
 * @retval  bool True if WWDG caused the reset
 */
bool WWDG_WasResetSource(void);

/**
 * @brief   Clear WWDG reset flag
 * @details Clears the WWDG reset flag in RCC
 * @param   None
 * @retval  None
 */
void WWDG_ClearResetFlag(void);

/**
 * @brief   Get current counter value
 * @details Reads the current WWDG counter value
 * @param   None
 * @retval  uint32_t Current counter value
 */
uint32_t WWDG_GetCounter(void);

/**
 * @brief   Check if refresh is in valid window
 * @details Checks if current counter is within refresh window
 * @param   None
 * @retval  bool True if refresh is allowed
 */
bool WWDG_IsInWindow(void);

/**
 * @brief   Calculate timeout from configuration
 * @details Calculates timeout values in microseconds
 * @param   prescaler WWDG prescaler value
 * @param   counter Counter value
 * @param   window Window value
 * @param   minTimeout Pointer to store minimum timeout
 * @param   maxTimeout Pointer to store maximum timeout
 * @retval  None
 */
void WWDG_CalculateTimeout(uint32_t prescaler, uint32_t counter, uint32_t window,
                           uint32_t* minTimeout, uint32_t* maxTimeout);

/**
 * @brief   Get WWDG status string
 * @details Converts status code to human-readable string
 * @param   status WWDG status code
 * @retval  const char* Status description string
 */
const char* WWDG_GetStatusString(WWDG_StatusTypeDef status);

/** @} */

/* Exported variables ---------------------------------------------------------*/

#ifdef HAL_WWDG_MODULE_ENABLED
/**
 * @brief   WWDG handle structure
 * @details Used by HAL functions to manage WWDG operations
 */
extern WWDG_HandleTypeDef hwwdg;
#endif /* HAL_WWDG_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __WWDG_H__ */
