/**
 ******************************************************************************
 * @file    adc_example.c
 * @author  Mahonri
 * @brief   ADC peripheral driver example code for STM32F429I Discovery board
 *          This file demonstrates typical usage of the ADC driver
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
#include "stm32f4xx_hal.h"
#include <stdio.h>

/* Private defines -----------------------------------------------------------*/
#define ADC_CHANNEL_TO_READ    ADC_CHANNEL_0  /* PA0 - User button on Discovery board */
#define ADC_POLLING_DELAY_MS   1000           /* 1 second delay between readings */
#define EXAMPLE_DELAY_MS       2000           /* 2 second delay between examples */
#define CONTINUOUS_DURATION_MS 5000           /* 5 seconds for continuous example */
#define CONTINUOUS_PRINT_DELAY_MS 500         /* Print every 500ms in continuous mode */
#define VALID_TEMP_MIN         -200.0f        /* Minimum valid temperature */
/* Timeout for interrupt example (ms) */
#define ADC_IT_TIMEOUT_MS      1000U

/* Private variables ---------------------------------------------------------*/
ADC_HandleStruct hadc_example;  /* ADC handle for example */

/* Private function prototypes -----------------------------------------------*/
void ADC_Example_BasicReading(void);
void ADC_Example_VoltageReading(void);
void ADC_Example_MultiChannelReading(void);
void ADC_Example_InternalSensors(void);
void ADC_Example_ContinuousConversion(void);
void ADC_Example_InterruptMode(void);

/* Minimal state for interrupt example */
static volatile bool adc_it_ready = false;

static void ADC_IT_ConvCompleteCb(ADC_HandleStruct* hadc, uint32_t value)
{
    (void)value;
    if (hadc != NULL) {
        hadc->last_value = HAL_ADC_GetValue(&hadc->hal_handle);
    }
    adc_it_ready = true;
}

static void ADC_IT_ErrorCb(ADC_HandleStruct* hadc)
{
    (void)hadc;
    adc_it_ready = true;
}

/**
 * @brief Main ADC example function
 * Call this function from main() to run ADC examples
 */
void ADC_Example_RunAll(void)
{
    printf("=== ADC Driver Examples ===\n\r");

    /* Run different ADC examples */
    ADC_Example_BasicReading();
    HAL_Delay(EXAMPLE_DELAY_MS);

    ADC_Example_VoltageReading();
    HAL_Delay(EXAMPLE_DELAY_MS);

    ADC_Example_MultiChannelReading();
    HAL_Delay(EXAMPLE_DELAY_MS);

    ADC_Example_InternalSensors();
    HAL_Delay(EXAMPLE_DELAY_MS);

    ADC_Example_ContinuousConversion();

    printf("=== All ADC Examples Completed ===\n\r");
}

/**
 * @brief Example 1: Basic ADC reading (raw values)
 * Demonstrates the most basic ADC usage - reading raw ADC values
 */
void ADC_Example_BasicReading(void)
{
    printf("\n--- Basic ADC Reading Example ---\n\r");

    /* ADC Configuration for basic reading */
    ADC_ConfigTypeDef adc_config = {
        .channel = ADC_CHANNEL_TO_READ,
        .resolution = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_84CYCLES,
        .conv_mode = ADC_MODE_SINGLE,
        .dma_enabled = false
    };

    /* Initialize ADC */
    HAL_StatusTypeDef status = ADC_Init(&hadc_example, &adc_config);
    if (status != HAL_OK) {
        printf("ADC Initialization failed: %s\n\r", ADC_GetStatusString(status));
        return;
    }

    /* Calibrate ADC (optional for STM32F4) */
    status = ADC_Calibrate(&hadc_example);
    if (status != HAL_OK) {
        printf("ADC Calibration failed: %s\n\r", ADC_GetStatusString(status));
    }

    printf("Reading ADC channel %s (%d) for 5 seconds...\n\r",
           ADC_GetChannelName(ADC_CHANNEL_TO_READ), ADC_CHANNEL_TO_READ);

    /* Read ADC values for 5 seconds */
    for (uint32_t i = 0; i < 5; i++) {
        uint32_t adc_value = 0;

        /* Read ADC channel */
        status = ADC_ReadChannel(&hadc_example, ADC_CHANNEL_TO_READ, &adc_value);
        if (status == HAL_OK) {
            printf("ADC Raw Value: %u (0x%03X)\n\r", adc_value, adc_value);
        } else {
            printf("ADC Read failed: %s\n\r", ADC_GetStatusString(status));
        }

        HAL_Delay(ADC_POLLING_DELAY_MS);
    }

    /* Deinitialize ADC */
    ADC_DeInit(&hadc_example);
    printf("Basic ADC reading example completed.\n\r");
}

/**
 * @brief Example 2: Voltage reading
 * Demonstrates reading ADC values as voltages
 */
void ADC_Example_VoltageReading(void)
{
    printf("\n--- Voltage Reading Example ---\n\r");

    /* ADC Configuration */
    ADC_ConfigTypeDef adc_config = {
        .channel = ADC_CHANNEL_TO_READ,
        .resolution = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_84CYCLES,
        .conv_mode = ADC_MODE_SINGLE,
        .dma_enabled = false
    };

    /* Initialize ADC */
    if (ADC_Init(&hadc_example, &adc_config) != HAL_OK) {
        printf("ADC Initialization failed\n\r");
        return;
    }

    printf("Reading ADC voltage from channel %s for 5 seconds...\n\r",
           ADC_GetChannelName(ADC_CHANNEL_TO_READ));

    /* Read voltage values for 5 seconds */
    for (uint32_t i = 0; i < 5; i++) {
        /* Read voltage directly */
        float voltage = ADC_ReadChannelVoltage(&hadc_example, ADC_CHANNEL_TO_READ);

        if (voltage >= 0.0f) {
            printf("ADC Voltage: %.3f V\n\r", voltage);
        } else {
            printf("ADC Voltage read failed\n\r");
        }

        HAL_Delay(ADC_POLLING_DELAY_MS);
    }

    /* Alternative: Manual conversion using raw value */
    uint32_t raw_value = 0;
    if (ADC_ReadChannel(&hadc_example, ADC_CHANNEL_TO_READ, &raw_value) == HAL_OK) {
        float manual_voltage = ADC_RawToVoltage(raw_value, ADC_RESOLUTION_12B);
        printf("Manual conversion: Raw %u -> %.3f V\n\r", raw_value, manual_voltage);
    }

    ADC_DeInit(&hadc_example);
    printf("Voltage reading example completed.\n\r");
}

/**
 * @brief Example 3: Multi-channel reading
 * Demonstrates reading from multiple ADC channels
 */
void ADC_Example_MultiChannelReading(void)
{
    printf("\n--- Multi-Channel Reading Example ---\n\r");

    /* Define channels to read */
    const uint32_t channels[] = {
        ADC_CHANNEL_0,   /* PA0 */
        ADC_CHANNEL_1,   /* PA1 */
        ADC_CHANNEL_4,   /* PA4 */
        ADC_CHANNEL_5    /* PA5 */
    };

    const uint32_t sampling_times[] = {
        ADC_SAMPLETIME_84CYCLES,
        ADC_SAMPLETIME_84CYCLES,
        ADC_SAMPLETIME_84CYCLES,
        ADC_SAMPLETIME_84CYCLES
    };

    const uint32_t num_channels = sizeof(channels) / sizeof(channels[0]);

    /* ADC Configuration for multi-channel */
    ADC_ConfigTypeDef adc_config = {
        .channel = ADC_CHANNEL_0,  /* Primary channel */
        .resolution = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_84CYCLES,
        .conv_mode = ADC_MODE_SINGLE,
        .dma_enabled = true  /* Enable DMA for multi-channel */
    };

    /* Initialize ADC */
    if (ADC_Init(&hadc_example, &adc_config) != HAL_OK) {
        printf("ADC Initialization failed\n\r");
        return;
    }

    /* Configure multi-channel */
    if (ADC_ConfigMultiChannel(&hadc_example, channels, sampling_times, num_channels) != HAL_OK) {
        printf("Multi-channel configuration failed\n\r");
        ADC_DeInit(&hadc_example);
        return;
    }

    printf("Reading %u ADC channels for 3 seconds...\n\r", num_channels);

    /* Buffer for ADC values */
    uint32_t adc_values[num_channels];

    /* Read multi-channel values for 3 seconds */
    for (uint32_t i = 0; i < 3; i++) {
        if (ADC_ReadMultiChannel(&hadc_example, adc_values, num_channels) == HAL_OK) {
            for (uint32_t ch = 0; ch < num_channels; ch++) {
                float voltage = ADC_RawToVoltage(adc_values[ch], ADC_RESOLUTION_12B);
                printf("Ch%u (%s): %u (%.3f V)  ",
                       ch, ADC_GetChannelName(channels[ch]), adc_values[ch], voltage);
            }
            printf("\n\r");
        } else {
            printf("Multi-channel read failed\n\r");
        }

        HAL_Delay(ADC_POLLING_DELAY_MS);
    }

    ADC_DeInit(&hadc_example);
    printf("Multi-channel reading example completed.\n\r");
}

/**
 * @brief Example 4: Internal sensors reading
 * Demonstrates reading internal temperature sensor, Vref, and Vbat
 */
void ADC_Example_InternalSensors(void)
{
    printf("\n--- Internal Sensors Reading Example ---\n\r");

    /* ADC Configuration for internal sensors */
    ADC_ConfigTypeDef adc_config = {
        .channel = ADC_CHANNEL_TEMPSENSOR,  /* Start with temperature */
        .resolution = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_84CYCLES,
        .conv_mode = ADC_MODE_SINGLE,
        .dma_enabled = false
    };

    /* Initialize ADC */
    if (ADC_Init(&hadc_example, &adc_config) != HAL_OK) {
        printf("ADC Initialization failed\n\r");
        return;
    }

    printf("Reading internal sensors...\n\r");

    /* Read temperature */
    float temperature = ADC_ReadTemperature(&hadc_example);
    if (temperature > VALID_TEMP_MIN) {  /* Valid temperature range check */
        printf("Internal Temperature: %.2f °C\n\r", temperature);
    } else {
        printf("Temperature reading failed\n\r");
    }

    /* Read internal reference voltage */
    float vref = ADC_ReadVrefInt(&hadc_example);
    if (vref > 0.0f) {
        printf("Internal Vref: %.3f V\n\r", vref);
    } else {
        printf("Vref reading failed\n\r");
    }

    /* Read battery voltage (if available) */
    float vbat = ADC_ReadVbat(&hadc_example);
    if (vbat > 0.0f) {
        printf("Battery Voltage: %.3f V\n\r", vbat);
    } else {
        printf("VBAT reading failed (may not be available)\n\r");
    }

    ADC_DeInit(&hadc_example);
    printf("Internal sensors reading example completed.\n\r");
}

/**
 * @brief Example 5: Continuous conversion
 * Demonstrates continuous ADC conversion mode
 */
void ADC_Example_ContinuousConversion(void)
{
    printf("\n--- Continuous Conversion Example ---\n\r");

    /* ADC Configuration for continuous mode */
    ADC_ConfigTypeDef adc_config = {
        .channel = ADC_CHANNEL_TO_READ,
        .resolution = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_84CYCLES,
        .conv_mode = ADC_MODE_CONTINUOUS,
        .dma_enabled = false
    };

    /* Initialize ADC */
    if (ADC_Init(&hadc_example, &adc_config) != HAL_OK) {
        printf("ADC Initialization failed\n\r");
        return;
    }

    printf("Starting continuous ADC conversion for 5 seconds...\n\r");

    /* Start continuous conversion */
    if (ADC_StartContinuousConversion(&hadc_example) != HAL_OK) {
        printf("Failed to start continuous conversion\n\r");
        ADC_DeInit(&hadc_example);
        return;
    }

    /* Read continuous values for 5 seconds */
    uint32_t start_time = HAL_GetTick();
    while ((HAL_GetTick() - start_time) < CONTINUOUS_DURATION_MS) {  /* 5 seconds */
        if (ADC_IsConversionComplete(&hadc_example)) {
            uint32_t adc_value = 0;
            if (ADC_GetValue(&hadc_example, &adc_value) == HAL_OK) {
                float voltage = ADC_RawToVoltage(adc_value, ADC_RESOLUTION_12B);
                printf("Continuous: %u (%.3f V)\n\r", adc_value, voltage);
            }
        }
        HAL_Delay(CONTINUOUS_PRINT_DELAY_MS);  /* Print every 500ms */
    }

    /* Stop continuous conversion */
    ADC_StopContinuousConversion(&hadc_example);
    ADC_DeInit(&hadc_example);

    printf("Continuous conversion example completed.\n\r");
}

/**
 * @brief Example 6: Interrupt-driven single conversion
 * Demonstrates using ADC in interrupt mode with callbacks
 */
void ADC_Example_InterruptMode(void)
{
    printf("\n--- Interrupt-driven ADC Example ---\n\r");

    ADC_ConfigTypeDef adc_config = {
        .channel = ADC_CHANNEL_TO_READ,
        .resolution = ADC_RESOLUTION_12B,
        .sampling_time = ADC_SAMPLETIME_84CYCLES,
        .conv_mode = ADC_MODE_SINGLE,
        .dma_enabled = false
    };

    if (ADC_Init(&hadc_example, &adc_config) != HAL_OK) {
        printf("ADC init failed for IT example\n\r");
        return;
    }

    ADC_RegisterConvCompleteCallback(&hadc_example, ADC_IT_ConvCompleteCb);
    ADC_RegisterErrorCallback(&hadc_example, ADC_IT_ErrorCb);

    adc_it_ready = false;
    if (ADC_ReadChannel_IT(&hadc_example, ADC_CHANNEL_TO_READ) != HAL_OK) {
        printf("Failed to start ADC IT conversion\n\r");
        ADC_DeInit(&hadc_example);
        return;
    }

    /* Wait for conversion or timeout */
    uint32_t timeout = ADC_IT_TIMEOUT_MS;
    while ((!adc_it_ready) && timeout--) {
        HAL_Delay(1);
    }

    if (!adc_it_ready) {
        printf("ADC IT conversion timed out\n\r");
        ADC_DeInit(&hadc_example);
        return;
    }

    uint32_t raw = hadc_example.last_value;
    float voltage = ADC_RawToVoltage(raw, hadc_example.config.resolution);
    printf("ADC IT result: raw=%lu, voltage=%.3f V\n\r", (unsigned long)raw, voltage);

    ADC_DeInit(&hadc_example);
    printf("Interrupt-driven ADC example completed.\n\r");
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
