/**
  ******************************************************************************
  * @file    iwdg.c
  * @brief   Independent Watchdog (IWDG) module implementation
  * @details This file provides code for the configuration
  *          and control of the Independent Watchdog.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "iwdg.h"
#include "log.h"

/* Private defines -----------------------------------------------------------*/
#define IWDG_DEFAULT_PRESCALER      IWDG_PRESCALER_32
#define IWDG_DEFAULT_RELOAD         1000U    /**< ~1 second with prescaler 32 */
#define IWDG_MSEC_PER_SEC           1000U    /**< Milliseconds per second */

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   IWDG handle structure
 * @details Used by HAL functions for IWDG peripheral operations
 */
IWDG_HandleTypeDef hiwdg;

/* Private function prototypes -----------------------------------------------*/
static IWDG_StatusTypeDef IWDG_ConvertHALStatus(HAL_StatusTypeDef halStatus);
static void IWDG_CalculatePrescalerReload(uint32_t timeout_ms, uint32_t* prescaler, uint32_t* reload);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Convert HAL status to IWDG status
 * @param   halStatus HAL status code
 * @retval  IWDG_StatusTypeDef Converted status
 */
static IWDG_StatusTypeDef IWDG_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus)
    {
        case HAL_OK:
            return IWDG_OK;
        case HAL_ERROR:
            return IWDG_ERROR;
        case HAL_TIMEOUT:
            return IWDG_TIMEOUT;
        default:
            return IWDG_ERROR;
    }
}

/**
 * @brief   Calculate prescaler and reload values for desired timeout
 * @param   timeout_ms Desired timeout in milliseconds
 * @param   prescaler Pointer to store calculated prescaler
 * @param   reload Pointer to store calculated reload value
 */
static void IWDG_CalculatePrescalerReload(uint32_t timeout_ms, uint32_t* prescaler, uint32_t* reload)
{
    /* Available prescaler values and their dividers */
    const uint32_t prescalers[] = {
        IWDG_PRESCALER_4, IWDG_PRESCALER_8, IWDG_PRESCALER_16,
        IWDG_PRESCALER_32, IWDG_PRESCALER_64, IWDG_PRESCALER_128, IWDG_PRESCALER_256
    };
    const uint32_t dividers[] = {4, 8, 16, 32, 64, 128, 256};
    const uint32_t numPrescalers = sizeof(dividers) / sizeof(dividers[0]);

    /* LSI frequency is approximately 32kHz */
    const uint32_t lsiFreq = IWDG_LSI_FREQ;

    /* Find the best prescaler/reload combination */
    for (uint32_t i = 0; i < numPrescalers; i++)
    {
        /* Calculate reload value: reload = (timeout_ms * lsiFreq) / (divider * 1000) */
        uint32_t reloadValue = (timeout_ms * lsiFreq) / (dividers[i] * IWDG_MSEC_PER_SEC);

        if (reloadValue <= IWDG_RELOAD_MAX)
        {
            *prescaler = prescalers[i];
            *reload = reloadValue;
            return;
        }
    }

    /* If timeout is too long, use maximum values */
    *prescaler = IWDG_PRESCALER_256;
    *reload = IWDG_RELOAD_MAX;
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief   Initialize the Independent Watchdog
 * @details Configures IWDG with default 1 second timeout
 * @param   None
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Init(void)
{
    log_debug("IWDG: Initializing Independent Watchdog");

    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_DEFAULT_PRESCALER;
    hiwdg.Init.Reload = IWDG_DEFAULT_RELOAD;

    HAL_StatusTypeDef halStatus = HAL_IWDG_Init(&hiwdg);

    log_debug("IWDG: Independent Watchdog initialized successfully");

    return IWDG_ConvertHALStatus(halStatus);
}

/**
 * @brief   Initialize IWDG with custom configuration
 * @details Allows custom configuration of IWDG parameters
 * @param   config Pointer to IWDG configuration structure
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Init_Custom(const IWDG_ConfigTypeDef* config)
{
    if (config == NULL)
    {
        return IWDG_INVALID_PARAM;
    }

    log_debug("IWDG: Initializing Independent Watchdog with custom configuration");

    if (config->Reload > IWDG_RELOAD_MAX)
    {
        return IWDG_INVALID_PARAM;
    }

    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = config->Prescaler;
    hiwdg.Init.Reload = config->Reload;

    HAL_StatusTypeDef halStatus = HAL_IWDG_Init(&hiwdg);

    log_debug("IWDG: Independent Watchdog initialized successfully with custom configuration");

    return IWDG_ConvertHALStatus(halStatus);
}

/**
 * @brief   Initialize IWDG with timeout in milliseconds
 * @details Automatically calculates prescaler and reload for desired timeout
 * @param   timeout_ms Desired timeout in milliseconds (max ~32000ms)
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Init_TimeoutMs(uint32_t timeout_ms)
{
    if (timeout_ms == 0 || timeout_ms > IWDG_TIMEOUT_32S)
    {
        return IWDG_INVALID_PARAM;
    }

    log_debug("IWDG: Initializing Independent Watchdog with timeout %lu ms", timeout_ms);

    uint32_t prescaler = 0;
    uint32_t reload = 0;

    IWDG_CalculatePrescalerReload(timeout_ms, &prescaler, &reload);

    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = prescaler;
    hiwdg.Init.Reload = reload;

    HAL_StatusTypeDef halStatus = HAL_IWDG_Init(&hiwdg);

    log_debug("IWDG: Independent Watchdog initialized successfully with timeout");

    return IWDG_ConvertHALStatus(halStatus);
}

/**
 * @brief   Refresh the watchdog counter
 * @details Must be called periodically to prevent system reset
 * @param   None
 * @retval  IWDG_StatusTypeDef Operation status
 */
IWDG_StatusTypeDef IWDG_Refresh(void)
{
    HAL_StatusTypeDef halStatus = HAL_IWDG_Refresh(&hiwdg);
    return IWDG_ConvertHALStatus(halStatus);
}

/**
 * @brief   Start the watchdog (cannot be stopped once started)
 * @details Once started, IWDG cannot be stopped except by reset
 * @param   None
 * @retval  IWDG_StatusTypeDef Operation status
 * @note    IWDG starts automatically after HAL_IWDG_Init()
 */
IWDG_StatusTypeDef IWDG_Start(void)
{
    /* IWDG is started automatically by HAL_IWDG_Init() */
    /* This function is provided for API consistency */
    return IWDG_OK;
}

/**
 * @brief   Check if last reset was caused by IWDG
 * @details Reads RCC reset flags to determine reset source
 * @param   None
 * @retval  bool True if IWDG caused the reset
 */
bool IWDG_WasResetSource(void)
{
    return (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET);
}

/**
 * @brief   Clear IWDG reset flag
 * @details Clears the IWDG reset flag in RCC
 * @param   None
 * @retval  None
 */
void IWDG_ClearResetFlag(void)
{
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

/**
 * @brief   Calculate timeout from prescaler and reload
 * @details Calculates actual timeout in milliseconds
 * @param   prescaler IWDG prescaler value
 * @param   reload IWDG reload value
 * @retval  uint32_t Timeout in milliseconds
 */
uint32_t IWDG_CalculateTimeout(uint32_t prescaler, uint32_t reload)
{
    uint32_t divider = 4U;

    switch (prescaler)
    {
        case IWDG_PRESCALER_4:   divider = 4U;   break;
        case IWDG_PRESCALER_8:   divider = 8U;   break;
        case IWDG_PRESCALER_16:  divider = 16U;  break;
        case IWDG_PRESCALER_32:  divider = 32U;  break;
        case IWDG_PRESCALER_64:  divider = 64U;  break;
        case IWDG_PRESCALER_128: divider = 128U; break;
        case IWDG_PRESCALER_256: divider = 256U; break;
        default:                 divider = 4U;   break;
    }

    /* timeout_ms = (reload * divider * 1000) / lsiFreq */
    return (reload * divider * IWDG_MSEC_PER_SEC) / IWDG_LSI_FREQ;
}

/**
 * @brief   Get IWDG status string
 * @details Converts status code to human-readable string
 * @param   status IWDG status code
 * @retval  const char* Status description string
 */
const char* IWDG_GetStatusString(IWDG_StatusTypeDef status)
{
    switch (status)
    {
        case IWDG_OK:
            return "IWDG_OK";
        case IWDG_ERROR:
            return "IWDG_ERROR";
        case IWDG_TIMEOUT:
            return "IWDG_TIMEOUT";
        case IWDG_INVALID_PARAM:
            return "IWDG_INVALID_PARAM";
        default:
            return "UNKNOWN_STATUS";
    }
}
