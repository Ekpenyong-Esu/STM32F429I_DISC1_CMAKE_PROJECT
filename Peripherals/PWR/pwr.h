/**
  ******************************************************************************
  * @file    pwr.h
  * @brief   Power Management module interface
  * @details This file contains all the function prototypes for
  *          power management operations including sleep modes,
  *          voltage regulation, and backup domain control.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

#ifndef __PWR_H__
#define __PWR_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief PWR Status enumeration
 */
typedef enum {
    PWR_OK = 0,                /**< Operation completed successfully */
    PWR_ERROR,                 /**< General error occurred */
    PWR_INVALID_PARAM,         /**< Invalid parameter provided */
    PWR_TIMEOUT,               /**< Operation timed out */
    PWR_NOT_READY              /**< Power system not ready */
} PWR_StatusTypeDef;

/**
 * @brief Sleep mode types
 */
typedef enum {
    PWR_SLEEP_MODE_WFI = 0,    /**< Wait For Interrupt - CPU sleeps */
    PWR_SLEEP_MODE_WFE         /**< Wait For Event - CPU sleeps */
} PWR_SleepModeTypeDef;

/**
 * @brief Stop mode entry types
 */
typedef enum {
    PWR_STOP_ENTRY_WFI = 0,    /**< Enter Stop with WFI instruction */
    PWR_STOP_ENTRY_WFE         /**< Enter Stop with WFE instruction */
} PWR_StopEntryTypeDef;

/**
 * @brief Voltage regulator mode for Stop mode
 */
typedef enum {
    PWR_REGULATOR_ON = 0,      /**< Main regulator ON during Stop */
    PWR_REGULATOR_LOW_POWER    /**< Low-power regulator during Stop */
} PWR_RegulatorTypeDef;

/**
 * @brief Standby mode wakeup source
 * @note  Uses PWR_SRC_ prefix to avoid conflicts with HAL PWR_WAKEUP_PIN macros
 * @note  For HAL functions, use HAL's PWR_WAKEUP_PIN1 macro directly
 */
typedef enum {
    PWR_SRC_WAKEUP_PIN = 0x001,        /**< WKUP pin (PA0) - use PWR_WAKEUP_PIN1 for HAL */
    PWR_SRC_RTC_ALARM = 0x002,         /**< RTC Alarm wakeup */
    PWR_SRC_RTC_WAKEUP = 0x004,        /**< RTC Wakeup timer */
    PWR_SRC_RTC_TIMESTAMP = 0x008      /**< RTC Timestamp */
} PWR_WakeupSourceTypeDef;

/**
 * @brief PVD (Programmable Voltage Detector) threshold
 */
typedef enum {
    PWR_PVD_LEVEL_2V0 = PWR_CR_PLS_LEV0, /**< 2.0V threshold */
    PWR_PVD_LEVEL_2V1 = PWR_CR_PLS_LEV1, /**< 2.1V threshold */
    PWR_PVD_LEVEL_2V3 = PWR_CR_PLS_LEV2, /**< 2.3V threshold */
    PWR_PVD_LEVEL_2V5 = PWR_CR_PLS_LEV3, /**< 2.5V threshold */
    PWR_PVD_LEVEL_2V6 = PWR_CR_PLS_LEV4, /**< 2.6V threshold */
    PWR_PVD_LEVEL_2V7 = PWR_CR_PLS_LEV5, /**< 2.7V threshold */
    PWR_PVD_LEVEL_2V8 = PWR_CR_PLS_LEV6, /**< 2.8V threshold */
    PWR_PVD_LEVEL_2V9 = PWR_CR_PLS_LEV7  /**< 2.9V threshold */
} PWR_PVDLevelTypeDef;

/**
 * @brief Power configuration structure
 */
typedef struct {
    bool enablePVD;                     /**< Enable Programmable Voltage Detector */
    PWR_PVDLevelTypeDef pvdLevel;       /**< PVD threshold level */
    bool enableBackupAccess;            /**< Enable backup domain access */
    bool enableWakeupPin;               /**< Enable WKUP pin */
} PWR_ConfigTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup PWR_Constants PWR Driver Constants
 * @{
 */

/**
 * @brief Backup registers count (STM32F429 has 20 backup registers)
 */
#define PWR_BACKUP_REG_COUNT    20U

/**
 * @brief Default configuration timeout
 */
#define PWR_TIMEOUT_VALUE       1000U   /**< 1 second */

/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup PWR_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize PWR module with configuration
 * @details Configures power management settings
 * @param   config Pointer to configuration structure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_Init(const PWR_ConfigTypeDef* config);

/**
 * @brief   Initialize PWR module with default settings
 * @details Sets up basic power management
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_InitDefault(void);

/**
 * @brief   Get default configuration
 * @details Fills structure with default values
 * @param   config Pointer to configuration structure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_GetDefaultConfig(PWR_ConfigTypeDef* config);

/** @} */

/** @defgroup PWR_Sleep_Functions Sleep Mode Functions
 * @{
 */

/**
 * @brief   Enter Sleep mode
 * @details CPU stops, peripherals continue running
 * @param   mode Sleep entry mode (WFI or WFE)
 * @retval  None
 */
void PWR_EnterSleepMode(PWR_SleepModeTypeDef mode);

/**
 * @brief   Enter Sleep mode for specified duration
 * @details Requires SysTick or RTC to be configured
 * @param   duration_ms Sleep duration in milliseconds
 * @retval  None
 */
void PWR_SleepFor(uint32_t duration_ms);

/** @} */

/** @defgroup PWR_Stop_Functions Stop Mode Functions
 * @{
 */

/**
 * @brief   Enter Stop mode
 * @details Most clocks stopped, RAM retained, wake via interrupt
 * @param   regulator Regulator mode (ON or LOW_POWER)
 * @param   entry Entry mode (WFI or WFE)
 * @retval  None
 */
void PWR_EnterStopMode(PWR_RegulatorTypeDef regulator, PWR_StopEntryTypeDef entry);

/**
 * @brief   Configure system after Stop mode wakeup
 * @details Restores clocks and peripherals after Stop mode
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureAfterStop(void);

/** @} */

/** @defgroup PWR_Standby_Functions Standby Mode Functions
 * @{
 */

/**
 * @brief   Enter Standby mode
 * @details Lowest power mode, only wakeup via WKUP pin, RTC, or reset
 *          All RAM content lost except backup domain
 * @param   None
 * @retval  None (does not return if successful)
 */
void PWR_EnterStandbyMode(void);

/**
 * @brief   Enable wakeup pin for Standby mode
 * @details PA0 (WKUP pin) can wake from Standby
 * @param   enable True to enable, false to disable
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableWakeupPin(bool enable);

/**
 * @brief   Check if wakeup was from Standby
 * @details Call early in main() to check wakeup source
 * @param   None
 * @retval  bool True if wakeup was from Standby mode
 */
bool PWR_WasStandbyWakeup(void);

/**
 * @brief   Clear Standby flag
 * @details Should be called after checking Standby wakeup
 * @param   None
 * @retval  None
 */
void PWR_ClearStandbyFlag(void);

/** @} */

/** @defgroup PWR_PVD_Functions PVD (Voltage Detection) Functions
 * @{
 */

/**
 * @brief   Enable Programmable Voltage Detector
 * @details Monitors VDD and triggers interrupt if below threshold
 * @param   level PVD threshold level
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnablePVD(PWR_PVDLevelTypeDef level);

/**
 * @brief   Disable Programmable Voltage Detector
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_DisablePVD(void);

/**
 * @brief   Get PVD output status
 * @details Returns whether VDD is below threshold
 * @param   None
 * @retval  bool True if VDD < threshold (power low)
 */
bool PWR_GetPVDStatus(void);

/**
 * @brief   Enable PVD interrupt
 * @details Requires EXTI line 16 and PVD_IRQHandler
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnablePVDInterrupt(void);

/** @} */

/** @defgroup PWR_Backup_Functions Backup Domain Functions
 * @{
 */

/**
 * @brief   Enable access to backup domain
 * @details Required before accessing RTC and backup registers
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableBackupAccess(void);

/**
 * @brief   Disable access to backup domain
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_DisableBackupAccess(void);

/**
 * @brief   Write to backup register
 * @details Data retained during Standby and VBAT modes
 * @param   regIndex Register index (0-19)
 * @param   data 32-bit data to store
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_WriteBackupRegister(uint32_t regIndex, uint32_t data);

/**
 * @brief   Read from backup register
 * @param   regIndex Register index (0-19)
 * @param   data Pointer to store read data
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ReadBackupRegister(uint32_t regIndex, uint32_t* data);

/** @} */

/** @defgroup PWR_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Get VBAT voltage status
 * @details Checks if VBAT is providing power to backup domain
 * @param   None
 * @retval  bool True if VBAT is active
 */
bool PWR_IsVBATActive(void);

/**
 * @brief   Enable voltage regulator for high performance
 * @details May increase power consumption
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableHighPerformance(void);

/**
 * @brief   Enable voltage regulator low-power mode
 * @details Reduces power consumption at lower performance
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableLowPowerMode(void);

/**
 * @brief   Get status string for error code
 * @param   status PWR status code
 * @retval  const char* Status description
 */
const char* PWR_GetStatusString(PWR_StatusTypeDef status);

/**
 * @brief   Get current power consumption estimate
 * @details Estimates based on active peripherals (approximate)
 * @param   None
 * @retval  uint32_t Estimated current in microamps
 */
uint32_t PWR_GetEstimatedCurrent(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __PWR_H__ */
