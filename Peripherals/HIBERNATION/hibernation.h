/**
  ******************************************************************************
  * @file    hibernation.h
  * @brief   Touchscreen hibernation manager for TFT display
  * @details This module implements automatic hibernation after 2 minutes
  *          of touchscreen inactivity to save power.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

#ifndef __HIBERNATION_H__
#define __HIBERNATION_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup Hibernation_Timeouts Timeout Definitions
 * @{
 */
#define HIBERNATION_TIMEOUT_MS          (2 * 60 * 1000)  /**< 2 minutes in milliseconds */
#define HIBERNATION_CHECK_INTERVAL_MS   (1000)           /**< Check every 1 second */
/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Hibernation status enumeration
 */
typedef enum {
    HIBERNATION_OK = 0,           /**< Operation completed successfully */
    HIBERNATION_ERROR,            /**< General error occurred */
    HIBERNATION_INVALID_PARAM,    /**< Invalid parameter provided */
    HIBERNATION_NOT_INITIALIZED,  /**< Hibernation manager not initialized */
    HIBERNATION_BUSY              /**< Hibernation operation in progress */
} Hibernation_StatusTypeDef;

/**
 * @brief Hibernation state enumeration
 */
typedef enum {
    HIBERNATION_STATE_ACTIVE = 0, /**< Display is active */
    HIBERNATION_STATE_HIBERNATING, /**< System is hibernating */
    HIBERNATION_STATE_WAKING_UP    /**< System is waking up */
} Hibernation_StateTypeDef;

/**
 * @brief Hibernation configuration structure
 */
typedef struct {
    uint32_t timeout_ms;              /**< Inactivity timeout in milliseconds */
    uint32_t check_interval_ms;       /**< How often to check for inactivity */
    bool enable_auto_hibernation;     /**< Enable/disable auto hibernation */
    bool enable_touch_wakeup;         /**< Enable wakeup on touch */
} Hibernation_ConfigTypeDef;

/**
 * @brief Hibernation handle structure
 */
typedef struct {
    Hibernation_StateTypeDef state;           /**< Current hibernation state */
    Hibernation_ConfigTypeDef config;         /**< Configuration settings */
    uint32_t last_activity_timestamp;         /**< Last touch activity timestamp */
    uint32_t hibernation_start_time;          /**< When hibernation started */
    bool is_initialized;                      /**< Initialization flag */
} Hibernation_HandleTypeDef;

/* Exported functions --------------------------------------------------------*/

/** @defgroup Hibernation_Functions Initialization Functions
 * @{
 */

/**
 * @brief   Initialize hibernation manager
 * @details Sets up the hibernation system with default configuration
 * @param   hhib Pointer to hibernation handle
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_Init(Hibernation_HandleTypeDef *hhib);

/**
 * @brief   Initialize hibernation manager with custom configuration
 * @details Sets up the hibernation system with user configuration
 * @param   hhib Pointer to hibernation handle
 * @param   config Pointer to configuration structure
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_InitWithConfig(Hibernation_HandleTypeDef *hhib,
                                                    Hibernation_ConfigTypeDef *config);

/**
 * @brief   Deinitialize hibernation manager
 * @details Cleans up hibernation system resources
 * @param   hhib Pointer to hibernation handle
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_DeInit(Hibernation_HandleTypeDef *hhib);

/**
 * @brief   Get default hibernation configuration
 * @details Returns default configuration values
 * @param   config Pointer to configuration structure to fill
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_GetDefaultConfig(Hibernation_ConfigTypeDef *config);

/** @} */

/** @defgroup Hibernation_Control Control Functions
 * @{
 */

/**
 * @brief   Configure hibernation settings
 * @details Updates hibernation configuration
 * @param   hhib Pointer to hibernation handle
 * @param   config Pointer to new configuration
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_Configure(Hibernation_HandleTypeDef *hhib,
                                              Hibernation_ConfigTypeDef *config);

/**
 * @brief   Enable/disable auto hibernation
 * @details Controls whether hibernation occurs automatically
 * @param   hhib Pointer to hibernation handle
 * @param   enable True to enable, false to disable
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_EnableAutoHibernation(Hibernation_HandleTypeDef *hhib,
                                                          bool enable);

/**
 * @brief   Force immediate hibernation
 * @details Puts system into hibernation immediately
 * @param   hhib Pointer to hibernation handle
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_EnterHibernation(Hibernation_HandleTypeDef *hhib);

/**
 * @brief   Force immediate wakeup
 * @details Wakes system from hibernation immediately
 * @param   hhib Pointer to hibernation handle
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_WakeUp(Hibernation_HandleTypeDef *hhib);

/** @} */

/** @defgroup Hibernation_Monitoring Monitoring Functions
 * @{
 */

/**
 * @brief   Report touch activity
 * @details Should be called whenever touchscreen activity is detected
 * @param   hhib Pointer to hibernation handle
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_ReportTouchActivity(Hibernation_HandleTypeDef *hhib);

/**
 * @brief   Check hibernation status and handle timeout
 * @details Should be called periodically to check for hibernation timeout
 * @param   hhib Pointer to hibernation handle
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_Process(Hibernation_HandleTypeDef *hhib);

/**
 * @brief   Get current hibernation state
 * @details Returns the current state of the hibernation system
 * @param   hhib Pointer to hibernation handle
 * @param   state Pointer to store current state
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_GetState(Hibernation_HandleTypeDef *hhib,
                                             Hibernation_StateTypeDef *state);

/**
 * @brief   Get time until hibernation
 * @details Returns milliseconds remaining until hibernation occurs
 * @param   hhib Pointer to hibernation handle
 * @param   remaining_ms Pointer to store remaining time
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_GetTimeUntilHibernation(Hibernation_HandleTypeDef *hhib,
                                                            uint32_t *remaining_ms);

/** @} */

/** @defgroup Hibernation_Callbacks Callback Functions
 * @{
 */

/**
 * @brief   Callback function type for hibernation events
 * @param   state New hibernation state
 */
typedef void (*Hibernation_CallbackTypeDef)(Hibernation_StateTypeDef state);

/**
 * @brief   Register hibernation callback
 * @details Sets callback function for hibernation state changes
 * @param   hhib Pointer to hibernation handle
 * @param   callback Callback function pointer
 * @retval  Hibernation_StatusTypeDef Operation status
 */
Hibernation_StatusTypeDef Hibernation_RegisterCallback(Hibernation_HandleTypeDef *hhib,
                                                     Hibernation_CallbackTypeDef callback);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __HIBERNATION_H__ */
