/**
 ******************************************************************************
 * @file    adc_example.h
 * @author  Mahonri
 * @brief   ADC peripheral driver example header for STM32F429I Discovery board
 *          This file provides function prototypes for ADC examples
 ******************************************************************************
 * @attention
 *
 * This software is provided as-is, without any express or implied warranties.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 ******************************************************************************
 */

#ifndef __ADC_EXAMPLE_H
#define __ADC_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* Exported functions ------------------------------------------------------- */

/**
 * @brief Main ADC example function
 * Call this function from main() to run all ADC examples
 */
void ADC_Example_RunAll(void);

/**
 * @brief Example 1: Basic ADC reading (raw values)
 * Demonstrates the most basic ADC usage - reading raw ADC values
 */
void ADC_Example_BasicReading(void);

/**
 * @brief Example 2: Voltage reading
 * Demonstrates reading ADC values as voltages
 */
void ADC_Example_VoltageReading(void);

/**
 * @brief Example 3: Multi-channel reading
 * Demonstrates reading from multiple ADC channels
 */
void ADC_Example_MultiChannelReading(void);

/**
 * @brief Example 4: Internal sensors reading
 * Demonstrates reading internal temperature sensor, Vref, and Vbat
 */
void ADC_Example_InternalSensors(void);

/**
 * @brief Example 5: Continuous conversion
 * Demonstrates continuous ADC conversion mode
 */
void ADC_Example_ContinuousConversion(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_EXAMPLE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
