/**
  ******************************************************************************
  * @file    hibernation.c
  * @brief   Touchscreen hibernation manager implementation
  * @details This module implements automatic hibernation after 2 minutes
  *          of touchscreen inactivity to save power.
  * @version 1.0
  * @date    2025-01-13
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "hibernation.h"
#include "pwr.h"
#include "rtc.h"
#include "log.h"
#include "stm32f4xx_hal.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/** @brief Global hibernation handle */
static Hibernation_HandleTypeDef *g_hhib = NULL;

/** @brief Callback function pointer */
static Hibernation_CallbackTypeDef g_hibernation_callback = NULL;

/* Private function prototypes -----------------------------------------------*/

static void Hibernation_EnterLowPowerMode(void);
static void Hibernation_ExitLowPowerMode(void);
static bool Hibernation_ShouldHibernate(Hibernation_HandleTypeDef *hhib);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize hibernation manager
 */
Hibernation_StatusTypeDef Hibernation_Init(Hibernation_HandleTypeDef *hhib)
{
    Hibernation_ConfigTypeDef default_config;

    if (hhib == NULL) {
        log_error("HIBERNATION: Invalid handle provided to Hibernation_Init");
        return HIBERNATION_INVALID_PARAM;
    }

    /* Get default configuration */
    if (Hibernation_GetDefaultConfig(&default_config) != HIBERNATION_OK) {
        log_error("HIBERNATION: Failed to get default configuration");
        return HIBERNATION_ERROR;
    }

    /* Initialize with default config */
    return Hibernation_InitWithConfig(hhib, &default_config);
}

/**
 * @brief   Initialize hibernation manager with custom configuration
 */
Hibernation_StatusTypeDef Hibernation_InitWithConfig(Hibernation_HandleTypeDef *hhib,
                                                    Hibernation_ConfigTypeDef *config)
{
    if (hhib == NULL || config == NULL) {
        log_error("HIBERNATION: Invalid parameters provided to Hibernation_InitWithConfig");
        return HIBERNATION_INVALID_PARAM;
    }

    /* Initialize structure */
    memset(hhib, 0, sizeof(Hibernation_HandleTypeDef));
    hhib->config = *config;
    hhib->state = HIBERNATION_STATE_ACTIVE;
    hhib->last_activity_timestamp = HAL_GetTick();
    hhib->is_initialized = true;
    g_hhib = hhib;

    /* Initialize PWR peripheral for low power modes */
    if (PWR_InitDefault() != PWR_OK) {
        log_error("HIBERNATION: Failed to initialize PWR peripheral");
        return HIBERNATION_ERROR;
    }

    /* Enable wakeup pin for touch interrupt */
    if (PWR_EnableWakeupPin(true) != PWR_OK) {
        log_error("HIBERNATION: Failed to enable wakeup pin");
        return HIBERNATION_ERROR;
    }

    log_info("HIBERNATION: Hibernation manager initialized with %d ms timeout",
             config->timeout_ms);

    return HIBERNATION_OK;
}

/**
 * @brief   Deinitialize hibernation manager
 */
Hibernation_StatusTypeDef Hibernation_DeInit(Hibernation_HandleTypeDef *hhib)
{
    if (hhib == NULL) {
        return HIBERNATION_INVALID_PARAM;
    }

    hhib->is_initialized = false;
    g_hhib = NULL;
    g_hibernation_callback = NULL;

    log_info("HIBERNATION: Hibernation manager deinitialized");
    return HIBERNATION_OK;
}

/**
 * @brief   Get default hibernation configuration
 */
Hibernation_StatusTypeDef Hibernation_GetDefaultConfig(Hibernation_ConfigTypeDef *config)
{
    if (config == NULL) {
        return HIBERNATION_INVALID_PARAM;
    }

    config->timeout_ms = HIBERNATION_TIMEOUT_MS;
    config->check_interval_ms = HIBERNATION_CHECK_INTERVAL_MS;
    config->enable_auto_hibernation = true;
    config->enable_touch_wakeup = true;

    return HIBERNATION_OK;
}

/**
 * @brief   Configure hibernation settings
 */
Hibernation_StatusTypeDef Hibernation_Configure(Hibernation_HandleTypeDef *hhib,
                                              Hibernation_ConfigTypeDef *config)
{
    if (hhib == NULL || config == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    hhib->config = *config;

    log_debug("HIBERNATION: Configuration updated - timeout: %d ms, auto: %s",
              config->timeout_ms, config->enable_auto_hibernation ? "enabled" : "disabled");

    return HIBERNATION_OK;
}

/**
 * @brief   Enable/disable auto hibernation
 */
Hibernation_StatusTypeDef Hibernation_EnableAutoHibernation(Hibernation_HandleTypeDef *hhib,
                                                          bool enable)
{
    if (hhib == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    hhib->config.enable_auto_hibernation = enable;

    log_info("HIBERNATION: Auto hibernation %s", enable ? "enabled" : "disabled");

    return HIBERNATION_OK;
}

/**
 * @brief   Force immediate hibernation
 */
Hibernation_StatusTypeDef Hibernation_EnterHibernation(Hibernation_HandleTypeDef *hhib)
{
    if (hhib == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    if (hhib->state == HIBERNATION_STATE_HIBERNATING) {
        return HIBERNATION_OK; /* Already hibernating */
    }

    log_info("HIBERNATION: Entering hibernation mode");

    hhib->state = HIBERNATION_STATE_HIBERNATING;
    hhib->hibernation_start_time = HAL_GetTick();

    /* Call callback if registered */
    if (g_hibernation_callback != NULL) {
        g_hibernation_callback(HIBERNATION_STATE_HIBERNATING);
    }

    /* Enter low power mode */
    Hibernation_EnterLowPowerMode();

    /* This code executes after wakeup */
    hhib->state = HIBERNATION_STATE_ACTIVE;
    hhib->last_activity_timestamp = HAL_GetTick();

    log_info("HIBERNATION: Woke up from hibernation");

    /* Call callback if registered */
    if (g_hibernation_callback != NULL) {
        g_hibernation_callback(HIBERNATION_STATE_ACTIVE);
    }

    return HIBERNATION_OK;
}

/**
 * @brief   Force immediate wakeup
 */
Hibernation_StatusTypeDef Hibernation_WakeUp(Hibernation_HandleTypeDef *hhib)
{
    if (hhib == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    if (hhib->state != HIBERNATION_STATE_HIBERNATING) {
        return HIBERNATION_OK; /* Not hibernating */
    }

    /* Force exit from low power mode */
    Hibernation_ExitLowPowerMode();

    hhib->state = HIBERNATION_STATE_ACTIVE;
    hhib->last_activity_timestamp = HAL_GetTick();

    log_info("HIBERNATION: Forced wakeup from hibernation");

    /* Call callback if registered */
    if (g_hibernation_callback != NULL) {
        g_hibernation_callback(HIBERNATION_STATE_ACTIVE);
    }

    return HIBERNATION_OK;
}

/**
 * @brief   Report touch activity
 */
Hibernation_StatusTypeDef Hibernation_ReportTouchActivity(Hibernation_HandleTypeDef *hhib)
{
    if (hhib == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    hhib->last_activity_timestamp = HAL_GetTick();

    /* If hibernating, wake up */
    if (hhib->state == HIBERNATION_STATE_HIBERNATING) {
        return Hibernation_WakeUp(hhib);
    }

    return HIBERNATION_OK;
}

/**
 * @brief   Check hibernation status and handle timeout
 */
Hibernation_StatusTypeDef Hibernation_Process(Hibernation_HandleTypeDef *hhib)
{
    if (hhib == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    /* Skip processing if auto hibernation is disabled */
    if (!hhib->config.enable_auto_hibernation) {
        return HIBERNATION_OK;
    }

    /* Check if it's time to hibernate */
    if (Hibernation_ShouldHibernate(hhib)) {
        return Hibernation_EnterHibernation(hhib);
    }

    return HIBERNATION_OK;
}

/**
 * @brief   Get current hibernation state
 */
Hibernation_StatusTypeDef Hibernation_GetState(Hibernation_HandleTypeDef *hhib,
                                             Hibernation_StateTypeDef *state)
{
    if (hhib == NULL || state == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    *state = hhib->state;
    return HIBERNATION_OK;
}

/**
 * @brief   Get time until hibernation
 */
Hibernation_StatusTypeDef Hibernation_GetTimeUntilHibernation(Hibernation_HandleTypeDef *hhib,
                                                            uint32_t *remaining_ms)
{
    if (hhib == NULL || remaining_ms == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed = current_time - hhib->last_activity_timestamp;

    if (elapsed >= hhib->config.timeout_ms) {
        *remaining_ms = 0;
    } else {
        *remaining_ms = hhib->config.timeout_ms - elapsed;
    }

    return HIBERNATION_OK;
}

/**
 * @brief   Register hibernation callback
 */
Hibernation_StatusTypeDef Hibernation_RegisterCallback(Hibernation_HandleTypeDef *hhib,
                                                     Hibernation_CallbackTypeDef callback)
{
    if (hhib == NULL || !hhib->is_initialized) {
        return HIBERNATION_INVALID_PARAM;
    }

    g_hibernation_callback = callback;

    log_debug("HIBERNATION: Callback registered");

    return HIBERNATION_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Enter low power mode (Stop mode)
 */
static void Hibernation_EnterLowPowerMode(void)
{
    log_debug("HIBERNATION: Entering Stop mode");

    /* Configure system for Stop mode */
    /* Keep regulator in normal mode for faster wakeup */
    PWR_EnterStopMode(PWR_REGULATOR_ON, PWR_STOP_ENTRY_WFI);

    /* Code execution resumes here after wakeup */
    /* Reconfigure system after Stop mode */
    PWR_ConfigureAfterStop();
}

/**
 * @brief   Exit low power mode
 */
static void Hibernation_ExitLowPowerMode(void)
{
    /* This function is called when forcing wakeup */
    /* The actual wakeup is handled by the interrupt */
    log_debug("HIBERNATION: Forcing exit from low power mode");
}

/**
 * @brief   Check if system should hibernate
 */
static bool Hibernation_ShouldHibernate(Hibernation_HandleTypeDef *hhib)
{
    if (hhib->state != HIBERNATION_STATE_ACTIVE) {
        return false;
    }

    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed = current_time - hhib->last_activity_timestamp;

    return (elapsed >= hhib->config.timeout_ms);
}
