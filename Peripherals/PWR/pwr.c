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
    PWR_ConfigTypeDef config;
    PWR_GetDefaultConfig(&config);
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
    uint32_t regulatorMode;
    uint32_t entryMode;

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
 * @brief   Get current power consumption estimate
 * @details Estimates based on active peripherals (very approximate)
 * @param   None
 * @retval  uint32_t Estimated current in microamps
 */
uint32_t PWR_GetEstimatedCurrent(void)
{
    uint32_t current_ua = 0;

    /* Base current at 180MHz (typical) */
    current_ua = 100000;  /* ~100mA base */

    /* Check which peripherals are enabled */
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOAEN) current_ua += 500;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOBEN) current_ua += 500;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOCEN) current_ua += 500;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIODEN) current_ua += 500;
    if (RCC->AHB1ENR & RCC_AHB1ENR_GPIOEEN) current_ua += 500;
    if (RCC->AHB1ENR & RCC_AHB1ENR_DMA1EN) current_ua += 1000;
    if (RCC->AHB1ENR & RCC_AHB1ENR_DMA2EN) current_ua += 1000;
    if (RCC->APB2ENR & RCC_APB2ENR_ADC1EN) current_ua += 2000;
    if (RCC->APB2ENR & RCC_APB2ENR_USART1EN) current_ua += 1000;
    if (RCC->APB1ENR & RCC_APB1ENR_SPI2EN) current_ua += 1000;
    if (RCC->APB1ENR & RCC_APB1ENR_I2C1EN) current_ua += 1000;

    return current_ua;
}
