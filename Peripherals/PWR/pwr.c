/**
  ******************************************************************************
  * @file    pwr.c
  * @brief   Power Management module implementation
  * @details This file provides code for power management including
  *          sleep modes, voltage regulation, and backup domain control.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "pwr.h"
#include "log.h"

/* Private defines -----------------------------------------------------------*/
#define BACKUP_REG_MAX_INDEX    (PWR_BACKUP_REG_COUNT - 1)

/* External variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;  /* May be defined in rtc.c if RTC is used */

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

/**
 * @brief   Enter Sleep mode
 * @details CPU stops, peripherals continue running
 * @param   mode Sleep entry mode (WFI or WFE)
 * @retval  None
 */
void PWR_EnterSleepMode(PWR_SleepModeTypeDef mode)
{
    /* Clear SLEEPDEEP bit to select Sleep mode */
    CLEAR_BIT(SCB->SCR, SCB_SCR_SLEEPDEEP_Msk);

    if (mode == PWR_SLEEP_MODE_WFI)
    {
        /* Request Wait For Interrupt */
        __WFI();
    }
    else
    {
        /* Request Wait For Event */
        __SEV();  /* Set event to clear pending events */
        __WFE();
        __WFE();  /* Enter sleep on next WFE */
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
    /* Use HAL delay which relies on SysTick interrupt to wake */
    /* This is a simple implementation - for precise timing use RTC */
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
}

/**
 * @brief   Configure system after Stop mode wakeup
 * @details Restores clocks and peripherals after Stop mode
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureAfterStop(void)
{
    /* After wakeup from Stop mode, system clock is HSI.
     * Need to reconfigure system clock to original settings */

    /* Re-enable HSE if it was used */
    __HAL_RCC_HSE_CONFIG(RCC_HSE_ON);

    /* Wait for HSE ready */
    uint32_t tickstart = HAL_GetTick();
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET)
    {
        if ((HAL_GetTick() - tickstart) > PWR_TIMEOUT_VALUE)
        {
            return PWR_TIMEOUT;
        }
    }

    /* Re-enable PLL */
    __HAL_RCC_PLL_ENABLE();

    /* Wait for PLL ready */
    tickstart = HAL_GetTick();
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET)
    {
        if ((HAL_GetTick() - tickstart) > PWR_TIMEOUT_VALUE)
        {
            return PWR_TIMEOUT;
        }
    }

    /* Select PLL as system clock source */
    __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_PLLCLK);

    /* Wait for clock switch */
    tickstart = HAL_GetTick();
    while (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_PLLCLK)
    {
        if ((HAL_GetTick() - tickstart) > PWR_TIMEOUT_VALUE)
        {
            return PWR_TIMEOUT;
        }
    }

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
 * @brief   Enable PVD interrupt
 * @details Requires EXTI line 16 and PVD_IRQHandler
 * @param   None
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_EnablePVDInterrupt(void)
{
    /* Enable EXTI line 16 (connected to PVD output) */
    HAL_NVIC_SetPriority(PVD_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(PVD_IRQn);

    return PWR_OK;
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
 * @brief   Write to backup register
 * @details Data retained during Standby and VBAT modes
 * @param   regIndex Register index (0-19)
 * @param   data 32-bit data to store
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_WriteBackupRegister(uint32_t regIndex, uint32_t data)
{
    if (regIndex > BACKUP_REG_MAX_INDEX)
    {
        return PWR_INVALID_PARAM;
    }

    /* Enable backup access */
    HAL_PWR_EnableBkUpAccess();

    /* Write to backup register using RTC */
    HAL_RTCEx_BKUPWrite(&hrtc, regIndex, data);

    return PWR_OK;
}

/**
 * @brief   Read from backup register
 * @param   regIndex Register index (0-19)
 * @param   data Pointer to store read data
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ReadBackupRegister(uint32_t regIndex, uint32_t* data)
{
    if (regIndex > BACKUP_REG_MAX_INDEX || data == NULL)
    {
        return PWR_INVALID_PARAM;
    }

    /* Read from backup register using RTC */
    *data = HAL_RTCEx_BKUPRead(&hrtc, regIndex);

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
 * @brief   Get status string for error code
 * @param   status PWR status code
 * @retval  const char* Status description
 */
const char* PWR_GetStatusString(PWR_StatusTypeDef status)
{
    switch (status)
    {
        case PWR_OK:
            return "PWR_OK";
        case PWR_ERROR:
            return "PWR_ERROR";
        case PWR_INVALID_PARAM:
            return "PWR_INVALID_PARAM";
        case PWR_TIMEOUT:
            return "PWR_TIMEOUT";
        case PWR_NOT_READY:
            return "PWR_NOT_READY";
        default:
            return "UNKNOWN_STATUS";
    }
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
            /* Light sleep doesn't need full restoration */
            break;

        case PWR_LOW_POWER_MODE_DEEP:
            status = PWR_EnterDeepLowPower(config->wakeupSources, config->keepPeripherals);
            /* Deep sleep already handles restoration */
            break;

        case PWR_LOW_POWER_MODE_STANDBY:
            status = PWR_EnterStandbyLowPower(config->wakeupSources);
            /* Standby resets the system, no restoration needed */
            break;

        case PWR_LOW_POWER_MODE_AUTO:
            status = PWR_AutoLowPowerMode(config->wakeupTimeMs, config->keepPeripherals, config->wakeupSources);
            /* Auto mode handles its own restoration */
            break;

        default:
            status = PWR_INVALID_PARAM;
            break;
    }

    /* Note: System will wake up here after low power mode */
    /* Restoration is handled by individual mode functions */
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

    /* Configure wakeup sources (may include external interrupts) */
    PWR_ConfigureWakeupSources(wakeupSources);

    /* Enter Sleep mode - peripherals stay active */
    PWR_EnterSleepMode(PWR_SLEEP_MODE_WFI);

    /* System wakes up here */
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
    log_debug("PWR: Entering deep low power mode (Stop)");

    /* Configure wakeup sources */
    PWR_StatusTypeDef status = PWR_ConfigureWakeupSources(wakeupSources);
    if (status != PWR_OK)
    {
        log_debug("PWR: Failed to configure wakeup sources");
        return status;
    }

    /* Optimize system for low power */
    PWR_OptimizeForLowPower(keepPeripherals);

    /* Ensure SysTick is disabled before entering Stop mode */
    /* SysTick can prevent proper entry into Stop mode */
    HAL_SuspendTick();

    /* Clear any pending interrupts that might prevent sleep */
    __disable_irq();

    /* Enter Stop mode with low power regulator */
    PWR_EnterStopMode(PWR_REGULATOR_LOW_POWER, PWR_STOP_ENTRY_WFI);

    /* System wakes up here - re-enable interrupts */
    __enable_irq();

    /* Resume SysTick */
    HAL_ResumeTick();

    /* System wakes up here - need to restore clocks */
    status = PWR_ConfigureAfterStop();
    if (status != PWR_OK)
    {
        log_debug("PWR: Failed to restore after Stop mode");
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

    /* Enter Standby mode - this will reset the system on wakeup */
    PWR_EnterStandbyMode();

    /* Should never reach here - system resets on wakeup from Standby */
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
    if (wakeupTimeMs < 10 && !keepPeripherals)
    {
        /* Very short wakeup time - use light sleep */
        selectedMode = PWR_LOW_POWER_MODE_LIGHT;
    }
    else if (wakeupTimeMs < 1000 || keepPeripherals)
    {
        /* Short wakeup time or need peripherals - use deep sleep (Stop) */
        selectedMode = PWR_LOW_POWER_MODE_DEEP;
    }
    else
    {
        /* Long wakeup time - use standby for maximum power savings */
        selectedMode = PWR_LOW_POWER_MODE_STANDBY;
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
            log_debug("PWR: Failed to enable wakeup pin");
            return status;
        }
        log_debug("PWR: Wakeup pin enabled");
    }

    /* RTC wakeup sources require RTC to be configured */
    /* This implementation assumes RTC is already initialized */

    if (sources & PWR_SRC_RTC_ALARM)
    {
        /* Configure RTC alarm for wakeup */
        /* This is a placeholder - actual implementation depends on RTC driver */
        log_debug("PWR: RTC Alarm wakeup source configured");

        /* Example RTC alarm configuration (would need RTC driver):
        RTC_AlarmTypeDef alarm;
        alarm.AlarmTime.Hours = 0;
        alarm.AlarmTime.Minutes = 0;
        alarm.AlarmTime.Seconds = 30; // Wake up in 30 seconds
        alarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
        alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
        alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
        alarm.AlarmDateWeekDay = 1;
        alarm.Alarm = RTC_ALARM_A;

        if (HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN) != HAL_OK)
        {
            return PWR_ERROR;
        }
        */
    }

    if (sources & PWR_SRC_RTC_WAKEUP)
    {
        /* Configure RTC wakeup timer */
        log_debug("PWR: RTC Wakeup timer source configured");

        /* Example RTC wakeup timer configuration:
        if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2048, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
        {
            return PWR_ERROR;
        }
        */
    }

    if (sources & PWR_SRC_RTC_TIMESTAMP)
    {
        /* Configure RTC timestamp for wakeup */
        log_debug("PWR: RTC Timestamp wakeup source configured");
        /* Timestamp events can wake from low power modes */
    }

    return PWR_OK;
}

/**
 * @brief   Optimize system for low power consumption
 * @details Disables unnecessary clocks and peripherals
 * @note    This function is declared as weak - applications can override
 *          with their own optimization strategy. Example application override:
 *
 *          PWR_StatusTypeDef PWR_OptimizeForLowPower(bool keepPeripherals)
 *          {
 *              if (!keepPeripherals)
 *              {
 *                  // Disable only peripherals not needed by this application
 *                  __HAL_RCC_SPI2_CLK_DISABLE();  // Not used
 *                  __HAL_RCC_TIM3_CLK_DISABLE();  // Not used
 *              }
 *              // Keep GPIOA, USART1 active for application needs
 *              return PWR_OK;
 *          }
 * @param   keepPeripherals Keep critical peripherals active
 * @retval  PWR_StatusTypeDef Operation status
 */
__weak PWR_StatusTypeDef PWR_OptimizeForLowPower(bool keepPeripherals)
{
    log_debug("PWR: Optimizing system for low power (keep peripherals: %d)", keepPeripherals);

    /* Disable unused peripheral clocks to save power */
    if (!keepPeripherals)
    {
        /* Disable non-critical GPIO ports (keep A, B, C for basic functionality) */
        __HAL_RCC_GPIOE_CLK_DISABLE();
        __HAL_RCC_GPIOF_CLK_DISABLE();
        __HAL_RCC_GPIOG_CLK_DISABLE();
        __HAL_RCC_GPIOH_CLK_DISABLE();
        __HAL_RCC_GPIOI_CLK_DISABLE();

        /* Disable unused communication peripherals */
        __HAL_RCC_USART2_CLK_DISABLE();
        __HAL_RCC_USART3_CLK_DISABLE();
        __HAL_RCC_USART6_CLK_DISABLE();

        /* Disable unused SPI peripherals (keep SPI1 if needed) */
        __HAL_RCC_SPI2_CLK_DISABLE();
        __HAL_RCC_SPI3_CLK_DISABLE();

        /* Disable unused I2C peripherals (keep I2C1 if needed) */
        __HAL_RCC_I2C2_CLK_DISABLE();
        __HAL_RCC_I2C3_CLK_DISABLE();

        /* Keep DMA enabled as it may be needed for peripherals */

        /* Disable unused timers */
        __HAL_RCC_TIM3_CLK_DISABLE();
        __HAL_RCC_TIM4_CLK_DISABLE();
        __HAL_RCC_TIM5_CLK_DISABLE();

        /* Disable ADC to save power */
        __HAL_RCC_ADC1_CLK_DISABLE();

        log_debug("PWR: Non-critical peripherals disabled for power savings");
    }

    /* Configure flash for low power (if supported) */
    /* For STM32F4, flash power down in sleep is not always available */
    /* FLASH->ACR |= FLASH_ACR_SLEEP_PD; // Not available on all variants */

    /* Disable unused oscillators if safe to do so */
    /* HSE can typically be kept enabled for quick wakeup */
    /* __HAL_RCC_HSE_CONFIG(RCC_HSE_OFF); // Usually not disabled */

    /* Additional power optimizations */
    /* Disable USB if not needed */
    /* __HAL_RCC_USB_OTG_FS_CLK_DISABLE(); */

    /* Disable Ethernet if not needed */
    /* __HAL_RCC_ETHMAC_CLK_DISABLE(); */
    /* __HAL_RCC_ETHMACTX_CLK_DISABLE(); */
    /* __HAL_RCC_ETHMACRX_CLK_DISABLE(); */

    return PWR_OK;
}

/**
 * @brief   Restore system after low power wakeup
 * @details Re-enables clocks and peripherals as needed
 * @note    This function is declared as weak - applications can override
 *          with their own restoration sequence. Example application override:
 *
 *          PWR_StatusTypeDef PWR_RestoreFromLowPower(void)
 *          {
 *              // Re-enable only peripherals needed by this application
 *              __HAL_RCC_GPIOA_CLK_ENABLE();  // For LED control
 *              __HAL_RCC_USART1_CLK_ENABLE(); // For communication
 *
 *              // Restore application-specific state
 *              RestoreApplicationState();
 *
 *              // Re-enable interrupts that were disabled
 *              HAL_NVIC_EnableIRQ(EXTI0_IRQn);
 *
 *              return PWR_OK;
 *          }
 * @retval  PWR_StatusTypeDef Operation status
 */
__weak PWR_StatusTypeDef PWR_RestoreFromLowPower(void)
{
    log_debug("PWR: Restoring system after low power wakeup (default implementation)");

    /* Re-enable peripheral clocks that were disabled during low power */
    /* This should match what was disabled in PWR_OptimizeForLowPower */

    /* Re-enable GPIO ports that may have been disabled */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    /* Re-enable communication peripherals */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_USART6_CLK_ENABLE();

    /* Re-enable SPI peripherals */
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_SPI3_CLK_ENABLE();

    /* Re-enable I2C peripherals */
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_I2C3_CLK_ENABLE();

    /* Re-enable DMA controllers */
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* Re-enable timers that may be needed */
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();
    __HAL_RCC_TIM5_CLK_ENABLE();

    /* Re-enable ADC if it was disabled */
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* Restore voltage regulator to high performance mode */
    /* This ensures full performance after wakeup */
    PWR_EnableHighPerformance();

    /* Re-enable backup access if it was disabled for power savings */
    PWR_EnableBackupAccess();

    /* Clear any pending wakeup flags */
    PWR_ClearStandbyFlag();

    /* Re-initialize system tick if needed */
    /* HAL_InitTick() may need to be called depending on HAL configuration */

    log_debug("PWR: System restoration completed");

    return PWR_OK;
}

/**
 * @brief   Get current low power mode status
 * @details Returns information about current power state
 * @param   mode Pointer to store current mode
 * @param   wakeupSource Pointer to store wakeup source (if applicable)
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_GetLowPowerStatus(PWR_LowPowerModeTypeDef* mode, PWR_WakeupSourceTypeDef* wakeupSource)
{
    if (mode == NULL)
    {
        return PWR_INVALID_PARAM;
    }

    /* Check if we just woke up from a low power mode */
    if (PWR_WasStandbyWakeup())
    {
        *mode = PWR_LOW_POWER_MODE_STANDBY;
        if (wakeupSource != NULL)
        {
            /* Determine wakeup source - this is simplified */
            /* In a real implementation, you'd check various flags */
            *wakeupSource = PWR_SRC_WAKEUP_PIN;  /* Default assumption */
        }
        PWR_ClearStandbyFlag();
    }
    else
    {
        /* Not in a low power mode, or just returned from Stop/Sleep */
        *mode = PWR_LOW_POWER_MODE_LIGHT;  /* Assume normal operation */
        if (wakeupSource != NULL)
        {
            *wakeupSource = 0;  /* No specific wakeup source */
        }
    }

    return PWR_OK;
}

/**
 * @brief   Configure advanced low power settings
 * @details Fine-tune power consumption vs performance tradeoffs
 * @param   flashPowerDown Enable flash power down in sleep
 * @param   disableBackupWrites Disable backup register writes to save power
 * @param   enableUltraLowPower Enable ultra low power features (if available)
 * @retval  PWR_StatusTypeDef Operation status
 */
PWR_StatusTypeDef PWR_ConfigureAdvancedLowPower(bool flashPowerDown, bool disableBackupWrites, bool enableUltraLowPower)
{
    log_debug("PWR: Configuring advanced low power settings");

    /* Configure flash power down */
    if (flashPowerDown)
    {
        /* Enable flash power down in Deep Sleep/Stop mode */
        /* Note: FLASH_ACR_SLEEP_PD may not be available on all STM32F4 variants */
        /* FLASH->ACR |= FLASH_ACR_SLEEP_PD; */
        log_debug("PWR: Flash power down requested but not supported on this MCU");
    }
    else
    {
        /* FLASH->ACR &= ~FLASH_ACR_SLEEP_PD; */
    }

    /* Configure backup domain writes */
    if (disableBackupWrites)
    {
        /* Disable backup access to save power */
        PWR_DisableBackupAccess();
        log_debug("PWR: Backup writes disabled");
    }
    else
    {
        /* Ensure backup access is enabled if needed */
        PWR_EnableBackupAccess();
    }

    /* Configure ultra low power features (STM32F4 specific) */
    if (enableUltraLowPower)
    {
        /* Enable ultra low power mode in Stop mode */
        /* Note: PWR_CR_ULP may not be available on all STM32F4 variants */
        /* PWR->CR |= PWR_CR_ULP; */
        log_debug("PWR: Ultra low power requested but not supported on this MCU");
    }
    else
    {
        /* PWR->CR &= ~PWR_CR_ULP; */
    }

    return PWR_OK;
}

/**
 * @brief   Calculate power savings for a low power mode
 * @details Estimates power savings compared to normal operation
 * @param   mode Low power mode to evaluate
 * @param   wakeupTimeMs Expected wakeup time
 * @retval  uint32_t Estimated power savings in microamps
 */
uint32_t PWR_CalculatePowerSavings(PWR_LowPowerModeTypeDef mode, uint32_t wakeupTimeMs)
{
    uint32_t normalCurrent = PWR_GetEstimatedCurrent();
    uint32_t lowPowerCurrent = 0;

    /* Estimate current consumption for each mode */
    switch (mode)
    {
        case PWR_LOW_POWER_MODE_LIGHT:
            /* Sleep mode: CPU stopped, peripherals active */
            lowPowerCurrent = normalCurrent / 10;  /* ~10% of normal current */
            break;

        case PWR_LOW_POWER_MODE_DEEP:
            /* Stop mode: Most clocks stopped, low power regulator */
            lowPowerCurrent = 10000;  /* ~10mA typical for Stop mode */
            break;

        case PWR_LOW_POWER_MODE_STANDBY:
            /* Standby mode: Minimum power consumption */
            lowPowerCurrent = 2000;  /* ~2mA typical for Standby mode */
            break;

        case PWR_LOW_POWER_MODE_AUTO:
            /* Use deep sleep as default for calculation */
            lowPowerCurrent = 10000;
            break;

        default:
            return 0;
    }

    /* Calculate power savings over the wakeup period */
    uint32_t savings = (normalCurrent - lowPowerCurrent) * wakeupTimeMs / 1000;

    return savings;
}

/**
 * @brief   Get current power consumption estimate
 * @details Estimates based on active peripherals (very approximate)
 * @param   None
 * @retval  uint32_t Estimated current in microamps
 */
uint32_t PWR_GetEstimatedCurrent(void)
{
    uint32_t current_ua = 0;

    /* Base current for STM32F429 at 180MHz (typical values) */
    current_ua = 80000;  /* ~80mA base current */

    /* Add current for active peripherals */
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOAEN) current_ua += 200;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOBEN) current_ua += 200;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOCEN) current_ua += 200;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIODEN) current_ua += 200;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOEEN) current_ua += 200;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOFEN) current_ua += 200;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOGEN) current_ua += 200;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOHEN) current_ua += 200;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOIEN) current_ua += 200;

    if (RCC->AHB1ENR & RCC_AHB1ENR_DMA1EN) current_ua += 500;
    if (RCC->AHB1ENR & RCC_AHB1ENR_DMA2EN) current_ua += 500;

    if (RCC->APB2ENR & RCC_APB2ENR_ADC1EN) current_ua += 1500;
    if (RCC->APB2ENR & RCC_APB2ENR_USART1EN) current_ua += 800;
    if (RCC->APB2ENR & RCC_APB2ENR_SPI1EN) current_ua += 600;

    if (RCC->APB1ENR & RCC_APB1ENR_USART2EN) current_ua += 800;
    if (RCC->APB1ENR & RCC_APB1ENR_USART3EN) current_ua += 800;
    if (RCC->APB1ENR & RCC_APB1ENR_SPI2EN) current_ua += 600;
    if (RCC->APB1ENR & RCC_APB1ENR_SPI3EN) current_ua += 600;
    if (RCC->APB1ENR & RCC_APB1ENR_I2C1EN) current_ua += 400;
    if (RCC->APB1ENR & RCC_APB1ENR_I2C2EN) current_ua += 400;
    if (RCC->APB1ENR & RCC_APB1ENR_I2C3EN) current_ua += 400;

    if (RCC->APB1ENR & RCC_APB1ENR_TIM2EN) current_ua += 300;
    if (RCC->APB1ENR & RCC_APB1ENR_TIM3EN) current_ua += 300;
    if (RCC->APB1ENR & RCC_APB1ENR_TIM4EN) current_ua += 300;
    if (RCC->APB1ENR & RCC_APB1ENR_TIM5EN) current_ua += 300;

    /* Add current for LTDC (LCD controller) if enabled */
    if (RCC->APB2ENR & RCC_APB2ENR_LTDCEN) current_ua += 5000;

    /* Add current for USB if enabled */
    if (RCC->AHB2ENR & RCC_AHB2ENR_OTGFSEN) current_ua += 2000;

    return current_ua;
}
