/**
 ******************************************************************************
 * @file    adc.h
 * @author  Mahonri
 * @brief   ADC peripheral driver for STM32F429I Discovery board
 *          This file provides a clean architecture interface for ADC operations
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/


/**
 * @brief ADC Conversion mode
 */
typedef enum {
    ADC_MODE_SINGLE         = 0,  /*!< Single conversion mode */
    ADC_MODE_CONTINUOUS     = 1,  /*!< Continuous conversion mode */
} ADC_ConversionModeTypeDef;

/**
 * @brief ADC Configuration structure
 */
typedef struct {
    uint32_t                    channel;        /*!< ADC channel to configure */
    uint32_t                    resolution;     /*!< ADC resolution */
    uint32_t                    sampling_time;  /*!< Sampling time */
    ADC_ConversionModeTypeDef   conv_mode;      /*!< Conversion mode */
    bool                        dma_enabled;    /*!< Enable DMA for conversions */
} ADC_ConfigTypeDef;

/**
 * @brief ADC Handle structure for clean abstraction
 */
typedef struct ADC_HandleStruct {
    ADC_HandleTypeDef   hal_handle;     /*!< STM32 HAL ADC handle */
    DMA_HandleTypeDef   hdma_adc;       /*!< DMA handle for ADC */
    ADC_ConfigTypeDef   config;         /*!< ADC configuration */
    bool                initialized;    /*!< Initialization status */
    bool                calibrated;     /*!< Calibration status */
    /* Optional user callbacks for interrupt/DMA driven operation */
    void (*conv_complete_cb)(struct ADC_HandleStruct* hadc, uint32_t value);
    void (*error_cb)(struct ADC_HandleStruct* hadc);
    /* Last conversion result and flag (useful for ISR contexts) */
    volatile uint32_t   last_value;
    volatile bool       conv_complete_flag;
} ADC_HandleStruct;


/* Exported constants --------------------------------------------------------*/

/* ADC Constants */
#define ADC_MAX_VALUE_12BIT     4095U   /*!< Maximum value for 12-bit ADC */
#define ADC_MAX_VALUE_10BIT     1023U   /*!< Maximum value for 10-bit ADC */
#define ADC_MAX_VALUE_8BIT      255U    /*!< Maximum value for 8-bit ADC */
#define ADC_MAX_VALUE_6BIT      63U     /*!< Maximum value for 6-bit ADC */

#define ADC_REFERENCE_VOLTAGE   3.3f    /*!< Reference voltage in Volts */
#define ADC_TIMEOUT_MS          1000U   /*!< Default timeout in milliseconds */

/* Number of available ADC instances */
#define ADC_INSTANCE_COUNT      3U      /*!< Number of ADC instances available */

/* Exported macro ------------------------------------------------------------*/

/**
 * @brief Convert ADC raw value to voltage
 * @param raw_value: Raw ADC value (0 to max_value based on resolution)
 * @param resolution: ADC resolution identifier (use HAL resolution macros)
 * @retval float: Voltage in Volts (0.0 to ADC_REFERENCE_VOLTAGE)
 *
 * @note This macro uses ADC_GetMaxValue(resolution) to obtain the numeric
 *       maximum for the configured resolution, so it works with HAL macros
 *       (e.g. ADC_RESOLUTION_12B) rather than expecting a bit-count.
 */
#define ADC_RAW_TO_VOLTAGE(raw_value, resolution) \
    ((float)(raw_value) * ADC_REFERENCE_VOLTAGE / (float)(ADC_GetMaxValue(resolution)))

/**
 * @brief Convert voltage to ADC raw value
 * @param voltage: Voltage in Volts (0.0 to ADC_REFERENCE_VOLTAGE)
 * @param resolution: ADC resolution identifier (use HAL resolution macros)
 * @retval uint32_t: Raw ADC value (0 to max_value based on resolution)
 *
 * @note This macro uses ADC_GetMaxValue(resolution) to obtain the numeric
 *       maximum for the configured resolution, so it works with HAL macros
 *       (e.g. ADC_RESOLUTION_12B) rather than expecting a bit-count.
 */
#define ADC_VOLTAGE_TO_RAW(voltage, resolution) \
    ((uint32_t)(((voltage) * (float)(ADC_GetMaxValue(resolution)) / ADC_REFERENCE_VOLTAGE)))

/* Exported functions prototypes ---------------------------------------------*/

/* ADC Initialization and Configuration */
/**
 * @brief Initialize ADC with the specified configuration
 * @param hadc: Pointer to ADC handle structure
 * @param config: Pointer to ADC configuration structure
 * @retval ADC_StatusTypeDef: Status of the operation
 *
 * @note This function must be called before using any other ADC functions
 * @note Example: ADC_Init(&hadc1, &adc_config);
 */
HAL_StatusTypeDef ADC_Init(ADC_HandleStruct* hadc, const ADC_ConfigTypeDef* config);

/**
 * @brief Deinitialize ADC
 * @param hadc: Pointer to ADC handle structure
 * @retval ADC_StatusTypeDef: Status of the operation
 *
 * @note Call this to free ADC resources when done
 */
HAL_StatusTypeDef ADC_DeInit(ADC_HandleStruct* hadc);

/**
 * @brief Configure ADC channel for reading
 * @param hadc: Pointer to ADC handle structure
 * @param channel: ADC channel to configure
 * @param sampling_time: Sampling time for the channel
 * @retval ADC_StatusTypeDef: Status of the operation
 *
 * @note Must be called before reading from a specific channel
 */
HAL_StatusTypeDef ADC_ConfigChannel(ADC_HandleStruct* hadc, uint32_t channel,
                                   uint32_t sampling_time);

/* ADC Calibration */
/**
 * @brief Calibrate ADC for better accuracy
 * @param hadc: Pointer to ADC handle structure
 * @retval ADC_StatusTypeDef: Status of the operation
 *
 * @note STM32F4 has limited calibration compared to newer STM32 series
 * @note Call this after ADC_Init for best accuracy
 */
HAL_StatusTypeDef ADC_Calibrate(ADC_HandleStruct* hadc);

/* ADC Single Conversion Operations */
/**
 * @brief Start ADC conversion
 * @param hadc: Pointer to ADC handle structure
 * @retval ADC_StatusTypeDef: Status of the operation
 *
 * @note Use with ADC_PollForConversion and ADC_GetValue for complete reading
 */
HAL_StatusTypeDef ADC_StartConversion(ADC_HandleStruct* hadc);

/**
 * @brief Wait for ADC conversion to complete
 * @param hadc: Pointer to ADC handle structure
 * @param timeout_ms: Timeout in milliseconds
 * @retval ADC_StatusTypeDef: Status of the operation
 *
 * @note Blocks until conversion is done or timeout occurs
 */
HAL_StatusTypeDef ADC_PollForConversion(ADC_HandleStruct* hadc, uint32_t timeout_ms);

/**
 * @brief Get ADC conversion result
 * @param hadc: Pointer to ADC handle structure
 * @param value: Pointer to store the conversion value
 * @retval ADC_StatusTypeDef: Status of the operation
 *
 * @note Returns raw ADC value (0-4095 for 12-bit, 0-1023 for 10-bit, etc.)
 */
HAL_StatusTypeDef ADC_GetValue(ADC_HandleStruct* hadc, uint32_t* value);

/**
 * @brief Complete ADC channel reading (convenience function)
 * @param hadc: Pointer to ADC handle structure
 * @param channel: ADC channel to read
 * @param value: Pointer to store the conversion value
 * @retval ADC_StatusTypeDef: Status of the operation
 *
 * @note This function does: ConfigChannel + StartConversion + PollForConversion + GetValue
 * @note Most convenient for single readings
 */
HAL_StatusTypeDef ADC_ReadChannel(ADC_HandleStruct* hadc, uint32_t channel, uint32_t* value);

/* ADC Voltage Conversion Utilities */
/**
 * @brief Convert raw ADC value to voltage
 * @param raw_value: Raw ADC value
 * @param resolution: ADC resolution
 * @retval float: Voltage in Volts
 *
 * @note Assumes 3.3V reference voltage
 * @note Example: ADC_RawToVoltage(2048, ADC_RESOLUTION_12BIT) returns 1.65V
 */
float ADC_RawToVoltage(uint32_t raw_value, uint32_t resolution);

/**
 * @brief Convert voltage to raw ADC value
 * @param voltage: Voltage in Volts
 * @param resolution: ADC resolution
 * @retval uint32_t: Raw ADC value
 *
 * @note Assumes 3.3V reference voltage
 * @note Example: ADC_VoltageToRaw(1.65f, ADC_RESOLUTION_12BIT) returns 2048
 */
uint32_t ADC_VoltageToRaw(float voltage, uint32_t resolution);

/**
 * @brief Read ADC channel and return voltage directly
 * @param hadc: Pointer to ADC handle structure
 * @param channel: ADC channel to read
 * @retval float: Voltage in Volts, or negative value on error
 *
 * @note Most convenient function for voltage measurements
 * @note Returns -1.0f on error
 */
float ADC_ReadChannelVoltage(ADC_HandleStruct* hadc, uint32_t channel);

/* ADC Continuous Conversion Operations */
HAL_StatusTypeDef ADC_StartContinuousConversion(ADC_HandleStruct* hadc);
HAL_StatusTypeDef ADC_StopContinuousConversion(ADC_HandleStruct* hadc);

/* ADC DMA Operations */
HAL_StatusTypeDef ADC_StartDMA(ADC_HandleStruct* hadc, uint32_t* buffer, uint32_t length);
HAL_StatusTypeDef ADC_StopDMA(ADC_HandleStruct* hadc);

/* Interrupt driven conversions and callbacks */
HAL_StatusTypeDef ADC_Start_IT(ADC_HandleStruct* hadc);
HAL_StatusTypeDef ADC_Stop_IT(ADC_HandleStruct* hadc);
HAL_StatusTypeDef ADC_ReadChannel_IT(ADC_HandleStruct* hadc, uint32_t channel);

/* Callback registration */
void ADC_RegisterConvCompleteCallback(ADC_HandleStruct* hadc, void (*callback)(ADC_HandleStruct*, uint32_t));
void ADC_RegisterErrorCallback(ADC_HandleStruct* hadc, void (*callback)(ADC_HandleStruct*));

/* ADC Multi-channel Operations */
HAL_StatusTypeDef ADC_ConfigMultiChannel(ADC_HandleStruct* hadc,
                                        const uint32_t* channels,
                                        const uint32_t* sampling_times,
                                        uint32_t num_channels);
HAL_StatusTypeDef ADC_ReadMultiChannel(ADC_HandleStruct* hadc, uint32_t* values, uint32_t num_channels);

/* ADC Status and Error Handling */
HAL_StatusTypeDef ADC_GetStatus(const ADC_HandleStruct* hadc);
bool ADC_IsReady(const ADC_HandleStruct* hadc);
bool ADC_IsConversionComplete(const ADC_HandleStruct* hadc);
void ADC_ErrorHandler(ADC_HandleStruct* hadc);

/* ADC Hardware Specific Functions */
HAL_StatusTypeDef ADC_SetResolution(ADC_HandleStruct* hadc, uint32_t resolution);
HAL_StatusTypeDef ADC_SetSamplingTime(ADC_HandleStruct* hadc, uint32_t channel,
                                     uint32_t sampling_time);

/* ADC Temperature and Internal Sensors */
float ADC_ReadTemperature(ADC_HandleStruct* hadc);
float ADC_ReadVrefInt(ADC_HandleStruct* hadc);
float ADC_ReadVbat(ADC_HandleStruct* hadc);

/* ADC Utility Functions */
uint32_t ADC_GetMaxValue(uint32_t resolution);
const char* ADC_GetChannelName(uint32_t channel);
const char* ADC_GetStatusString(HAL_StatusTypeDef status);

/* ADC1 Instance - Most commonly used */
extern ADC_HandleStruct hadc1;

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
