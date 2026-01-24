/**
 ******************************************************************************
 * @file    adc.c
 * @author  Mahonri
 * @brief   ADC peripheral driver implementation for STM32F429I Discovery board
 *          This file provides the implementation of ADC operations with HAL integration
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "adc.h"
#include <string.h>
#include "log.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define ADC_CALIBRATION_TIMEOUT     10000U  /*!< Calibration timeout in ms */
#define ADC_CONVERSION_TIMEOUT      1000U   /*!< Conversion timeout in ms */

/* Temperature sensor constants for STM32F429 */
#define ADC_TEMP_V25                0.76f   /*!< Voltage at 25°C */
#define ADC_TEMP_AVG_SLOPE          0.0025f /*!< Temperature slope in V/°C */
#define ADC_TEMP_25C                25.0f   /*!< Temperature at 25°C */
#define ADC_VBAT_DIVIDER            2.0f    /*!< VBAT internal divider */
#define ADC_ABSOLUTE_ZERO          -273.15f /*!< Absolute zero temperature */

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Global ADC1 handle - most commonly used instance */
ADC_HandleStruct hadc1 = {0};

/* ADC channel to GPIO pin mapping table */
static const struct {
    uint32_t channel;
    GPIO_TypeDef* port;
    uint16_t pin;
    const char* name;
} adc_channel_map[] = {
    {ADC_CHANNEL_0,  GPIOA, GPIO_PIN_0,  "PA0"},
    {ADC_CHANNEL_1,  GPIOA, GPIO_PIN_1,  "PA1"},
    {ADC_CHANNEL_2,  GPIOA, GPIO_PIN_2,  "PA2"},
    {ADC_CHANNEL_3,  GPIOA, GPIO_PIN_3,  "PA3"},
    {ADC_CHANNEL_4,  GPIOA, GPIO_PIN_4,  "PA4"},
    {ADC_CHANNEL_5,  GPIOA, GPIO_PIN_5,  "PA5"},
    {ADC_CHANNEL_6,  GPIOA, GPIO_PIN_6,  "PA6"},
    {ADC_CHANNEL_7,  GPIOA, GPIO_PIN_7,  "PA7"},
    {ADC_CHANNEL_8,  GPIOB, GPIO_PIN_0,  "PB0"},
    {ADC_CHANNEL_9,  GPIOB, GPIO_PIN_1,  "PB1"},
    {ADC_CHANNEL_10, GPIOC, GPIO_PIN_0,  "PC0"},
    {ADC_CHANNEL_11, GPIOC, GPIO_PIN_1,  "PC1"},
    {ADC_CHANNEL_12, GPIOC, GPIO_PIN_2,  "PC2"},
    {ADC_CHANNEL_13, GPIOC, GPIO_PIN_3,  "PC3"},
    {ADC_CHANNEL_14, GPIOC, GPIO_PIN_4,  "PC4"},
    {ADC_CHANNEL_15, GPIOC, GPIO_PIN_5,  "PC5"},
    {ADC_CHANNEL_TEMPSENSOR,   NULL,  0,           "TEMP"},
    {ADC_CHANNEL_VREFINT,   NULL,  0,           "VREF"},
    {ADC_CHANNEL_VBAT,   NULL,  0,           "VBAT"},
};

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef ADC_ConfigureGPIO(uint32_t channel);
static HAL_StatusTypeDef ADC_ConfigureClocks(void);
static HAL_StatusTypeDef ADC_ConfigureDMA(ADC_HandleStruct* hadc);
static uint32_t ADC_ChannelToHAL(uint32_t channel);
static uint32_t ADC_ResolutionToHAL(uint32_t resolution);
static uint32_t ADC_SamplingTimeToHAL(uint32_t sampling_time);
static ADC_HandleStruct* ADC_MapHALHandle(ADC_HandleTypeDef* hal);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize ADC with the specified configuration
 * @param hadc: Pointer to ADC handle structure
 * @param config: Pointer to ADC configuration structure
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_Init(ADC_HandleStruct* hadc, const ADC_ConfigTypeDef* config)
{
    log_debug("ADC: Initializing ADC");

    if (hadc == NULL || config == NULL) {
    return HAL_ERROR;
    }

    /* Configure system clocks for ADC */
    if (ADC_ConfigureClocks() != HAL_OK) {
        return HAL_ERROR;
    }

    /* Configure GPIO for the selected channel */
    if (ADC_ConfigureGPIO(config->channel) != HAL_OK) {
        return HAL_ERROR;
    }

    /* Configure DMA if enabled */
    if (config->dma_enabled) {
        if (ADC_ConfigureDMA(hadc) != HAL_OK) {
            return HAL_ERROR;
        }
    }

    /* Initialize HAL ADC handle */
    hadc->hal_handle.Instance = ADC1;  // Use ADC1 by default
    hadc->hal_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc->hal_handle.Init.Resolution = ADC_ResolutionToHAL(config->resolution);
    hadc->hal_handle.Init.ScanConvMode = DISABLE;
    hadc->hal_handle.Init.ContinuousConvMode = (config->conv_mode == ADC_MODE_CONTINUOUS) ? ENABLE : DISABLE;
    hadc->hal_handle.Init.DiscontinuousConvMode = DISABLE;
    hadc->hal_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc->hal_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc->hal_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc->hal_handle.Init.NbrOfConversion = 1;
    hadc->hal_handle.Init.DMAContinuousRequests = config->dma_enabled ? ENABLE : DISABLE;
    hadc->hal_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    /* Initialize ADC using HAL */
    HAL_StatusTypeDef hal_status = HAL_ADC_Init(&hadc->hal_handle);
    if (hal_status != HAL_OK) {
        return hal_status;
    }

    /* Store configuration */
    hadc->config = *config;
    hadc->initialized = true;
    hadc->calibrated = false;

    log_debug("ADC: ADC initialized successfully");

    return HAL_OK;
}

/**
 * @brief Deinitialize ADC
 * @param hadc: Pointer to ADC handle structure
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_DeInit(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    /* Deinitialize HAL ADC */
    HAL_StatusTypeDef hal_status = HAL_ADC_DeInit(&hadc->hal_handle);
    if (hal_status != HAL_OK) {
        return hal_status;
    }

    /* Deinitialize DMA if it was configured */
    if (hadc->config.dma_enabled) {
        hal_status = HAL_DMA_DeInit(&hadc->hdma_adc);
        if (hal_status != HAL_OK) {
            return hal_status;
        }
    }

    /* Reset status flags */
    hadc->initialized = false;
    hadc->calibrated = false;

    return HAL_OK;
}

/**
 * @brief Configure ADC channel
 * @param hadc: Pointer to ADC handle structure
 * @param channel: ADC channel to configure
 * @param sampling_time: Sampling time for the channel
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_ConfigChannel(ADC_HandleStruct* hadc, uint32_t channel,
                                   uint32_t sampling_time)
{
    if (hadc == NULL) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    /* Configure GPIO for the channel if needed */
    ADC_ConfigureGPIO(channel);

    /* Configure ADC channel using HAL */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_ChannelToHAL(channel);
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SamplingTimeToHAL(sampling_time);

    HAL_StatusTypeDef hal_status = HAL_ADC_ConfigChannel(&hadc->hal_handle, &sConfig);
    return hal_status;
}

/**
 * @brief Calibrate ADC
 * @param hadc: Pointer to ADC handle structure
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_Calibrate(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    /* Start calibration - Note: STM32F4 doesn't have automatic calibration like STM32L4 */
    /* For STM32F4, we just mark as calibrated since hardware calibration is not available */
    hadc->calibrated = true;

    return HAL_OK;
}

/**
 * @brief Check if ADC is calibrated
 * @param hadc: Pointer to ADC handle structure
 * @retval bool: true if calibrated, false otherwise
 */
bool ADC_IsCalibrated(const ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
        return false;
    }
    return hadc->calibrated;
}

/**
 * @brief Start ADC conversion
 * @param hadc: Pointer to ADC handle structure
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_StartConversion(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    /* Start ADC conversion using HAL */
    HAL_StatusTypeDef hal_status = HAL_ADC_Start(&hadc->hal_handle);
    return hal_status;
}

/**
 * @brief Poll for conversion completion
 * @param hadc: Pointer to ADC handle structure
 * @param timeout_ms: Timeout in milliseconds
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_PollForConversion(ADC_HandleStruct* hadc, uint32_t timeout_ms)
{
    if (hadc == NULL) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    HAL_StatusTypeDef hal_status = HAL_ADC_PollForConversion(&hadc->hal_handle, timeout_ms);
    return hal_status;
}

/**
 * @brief Get ADC conversion value
 * @param hadc: Pointer to ADC handle structure
 * @param value: Pointer to store the conversion value
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_GetValue(ADC_HandleStruct* hadc, uint32_t* value)
{
    if (hadc == NULL || value == NULL) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    *value = HAL_ADC_GetValue(&hadc->hal_handle);
    return HAL_OK;
}

/**
 * @brief Read ADC channel value (complete operation)
 * @param hadc: Pointer to ADC handle structure
 * @param channel: ADC channel to read
 * @param value: Pointer to store the conversion value
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_ReadChannel(ADC_HandleStruct* hadc, uint32_t channel, uint32_t* value)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (hadc == NULL || value == NULL) {
    return HAL_ERROR;
    }

    /* Configure the channel */
    status = ADC_ConfigChannel(hadc, channel, ADC_SAMPLETIME_84CYCLES);
    if (status != HAL_OK) {
        return status;
    }

    /* Start conversion */
    status = ADC_StartConversion(hadc);
    if (status != HAL_OK) {
        return status;
    }

    /* Wait for conversion to complete */
    status = ADC_PollForConversion(hadc, ADC_CONVERSION_TIMEOUT);
    if (status != HAL_OK) {
        return status;
    }

    /* Get the value */
    status = ADC_GetValue(hadc, value);
    return status;
}

/**
 * @brief Convert raw ADC value to voltage
 * @param raw_value: Raw ADC value
 * @param resolution: ADC resolution
 * @retval float: Voltage in Volts
 */
float ADC_RawToVoltage(uint32_t raw_value, uint32_t resolution)
{
    uint32_t max_value = ADC_GetMaxValue(resolution);
    return ((float)raw_value * ADC_REFERENCE_VOLTAGE) / (float)max_value;
}

/**
 * @brief Convert voltage to raw ADC value
 * @param voltage: Voltage in Volts
 * @param resolution: ADC resolution
 * @retval uint32_t: Raw ADC value
 */
uint32_t ADC_VoltageToRaw(float voltage, uint32_t resolution)
{
    uint32_t max_value = ADC_GetMaxValue(resolution);
    return (uint32_t)((voltage * (float)max_value) / ADC_REFERENCE_VOLTAGE);
}

/**
 * @brief Read ADC channel voltage
 * @param hadc: Pointer to ADC handle structure
 * @param channel: ADC channel to read
 * @retval float: Voltage in Volts
 */
float ADC_ReadChannelVoltage(ADC_HandleStruct* hadc, uint32_t channel)
{
    uint32_t raw_value = 0;
    HAL_StatusTypeDef status = ADC_ReadChannel(hadc, channel, &raw_value);

    if (status != HAL_OK) {
        return -1.0f;  // Error indication
    }

    return ADC_RawToVoltage(raw_value, hadc->config.resolution);
}

/**
 * @brief Get maximum value for given resolution
 * @param resolution: ADC resolution
 * @retval uint32_t: Maximum value
 */
uint32_t ADC_GetMaxValue(uint32_t resolution)
{
    switch (resolution) {
    case ADC_RESOLUTION_12B: return ADC_MAX_VALUE_12BIT;
    case ADC_RESOLUTION_10B: return ADC_MAX_VALUE_10BIT;
    case ADC_RESOLUTION_8B:  return ADC_MAX_VALUE_8BIT;
    case ADC_RESOLUTION_6B:  return ADC_MAX_VALUE_6BIT;
        default: return ADC_MAX_VALUE_12BIT;
    }
}

/**
 * @brief Get channel name string
 * @param channel: ADC channel
 * @retval const char*: Channel name
 */
const char* ADC_GetChannelName(uint32_t channel)
{
    for (uint32_t i = 0; i < sizeof(adc_channel_map) / sizeof(adc_channel_map[0]); i++) {
        if (adc_channel_map[i].channel == channel) {
            return adc_channel_map[i].name;
        }
    }
    return "UNKNOWN";
}

/**
 * @brief Get status string
 * @param status: ADC status
 * @retval const char*: Status string
 */
const char* ADC_GetStatusString(HAL_StatusTypeDef status)
{
    switch (status) {
        case HAL_OK:      return "OK";
        case HAL_ERROR:   return "ERROR";
        case HAL_BUSY:    return "BUSY";
        case HAL_TIMEOUT: return "TIMEOUT";
        default:          return "UNKNOWN";
    }
}

/**
 * @brief Start continuous conversion
 * @param hadc: Pointer to ADC handle structure
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_StartContinuousConversion(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
        return HAL_ERROR;
    }

    if (!hadc->initialized) {
        return HAL_ERROR;
    }

    /* Enable continuous mode */
    hadc->hal_handle.Init.ContinuousConvMode = ENABLE;

    HAL_StatusTypeDef hal_status = HAL_ADC_Start(&hadc->hal_handle);
    return hal_status;
}

/**
 * @brief Stop continuous conversion
 * @param hadc: Pointer to ADC handle structure
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_StopContinuousConversion(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
        return HAL_ERROR;
    }

    if (!hadc->initialized) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef hal_status = HAL_ADC_Stop(&hadc->hal_handle);
    return hal_status;
}

/**
 * @brief Start DMA conversion
 * @param hadc: Pointer to ADC handle structure
 * @param buffer: Buffer to store conversion results
 * @param length: Number of conversions
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_StartDMA(ADC_HandleStruct* hadc, uint32_t* buffer, uint32_t length)
{
    if (hadc == NULL || buffer == NULL || length == 0) {
        return HAL_ERROR;
    }

    if (!hadc->initialized) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef hal_status = HAL_ADC_Start_DMA(&hadc->hal_handle, buffer, length);
    return hal_status;
}

/**
 * @brief Stop DMA conversion
 * @param hadc: Pointer to ADC handle structure
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_StopDMA(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
        return HAL_ERROR;
    }

    if (!hadc->initialized) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef hal_status = HAL_ADC_Stop_DMA(&hadc->hal_handle);
    return hal_status;
}

/**
 * @brief Start ADC conversion in interrupt mode
 */
HAL_StatusTypeDef ADC_Start_IT(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
        return HAL_ERROR;
    }
    if (!hadc->initialized) {
        return HAL_ERROR;
    }

    hadc->conv_complete_flag = false;
    return HAL_ADC_Start_IT(&hadc->hal_handle);
}

/**
 * @brief Stop ADC conversion in interrupt mode
 */
HAL_StatusTypeDef ADC_Stop_IT(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
        return HAL_ERROR;
    }
    if (!hadc->initialized) {
        return HAL_ERROR;
    }

    return HAL_ADC_Stop_IT(&hadc->hal_handle);
}

/**
 * @brief Read a single channel using interrupt mode (configures channel then starts IT)
 */
HAL_StatusTypeDef ADC_ReadChannel_IT(ADC_HandleStruct* hadc, uint32_t channel)
{
    if (hadc == NULL) {
        return HAL_ERROR;
    }
    if (!hadc->initialized) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = ADC_ConfigChannel(hadc, channel, ADC_SAMPLETIME_84CYCLES);
    if (status != HAL_OK) {
        return status;
    }

    hadc->conv_complete_flag = false;
    return HAL_ADC_Start_IT(&hadc->hal_handle);
}

/**
 * @brief Register conversion complete callback
 */
void ADC_RegisterConvCompleteCallback(ADC_HandleStruct* hadc, void (*callback)(ADC_HandleStruct*, uint32_t))
{
    if (hadc == NULL) {
        return;
    }
    hadc->conv_complete_cb = callback;
}

/**
 * @brief Register error callback
 */
void ADC_RegisterErrorCallback(ADC_HandleStruct* hadc, void (*callback)(ADC_HandleStruct*))
{
    if (hadc == NULL) {
        return;
    }
    hadc->error_cb = callback;
}

/**
 * @brief Map HAL ADC handle to our ADC_HandleStruct wrapper
 * @note Currently only maps ADC1/hadc1. Returns NULL if not found.
 */
static ADC_HandleStruct* ADC_MapHALHandle(ADC_HandleTypeDef* hal)
{
    if (hal == &hadc1.hal_handle) {
        return &hadc1;
    }
    return NULL;
}

/* HAL callback implementations to bridge to user callbacks */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    ADC_HandleStruct* wrapper = ADC_MapHALHandle(hadc);
    if (wrapper == NULL) {
        return;
    }

    uint32_t value = HAL_ADC_GetValue(hadc);
    wrapper->last_value = value;
    wrapper->conv_complete_flag = true;

    if (wrapper->conv_complete_cb) {
        wrapper->conv_complete_cb(wrapper, value);
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef* hadc)
{
    ADC_HandleStruct* wrapper = ADC_MapHALHandle(hadc);
    if (wrapper == NULL) {
        return;
    }

    if (wrapper->error_cb) {
        wrapper->error_cb(wrapper);
    }
}

/**
 * @brief Configure multiple channels
 * @param hadc: Pointer to ADC handle structure
 * @param channels: Array of channels to configure
 * @param sampling_times: Array of sampling times for each channel
 * @param num_channels: Number of channels
 * @retval HAL_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_ConfigMultiChannel(ADC_HandleStruct* hadc,
                                        const uint32_t* channels,
                                        const uint32_t* sampling_times,
                                        uint32_t num_channels)
{
    if (hadc == NULL || channels == NULL || sampling_times == NULL || num_channels == 0) {
        return HAL_ERROR;
    }

    if (!hadc->initialized) {
        return HAL_ERROR;
    }

    /* Configure each channel */
    for (uint32_t i = 0; i < num_channels; i++) {
        HAL_StatusTypeDef status = ADC_ConfigChannel(hadc, channels[i], sampling_times[i]);
        if (status != HAL_OK) {
            return status;
        }
    }

    /* Update ADC configuration for multiple channels */
    hadc->hal_handle.Init.NbrOfConversion = num_channels;
    hadc->hal_handle.Init.ScanConvMode = (num_channels > 1) ? ENABLE : DISABLE;

    HAL_StatusTypeDef hal_status = HAL_ADC_Init(&hadc->hal_handle);
    return hal_status;
}

/**
 * @brief Read multiple channels
 * @param hadc: Pointer to ADC handle structure
 * @param values: Buffer to store conversion results
 * @param num_channels: Number of channels to read
 * @retval ADC_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_ReadMultiChannel(ADC_HandleStruct* hadc, uint32_t* values, uint32_t num_channels)
{
    if (hadc == NULL || values == NULL || num_channels == 0) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    /* Start DMA conversion for multiple channels */
    HAL_StatusTypeDef status = ADC_StartDMA(hadc, values, num_channels);
    if (status != HAL_OK) {
        return status;
    }

    /* Wait for conversion completion */
    status = ADC_PollForConversion(hadc, ADC_CONVERSION_TIMEOUT);
    if (status != HAL_OK) {
        ADC_StopDMA(hadc);
        return status;
    }

    return HAL_OK;
}

/**
 * @brief Read internal temperature sensor
 * @param hadc: Pointer to ADC handle structure
 * @retval float: Temperature in Celsius
 */
float ADC_ReadTemperature(ADC_HandleStruct* hadc)
{
    uint32_t temp_raw = 0;
    HAL_StatusTypeDef status = ADC_ReadChannel(hadc, ADC_CHANNEL_TEMPSENSOR, &temp_raw);

    if (status != HAL_OK) {
        return ADC_ABSOLUTE_ZERO;  // Error indication (absolute zero)
    }

    /* Temperature calculation for STM32F429 */
    /* Formula from reference manual: Temp = (VSENSE - V25) / Avg_Slope + 25 */
    /* V25 = 0.76V, Avg_Slope = 2.5mV/°C */
    float voltage = ADC_RawToVoltage(temp_raw, hadc->config.resolution);
    float temperature = ((voltage - ADC_TEMP_V25) / ADC_TEMP_AVG_SLOPE) + ADC_TEMP_25C;

    return temperature;
}

/**
 * @brief Read internal voltage reference
 * @param hadc: Pointer to ADC handle structure
 * @retval float: Internal reference voltage
 */
float ADC_ReadVrefInt(ADC_HandleStruct* hadc)
{
    uint32_t vref_raw = 0;
    HAL_StatusTypeDef status = ADC_ReadChannel(hadc, ADC_CHANNEL_VREFINT, &vref_raw);

    if (status != HAL_OK) {
        return -1.0f;  // Error indication
    }

    /* VREFINT typically 1.2V */
    return ADC_RawToVoltage(vref_raw, hadc->config.resolution);
}

/**
 * @brief Read battery voltage
 * @param hadc: Pointer to ADC handle structure
 * @retval float: Battery voltage
 */
float ADC_ReadVbat(ADC_HandleStruct* hadc)
{
    uint32_t vbat_raw = 0;
    HAL_StatusTypeDef status = ADC_ReadChannel(hadc, ADC_CHANNEL_VBAT, &vbat_raw);

    if (status != HAL_OK) {
        return -1.0f;  // Error indication
    }

    /* VBAT is divided by 2 internally */
    float voltage = ADC_RawToVoltage(vbat_raw, hadc->config.resolution);
    return voltage * ADC_VBAT_DIVIDER;  // Compensate for internal division
}

/**
 * @brief Set ADC resolution
 * @param hadc: Pointer to ADC handle structure
 * @param resolution: New resolution setting
 * @retval ADC_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_SetResolution(ADC_HandleStruct* hadc, uint32_t resolution)
{
    if (hadc == NULL) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    hadc->hal_handle.Init.Resolution = ADC_ResolutionToHAL(resolution);
    hadc->config.resolution = resolution;

    HAL_StatusTypeDef hal_status = HAL_ADC_Init(&hadc->hal_handle);
    return hal_status;
}

/**
 * @brief Set sampling time for a specific channel
 * @param hadc: Pointer to ADC handle structure
 * @param channel: ADC channel
 * @param sampling_time: New sampling time
 * @retval ADC_StatusTypeDef: Status of the operation
 */
HAL_StatusTypeDef ADC_SetSamplingTime(ADC_HandleStruct* hadc, uint32_t channel,
                                     uint32_t sampling_time)
{
    return ADC_ConfigChannel(hadc, channel, sampling_time);
}

/**
 * @brief Get ADC status
 * @param hadc: Pointer to ADC handle structure
 * @retval ADC_StatusTypeDef: Current status
 */
HAL_StatusTypeDef ADC_GetStatus(const ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
    return HAL_ERROR;
    }

    if (!hadc->initialized) {
    return HAL_ERROR;
    }

    if (hadc->hal_handle.State == HAL_ADC_STATE_BUSY) {
    return HAL_BUSY;
    }

    return HAL_OK;
}

/**
 * @brief Check if ADC is ready
 * @param hadc: Pointer to ADC handle structure
 * @retval bool: true if ready, false otherwise
 */
bool ADC_IsReady(const ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
        return false;
    }
    return hadc->initialized && hadc->calibrated;
}

/**
 * @brief Check if conversion is complete
 * @param hadc: Pointer to ADC handle structure
 * @retval bool: true if complete, false otherwise
 */
bool ADC_IsConversionComplete(const ADC_HandleStruct* hadc)
{
    if (hadc == NULL || !hadc->initialized) {
        return false;
    }

    return (hadc->hal_handle.Instance->SR & ADC_FLAG_EOC) != 0;
}

/**
 * @brief ADC error handler
 * @param hadc: Pointer to ADC handle structure
 * @retval None
 */
void ADC_ErrorHandler(ADC_HandleStruct* hadc)
{
    if (hadc != NULL) {
        /* Reset initialization flags */
        hadc->initialized = false;
        hadc->calibrated = false;

        /* You can add custom error handling here */
        /* For example: toggle an LED, send error message, etc. */
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Configure GPIO pins for ADC channel
 * @param channel: ADC channel
 * @retval ADC_StatusTypeDef: Status of the operation
 */
static HAL_StatusTypeDef ADC_ConfigureGPIO(uint32_t channel)
{
    /* Find channel in mapping table */
    for (uint32_t i = 0; i < sizeof(adc_channel_map) / sizeof(adc_channel_map[0]); i++) {
        if (adc_channel_map[i].channel == channel) {
            if (adc_channel_map[i].port != NULL) {
                /* Configure GPIO pin for analog input */
                GPIO_InitTypeDef GPIO_InitStruct = {0};
                GPIO_InitStruct.Pin = adc_channel_map[i].pin;
                GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                HAL_GPIO_Init(adc_channel_map[i].port, &GPIO_InitStruct);

                return HAL_OK;
            }
            /* Internal channels don't need GPIO configuration */
            return HAL_OK;
        }
    }

    return HAL_ERROR;
}

/**
 * @brief Configure ADC clocks
 * @retval ADC_StatusTypeDef: Status of the operation
 */
static HAL_StatusTypeDef ADC_ConfigureClocks(void)
{
    /* Enable ADC1 clock */
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* Enable GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    return HAL_OK;
}

/**
 * @brief Configure DMA for ADC
 * @param hadc: Pointer to ADC handle structure
 * @retval HAL_StatusTypeDef: Status of the operation
 * @note For STM32F429, ADC1 uses DMA2 Stream 0 Channel 0
 */
static HAL_StatusTypeDef ADC_ConfigureDMA(ADC_HandleStruct* hadc)
{
    if (hadc == NULL) {
        return HAL_ERROR;
    }

    /* Enable DMA2 clock (ADC1 uses DMA2) */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* Configure DMA handle */
    hadc->hdma_adc.Instance = DMA2_Stream0;
    hadc->hdma_adc.Init.Channel = DMA_CHANNEL_0;
    hadc->hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hadc->hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    hadc->hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
    hadc->hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hadc->hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hadc->hdma_adc.Init.Mode = DMA_NORMAL;
    hadc->hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
    hadc->hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    /* Initialize DMA */
    HAL_StatusTypeDef hal_status = HAL_DMA_Init(&hadc->hdma_adc);
    if (hal_status != HAL_OK) {
        return hal_status;
    }

    /* Link DMA to ADC */
    __HAL_LINKDMA(&hadc->hal_handle, DMA_Handle, hadc->hdma_adc);

    return HAL_OK;
}

/**
 * @brief Convert custom channel enum to HAL channel
 * @param channel: Custom ADC channel
 * @retval uint32_t: HAL ADC channel
 * @note Since we now use HAL constants directly, this is essentially an identity function
 */
static uint32_t ADC_ChannelToHAL(uint32_t channel)
{
    /* Channel enum values now directly use HAL constants, so just return the value */
    return (uint32_t)channel;
}

/**
 * @brief Convert custom resolution enum to HAL resolution
 * @param resolution: Custom ADC resolution
 * @retval uint32_t: HAL ADC resolution
 */
static uint32_t ADC_ResolutionToHAL(uint32_t resolution)
{
    switch (resolution) {
    case ADC_RESOLUTION_12B: return ADC_RESOLUTION_12B;
    case ADC_RESOLUTION_10B: return ADC_RESOLUTION_10B;
    case ADC_RESOLUTION_8B:  return ADC_RESOLUTION_8B;
    case ADC_RESOLUTION_6B:  return ADC_RESOLUTION_6B;
        default:                   return ADC_RESOLUTION_12B;
    }
}

/**
 * @brief Convert custom sampling time enum to HAL sampling time
 * @param sampling_time: Custom ADC sampling time
 * @retval uint32_t: HAL ADC sampling time
 */
static uint32_t ADC_SamplingTimeToHAL(uint32_t sampling_time)
{
    switch (sampling_time) {
        case ADC_SAMPLETIME_3CYCLES:   return ADC_SAMPLETIME_3CYCLES;
        case ADC_SAMPLETIME_15CYCLES:  return ADC_SAMPLETIME_15CYCLES;
        case ADC_SAMPLETIME_28CYCLES:  return ADC_SAMPLETIME_28CYCLES;
        case ADC_SAMPLETIME_56CYCLES:  return ADC_SAMPLETIME_56CYCLES;
        case ADC_SAMPLETIME_84CYCLES:  return ADC_SAMPLETIME_84CYCLES;
        case ADC_SAMPLETIME_112CYCLES: return ADC_SAMPLETIME_112CYCLES;
        case ADC_SAMPLETIME_144CYCLES: return ADC_SAMPLETIME_144CYCLES;
        case ADC_SAMPLETIME_480CYCLES: return ADC_SAMPLETIME_480CYCLES;
        default:                       return ADC_SAMPLETIME_84CYCLES;
    }
}

void ADC_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&hadc1.hal_handle);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
