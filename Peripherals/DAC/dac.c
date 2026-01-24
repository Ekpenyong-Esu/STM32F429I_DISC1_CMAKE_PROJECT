/**
 ******************************************************************************
 * @file    dac.c
 * @author  Mahonri
 * @brief   DAC peripheral driver implementation for STM32F429I Discovery board
 * @date    2025
 *
 * @details This file contains the implementation of the DAC peripheral driver.
 *          It provides a clean abstraction layer over the STM32 HAL DAC functions
 *          with additional error checking and utility functions.
 *
 *          Key features:
 *          - Parameter validation
 *          - State management
 *          - Error handling
 *          - Voltage conversion utilities
 *          - GPIO configuration
 *
 * @note    This implementation is specifically designed for STM32F429I Discovery board
 * @attention
 *          This software is provided as-is, without any express or implied warranties.
 *          In no event will the authors be held liable for any damages arising from
 *          the use of this software.
 *
 * @section dac_implementation Implementation Notes
 * - Uses STM32 HAL layer for low-level hardware access
 * - Implements MSP callbacks for GPIO configuration
 * - Provides thread-safe operations (when used with HAL locks)
 * - Supports 12-bit resolution with right-aligned data format
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "dac.h"                    /**< DAC driver header */
#include "stm32f4xx_hal_dac.h"      /**< STM32 HAL DAC header */
#include "log.h"             /**< Logging utilities */
#include <string.h>                 /**< For memset function */



/**
 * @brief Initialize the DAC peripheral
 *
 * This function performs the following initialization steps:
 * 1. Validates input parameters
 * 2. Clears the handle structure
 * 3. Stores configuration
 * 4. Initializes STM32 HAL DAC
 * 5. Configures the DAC channel
 * 6. Sets initialization flag
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] config Pointer to DAC configuration structure
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Initialization successful
 *         - HAL_ERROR: Initialization failed
 */
HAL_StatusTypeDef DAC_Init(DAC_HandleStruct* hdac, const DAC_ConfigTypeDef* config)
{
    log_debug("DAC: Initializing DAC");

    HAL_StatusTypeDef status = HAL_ERROR;

    /* Parameter validation */
    if (!hdac || !config) {
        return HAL_ERROR;
    }

    /* Clear handle structure to ensure clean state */
    memset(hdac, 0, sizeof(*hdac));

    /* Store configuration for later reference */
    hdac->config = *config;

    /* Initialize STM32 HAL DAC handle */
    hdac->hal_handle.Instance = DAC;

    /* Initialize DAC peripheral */
    if (HAL_DAC_Init(&hdac->hal_handle) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Configure DAC channel */
    DAC_ChannelConfTypeDef sConfig = {0};
    sConfig.DAC_Trigger = config->trigger;
    sConfig.DAC_OutputBuffer = config->output_buffer;

    if (HAL_DAC_ConfigChannel(&hdac->hal_handle, &sConfig, config->channel) != HAL_OK) {
        /* Cleanup on configuration failure */
        HAL_DAC_DeInit(&hdac->hal_handle);
        return HAL_ERROR;
    }

    /* Mark as initialized */
    hdac->initialized = true;
    status = HAL_OK;

    log_debug("DAC: DAC initialized successfully");

    return status;
}

/**
 * @brief Deinitialize the DAC peripheral
 *
 * This function performs the following deinitialization steps:
 * 1. Validates input parameters and state
 * 2. Deinitializes STM32 HAL DAC
 * 3. Clears initialization flag
 *
 * @param[in] hdac Pointer to DAC handle structure
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Deinitialization successful
 *         - HAL_ERROR: Deinitialization failed
 */
HAL_StatusTypeDef DAC_DeInit(DAC_HandleStruct* hdac)
{
    /* Parameter and state validation */
    if (!hdac || !hdac->initialized) {
        return HAL_ERROR;
    }

    /* Deinitialize STM32 HAL DAC */
    if (HAL_DAC_DeInit(&hdac->hal_handle) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Clear initialization flag */
    hdac->initialized = false;

    return HAL_OK;
}

/**
 * @brief Set DAC output value and start conversion
 *
 * This function sets the DAC output value and immediately starts conversion.
 * It combines the operations of setting value and starting conversion for
 * convenience in simple applications.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] channel DAC channel (must match initialized channel)
 * @param[in] value DAC output value (0 to DAC_MAX_VALUE_12BIT)
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Operation successful
 *         - HAL_ERROR: Operation failed
 */
HAL_StatusTypeDef DAC_SetValue(DAC_HandleStruct* hdac, uint32_t channel, uint32_t value)
{
    /* Parameter and state validation */
    if (!hdac || !hdac->initialized) {
        return HAL_ERROR;
    }

    /* Set DAC value with 12-bit right alignment */
    if (HAL_DAC_SetValue(&hdac->hal_handle, channel, DAC_ALIGN_12B_R, value) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Start DAC conversion */
    if (HAL_DAC_Start(&hdac->hal_handle, channel) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Start DAC conversion
 *
 * This function starts DAC conversion using the previously set value.
 * Use this function for fine-grained control over conversion timing.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] channel DAC channel to start
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Conversion started successfully
 *         - HAL_ERROR: Operation failed
 */
HAL_StatusTypeDef DAC_Start(DAC_HandleStruct* hdac, uint32_t channel)
{
    /* Parameter and state validation */
    if (!hdac || !hdac->initialized) {
        return HAL_ERROR;
    }

    /* Start DAC conversion */
    return HAL_DAC_Start(&hdac->hal_handle, channel);
}

/**
 * @brief Stop DAC conversion
 *
 * This function stops the ongoing DAC conversion. The DAC output will
 * hold the last converted value.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] channel DAC channel to stop
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Conversion stopped successfully
 *         - HAL_ERROR: Operation failed
 */
HAL_StatusTypeDef DAC_Stop(DAC_HandleStruct* hdac, uint32_t channel)
{
    /* Parameter and state validation */
    if (!hdac || !hdac->initialized) {
        return HAL_ERROR;
    }

    /* Stop DAC conversion */
    return HAL_DAC_Stop(&hdac->hal_handle, channel);
}

/**
 * @brief Get current DAC output value
 *
 * This function reads the current value from the DAC data holding register.
 * Note that this returns the register value, not the actual analog output.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] channel DAC channel to read from
 *
 * @return uint32_t Current DAC register value (0 to DAC_MAX_VALUE_12BIT)
 *         Returns 0 if parameters are invalid
 */
uint32_t DAC_GetValue(const DAC_HandleStruct* hdac, uint32_t channel)
{
    /* Parameter and state validation */
    if (!hdac || !hdac->initialized) {
        return 0;
    }

    /* Read DAC register value */
    return HAL_DAC_GetValue(&hdac->hal_handle, channel);
}

/**
 * @brief Convert raw DAC value to voltage
 *
 * Utility function to convert a 12-bit DAC value to the corresponding
 * output voltage using the formula: Vout = (D * Vref) / (2^N - 1)
 *
 * @param[in] raw_value Raw DAC value (0 to DAC_MAX_VALUE_12BIT)
 *
 * @return float Output voltage in volts
 */
float DAC_RawToVoltage(uint32_t raw_value)
{
    /* Convert using DAC transfer function */
    return ((float)raw_value * DAC_REFERENCE_VOLTAGE) / DAC_MAX_VALUE_12BIT;
}

/**
 * @brief Convert voltage to raw DAC value
 *
 * Utility function to convert a voltage to the corresponding 12-bit DAC value.
 * Values outside the valid range are clamped to prevent overflow.
 *
 * @param[in] voltage Input voltage in volts
 *
 * @return uint32_t Raw DAC value (0 to DAC_MAX_VALUE_12BIT)
 */
uint32_t DAC_VoltageToRaw(float voltage)
{
    /* Clamp voltage to valid range */
    if (voltage < 0.0f) {
        voltage = 0.0f;
    }
    if (voltage > DAC_REFERENCE_VOLTAGE) {
        voltage = DAC_REFERENCE_VOLTAGE;
    }

    /* Convert using DAC transfer function */
    return (uint32_t)((voltage * DAC_MAX_VALUE_12BIT) / DAC_REFERENCE_VOLTAGE);
}

/**
 * @brief DAC MSP Initialization callback
 *
 * This callback function is called by HAL_DAC_Init() to initialize
 * the DAC MSP (MCU Support Package). It configures:
 * - DAC peripheral clock
 * - GPIOA clock
 * - GPIO pin PA4 as analog output
 *
 * @param[in] hdac Pointer to DAC handle (HAL structure)
 *
 * @note This function is automatically called by HAL_DAC_Init()
 * @note GPIO configuration is specific to STM32F429I Discovery board
 */
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Check if DAC instance is valid */
    if (hdac->Instance == DAC) {
        /* Enable DAC peripheral clock */
        __HAL_RCC_DAC_CLK_ENABLE();

        /* Enable GPIOA clock for DAC output pin */
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* Configure PA4 (DAC_OUT1) as analog, no pull-up/pull-down */
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
