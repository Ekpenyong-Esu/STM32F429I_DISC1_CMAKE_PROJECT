/**
  ******************************************************************************
  * @file    rng.c
  * @brief   Random Number Generator (RNG) module implementation
  * @details This file provides code for hardware random number generation
  *          using the STM32F429's true RNG peripheral.
  * @version 1.0
  * @date    2025-12-11
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "rng.h"
#include "log.h"

/* Private defines -----------------------------------------------------------*/
#define RNG_FLOAT_DIVISOR       4294967296.0f   /**< 2^32 for float conversion */

/* Private variables ---------------------------------------------------------*/
/**
 * @brief   RNG handle structure
 * @details Used by HAL functions for RNG peripheral operations
 */
RNG_HandleTypeDef hrng;

/* Private function prototypes -----------------------------------------------*/
static RNG_StatusTypeDef RNG_ConvertHALStatus(HAL_StatusTypeDef halStatus);
static RNG_StatusTypeDef RNG_CheckErrors(void);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Convert HAL status to RNG status
 * @param   halStatus HAL status code
 * @retval  RNG_StatusTypeDef Converted status
 */
static RNG_StatusTypeDef RNG_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus)
    {
        case HAL_OK:
            return RNG_OK;
        case HAL_ERROR:
            return RNG_ERROR;
        case HAL_TIMEOUT:
            return RNG_TIMEOUT;
        default:
            return RNG_ERROR;
    }
}

/**
 * @brief   Check for RNG errors
 * @retval  RNG_StatusTypeDef Error status
 */
static RNG_StatusTypeDef RNG_CheckErrors(void)
{
    /* Check for seed error */
    if (__HAL_RNG_GET_FLAG(&hrng, RNG_FLAG_SECS))
    {
        return RNG_SEED_ERROR;
    }

    /* Check for clock error */
    if (__HAL_RNG_GET_FLAG(&hrng, RNG_FLAG_CECS))
    {
        return RNG_CLOCK_ERROR;
    }

    return RNG_OK;
}

/* Public functions ----------------------------------------------------------*/

/**
 * @brief   Initialize the Random Number Generator
 * @details Configures and enables the RNG peripheral
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_Init(void)
{
    log_debug("RNG: Initializing Random Number Generator");

    /* Enable RNG clock */
    __HAL_RCC_RNG_CLK_ENABLE();

    hrng.Instance = RNG;

    HAL_StatusTypeDef halStatus = HAL_RNG_Init(&hrng);
    if (halStatus != HAL_OK)
    {
        return RNG_ConvertHALStatus(halStatus);
    }

    /* Check for initial errors */
    RNG_StatusTypeDef status = RNG_CheckErrors();

    if (status == RNG_OK) {
        log_debug("RNG: Random Number Generator initialized successfully");
    }

    return status;
}

/**
 * @brief   Deinitialize the Random Number Generator
 * @details Disables RNG peripheral and releases resources
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_DeInit(void)
{
    HAL_StatusTypeDef halStatus = HAL_RNG_DeInit(&hrng);

    /* Disable RNG clock */
    __HAL_RCC_RNG_CLK_DISABLE();

    return RNG_ConvertHALStatus(halStatus);
}

/**
 * @brief   Generate a 32-bit random number
 * @details Generates a single 32-bit true random number
 * @param   randomNumber Pointer to store the random number
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_Generate(uint32_t* randomNumber)
{
    return RNG_GenerateWithTimeout(randomNumber, RNG_TIMEOUT_DEFAULT);
}

/**
 * @brief   Generate a random number with timeout
 * @details Generates random number with specified timeout
 * @param   randomNumber Pointer to store the random number
 * @param   timeout Timeout in milliseconds (reserved for future use)
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateWithTimeout(uint32_t* randomNumber, uint32_t timeout)
{
    (void)timeout;  /* Currently unused - HAL handles timeout internally */

    if (randomNumber == NULL)
    {
        return RNG_ERROR;
    }

    /* Check for errors first */
    RNG_StatusTypeDef status = RNG_CheckErrors();
    if (status != RNG_OK)
    {
        return status;
    }

    HAL_StatusTypeDef halStatus = HAL_RNG_GenerateRandomNumber(&hrng, randomNumber);
    if (halStatus != HAL_OK)
    {
        return RNG_ConvertHALStatus(halStatus);
    }

    return RNG_OK;
}

/**
 * @brief   Generate multiple random numbers
 * @details Fills buffer with random 32-bit values
 * @param   buffer Pointer to buffer for random numbers
 * @param   count Number of 32-bit random numbers to generate
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateBuffer(uint32_t* buffer, uint32_t count)
{
    if (buffer == NULL || count == 0)
    {
        return RNG_ERROR;
    }

    for (uint32_t i = 0; i < count; i++)
    {
        RNG_StatusTypeDef status = RNG_Generate(&buffer[i]);
        if (status != RNG_OK)
        {
            return status;
        }
    }

    return RNG_OK;
}

/**
 * @brief   Generate random bytes
 * @details Fills buffer with random bytes
 * @param   buffer Pointer to byte buffer
 * @param   length Number of bytes to generate
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateBytes(uint8_t* buffer, uint32_t length)
{
    if (buffer == NULL || length == 0)
    {
        return RNG_ERROR;
    }

    uint32_t randomWord = 0;
    uint32_t bytesRemaining = length;
    uint32_t bufferIndex = 0;

    while (bytesRemaining > 0)
    {
        /* Generate a new random word */
        RNG_StatusTypeDef status = RNG_Generate(&randomWord);
        if (status != RNG_OK)
        {
            return status;
        }

        /* Extract bytes from the random word */
        uint32_t bytesToCopy = (bytesRemaining >= 4) ? 4 : bytesRemaining;
        for (uint32_t i = 0; i < bytesToCopy; i++)
        {
            buffer[bufferIndex++] = (uint8_t)(randomWord >> (i * 8));
        }

        bytesRemaining -= bytesToCopy;
    }

    return RNG_OK;
}

/**
 * @brief   Generate random number in range [0, max)
 * @details Returns random number less than max
 * @param   max Upper bound (exclusive)
 * @param   result Pointer to store result
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateRange(uint32_t max, uint32_t* result)
{
    if (result == NULL || max == 0)
    {
        return RNG_ERROR;
    }

    uint32_t randomNumber = 0;
    RNG_StatusTypeDef status = RNG_Generate(&randomNumber);
    if (status != RNG_OK)
    {
        return status;
    }

    /* Use modulo for range limiting (note: slight bias for non-power-of-2 max) */
    *result = randomNumber % max;

    return RNG_OK;
}

/**
 * @brief   Generate random number in range [min, max]
 * @details Returns random number between min and max inclusive
 * @param   min Lower bound (inclusive)
 * @param   max Upper bound (inclusive)
 * @param   result Pointer to store result
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateInRange(uint32_t min, uint32_t max, uint32_t* result)
{
    if (result == NULL || min > max)
    {
        return RNG_ERROR;
    }

    if (min == max)
    {
        *result = min;
        return RNG_OK;
    }

    uint32_t range = max - min + 1;
    uint32_t randomValue = 0;

    RNG_StatusTypeDef status = RNG_GenerateRange(range, &randomValue);
    if (status != RNG_OK)
    {
        return status;
    }

    *result = min + randomValue;

    return RNG_OK;
}

/**
 * @brief   Generate random boolean
 * @details Returns true or false randomly
 * @param   result Pointer to store boolean result
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateBool(bool* result)
{
    if (result == NULL)
    {
        return RNG_ERROR;
    }

    uint32_t randomNumber = 0;
    RNG_StatusTypeDef status = RNG_Generate(&randomNumber);
    if (status != RNG_OK)
    {
        return status;
    }

    *result = (randomNumber & 0x01) ? true : false;

    return RNG_OK;
}

/**
 * @brief   Generate random float [0.0, 1.0)
 * @details Returns random float in range [0, 1)
 * @param   result Pointer to store float result
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_GenerateFloat(float* result)
{
    if (result == NULL)
    {
        return RNG_ERROR;
    }

    uint32_t randomNumber = 0;
    RNG_StatusTypeDef status = RNG_Generate(&randomNumber);
    if (status != RNG_OK)
    {
        return status;
    }

    *result = (float)randomNumber / RNG_FLOAT_DIVISOR;

    return RNG_OK;
}

/**
 * @brief   Check if RNG is ready
 * @details Checks if RNG has random data available
 * @param   None
 * @retval  bool True if random number is ready
 */
bool RNG_IsReady(void)
{
    return (__HAL_RNG_GET_FLAG(&hrng, RNG_FLAG_DRDY) != RESET);
}

/**
 * @brief   Check for RNG errors
 * @details Checks for seed or clock errors
 * @param   None
 * @retval  bool True if error detected
 */
bool RNG_HasError(void)
{
    return (RNG_CheckErrors() != RNG_OK);
}

/**
 * @brief   Clear RNG errors
 * @details Clears any pending error flags
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_ClearErrors(void)
{
    /* Clear seed error flag */
    __HAL_RNG_CLEAR_FLAG(&hrng, RNG_FLAG_SECS);

    /* Clear clock error flag */
    __HAL_RNG_CLEAR_FLAG(&hrng, RNG_FLAG_CECS);

    /* Reinitialize RNG to recover from errors */
    HAL_RNG_DeInit(&hrng);
    HAL_RNG_Init(&hrng);

    return RNG_OK;
}

/**
 * @brief   Get RNG status string
 * @details Converts status code to human-readable string
 * @param   status RNG status code
 * @retval  const char* Status description string
 */
const char* RNG_GetStatusString(RNG_StatusTypeDef status)
{
    switch (status)
    {
        case RNG_OK:
            return "RNG_OK";
        case RNG_ERROR:
            return "RNG_ERROR";
        case RNG_TIMEOUT:
            return "RNG_TIMEOUT";
        case RNG_CLOCK_ERROR:
            return "RNG_CLOCK_ERROR";
        case RNG_SEED_ERROR:
            return "RNG_SEED_ERROR";
        case RNG_NOT_READY:
            return "RNG_NOT_READY";
        default:
            return "UNKNOWN_STATUS";
    }
}

/**
 * @brief   Enable RNG interrupt mode
 * @details Enables interrupt-based random number generation
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_EnableInterrupt(void)
{
    /* Enable RNG interrupt */
    __HAL_RNG_ENABLE_IT(&hrng);

    /* Configure NVIC */
    HAL_NVIC_SetPriority(HASH_RNG_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(HASH_RNG_IRQn);

    return RNG_OK;
}

/**
 * @brief   Disable RNG interrupt mode
 * @details Disables interrupt-based generation
 * @param   None
 * @retval  RNG_StatusTypeDef Operation status
 */
RNG_StatusTypeDef RNG_DisableInterrupt(void)
{
    /* Disable RNG interrupt */
    __HAL_RNG_DISABLE_IT(&hrng);

    HAL_NVIC_DisableIRQ(HASH_RNG_IRQn);

    return RNG_OK;
}
