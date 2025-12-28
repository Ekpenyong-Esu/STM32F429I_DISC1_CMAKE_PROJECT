/**
 ******************************************************************************
 * @file    dac.h
 * @author  Mahonri
 * @brief   DAC peripheral driver for STM32F429I Discovery board
 *          Clean architecture interface for DAC operations
 * @date    2025
 *
 * @details This header file provides the interface for the Digital-to-Analog
 *          Converter (DAC) peripheral driver. It abstracts the STM32 HAL layer
 *          to provide a clean, easy-to-use API for DAC operations.
 *
 *          Features supported:
 *          - Single channel operation (DAC_CHANNEL_1)
 *          - Configurable trigger modes
 *          - Output buffer control
 *          - Voltage conversion utilities
 *          - Error handling
 *
 * @note    This driver is designed for STM32F429I Discovery board
 * @attention
 *          This software is provided as-is, without any express or implied warranties.
 *          In no event will the authors be held liable for any damages arising from
 *          the use of this software.
 *
 * @section dac_usage Usage Example
 * @code
 * DAC_HandleStruct hdac;
 * DAC_ConfigTypeDef config = {
 *     .channel = DAC_CHANNEL_1,
 *     .trigger = DAC_TRIGGER_NONE,
 *     .output_buffer = DAC_OUTPUTBUFFER_ENABLE
 * };
 *
 * DAC_Init(&hdac, &config);
 * DAC_SetValue(&hdac, DAC_CHANNEL_1, 2048); // Output ~1.65V
 * DAC_DeInit(&hdac);
 * @endcode
 ******************************************************************************
 */

#ifndef __DAC_H
#define __DAC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"                /**< STM32F4xx standard peripheral drivers */
#include <stdint.h>                  /**< Standard integer types */
#include <stdbool.h>                 /**< Standard boolean type */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief DAC Configuration structure
 *
 * This structure defines the configuration parameters for DAC initialization.
 * All fields should use STM32 HAL-defined macros for portability.
 */
typedef struct {
    uint32_t channel;        /**< DAC channel to configure (e.g. DAC_CHANNEL_1)
                                 @note Only DAC_CHANNEL_1 is supported in this implementation */
    uint32_t trigger;        /**< Trigger selection (e.g. DAC_TRIGGER_NONE, DAC_TRIGGER_SOFTWARE)
                                 @note Software trigger is recommended for basic applications */
    uint32_t output_buffer;  /**< Output buffer control (DAC_OUTPUTBUFFER_ENABLE/DISABLE)
                                 @note Enable buffer for better performance, disable for low power */
} DAC_ConfigTypeDef;

/**
 * @brief DAC Handle structure
 *
 * Main structure containing all DAC-related information and state.
 * This structure encapsulates the STM32 HAL handle and driver-specific data.
 */
typedef struct DAC_HandleStruct {
    DAC_HandleTypeDef    hal_handle;     /**< STM32 HAL DAC handle - contains hardware registers */
    DAC_ConfigTypeDef    config;         /**< Current DAC configuration settings */
    bool                 initialized;    /**< Initialization status flag
                                             @note Must be true before calling other functions */
} DAC_HandleStruct;

/* Exported constants --------------------------------------------------------*/

/** @defgroup DAC_Constants DAC Driver Constants
 * @{
 */

/**
 * @brief Maximum value for 12-bit DAC
 * @note 2^12 - 1 = 4095
 */
#define DAC_MAX_VALUE_12BIT    4095U

/**
 * @brief Reference voltage for voltage calculations
 * @note STM32F429I Discovery board uses 3.3V reference
 */
#define DAC_REFERENCE_VOLTAGE  3.3f

/** @} */ /* End of DAC_Constants */

/* Exported functions --------------------------------------------------------*/

/** @defgroup DAC_Functions DAC Driver Functions
 * @{
 */

/** @defgroup DAC_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief Initialize the DAC peripheral
 *
 * This function initializes the DAC peripheral with the specified configuration.
 * It configures the GPIO pins, enables clocks, and sets up the DAC channel.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] config Pointer to DAC configuration structure
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Initialization successful
 *         - HAL_ERROR: Initialization failed (invalid parameters or HAL error)
 *
 * @note This function must be called before using any other DAC functions
 * @note The DAC handle must remain valid for the lifetime of DAC operations
 *
 * @pre hdac != NULL and config != NULL
 * @post hdac->initialized == true on success
 */
HAL_StatusTypeDef DAC_Init(DAC_HandleStruct* hdac, const DAC_ConfigTypeDef* config);

/**
 * @brief Deinitialize the DAC peripheral
 *
 * This function deinitializes the DAC peripheral and releases associated resources.
 * It stops any ongoing conversions and disables the DAC.
 *
 * @param[in] hdac Pointer to DAC handle structure
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Deinitialization successful
 *         - HAL_ERROR: Deinitialization failed (invalid parameters or HAL error)
 *
 * @note Call this function when DAC operations are complete to free resources
 * @note The DAC handle can be reused after deinitialization
 *
 * @pre hdac != NULL and hdac->initialized == true
 * @post hdac->initialized == false
 */
HAL_StatusTypeDef DAC_DeInit(DAC_HandleStruct* hdac);

/** @} */ /* End of DAC_Init_Config */

/** @defgroup DAC_Output_Operations Output Operations
 * @{
 */

/**
 * @brief Set DAC output value and start conversion
 *
 * This function sets the DAC output to the specified value and starts the conversion.
 * The value is automatically aligned to 12-bit right-aligned format.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] channel DAC channel (must match initialized channel)
 * @param[in] value DAC output value (0 to DAC_MAX_VALUE_12BIT)
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Value set and conversion started successfully
 *         - HAL_ERROR: Operation failed (invalid parameters or HAL error)
 *
 * @note This is a convenience function that combines DAC_SetValue and DAC_Start
 * @note For continuous output, call this function once. For dynamic changes,
 *       call repeatedly with new values.
 *
 * @pre hdac != NULL and hdac->initialized == true
 * @pre value <= DAC_MAX_VALUE_12BIT
 */
HAL_StatusTypeDef DAC_SetValue(DAC_HandleStruct* hdac, uint32_t channel, uint32_t value);

/**
 * @brief Start DAC conversion
 *
 * This function starts DAC conversion using the previously set value.
 * Must be called after setting a value with HAL_DAC_SetValue if not using DAC_SetValue.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] channel DAC channel to start
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Conversion started successfully
 *         - HAL_ERROR: Operation failed (invalid parameters or HAL error)
 *
 * @note Use this for fine-grained control over when conversion starts
 * @note DAC must be properly configured before calling this function
 *
 * @pre hdac != NULL and hdac->initialized == true
 */
HAL_StatusTypeDef DAC_Start(DAC_HandleStruct* hdac, uint32_t channel);

/**
 * @brief Stop DAC conversion
 *
 * This function stops the ongoing DAC conversion.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] channel DAC channel to stop
 *
 * @return HAL_StatusTypeDef
 *         - HAL_OK: Conversion stopped successfully
 *         - HAL_ERROR: Operation failed (invalid parameters or HAL error)
 *
 * @note The DAC output will hold the last value when stopped
 *
 * @pre hdac != NULL and hdac->initialized == true
 */
HAL_StatusTypeDef DAC_Stop(DAC_HandleStruct* hdac, uint32_t channel);

/**
 * @brief Get current DAC output value
 *
 * This function reads the current value from the DAC data holding register.
 *
 * @param[in] hdac Pointer to DAC handle structure
 * @param[in] channel DAC channel to read from
 *
 * @return uint32_t Current DAC output value (0 to DAC_MAX_VALUE_12BIT)
 *         Returns 0 if parameters are invalid
 *
 * @note This reads the register value, not the actual output voltage
 * @note Useful for debugging and verification
 *
 * @pre hdac != NULL and hdac->initialized == true
 */
uint32_t DAC_GetValue(const DAC_HandleStruct* hdac, uint32_t channel);

/** @} */ /* End of DAC_Output_Operations */

/** @defgroup DAC_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief Convert raw DAC value to voltage
 *
 * This utility function converts a raw 12-bit DAC value to the corresponding
 * output voltage based on the reference voltage.
 *
 * @param[in] raw_value Raw DAC value (0 to DAC_MAX_VALUE_12BIT)
 *
 * @return float Output voltage in volts
 *
 * @note Uses DAC_REFERENCE_VOLTAGE as the reference
 * @note Formula: voltage = (raw_value * Vref) / DAC_MAX_VALUE_12BIT
 *
 * @pre raw_value <= DAC_MAX_VALUE_12BIT
 */
float DAC_RawToVoltage(uint32_t raw_value);

/**
 * @brief Convert voltage to raw DAC value
 *
 * This utility function converts a voltage value to the corresponding
 * raw 12-bit DAC value.
 *
 * @param[in] voltage Input voltage in volts
 *
 * @return uint32_t Raw DAC value (0 to DAC_MAX_VALUE_12BIT)
 *
 * @note Uses DAC_REFERENCE_VOLTAGE as the reference
 * @note Formula: raw_value = (voltage * DAC_MAX_VALUE_12BIT) / Vref
 * @note Values outside valid range are clamped
 *
 * @pre voltage >= 0.0f
 */
uint32_t DAC_VoltageToRaw(float voltage);

/** @} */ /* End of DAC_Utility_Functions */

/** @} */ /* End of DAC_Functions */

#ifdef __cplusplus
}
#endif

#endif /* __DAC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
