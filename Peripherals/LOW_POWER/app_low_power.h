/**
  ******************************************************************************
  * @file    app_low_power.h
  * @brief   Application-specific low power mode interface
  * @details This file contains function prototypes for GUI application
  *          low power management including display and touchscreen control.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

#ifndef __APP_LOW_POWER_H__
#define __APP_LOW_POWER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pwr.h"
#include <stdbool.h>

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize application low power management
 * @details Sets up activity monitoring and low power timeouts
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef APP_LowPowerInit(void);

/**
 * @brief   Update activity timestamp
 * @details Call this function whenever user interacts with the GUI
 * @retval  None
 */
void APP_UpdateActivity(void);
void APP_TouchActivity(void);

/**
 * @brief   Check if system should enter low power mode
 * @details Monitors activity and decides if low power mode is needed
 * @retval  bool True if low power mode should be entered
 */
bool APP_ShouldEnterLowPower(void);

/**
 * @brief   Enter application-optimized low power mode
 * @details Automatically selects best low power mode based on inactivity time
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef APP_EnterLowPowerMode(void);

/* Auto-sleep request helpers (set internally by dim complete callback) */
bool APP_IsAutoSleepRequested(void);
void APP_ClearAutoSleepRequest(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LOW_POWER_H__ */
