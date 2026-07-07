/**
  ******************************************************************************
  * @file    wwdg.c
  * @brief   Window Watchdog (WWDG) module implementation
  * @details This file provides code for the configuration
  *          and control of the Window Watchdog.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "wwdg.h"
#include "log.h"

/* Private defines -----------------------------------------------------------*/
#define WWDG_DEFAULT_PRESCALER      WWDG_PRESCALER_8
#define WWDG_DEFAULT_WINDOW         0x50U
#define WWDG_DEFAULT_COUNTER        0x7FU
#define WWDG_COUNTER_MASK           0x3FU   /**< Counter bit mask (6 bits) */
#define WWDG_TICKS_PER_COUNT        4096U   /**< WWDG clock divider */
#define WWDG_USEC_PER_SEC           1000000U /**< Microseconds per second */

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   WWDG handle structure
 * @details Used by HAL functions for WWDG peripheral operations
 */
WWDG_HandleTypeDef hwwdg;

/**
 * @brief   User callback for Early Wakeup Interrupt
 */
static WWDG_EWI_Callback_t wwdg_ewi_callback = NULL;

/**
 * @brief   Stored counter value for refresh
 */
static uint32_t wwdg_counter_value = WWDG_DEFAULT_COUNTER;

/* Private function prototypes -----------------------------------------------*/
static WWDG_StatusTypeDef WWDG_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Convert HAL status to WWDG status
 * @param   halStatus HAL status code
 * @retval  WWDG_StatusTypeDef Converted status
 */
static WWDG_StatusTypeDef WWDG_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus)
    {
        case HAL_OK:
            return WWDG_OK;
        case HAL_ERROR:
            return WWDG_ERROR;
        case HAL_TIMEOUT:
            return WWDG_TIMEOUT;
        default:
            return WWDG_ERROR;
    }
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief   Initialize the Window Watchdog
 * @details Configures WWDG with default settings
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Init(void)
{
    log_debug("WWDG: Initializing Window Watchdog");

    /* Enable WWDG clock */
    __HAL_RCC_WWDG_CLK_ENABLE();

    hwwdg.Instance = WWDG;
    hwwdg.Init.Prescaler = WWDG_DEFAULT_PRESCALER;
    hwwdg.Init.Window = WWDG_DEFAULT_WINDOW;
    hwwdg.Init.Counter = WWDG_DEFAULT_COUNTER;
    hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;

    wwdg_counter_value = WWDG_DEFAULT_COUNTER;

    HAL_StatusTypeDef halStatus = HAL_WWDG_Init(&hwwdg);

    log_debug("WWDG: Window Watchdog initialized successfully");

    return WWDG_ConvertHALStatus(halStatus);
}

/**
 * @brief   Initialize WWDG with custom configuration
 * @details Allows custom configuration of WWDG parameters
 * @param   config Pointer to WWDG configuration structure
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Init_Custom(const WWDG_ConfigTypeDef* config)
{
    if (config == NULL)
    {
        return WWDG_INVALID_PARAM;
    }

    log_debug("WWDG: Initializing Window Watchdog with custom configuration");

    /* Validate counter range */
    if (config->Counter < WWDG_COUNTER_MIN || config->Counter > WWDG_COUNTER_MAX)
    {
        return WWDG_INVALID_PARAM;
    }

    /* Validate window range */
    if (config->Window < WWDG_WINDOW_MIN || config->Window > WWDG_WINDOW_MAX)
    {
        return WWDG_INVALID_PARAM;
    }

    /* Window must be less than counter */
    if (config->Window >= config->Counter)
    {
        return WWDG_INVALID_PARAM;
    }

    /* Enable WWDG clock */
    __HAL_RCC_WWDG_CLK_ENABLE();

    hwwdg.Instance = WWDG;
    hwwdg.Init.Prescaler = config->Prescaler;
    hwwdg.Init.Window = config->Window;
    hwwdg.Init.Counter = config->Counter;
    hwwdg.Init.EWIMode = config->EWIMode;

    wwdg_counter_value = config->Counter;

    HAL_StatusTypeDef halStatus = HAL_WWDG_Init(&hwwdg);

    log_debug("WWDG: Window Watchdog initialized successfully with custom configuration");

    return WWDG_ConvertHALStatus(halStatus);
}

/**
 * @brief   Deinitialize the Window Watchdog
 * @details Disables WWDG clock (Note: WWDG cannot be stopped once started)
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_DeInit(void)
{
    /* Note: WWDG cannot be disabled once started - only reset stops it */
    /* We can only disable the clock to prevent further configuration */

    /* Disable WWDG clock */
    __HAL_RCC_WWDG_CLK_DISABLE();

    /* Clear callback */
    wwdg_ewi_callback = NULL;

    /* Clear stored counter value */
    wwdg_counter_value = WWDG_DEFAULT_COUNTER;

    return WWDG_OK;
}

/**
 * @brief   Refresh the watchdog counter
 * @details Must be called within the valid window to prevent reset
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_Refresh(void)
{
    /* Check if we're in the valid window */
    if (!WWDG_IsInWindow())
    {
        return WWDG_WINDOW_ERROR;
    }

    HAL_StatusTypeDef halStatus = HAL_WWDG_Refresh(&hwwdg);
    return WWDG_ConvertHALStatus(halStatus);
}

/**
 * @brief   Refresh with specific counter value
 * @details Allows setting a new counter value during refresh
 * @param   counter New counter value (0x40 - 0x7F)
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_RefreshWithCounter(uint32_t counter)
{
    if (counter < WWDG_COUNTER_MIN || counter > WWDG_COUNTER_MAX)
    {
        return WWDG_INVALID_PARAM;
    }

    /* Check if we're in the valid window */
    if (!WWDG_IsInWindow())
    {
        return WWDG_WINDOW_ERROR;
    }

    /* Update counter value */
    wwdg_counter_value = counter;

    /* Write new counter value directly */
    WRITE_REG(hwwdg.Instance->CR, (WWDG_CR_WDGA | counter));

    return WWDG_OK;
}

/**
 * @brief   Start the watchdog
 * @details Enables WWDG countdown
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 * @note    WWDG is started automatically by HAL_WWDG_Init()
 */
WWDG_StatusTypeDef WWDG_Start(void)
{
    /* WWDG is started automatically by HAL_WWDG_Init() */
    return WWDG_OK;
}

/**
 * @brief   Register Early Wakeup Interrupt callback
 * @details Callback is called when counter reaches 0x40
 * @param   callback Function pointer to callback
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_RegisterEWICallback(WWDG_EWI_Callback_t callback)
{
    if (callback == NULL)
    {
        return WWDG_INVALID_PARAM;
    }

    wwdg_ewi_callback = callback;
    return WWDG_OK;
}

/**
 * @brief   Enable Early Wakeup Interrupt
 * @details Enables interrupt when counter reaches 0x40
 * @param   None
 * @retval  WWDG_StatusTypeDef Operation status
 */
WWDG_StatusTypeDef WWDG_EnableEWI(void)
{
    /* Enable EWI interrupt */
    __HAL_WWDG_ENABLE_IT(&hwwdg, WWDG_IT_EWI);

    /* Configure NVIC */
    HAL_NVIC_SetPriority(WWDG_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(WWDG_IRQn);

    return WWDG_OK;
}

/**
 * @brief   Check if last reset was caused by WWDG
 * @details Reads RCC reset flags to determine reset source
 * @param   None
 * @retval  bool True if WWDG caused the reset
 */
bool WWDG_WasResetSource(void)
{
    return (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET);
}

/**
 * @brief   Clear WWDG reset flag
 * @details Clears the WWDG reset flag in RCC
 * @param   None
 * @retval  None
 */
void WWDG_ClearResetFlag(void)
{
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

/**
 * @brief   Get current counter value
 * @details Reads the current WWDG counter value
 * @param   None
 * @retval  uint32_t Current counter value
 */
uint32_t WWDG_GetCounter(void)
{
    return (hwwdg.Instance->CR & WWDG_CR_T);
}

/**
 * @brief   Check if refresh is in valid window
 * @details Checks if current counter is within refresh window
 * @param   None
 * @retval  bool True if refresh is allowed
 */
bool WWDG_IsInWindow(void)
{
    uint32_t counter = WWDG_GetCounter();
    uint32_t window = hwwdg.Init.Window;

    /* Refresh is valid when counter < window AND counter > 0x3F */
    return (counter <= window) && (counter > WWDG_COUNTER_MIN - 1);
}

/**
 * @brief   Calculate timeout from configuration
 * @details Calculates timeout values in microseconds
 * @param   prescaler WWDG prescaler value
 * @param   counter Counter value
 * @param   window Window value
 * @param   minTimeout Pointer to store minimum timeout (window open)
 * @param   maxTimeout Pointer to store maximum timeout (reset)
 */
void WWDG_CalculateTimeout(uint32_t prescaler, uint32_t counter, uint32_t window,
                           uint32_t* minTimeout, uint32_t* maxTimeout)
{
    uint32_t prescalerDiv = 1U;

    switch (prescaler)
    {
        case WWDG_PRESCALER_1: prescalerDiv = 1U;  break;
        case WWDG_PRESCALER_2: prescalerDiv = 2U;  break;
        case WWDG_PRESCALER_4: prescalerDiv = 4U;  break;
        case WWDG_PRESCALER_8: prescalerDiv = 8U;  break;
        default:               prescalerDiv = 1U;  break;
    }

    /* WWDG clock = PCLK1 / 4096 / prescaler */
    /* Timeout = (counter - 0x3F) * 4096 * prescaler / PCLK1 */

    uint32_t ticksToReset = (counter & WWDG_COUNTER_MASK);
    uint32_t ticksToWindow = (counter - window);

    if (minTimeout != NULL)
    {
        /* Time until window opens (in microseconds) */
        *minTimeout = (ticksToWindow * WWDG_TICKS_PER_COUNT * prescalerDiv * WWDG_USEC_PER_SEC) / WWDG_PCLK1_FREQ;
    }

    if (maxTimeout != NULL)
    {
        /* Time until reset (in microseconds) */
        *maxTimeout = (ticksToReset * WWDG_TICKS_PER_COUNT * prescalerDiv * WWDG_USEC_PER_SEC) / WWDG_PCLK1_FREQ;
    }
}

/**
 * @brief   Get WWDG status string
 * @details Converts status code to human-readable string
 * @param   status WWDG status code
 * @retval  const char* Status description string
 */
const char* WWDG_GetStatusString(WWDG_StatusTypeDef status)
{
    switch (status)
    {
        case WWDG_OK:
            return "WWDG_OK";
        case WWDG_ERROR:
            return "WWDG_ERROR";
        case WWDG_TIMEOUT:
            return "WWDG_TIMEOUT";
        case WWDG_INVALID_PARAM:
            return "WWDG_INVALID_PARAM";
        case WWDG_WINDOW_ERROR:
            return "WWDG_WINDOW_ERROR";
        default:
            return "UNKNOWN_STATUS";
    }
}

/**
 * @brief   WWDG Early Wakeup Interrupt Handler Callback
 * @details Called by HAL when EWI interrupt occurs
 * @param   hwwdg_ptr Pointer to WWDG handle
 */
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg_ptr)
{
    (void)hwwdg_ptr;

    /* Call user callback if registered */
    if (wwdg_ewi_callback != NULL)
    {
        wwdg_ewi_callback();
    }

    /* Refresh watchdog to prevent reset */
    HAL_WWDG_Refresh(&hwwdg);
}
