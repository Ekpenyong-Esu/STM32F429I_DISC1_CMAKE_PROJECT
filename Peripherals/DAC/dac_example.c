/**
 ******************************************************************************
 * @file    dac_example.c
 * @author  Mahonri
 * @brief   Example code for STM32F429I Discovery DAC peripheral driver
 * @date    2025
 *
 * @details This file contains comprehensive examples demonstrating the usage
 *          of the DAC peripheral driver. The examples show:
 *          - Basic static voltage output
 *          - Sine wave generation
 *          - Triangle wave generation
 *
 *          Each example includes proper initialization, operation, and cleanup.
 *
 * @note    These examples are designed for STM32F429I Discovery board
 * @note    Connect an oscilloscope to PA4 (DAC_OUT1) to observe the outputs
 *
 * @section example_usage Usage Instructions
 * 1. Include this file in your project
 * 2. Call DAC_Example_RunAll() from your main function
 * 3. Monitor the output on PA4 pin
 * 4. Use printf output for debugging (requires UART setup)
 *
 * @section example_hardware Hardware Requirements
 * - STM32F429I Discovery board
 * - Oscilloscope or multimeter (recommended for verification)
 * - Optional: UART connection for printf output
 *
 * @attention
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * @section example_output Expected Output
 * - Basic Output: 0V, ~1.65V, 3.3V static voltages
 * - Sine Wave: 1kHz sine wave with 1.65V offset
 * - Triangle Wave: Triangle wave with same frequency
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "dac.h"                    /**< DAC driver header */
#include "stm32f4xx_hal.h"          /**< STM32 HAL header for delay functions */
#include <stdio.h>                  /**< Standard I/O for printf */
#include <math.h>                   /**< Math library for sine function */

#define _USE_MATH_DEFINES           /**< Enable math constants like M_PI */

/* Private defines -----------------------------------------------------------*/

/**
 * @brief DAC channel configuration for examples
 * @note PA4 is the DAC output pin on STM32F429I Discovery board
 */
#define DAC_CHANNEL_TO_USE      DAC_CHANNEL_1

/**
 * @brief Number of points in waveform generation
 * @note Higher values = smoother waveforms but slower generation
 */
#define DAC_WAVEFORM_POINTS     100

/**
 * @brief Delay between waveform points in milliseconds
 * @note Affects the frequency of generated waveforms
 */
#define DAC_OUTPUT_DELAY_MS     10

/**
 * @brief Duration to run each example in milliseconds
 * @note Allows time to observe each example on oscilloscope
 */
#define DAC_EXAMPLE_DURATION_MS 2000

/**
 * @brief Maximum 12-bit DAC value
 * @note 2^12 - 1 = 4095
 */
#define DAC_MAX_VALUE           4095U

/**
 * @brief Delay for holding static voltages in milliseconds
 */
#define DAC_DELAY_500MS         500

/**
 * @brief Mid-scale DAC value (1/4 of full scale)
 */
#define DAC_MID_SCALE_VALUE     1024

/**
 * @brief 3/4 scale DAC value
 */
#define DAC_THREE_QUARTER_VALUE 3072

/**
 * @brief Scale factor for sine wave generation (2 * π for full cycle)
 */
#define DAC_SINE_SCALE_FACTOR   2.0f

/**
 * @brief Offset for sine wave (0.5 to center the wave)
 */
#define DAC_SINE_OFFSET         0.5f

/**
 * @brief Half of waveform points as float (prevents integer division issues)
 */
#define DAC_WAVEFORM_HALF       (DAC_WAVEFORM_POINTS / 2.0f)

/* Private variables ---------------------------------------------------------*/

/**
 * @brief Global DAC handle for example usage
 * @note Made global to be accessible by all example functions
 */
DAC_HandleStruct hdac_example;

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Output static voltages on DAC
 * @details Demonstrates basic DAC operation with fixed voltage levels
 */
void DAC_Example_BasicOutput(void);

/**
 * @brief Output a sine wave on DAC
 * @details Generates a sine wave using software calculation
 */
void DAC_Example_SineWave(void);

/**
 * @brief Output a triangle wave on DAC
 * @details Generates a triangle wave using linear interpolation
 */
void DAC_Example_TriangleWave(void);

/**
 * @brief Advanced DAC control with separate Start/Stop
 * @details Demonstrates fine-grained control over DAC conversion timing
 */
void DAC_Example_AdvancedControl(void);

/**
 * @brief Run all DAC examples in sequence
 * @details Main entry point for running all DAC examples
 */
void DAC_Example_RunAll(void);

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Main entry point for DAC examples
 *
 * This function runs all DAC examples in sequence with delays between them.
 * Each example runs for DAC_EXAMPLE_DURATION_MS milliseconds.
 *
 * Call this function from main() to demonstrate all DAC capabilities:
 * @code
 * int main(void) {
 *     // Initialize system...
 *     DAC_Example_RunAll();
 *     while(1);
 * }
 * @endcode
 */
void DAC_Example_RunAll(void)
{
    printf("=== DAC Driver Examples ===\n\r");
    printf("Connect oscilloscope to PA4 (DAC_OUT1) to observe outputs\n\r\n");

    /* Run basic output example */
    DAC_Example_BasicOutput();
    printf("Waiting before next example...\n\r");
    HAL_Delay(DAC_EXAMPLE_DURATION_MS);

    /* Run sine wave example */
    DAC_Example_SineWave();
    printf("Waiting before next example...\n\r");
    HAL_Delay(DAC_EXAMPLE_DURATION_MS);

    /* Run triangle wave example */
    DAC_Example_TriangleWave();
    printf("Waiting before next example...\n\r");
    HAL_Delay(DAC_EXAMPLE_DURATION_MS);

    /* Run advanced control example */
    DAC_Example_AdvancedControl();
    printf("Waiting before next example...\n\r");
    HAL_Delay(DAC_EXAMPLE_DURATION_MS);

    printf("=== All DAC Examples Completed ===\n\r");
}

/**
 * @brief Example 1: Output static voltages on DAC
 *
 * This example demonstrates the most basic DAC usage:
 * 1. Initializes DAC with no trigger and output buffer enabled
 * 2. Outputs three different voltage levels (0V, mid-scale, max)
 * 3. Holds each voltage for 500ms for observation
 * 4. Deinitializes the DAC
 *
 * Expected output voltages:
 * - 0V (raw value 0)
 * - ~1.65V (raw value 2048, mid-scale)
 * - 3.3V (raw value 4095, max)
 *
 * @note Use a multimeter or oscilloscope to verify output voltages
 * @note The output buffer helps maintain stable voltage levels
 */
void DAC_Example_BasicOutput(void)
{
    printf("\n--- Basic DAC Output Example ---\n\r");
    printf("Outputting: 0V -> Mid-scale -> Max voltage\n\r");

    /* Configure DAC for basic operation */
    DAC_ConfigTypeDef dac_config = {
        .channel = DAC_CHANNEL_TO_USE,
        .trigger = DAC_TRIGGER_NONE,           /* No external trigger */
        .output_buffer = DAC_OUTPUTBUFFER_ENABLE  /* Enable for stable output */
    };

    /* Initialize DAC */
    if (DAC_Init(&hdac_example, &dac_config) != HAL_OK) {
        printf("DAC Initialization failed\n\r");
        return;
    }

    /* Define test voltages: 0V, mid-scale, max voltage */
    uint32_t values[] = {0, DAC_MAX_VALUE / 2, DAC_MAX_VALUE};
    const char* labels[] = {"0V", "Mid-scale (~1.65V)", "Max (3.3V)"};

    /* Output each voltage level */
    for (int i = 0; i < 3; i++) {
        /* Set DAC output and start conversion */
        if (DAC_SetValue(&hdac_example, DAC_CHANNEL_TO_USE, values[i]) == HAL_OK) {
            printf("DAC Output: %s (Raw: %lu)\n\r", labels[i], (unsigned long)values[i]);
        } else {
            printf("DAC SetValue failed for %s\n\r", labels[i]);
        }

        /* Hold voltage for observation */
        HAL_Delay(DAC_DELAY_500MS);
    }

    /* Cleanup */
    DAC_DeInit(&hdac_example);
    printf("Basic DAC output example completed.\n\r");
}

/**
 * @brief Example 2: Output a sine wave on the DAC
 *
 * This example generates a sine wave using software calculation:
 * 1. Calculates sine values for one full cycle (2π radians)
 * 2. Scales the sine wave from [-1, 1] to [0, 1] range
 * 3. Converts to DAC range [0, 4095]
 * 4. Outputs each point with a delay to control frequency
 *
 * Waveform characteristics:
 * - Frequency: ~1kHz (100 points * 10ms delay = 1 second per cycle)
 * - Amplitude: Full-scale (0V to 3.3V)
 * - Offset: Mid-scale (1.65V center)
 *
 * @note The waveform frequency depends on DAC_WAVEFORM_POINTS and DAC_OUTPUT_DELAY_MS
 * @note For higher frequencies, reduce the delay or use DMA
 */
void DAC_Example_SineWave(void)
{
    printf("\n--- Sine Wave Output Example ---\n\r");
    printf("Generating sine wave: 100 points, ~1kHz frequency\n\r");

    /* Configure DAC for waveform generation */
    DAC_ConfigTypeDef dac_config = {
        .channel = DAC_CHANNEL_TO_USE,
        .trigger = DAC_TRIGGER_NONE,
        .output_buffer = DAC_OUTPUTBUFFER_ENABLE
    };

    /* Initialize DAC */
    if (DAC_Init(&hdac_example, &dac_config) != HAL_OK) {
        printf("DAC Initialization failed\n\r");
        return;
    }

    /* Generate and output sine wave */
    for (int idx = 0; idx < DAC_WAVEFORM_POINTS; idx++) {
        /* Calculate angle for current point (0 to 2π) */
        float angle = DAC_SINE_SCALE_FACTOR * (float)M_PI * (float)idx / (float)DAC_WAVEFORM_POINTS;

        /* Generate sine value and scale to [0, 1] range, then to DAC range */
        /* sin(angle) ranges from [-1, 1], we add 0.5 to get [0, 1] */
        uint32_t value = (uint32_t)((sinf(angle) * DAC_SINE_OFFSET + DAC_SINE_OFFSET) * DAC_MAX_VALUE);

        /* Output the calculated value */
        DAC_SetValue(&hdac_example, DAC_CHANNEL_TO_USE, value);

        /* Delay between points (controls waveform frequency) */
        HAL_Delay(DAC_OUTPUT_DELAY_MS);
    }

    /* Cleanup */
    DAC_DeInit(&hdac_example);
    printf("Sine wave output example completed.\n\r");
}

/**
 * @brief Example 3: Output a triangle wave on the DAC
 *
 * This example generates a triangle wave using linear interpolation:
 * 1. First half: Rising edge from 0 to max value
 * 2. Second half: Falling edge from max to 0 value
 * 3. Linear interpolation between points
 *
 * Waveform characteristics:
 * - Frequency: Same as sine wave (~1kHz)
 * - Amplitude: Full-scale (0V to 3.3V)
 * - Shape: Linear rising/falling edges
 *
 * @note Triangle waves have odd harmonics, useful for testing
 * @note The linear nature makes it good for frequency response testing
 */
void DAC_Example_TriangleWave(void)
{
    printf("\n--- Triangle Wave Output Example ---\n\r");
    printf("Generating triangle wave: linear rising/falling edges\n\r");

    /* Configure DAC for waveform generation */
    DAC_ConfigTypeDef dac_config = {
        .channel = DAC_CHANNEL_TO_USE,
        .trigger = DAC_TRIGGER_NONE,
        .output_buffer = DAC_OUTPUTBUFFER_ENABLE
    };

    /* Initialize DAC */
    if (DAC_Init(&hdac_example, &dac_config) != HAL_OK) {
        printf("DAC Initialization failed\n\r");
        return;
    }

    /* Generate and output triangle wave */
    for (int idx = 0; idx < DAC_WAVEFORM_POINTS; idx++) {
        uint32_t value = 0;

        if (idx < DAC_WAVEFORM_POINTS / 2) {
            /* Rising edge: scale from 0 to max using linear interpolation */
            value = (uint32_t)((float)idx / DAC_WAVEFORM_HALF * DAC_MAX_VALUE);
        } else {
            /* Falling edge: scale from max to 0 using linear interpolation */
            value = (uint32_t)(((float)(DAC_WAVEFORM_POINTS - idx) / DAC_WAVEFORM_HALF) * DAC_MAX_VALUE);
        }

        /* Output the calculated value */
        DAC_SetValue(&hdac_example, DAC_CHANNEL_TO_USE, value);

        /* Delay between points */
        HAL_Delay(DAC_OUTPUT_DELAY_MS);
    }

    /* Cleanup */
    DAC_DeInit(&hdac_example);
    printf("Triangle wave output example completed.\n\r");
}

/**
 * @brief Example 4: Advanced DAC control with separate Start/Stop
 *
 * This example demonstrates the use of DAC_Start and DAC_Stop functions
 * for fine-grained control over DAC conversion timing. This is useful for:
 * - Synchronizing DAC output with other peripherals
 * - Implementing custom timing sequences
 * - Advanced waveform generation with precise timing control
 *
 * The example shows how to set a value first, then start conversion later,
 * and demonstrates the difference between DAC_SetValue (combined) vs
 * separate DAC_SetValue + DAC_Start operations.
 */
void DAC_Example_AdvancedControl(void)
{
    printf("\n--- Advanced DAC Control Example ---\n\r");
    printf("Demonstrating separate DAC_Start/DAC_Stop control\n\r");

    /* Configure DAC for advanced control */
    DAC_ConfigTypeDef dac_config = {
        .channel = DAC_CHANNEL_TO_USE,
        .trigger = DAC_TRIGGER_NONE,
        .output_buffer = DAC_OUTPUTBUFFER_ENABLE
    };

    /* Initialize DAC */
    if (DAC_Init(&hdac_example, &dac_config) != HAL_OK) {
        printf("DAC Initialization failed\n\r");
        return;
    }

    printf("Method 1: Using DAC_SetValue (combined set + start)\n\r");

    /* Method 1: Combined operation */
    if (DAC_SetValue(&hdac_example, DAC_CHANNEL_TO_USE, DAC_MID_SCALE_VALUE) == HAL_OK) {
        printf("DAC output set to mid-scale (1.25V) using combined method\n\r");
        HAL_Delay(DAC_DELAY_500MS);
    }

    /* Stop the DAC */
    DAC_Stop(&hdac_example, DAC_CHANNEL_TO_USE);
    printf("DAC conversion stopped\n\r");
    HAL_Delay(DAC_DELAY_500MS);

    printf("\nMethod 2: Using separate DAC_SetValue + DAC_Start\n\r");

    /* Method 2: Separate operations for fine control */
    /* First, set the value without starting conversion */
    if (HAL_DAC_SetValue(&hdac_example.hal_handle, DAC_CHANNEL_TO_USE, DAC_ALIGN_12B_R, DAC_THREE_QUARTER_VALUE) == HAL_OK) {
        printf("DAC value set to 3/4 scale (2.25V) - conversion not started yet\n\r");
        HAL_Delay(DAC_DELAY_500MS); // Value is set but not converting

        /* Now start the conversion */
        if (DAC_Start(&hdac_example, DAC_CHANNEL_TO_USE) == HAL_OK) {
            printf("DAC conversion started - output now active\n\r");
            HAL_Delay(DAC_DELAY_500MS);
        }
    }

    /* Demonstrate stopping */
    DAC_Stop(&hdac_example, DAC_CHANNEL_TO_USE);
    printf("DAC conversion stopped again\n\r");

    /* Show how to read current value */
    uint32_t current_value = DAC_GetValue(&hdac_example, DAC_CHANNEL_TO_USE);
    printf("Current DAC register value: %u\n\r", current_value);

    /* Cleanup */
    DAC_DeInit(&hdac_example);
    printf("Advanced DAC control example completed.\n\r");
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
