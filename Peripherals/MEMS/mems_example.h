/**
  ******************************************************************************
  * @file    mems_example.h
  * @brief   MEMS sensor examples header for STM32F429 Discovery Board
  * @details This file contains function prototypes for MEMS sensor examples
  *          demonstrating various use cases and capabilities.
  * @version 1.0
  * @date    2025-09-04
  ******************************************************************************
  */

#ifndef __MEMS_EXAMPLE_H__
#define __MEMS_EXAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "mems.h"

/* Exported constants --------------------------------------------------------*/
#define MEMS_EXAMPLE_BUFFER_SIZE        100
#define MEMS_EXAMPLE_CALIBRATION_TIME   5000  /* 5 seconds */
#define MEMS_EXAMPLE_SAMPLING_RATE      100   /* Hz */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief MEMS example result structure
 */
typedef struct {
    bool Success;                       /**< Example execution success */
    uint32_t ExecutionTime;            /**< Execution time in milliseconds */
    float AverageValue;                /**< Average measured value */
    float MaxValue;                    /**< Maximum measured value */
    float MinValue;                    /**< Minimum measured value */
    const char *Description;           /**< Example description */
} MEMS_ExampleResultTypeDef;

/**
 * @brief Motion detection result
 */
typedef struct {
    bool MotionDetected;               /**< Motion detection flag */
    float MotionIntensity;             /**< Motion intensity level */
    MEMS_AxesTypeDef MotionVector;     /**< Motion vector */
    uint32_t MotionDuration;           /**< Motion duration in ms */
} MEMS_MotionResultTypeDef;

/* Exported function prototypes ---------------------------------------------*/

/**
 * @brief Basic MEMS sensor example
 * @param hmems Pointer to MEMS handle structure
 * @param hspi Pointer to SPI handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_Basic(MEMS_HandleTypeDef *hmems, SPI_HandleTypeDef *hspi);

/**
 * @brief Gyroscope data reading example
 * @param hmems Pointer to MEMS handle structure
 * @param duration_ms Duration of data collection in milliseconds
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_GyroReading(MEMS_HandleTypeDef *hmems, uint32_t duration_ms);

/**
 * @brief Gyroscope calibration example
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_GyroCalibration(MEMS_HandleTypeDef *hmems);

/**
 * @brief Temperature monitoring example
 * @param hmems Pointer to MEMS handle structure
 * @param sample_count Number of temperature samples to collect
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_TemperatureMonitoring(MEMS_HandleTypeDef *hmems, uint16_t sample_count);

/**
 * @brief Motion detection example
 * @param hmems Pointer to MEMS handle structure
 * @param threshold Motion detection threshold in dps
 * @param duration_ms Monitoring duration in milliseconds
 * @retval MEMS_MotionResultTypeDef Motion detection result
 */
MEMS_MotionResultTypeDef MEMS_Example_MotionDetection(MEMS_HandleTypeDef *hmems, float threshold, uint32_t duration_ms);

/**
 * @brief Self-test example
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_SelfTest(MEMS_HandleTypeDef *hmems);

/**
 * @brief Power management example
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_PowerManagement(MEMS_HandleTypeDef *hmems);

/**
 * @brief Interrupt configuration example
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_InterruptConfig(MEMS_HandleTypeDef *hmems);

/**
 * @brief Data logging example
 * @param hmems Pointer to MEMS handle structure
 * @param log_duration_ms Duration of data logging in milliseconds
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_DataLogging(MEMS_HandleTypeDef *hmems, uint32_t log_duration_ms);

/**
 * @brief Performance test example
 * @param hmems Pointer to MEMS handle structure
 * @param test_iterations Number of test iterations
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_PerformanceTest(MEMS_HandleTypeDef *hmems, uint32_t test_iterations);

/**
 * @brief Full scale range comparison example
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_FullScaleComparison(MEMS_HandleTypeDef *hmems);

/**
 * @brief Error handling demonstration
 * @param hmems Pointer to MEMS handle structure
 * @retval MEMS_ExampleResultTypeDef Example execution result
 */
MEMS_ExampleResultTypeDef MEMS_Example_ErrorHandling(MEMS_HandleTypeDef *hmems);

/* Helper function prototypes -----------------------------------------------*/

/**
 * @brief Print MEMS data to console
 * @param axes Pointer to axes data structure
 * @param label Data label string
 */
void MEMS_Example_PrintData(const MEMS_AxesTypeDef *axes, const char *label);

/**
 * @brief Print example result
 * @param result Pointer to example result structure
 */
void MEMS_Example_PrintResult(const MEMS_ExampleResultTypeDef *result);

/**
 * @brief Calculate statistics from data array
 * @param data Pointer to data array
 * @param length Array length
 * @param average Pointer to store average value
 * @param min_val Pointer to store minimum value
 * @param max_val Pointer to store maximum value
 */
void MEMS_Example_CalculateStats(const float *data, uint32_t length,
                                float *average, float *min_val, float *max_val);

/**
 * @brief Wait for user input (for interactive examples)
 * @param prompt Prompt message to display
 */
void MEMS_Example_WaitForUser(const char *prompt);

/**
 * @brief Get system tick count
 * @retval uint32_t Current tick count in milliseconds
 */
uint32_t MEMS_Example_GetTick(void);

#ifdef __cplusplus
}
#endif

#endif /* __MEMS_EXAMPLE_H__ */
