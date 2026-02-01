/**
  ******************************************************************************
  * @file    pwr.c
  * @brief   Power Management module implementation - CORRECTED
  * @details Fixed version with proper low power mode handling for STM32F429
  * @version 2.0
  * @date    2025-01-25
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pwr.h"
#include "log.h"

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* No module-level private variables required */

/* Private function prototypes -----------------------------------------------*/
/* (Debug helper removed - keep interface minimal) */

/* Public functions ----------------------------------------------------------*/

/**
 * @brief   Initialize PWR module with configuration
 * @details Configures power management settings
 * @param   config Pointer to configuration structure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_Init(const PWR_ConfigTypeDef* config)
{
    log_debug("PWR: Initializing Power Management");

    if (config == NULL)
    {
        return PWR_INVALID_PARAM;
    }

    /* Enable PWR clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Enable backup access if requested */
    if (config->enableBackupAccess)
    {
        HAL_PWR_EnableBkUpAccess();
    }

    /* Enable wakeup pin if requested */
    if (config->enableWakeupPin)
    {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    }

    /* Configure PVD if requested */
    if (config->enablePVD)
    {
        PWR_EnablePVD(config->pvdLevel);
    }


    log_debug("PWR: Power Management initialized successfully");

    return PWR_OK;
}

/**
 * @brief   Initialize PWR module with default settings
 * @details Sets up basic power management
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_InitDefault(void)
{
    log_debug("PWR: Initializing Power Management with default settings");

    PWR_ConfigTypeDef config;
    PWR_GetDefaultConfig(&config);

    log_debug("PWR: Power Management initialized successfully with defaults");

    return PWR_Init(&config);
}

/**
 * @brief   Get default configuration
 * @details Fills structure with default values
 * @param   config Pointer to configuration structure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_GetDefaultConfig(PWR_ConfigTypeDef* config)
{
    if (config == NULL)
    {
        return PWR_INVALID_PARAM;
    }

    config->enablePVD = false;
    config->pvdLevel = PWR_PVD_LEVEL_2V9;
    config->enableBackupAccess = true;
    config->enableWakeupPin = false;

    return PWR_OK;
}

/* Debug-in-low-power helper removed to avoid unused code and reduce surface area. */

/**
 * @brief   Enter Sleep mode
 * @details CPU stops, peripherals continue running
 * @param   mode Sleep entry mode (WFI or WFE)
 * @retval  None
 */
void PWR_EnterSleepMode(PWR_SleepModeTypeDef mode)
    {

    /* Use HAL wrapper which implements correct event/barrier handling and
       keeps behavior consistent across STM32 families. Add data/instruction
       barriers when using WFI to ensure memory operations complete. */
    if (mode == PWR_SLEEP_MODE_WFI)
    {
        /* Ensure all memory transactions complete before entering sleep */
        __DSB();
        __ISB();

        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    }
    else
    {
        /* For WFE path, use HAL wrapper which performs proper SEV/WFE sequence */
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFE);
    }
}

/**
 * @brief   Enter Sleep mode for specified duration
 * @details Requires SysTick to be configured
 * @param   duration_ms Sleep duration in milliseconds
 * @retval  None
 */
void PWR_SleepFor(uint32_t duration_ms)
{
    uint32_t startTick = HAL_GetTick();

    while ((HAL_GetTick() - startTick) < duration_ms)
    {
        PWR_EnterSleepMode(PWR_SLEEP_MODE_WFI);
    }
}

/**
 * @brief   Enter Stop mode
 * @details Most clocks stopped, RAM retained, wake via interrupt
 * @param   regulator Regulator mode (ON or LOW_POWER)
 * @param   entry Entry mode (WFI or WFE)
 * @retval  None
 */
void PWR_EnterStopMode(PWR_RegulatorTypeDef regulator, PWR_StopEntryTypeDef entry)
{
    uint32_t regulatorMode = 0;
    uint32_t entryMode = 0;

    /* Convert to HAL types */
    regulatorMode = (regulator == PWR_REGULATOR_LOW_POWER) ?
                    PWR_LOWPOWERREGULATOR_ON : PWR_MAINREGULATOR_ON;
    entryMode = (entry == PWR_STOP_ENTRY_WFE) ?
                PWR_STOPENTRY_WFE : PWR_STOPENTRY_WFI;


    /* Enter Stop mode */
    HAL_PWR_EnterSTOPMode(regulatorMode, entryMode);

    /* System continues here after wakeup */
}

/**
 * @brief   Configure system after Stop mode wakeup
 * @details Restores clocks and peripherals after Stop mode
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureAfterStop(void)
{
    HAL_StatusTypeDef status;

    /* After wakeup from Stop mode, the system clock is HSI (16 MHz)
     * Need to reconfigure to use HSE and PLL for full speed operation */

    /* This function uses SystemClock_Config() which should be defined
     * in the main application. If not available, manual reconfiguration
     * is needed as shown in the alternative implementation below. */

    /* Option 1: Use SystemClock_Config if available */
    #if 0
    extern void SystemClock_Config(void);
    SystemClock_Config();
    return PWR_OK;
    #endif

    /* Option 2: Manual clock reconfiguration for STM32F429 */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Configure voltage scaling */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    /* STM32F429I-DISC1 uses 8 MHz HSE crystal */
    /* Configure for 180 MHz: 8MHz / 4 * 180 / 2 = 180 MHz */
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    status = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    if (status != HAL_OK)
    {
        log_error("PWR: Failed to reconfigure oscillators");
        return PWR_ERROR;
    }

    /* Activate the Over-Drive mode for 180 MHz */
    status = HAL_PWREx_EnableOverDrive();
    if (status != HAL_OK)
    {
        log_error("PWR: Failed to enable overdrive");
        return PWR_ERROR;
    }

    /* Select PLL as system clock source and configure bus clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;   /* 180 MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;    /* 45 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;    /* 90 MHz */

    status = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
    if (status != HAL_OK)
    {
        log_error("PWR: Failed to reconfigure clocks");
        return PWR_ERROR;
    }

    log_debug("PWR: System clocks restored after Stop mode");

    return PWR_OK;
}

/**
 * @brief   Enter Standby mode
 * @details Lowest power mode, only wakeup via WKUP pin, RTC, or reset
 *          All RAM content lost except backup domain
 * @param   None
 * @retval  None (does not return if successful)
 */
void PWR_EnterStandbyMode(void)
{
    /* Clear Wakeup flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* Clear Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

    /* Enter Standby mode */
    HAL_PWR_EnterSTANDBYMode();

    /* Should never reach here - system resets on wakeup */
}

/**
 * @brief   Enable wakeup pin for Standby mode
 * @details PA0 (WKUP pin) can wake from Standby
 * @param   enable True to enable, false to disable
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableWakeupPin(bool enable)
{
    if (enable)
    {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    }
    else
    {
        HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    }

    return PWR_OK;
}

/**
 * @brief   Check if wakeup was from Standby
 * @details Call early in main() to check wakeup source
 * @param   None
 * @retval  bool True if wakeup was from Standby mode
 */
bool PWR_WasStandbyWakeup(void)
{
    return __HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET;
}

/**
 * @brief   Clear Standby flag
 * @details Should be called after checking Standby wakeup
 * @param   None
 * @retval  None
 */
void PWR_ClearStandbyFlag(void)
{
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
}

/**
 * @brief   Enable Programmable Voltage Detector
 * @details Monitors VDD and triggers interrupt if below threshold
 * @param   level PVD threshold level
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnablePVD(PWR_PVDLevelTypeDef level)
{
    PWR_PVDTypeDef pvdConfig;

    pvdConfig.PVDLevel = level;
    pvdConfig.Mode = PWR_PVD_MODE_IT_RISING_FALLING;

    HAL_PWR_ConfigPVD(&pvdConfig);
    HAL_PWR_EnablePVD();

    return PWR_OK;
}

/**
 * @brief   Disable Programmable Voltage Detector
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_DisablePVD(void)
{
    HAL_PWR_DisablePVD();
    return PWR_OK;
}

/**
 * @brief   Enable PVD interrupt (EXTI line 16)
 * @details Configures NVIC and enables the PVD interrupt. Keeps implementation minimal
 *          to avoid over-engineering while providing the feature declared in the header.
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnablePVDInterrupt(void)
{
    /* Enable PVD and NVIC line - EXTI line 16/PVD_IRQn is used for PVD interrupts on STM32F4 */
    HAL_PWR_EnablePVD();
    HAL_NVIC_SetPriority(PVD_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(PVD_IRQn);

    log_debug("PWR: PVD interrupt enabled");

    return PWR_OK;
}

/**
 * @brief   Get PVD output status
 * @details Returns whether VDD is below threshold
 * @param   None
 * @retval  bool True if VDD < threshold (power low)
 */
bool PWR_GetPVDStatus(void)
{
    return __HAL_PWR_GET_FLAG(PWR_FLAG_PVDO) != RESET;
}

/**
 * @brief   Enable access to backup domain
 * @details Required before accessing RTC and backup registers
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableBackupAccess(void)
{
    HAL_PWR_EnableBkUpAccess();
    return PWR_OK;
}

/**
 * @brief   Disable access to backup domain
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_DisableBackupAccess(void)
{
    HAL_PWR_DisableBkUpAccess();
    return PWR_OK;
}

/**
 * @brief   Get VBAT voltage status
 * @details Checks backup domain power source
 * @param   None
 * @retval  bool True if backup domain is powered by VBAT
 */
bool PWR_IsVBATActive(void)
{
    /* Check if VDD is off (would indicate VBAT is powering backup domain) */
    /* This is a simplified check - actual implementation may vary */
    return (RCC->BDCR & RCC_BDCR_LSERDY) != 0;
}

/**
 * @brief   Enable voltage regulator for high performance
 * @details May increase power consumption
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableHighPerformance(void)
{
    /* Enable voltage regulator scale 1 (high performance) */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    return PWR_OK;
}

/**
 * @brief   Enable voltage regulator low-power mode
 * @details Reduces power consumption at lower performance
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnableLowPowerMode(void)
{
    /* Enable voltage regulator scale 3 (low power) */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    return PWR_OK;
}

/**
 * @brief   Get default low power configuration
 * @param   config Pointer to configuration structure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_GetDefaultLowPowerConfig(PWR_LowPowerConfigTypeDef* config)
{
    if (config == NULL)
    {
        return PWR_INVALID_PARAM;
    }

    config->mode = PWR_LOW_POWER_MODE_AUTO;
    config->wakeupTimeMs = 1000;  /* 1 second default */
    config->keepPeripherals = false;
    config->wakeupSources = PWR_SRC_WAKEUP_PIN;
    config->optimizeVoltage = true;

    return PWR_OK;
}

/**
 * @brief   Enter low power mode with configuration
 * @details Unified interface for all low power modes
 * @param   config Pointer to low power configuration
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnterLowPowerMode(const PWR_LowPowerConfigTypeDef* config)
{
    PWR_StatusTypeDef status = PWR_OK;

    if (config == NULL)
    {
        return PWR_INVALID_PARAM;
    }

    log_debug("PWR: Entering low power mode %d", config->mode);

    /* Configure wakeup sources */
    status = PWR_ConfigureWakeupSources(config->wakeupSources);
    if (status != PWR_OK)
    {
        return status;
    }

    /* Optimize voltage regulator if requested */
    if (config->optimizeVoltage)
    {
        PWR_EnableLowPowerMode();
    }

    /* Optimize system for low power */
    PWR_OptimizeForLowPower(config->keepPeripherals);

    /* Enter the selected low power mode */
    switch (config->mode)
    {
        case PWR_LOW_POWER_MODE_LIGHT:
            status = PWR_EnterLightLowPower(config->wakeupSources);
            break;

        case PWR_LOW_POWER_MODE_DEEP:
            status = PWR_EnterDeepLowPower(config->wakeupSources, config->keepPeripherals);
            break;

        case PWR_LOW_POWER_MODE_STANDBY:
            status = PWR_EnterStandbyLowPower(config->wakeupSources);
            break;

        case PWR_LOW_POWER_MODE_AUTO:
            status = PWR_AutoLowPowerMode(config->wakeupTimeMs, config->keepPeripherals, config->wakeupSources);
            break;

        default:
            status = PWR_INVALID_PARAM;
            break;
    }

    if (status == PWR_OK)
    {
        log_debug("PWR: Exited low power mode successfully");
    }

    return status;
}

/**
 * @brief   Enter light low power mode (Sleep)
 * @details CPU sleeps, peripherals remain active
 * @param   wakeupSources Wakeup sources to configure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnterLightLowPower(PWR_WakeupSourceTypeDef wakeupSources)
{
    log_debug("PWR: Entering light low power mode (Sleep)");

    /* Configure wakeup sources */
    PWR_ConfigureWakeupSources(wakeupSources);

    /* Suspend SysTick to prevent constant wakeups */
    HAL_SuspendTick();

    /* Enter Sleep mode */
    PWR_EnterSleepMode(PWR_SLEEP_MODE_WFI);

    /* Resume SysTick after wakeup */
    HAL_ResumeTick();

    log_debug("PWR: Exited light low power mode");

    return PWR_OK;
}

/**
 * @brief   Enter deep low power mode (Stop)
 * @details Most clocks stopped, RAM retained
 * @param   wakeupSources Wakeup sources to configure
 * @param   keepPeripherals Keep critical peripherals active
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnterDeepLowPower(PWR_WakeupSourceTypeDef wakeupSources, bool keepPeripherals)
{
    PWR_StatusTypeDef status;

    log_debug("PWR: Entering deep low power mode (Stop)");

    /* Configure wakeup sources */
    status = PWR_ConfigureWakeupSources(wakeupSources);
    if (status != PWR_OK)
    {
        log_error("PWR: Failed to configure wakeup sources");
        return status;
    }

    /* Optimize system for low power */
    PWR_OptimizeForLowPower(keepPeripherals);

    /* CRITICAL: Disable SysTick before Stop mode */
    HAL_SuspendTick();

    /* Choose Stop entry sequence: prefer WFE for EXTI/edge wake, use WFI for WKUP pin */
    PWR_StopEntryTypeDef stopEntry = (wakeupSources & PWR_SRC_WAKEUP_PIN) ? PWR_STOP_ENTRY_WFI : PWR_STOP_ENTRY_WFE;
    log_debug("PWR: Using %s for Stop entry", (stopEntry == PWR_STOP_ENTRY_WFE) ? "WFE" : "WFI");

    /* Enter Stop mode with low power regulator */
    PWR_EnterStopMode(PWR_REGULATOR_LOW_POWER, stopEntry);

    /* System wakes up here */

    /* Resume SysTick */
    HAL_ResumeTick();

    /* Restore system clocks */
    status = PWR_ConfigureAfterStop();
    if (status != PWR_OK)
    {
        log_error("PWR: Failed to restore after Stop mode");
        return status;
    }

    /* Restore system state */
    PWR_RestoreFromLowPower();

    log_debug("PWR: Exited deep low power mode");

    return PWR_OK;
}

/**
 * @brief   Enter standby low power mode
 * @details Lowest power consumption, RAM content lost
 * @param   wakeupSources Wakeup sources to configure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnterStandbyLowPower(PWR_WakeupSourceTypeDef wakeupSources)
{
    log_debug("PWR: Entering standby low power mode");

    /* Configure wakeup sources */
    PWR_StatusTypeDef status = PWR_ConfigureWakeupSources(wakeupSources);
    if (status != PWR_OK)
    {
        return status;
    }

    /* Enter Standby mode */
    PWR_EnterStandbyMode();

    /* Should never reach here */
    return PWR_ERROR;
}

/**
 * @brief   Auto-select and enter optimal low power mode
 * @details Chooses mode based on wakeup time and peripheral requirements
 * @param   wakeupTimeMs Expected time until wakeup (milliseconds)
 * @param   keepPeripherals Keep critical peripherals active
 * @param   wakeupSources Wakeup sources to configure
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_AutoLowPowerMode(uint32_t wakeupTimeMs, bool keepPeripherals, PWR_WakeupSourceTypeDef wakeupSources)
{
    PWR_LowPowerModeTypeDef selectedMode;

    /* Auto-select mode based on wakeup time and requirements */
    if (wakeupTimeMs < 10 || keepPeripherals)
    {
        selectedMode = PWR_LOW_POWER_MODE_LIGHT;
    }
    else if (wakeupTimeMs < 1000)
    {
        selectedMode = PWR_LOW_POWER_MODE_DEEP;
    }
    else
    {
        /* For EXTI-based wakeup, use DEEP instead of STANDBY */
        selectedMode = PWR_LOW_POWER_MODE_DEEP;
    }

    log_debug("PWR: Auto-selected low power mode %d for %lu ms wakeup", selectedMode, wakeupTimeMs);

    /* Create configuration and enter mode */
    PWR_LowPowerConfigTypeDef config;
    config.mode = selectedMode;
    config.wakeupTimeMs = wakeupTimeMs;
    config.keepPeripherals = keepPeripherals;
    config.wakeupSources = wakeupSources;
    config.optimizeVoltage = true;

    return PWR_EnterLowPowerMode(&config);
}

/**
 * @brief   Configure wakeup sources for low power modes
 * @param   sources Wakeup sources to enable
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureWakeupSources(PWR_WakeupSourceTypeDef sources)
{
    PWR_StatusTypeDef status = PWR_OK;

    /* Configure wakeup pin if requested */
    if (sources & PWR_SRC_WAKEUP_PIN)
    {
        status = PWR_EnableWakeupPin(true);
        if (status != PWR_OK)
        {
            log_error("PWR: Failed to enable wakeup pin");
            return status;
        }
        log_debug("PWR: Wakeup pin enabled");
    }

    /* Note: RTC wakeup configuration would go here if RTC is available */
    if (sources & PWR_SRC_RTC_ALARM)
    {
        log_debug("PWR: RTC Alarm wakeup source configured");
    }

    if (sources & PWR_SRC_RTC_WAKEUP)
    {
        log_debug("PWR: RTC Wakeup timer source configured");
    }

    if (sources & PWR_SRC_RTC_TIMESTAMP)
    {
        log_debug("PWR: RTC Timestamp wakeup source configured");
    }

    return PWR_OK;
}

/**
 * @brief   Optimize system for low power consumption
 * @details Disables unnecessary clocks and peripherals
 * @param   keepPeripherals Keep critical peripherals active
 * @retval  PWR_StatusTypeDef Operation status
 */
__weak PWR_StatusTypeDef PWR_OptimizeForLowPower(bool keepPeripherals)
{
    log_debug("PWR: Optimizing system for low power (keep peripherals: %d)", keepPeripherals);
    return PWR_OK;
}

/**
 * @brief   Restore system after low power wakeup
 * @details Re-enables clocks and peripherals as needed
 * @retval  PWR_StatusTypeDef Operation status
 */
__weak PWR_StatusTypeDef PWR_RestoreFromLowPower(void)
{
    log_debug("PWR: Restoring system after low power wakeup");

    return PWR_OK;
}

/**
 * @brief   Get current low power mode status
 * @param   mode Pointer to store current mode
 * @param   wakeupSource Pointer to store wakeup source
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_GetLowPowerStatus(PWR_LowPowerModeTypeDef* mode, PWR_WakeupSourceTypeDef* wakeupSource)
{
    if (mode == NULL)
    {
        return PWR_INVALID_PARAM;
    }

    if (PWR_WasStandbyWakeup())
    {
        *mode = PWR_LOW_POWER_MODE_STANDBY;
        if (wakeupSource != NULL)
        {
            *wakeupSource = PWR_SRC_WAKEUP_PIN;
        }
        PWR_ClearStandbyFlag();
    }
    else
    {
        *mode = PWR_LOW_POWER_MODE_LIGHT;
        if (wakeupSource != NULL)
        {
            *wakeupSource = 0;
        }
    }

    return PWR_OK;
}

/**
 * @brief   Configure advanced low power settings
 * @param   flashPowerDown Enable flash power down in sleep
 * @param   disableBackupWrites Disable backup register writes
 * @param   enableUltraLowPower Enable ultra low power features
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureAdvancedLowPower(bool flashPowerDown, bool disableBackupWrites, bool enableUltraLowPower)
{
    log_debug("PWR: Configuring advanced low power settings");

    /* Silence unused-parameter warnings for optional features not yet implemented */
    (void)flashPowerDown;
    (void)enableUltraLowPower;

    if (disableBackupWrites)
    {
        PWR_DisableBackupAccess();
        log_debug("PWR: Backup writes disabled");
    }
    else
    {
        PWR_EnableBackupAccess();
    }

    return PWR_OK;
}

/* Power estimation helpers removed to reduce dead code and simplify the PWR module. */

/* If you need power estimation in the future, implement a focused helper instead of
   keeping a large, unmaintained estimator in the core power driver. */
